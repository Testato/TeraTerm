/*
Copyright (c) 1998-2001, Robert O'Callahan
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of
conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this list
of conditions and the following disclaimer in the documentation and/or other materials
provided with the distribution.

The name of Robert O'Callahan may not be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
This code is copyright (C) 1998-1999 Robert O'Callahan.
See LICENSE.TXT for the license.
*/

#include "ttxssh.h"
#include "keyfiles.h"

#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <openssl/rsa.h>
#include <openssl/dsa.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>

static char ID_string[] = "SSH PRIVATE KEY FILE FORMAT 1.1\n";

static BIGNUM FAR *get_bignum(unsigned char FAR * bytes)
{
	int bits = get_ushort16_MSBfirst(bytes);

	return BN_bin2bn(bytes + 2, (bits + 7) / 8, NULL);
}

/* normalize the RSA key by precomputing various bits of it.
   This code is adapted from libeay's rsa_gen.c
   It's needed to work around "issues" with LIBEAY/RSAREF.
   If this function returns 0, then something went wrong and the
   key must be discarded. */
static BOOL normalize_key(RSA FAR * key)
{
	BOOL OK = FALSE;
	BIGNUM *r = BN_new();
	BN_CTX *ctx = BN_CTX_new();

	if (BN_cmp(key->p, key->q) < 0) {
		BIGNUM *tmp = key->p;

		key->p = key->q;
		key->q = tmp;
	}

	if (r != NULL && ctx != NULL) {
		key->dmp1 = BN_new();
		key->dmq1 = BN_new();
		key->iqmp = BN_mod_inverse(NULL, key->q, key->p, ctx);

		if (key->dmp1 != NULL && key->dmq1 != NULL && key->iqmp != NULL) {
			OK = BN_sub(r, key->p, BN_value_one())
				&& BN_mod(key->dmp1, key->d, r, ctx)
				&& BN_sub(r, key->q, BN_value_one())
				&& BN_mod(key->dmq1, key->d, r, ctx);
		}
	}

	BN_free(r);
	BN_CTX_free(ctx);

	return OK;
}

static RSA FAR *read_RSA_private_key(PTInstVar pvar,
									 char FAR * relative_name,
									 char FAR * passphrase,
									 BOOL FAR * invalid_passphrase,
									 BOOL is_auto_login)
{
	char filename[2048];
	int fd;
	unsigned int length, amount_read;
	unsigned char *keyfile_data;
	unsigned int index;
	int cipher;
	RSA FAR *key;
	unsigned int E_index, N_index, D_index, U_index, P_index, Q_index = 0;

	*invalid_passphrase = FALSE;

	get_teraterm_dir_relative_name(filename, sizeof(filename),
								   relative_name);

	fd = _open(filename, _O_RDONLY | _O_SEQUENTIAL | _O_BINARY);
	if (fd == -1) {
		if (errno == ENOENT) {
#ifndef NO_I18N
			strcpy(pvar->ts->UIMsg, "An error occurred while trying to read the key file.\n"
									"The specified filename does not exist.");
			UTIL_get_lang_msg("MSG_KEYFILES_READ_ENOENT_ERROR", pvar);
			notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
			notify_nonfatal_error(pvar,
								  "An error occurred while trying to read the key file.\n"
								  "The specified filename does not exist.");
#endif
		} else {
#ifndef NO_I18N
			strcpy(pvar->ts->UIMsg, "An error occurred while trying to read the key file.");
			UTIL_get_lang_msg("MSG_KEYFILES_READ_ERROR", pvar);
			notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
			notify_nonfatal_error(pvar,
								  "An error occurred while trying to read the key file.");
#endif
		}
		return NULL;
	}

	length = (int) _lseek(fd, 0, SEEK_END);
	_lseek(fd, 0, SEEK_SET);

	if (length >= 0 && length < 0x7FFFFFFF) {
		keyfile_data = malloc(length + 1);
		if (keyfile_data == NULL) {
#ifndef NO_I18N
			strcpy(pvar->ts->UIMsg, "Memory ran out while trying to allocate space to read the key file.");
			UTIL_get_lang_msg("MSG_KEYFILES_READ_ALLOC_ERROR", pvar);
			notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
			notify_nonfatal_error(pvar,
								  "Memory ran out while trying to allocate space to read the key file.");
#endif
			_close(fd);
			return NULL;
		}
	} else {
#ifndef NO_I18N
		strcpy(pvar->ts->UIMsg, "An error occurred while trying to read the key file.");
		UTIL_get_lang_msg("MSG_KEYFILES_READ_ERROR", pvar);
		notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
		notify_nonfatal_error(pvar,
							  "An error occurred while trying to read the key file.");
#endif
		_close(fd);
		return NULL;
	}

	amount_read = _read(fd, keyfile_data, length);
	/* terminate it with 0 so that the strncmp below is guaranteed not to
	   crash */
	keyfile_data[length] = 0;

	_close(fd);

	if (amount_read != length) {
#ifndef NO_I18N
		strcpy(pvar->ts->UIMsg, "An error occurred while trying to read the key file.");
		UTIL_get_lang_msg("MSG_KEYFILES_READ_ERROR", pvar);
		notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
		notify_nonfatal_error(pvar,
							  "An error occurred while trying to read the key file.");
#endif
		free(keyfile_data);
		return NULL;
	}

	if (strcmp(keyfile_data, ID_string) != 0) {
#ifndef NO_I18N
		strcpy(pvar->ts->UIMsg, "The specified key file does not contain an SSH private key.");
		UTIL_get_lang_msg("MSG_KEYFILES_PRIVATEKEY_NOTCONTAIN_ERROR", pvar);
		notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
		notify_nonfatal_error(pvar,
							  "The specified key file does not contain an SSH private key.");
#endif
		free(keyfile_data);
		return NULL;
	}

	index = NUM_ELEM(ID_string);

	if (length < index + 9) {
#ifndef NO_I18N
		strcpy(pvar->ts->UIMsg,
			   "The specified key file has been truncated and does not contain a valid private key.");
		UTIL_get_lang_msg("MSG_KEYFILES_PRIVATEKEY_TRUNCATE_ERROR", pvar);
		notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
		notify_nonfatal_error(pvar,
							  "The specified key file has been truncated and does not contain a valid private key.");
#endif
		free(keyfile_data);
		return NULL;
	}

	cipher = keyfile_data[index];
	/* skip reserved int32, public key bits int32 */
	index += 9;
	/* skip public key e and n mp_ints */
	if (length < index + 2) {
#ifndef NO_I18N
		strcpy(pvar->ts->UIMsg,
			   "The specified key file has been truncated and does not contain a valid private key.");
		UTIL_get_lang_msg("MSG_KEYFILES_PRIVATEKEY_TRUNCATE_ERROR", pvar);
		notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
		notify_nonfatal_error(pvar,
							  "The specified key file has been truncated and does not contain a valid private key.");
#endif
		free(keyfile_data);
		return NULL;
	}
	N_index = index;
	index += (get_ushort16_MSBfirst(keyfile_data + index) + 7) / 8 + 2;
	if (length < index + 2) {
#ifndef NO_I18N
		strcpy(pvar->ts->UIMsg,
			   "The specified key file has been truncated and does not contain a valid private key.");
		UTIL_get_lang_msg("MSG_KEYFILES_PRIVATEKEY_TRUNCATE_ERROR", pvar);
		notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
		notify_nonfatal_error(pvar,
							  "The specified key file has been truncated and does not contain a valid private key.");
#endif
		free(keyfile_data);
		return NULL;
	}
	E_index = index;
	index += (get_ushort16_MSBfirst(keyfile_data + index) + 7) / 8 + 2;
	/* skip comment */
	if (length < index + 4) {
#ifndef NO_I18N
		strcpy(pvar->ts->UIMsg,
			   "The specified key file has been truncated and does not contain a valid private key.");
		UTIL_get_lang_msg("MSG_KEYFILES_PRIVATEKEY_TRUNCATE_ERROR", pvar);
		notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
		notify_nonfatal_error(pvar,
							  "The specified key file has been truncated and does not contain a valid private key.");
#endif
		free(keyfile_data);
		return NULL;
	}
	index += get_uint32_MSBfirst(keyfile_data + index) + 4;

	if (length < index + 6) {
#ifndef NO_I18N
		strcpy(pvar->ts->UIMsg,
			   "The specified key file has been truncated and does not contain a valid private key.");
		UTIL_get_lang_msg("MSG_KEYFILES_PRIVATEKEY_TRUNCATE_ERROR", pvar);
		notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
		notify_nonfatal_error(pvar,
							  "The specified key file has been truncated and does not contain a valid private key.");
#endif
		free(keyfile_data);
		return NULL;
	}
	if (cipher != SSH_CIPHER_NONE) {
		if ((length - index) % 8 != 0) {
#ifndef NO_I18N
			strcpy(pvar->ts->UIMsg,
				   "The specified key file cannot be decrypted using the passphrase.\n"
				   "The file does not have the correct length.");
			UTIL_get_lang_msg("MSG_KEYFILES_PRIVATEKEY_LENGTH_ERROR", pvar);
			notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
			notify_nonfatal_error(pvar,
								  "The specified key file cannot be decrypted using the passphrase.\n"
								  "The file does not have the correct length.");
#endif
			free(keyfile_data);
			return NULL;
		}
		if (!CRYPT_passphrase_decrypt
			(cipher, passphrase, keyfile_data + index, length - index)) {
#ifndef NO_I18N
			strcpy(pvar->ts->UIMsg,
				   "The specified key file cannot be decrypted using the passphrase.\n"
				   "The cipher type used to encrypt the file is not supported by TTSSH for this purpose.");
			UTIL_get_lang_msg("MSG_KEYFILES_PRIVATEKEY_NOCIPHER_ERROR", pvar);
			notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
			notify_nonfatal_error(pvar,
								  "The specified key file cannot be decrypted using the passphrase.\n"
								  "The cipher type used to encrypt the file is not supported by TTSSH for this purpose.");
#endif
			free(keyfile_data);
			return NULL;
		}
	}

	if (keyfile_data[index] != keyfile_data[index + 2]
		|| keyfile_data[index + 1] != keyfile_data[index + 3]) {
		*invalid_passphrase = TRUE;
#ifndef NO_I18N
		if (is_auto_login) {
			strcpy(pvar->ts->UIMsg, "The specified key file cannot be decrypted using the empty passphrase.\n"
									"For auto-login, you must create a key file with no passphrase.\n"
									"BEWARE: This means the key can easily be stolen.");
			UTIL_get_lang_msg("MSG_KEYFILES_PASSPHRASE_EMPTY_ERROR", pvar);
			notify_nonfatal_error(pvar, pvar->ts->UIMsg);
		}
		else {
			strcpy(pvar->ts->UIMsg, "The specified key file cannot be decrypted using the passphrase.\n"
									"The passphrase is incorrect.");
			UTIL_get_lang_msg("MSG_KEYFILES_PASSPHRASE_ERROR", pvar);
			notify_nonfatal_error(pvar, pvar->ts->UIMsg);
		}
#else
		notify_nonfatal_error(pvar, is_auto_login
							  ?
							  "The specified key file cannot be decrypted using the empty passphrase.\n"
							  "For auto-login, you must create a key file with no passphrase.\n"
							  "BEWARE: This means the key can easily be stolen."
							  :
							  "The specified key file cannot be decrypted using the passphrase.\n"
							  "The passphrase is incorrect.");
#endif
		memset(keyfile_data, 0, length);
		free(keyfile_data);
		return NULL;
	}
	index += 4;

	D_index = index;
	if (length >= D_index + 2) {
		U_index =
			D_index + (get_ushort16_MSBfirst(keyfile_data + D_index) +
					   7) / 8 + 2;
		if (length >= U_index + 2) {
			P_index =
				U_index + (get_ushort16_MSBfirst(keyfile_data + U_index) +
						   7) / 8 + 2;
			if (length >= P_index + 2) {
				Q_index =
					P_index +
					(get_ushort16_MSBfirst(keyfile_data + P_index) +
					 7) / 8 + 2;
			}
		}
	}
	if (Q_index == 0
		|| length <
		Q_index + (get_ushort16_MSBfirst(keyfile_data + Q_index) + 7) / 8 +
		2) {
#ifndef NO_I18N
		strcpy(pvar->ts->UIMsg,
			   "The specified key file has been truncated and does not contain a valid private key.");
		UTIL_get_lang_msg("MSG_KEYFILES_PRIVATEKEY_TRUNCATE_ERROR", pvar);
		notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
		notify_nonfatal_error(pvar,
							  "The specified key file has been truncated and does not contain a valid private key.");
#endif
		memset(keyfile_data, 0, length);
		free(keyfile_data);
		return NULL;
	}

	key = RSA_new();
	key->n = get_bignum(keyfile_data + N_index);
	key->e = get_bignum(keyfile_data + E_index);
	key->d = get_bignum(keyfile_data + D_index);
	key->p = get_bignum(keyfile_data + P_index);
	key->q = get_bignum(keyfile_data + Q_index);

	if (!normalize_key(key)) {
#ifndef NO_I18N
		strcpy(pvar->ts->UIMsg, "Error in crytography library.\n"
								"Perhaps the stored key is invalid.");
		UTIL_get_lang_msg("MSG_KEYFILES_CRYPTOLIB_ERROR", pvar);
		notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
		notify_nonfatal_error(pvar, "Error in crytography library.\n"
							  "Perhaps the stored key is invalid.");
#endif

		RSA_free(key);
		key = NULL;
	}

	memset(keyfile_data, 0, length);
	free(keyfile_data);
	return key;
}

CRYPTKeyPair FAR *KEYFILES_read_private_key(PTInstVar pvar,
											char FAR * relative_name,
											char FAR * passphrase,
											BOOL FAR * invalid_passphrase,
											BOOL is_auto_login)
{
	RSA FAR *RSA_key = read_RSA_private_key(pvar, relative_name,
											passphrase, invalid_passphrase,
											is_auto_login);

	if (RSA_key == NULL) {
		return FALSE;
	} else {
		CRYPTKeyPair FAR *result =
			(CRYPTKeyPair FAR *) malloc(sizeof(CRYPTKeyPair));

		// �t���[����Ƃ��� 0 ���ǂ����Ŕ��ʂ��邽�ߒǉ��B(2004.12.20 yutaka)
		ZeroMemory(result, sizeof(CRYPTKeyPair)); 

		result->RSA_key = RSA_key;
		return result;
	}
}


//
// SSH2
//

CRYPTKeyPair *read_SSH2_private_key(PTInstVar pvar,
							char FAR * relative_name,
							char FAR * passphrase,
							BOOL FAR * invalid_passphrase,
							BOOL is_auto_login,
							char *errmsg,
							int errmsg_len)
{
	char filename[2048];
	FILE *fp = NULL;
	CRYPTKeyPair *result = NULL;
	EVP_PKEY *pk = NULL;
	unsigned long err = 0;

	OpenSSL_add_all_algorithms();
	ERR_load_crypto_strings();
	//seed_prng();

	// ���΃p�X���΃p�X�֕ϊ�����B�������邱�Ƃɂ��A�u�h�b�g�Ŏn�܂�v�f�B���N�g����
	// ����t�@�C����ǂݍ��ނ��Ƃ��ł���B(2005.2.7 yutaka)
	get_teraterm_dir_relative_name(filename, sizeof(filename),
								   relative_name);

	fp = fopen(filename, "r");
	if (fp == NULL) {
		_snprintf(errmsg, errmsg_len, strerror(errno));
		goto error;
	}

	result = (CRYPTKeyPair *)malloc(sizeof(CRYPTKeyPair));
	ZeroMemory(result, sizeof(CRYPTKeyPair)); 

	// �t�@�C������p�X�t���[�Y�����ɔ閧����ǂݍ��ށB
	pk = PEM_read_PrivateKey(fp, NULL, NULL, passphrase);
	if (pk == NULL) {
		err = ERR_get_error();
		ERR_error_string_n(err, errmsg, errmsg_len);
		*invalid_passphrase = TRUE;
		goto error;
	}

	if (pk->type == EVP_PKEY_RSA) { // RSA key
		result->RSA_key = EVP_PKEY_get1_RSA(pk);
		result->DSA_key = NULL;

		// RSA�ڂ���܂���L���ɂ���i�^�C�~���O�U������̖h��j
		if (RSA_blinding_on(result->RSA_key, NULL) != 1) {
			err = ERR_get_error();
			ERR_error_string_n(err, errmsg, errmsg_len);
			goto error;
		}

	} else if (pk->type == EVP_PKEY_DSA) { // DSA key
		result->RSA_key = NULL;
		result->DSA_key = EVP_PKEY_get1_DSA(pk);

	} else {
		goto error;
	}

	if (pk != NULL)
		EVP_PKEY_free(pk);

	fclose(fp);
	return (result);

error:
	if (pk != NULL)
		EVP_PKEY_free(pk);

	if (result != NULL)
		CRYPT_free_key_pair(result);

	if (fp != NULL)
		fclose(fp);

	return (NULL);
}



/*
 * $Log: not supported by cvs2svn $
 * Revision 1.6  2007/02/17 16:20:21  yasuhide
 * SSH2 ����p�����F�؂Ńp�X�t���[�Y���ԈႦ���ہA�p�X�t���[�Y�_�C�A���O�Ƀt�H�[�J�X���ڂ�
 *
 * Revision 1.5  2006/11/29 16:58:52  maya
 * �\�����b�Z�[�W�̓ǂݍ��ݑΉ�
 *
 * Revision 1.4  2005/02/07 14:33:47  yutakakn
 * �h�b�g�Ŏn�܂�f�B���N�g���ɂ���SSH2�閧���t�@�C�����ǂݍ��߂Ȃ����֑Ώ������B
 *
 * Revision 1.3  2004/12/27 14:35:41  yutakakn
 * SSH2�閧���ǂݍ��ݎ��s���̃G���[���b�Z�[�W�����������B
 *
 * Revision 1.2  2004/12/22 17:28:14  yutakakn
 * SSH2���J���F��(RSA/DSA)���T�|�[�g�����B
 *
 */