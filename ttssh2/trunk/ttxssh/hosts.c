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
#include "util.h"
#include "resource.h"
#include "matcher.h"
#include "ssh.h"
#include "hosts.h"

#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/dsa.h>

#include <fcntl.h>
#include <io.h>
#include <errno.h>
#include <sys/stat.h>


// BASE64�\��������i�����ł�'='�͊܂܂�Ă��Ȃ��j
static char base64[] ="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


// �z�X�g�L�[�̏����� (2006.3.21 yutaka)
static void init_hostkey(Key *key)
{
	key->type = KEY_UNSPEC;

	// SSH1
	key->bits = 0;
	if (key->exp != NULL) {
		free(key->exp);
		key->exp = NULL;
	}
	if (key->mod != NULL) {
		free(key->mod);
		key->mod = NULL;
	}

	// SSH2
	if (key->dsa != NULL) {
		DSA_free(key->dsa);
		key->dsa = NULL;
	}
	if (key->rsa != NULL) {
		RSA_free(key->rsa);
		key->rsa = NULL;
	}
}


static char FAR *FAR * parse_multi_path(char FAR * buf)
{
	int i;
	int ch;
	int num_paths = 1;
	char FAR *FAR * result;
	int last_path_index;

	for (i = 0; (ch = buf[i]) != 0; i++) {
		if (ch == ';') {
			num_paths++;
		}
	}

	result =
		(char FAR * FAR *) malloc(sizeof(char FAR *) * (num_paths + 1));

	last_path_index = 0;
	num_paths = 0;
	for (i = 0; (ch = buf[i]) != 0; i++) {
		if (ch == ';') {
			buf[i] = 0;
			result[num_paths] = _strdup(buf + last_path_index);
			num_paths++;
			buf[i] = ch;
			last_path_index = i + 1;
		}
	}
	if (i > last_path_index) {
		result[num_paths] = _strdup(buf + last_path_index);
		num_paths++;
	}
	result[num_paths] = NULL;
	return result;
}

void HOSTS_init(PTInstVar pvar)
{
	pvar->hosts_state.prefetched_hostname = NULL;
#if 0
	pvar->hosts_state.key_exp = NULL;
	pvar->hosts_state.key_mod = NULL;
#else
	init_hostkey(&pvar->hosts_state.hostkey);
#endif
	pvar->hosts_state.hosts_dialog = NULL;
	pvar->hosts_state.file_names = NULL;
}

void HOSTS_open(PTInstVar pvar)
{
	pvar->hosts_state.file_names =
		parse_multi_path(pvar->session_settings.KnownHostsFiles);
}

//
// known_hosts�t�@�C���̓��e�����ׂ� pvar->hosts_state.file_data �֓ǂݍ���
//
static int begin_read_file(PTInstVar pvar, char FAR * name,
						   int suppress_errors)
{
	int fd;
	int length;
	int amount_read;
	char buf[2048];

	get_teraterm_dir_relative_name(buf, sizeof(buf), name);
	fd = _open(buf, _O_RDONLY | _O_SEQUENTIAL | _O_BINARY);
	if (fd == -1) {
		if (!suppress_errors) {
			if (errno == ENOENT) {
#ifdef I18N
				strcpy(pvar->ts->UIMsg, "An error occurred while trying to read a known_hosts file.\n"
										"The specified filename does not exist.");
				UTIL_get_lang_msg("MSG_HOSTS_READ_ENOENT_ERROR", pvar);
				notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
				notify_nonfatal_error(pvar,
									  "An error occurred while trying to read a known_hosts file.\n"
									  "The specified filename does not exist.");
#endif
			} else {
#ifdef I18N
				strcpy(pvar->ts->UIMsg, "An error occurred while trying to read a known_hosts file.");
				UTIL_get_lang_msg("MSG_HOSTS_READ_ERROR", pvar);
				notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
				notify_nonfatal_error(pvar,
									  "An error occurred while trying to read a known_hosts file.");
#endif
			}
		}
		return 0;
	}

	length = (int) _lseek(fd, 0, SEEK_END);
	_lseek(fd, 0, SEEK_SET);

	if (length >= 0 && length < 0x7FFFFFFF) {
		pvar->hosts_state.file_data = malloc(length + 1);
		if (pvar->hosts_state.file_data == NULL) {
			if (!suppress_errors) {
#ifdef I18N
				strcpy(pvar->ts->UIMsg, "Memory ran out while trying to allocate space to read a known_hosts file.");
				UTIL_get_lang_msg("MSG_HOSTS_ALLOC_ERROR", pvar);
				notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
				notify_nonfatal_error(pvar,
									  "Memory ran out while trying to allocate space to read a known_hosts file.");
#endif
			}
			_close(fd);
			return 0;
		}
	} else {
		if (!suppress_errors) {
#ifdef I18N
			strcpy(pvar->ts->UIMsg, "An error occurred while trying to read a known_hosts file.");
			UTIL_get_lang_msg("MSG_HOSTS_READ_ERROR", pvar);
			notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
			notify_nonfatal_error(pvar,
								  "An error occurred while trying to read a known_hosts file.");
#endif
		}
		_close(fd);
		return 0;
	}

	amount_read = _read(fd, pvar->hosts_state.file_data, length);
	pvar->hosts_state.file_data[length] = 0;

	_close(fd);

	if (amount_read != length) {
		if (!suppress_errors) {
#ifdef I18N
			strcpy(pvar->ts->UIMsg, "An error occurred while trying to read a known_hosts file.");
			UTIL_get_lang_msg("MSG_HOSTS_READ_ERROR", pvar);
			notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
			notify_nonfatal_error(pvar,
								  "An error occurred while trying to read a known_hosts file.");
#endif
		}
		free(pvar->hosts_state.file_data);
		pvar->hosts_state.file_data = NULL;
		return 0;
	} else {
		return 1;
	}
}

static int end_read_file(PTInstVar pvar, int suppress_errors)
{
	free(pvar->hosts_state.file_data);
	pvar->hosts_state.file_data = NULL;
	return 1;
}

static int begin_read_host_files(PTInstVar pvar, int suppress_errors)
{
	pvar->hosts_state.file_num = 0;
	pvar->hosts_state.file_data = NULL;
	return 1;
}

// MIME64�̕�������X�L�b�v����
static int eat_base64(char FAR * data)
{
	int index = 0;
	int ch;

	for (;;) {
		ch = data[index];
		if (ch == '=' || strchr(base64, ch)) {
			// BASE64�̍\������������������ index ��i�߂�
			index++;
		} else {
			break;
		}
	}

	return index;
}

static int eat_spaces(char FAR * data)
{
	int index = 0;
	int ch;

	while ((ch = data[index]) == ' ' || ch == '\t') {
		index++;
	}
	return index;
}

static int eat_digits(char FAR * data)
{
	int index = 0;
	int ch;

	while ((ch = data[index]) >= '0' && ch <= '9') {
		index++;
	}
	return index;
}

static int eat_to_end_of_line(char FAR * data)
{
	int index = 0;
	int ch;

	while ((ch = data[index]) != '\n' && ch != '\r' && ch != 0) {
		index++;
	}

	while ((ch = data[index]) == '\n' || ch == '\r') {
		index++;
	}

	return index;
}

static int eat_to_end_of_pattern(char FAR * data)
{
	int index = 0;
	int ch;

	while (ch = data[index], is_pattern_char(ch)) {
		index++;
	}

	return index;
}

// 
// BASE64�f�R�[�h�������s���B(rfc1521)
// src�o�b�t�@�� null-terminate ���Ă���K�v����B
//
static int uudecode(unsigned char *src, int srclen, unsigned char *target, int targsize)
{
	char pad = '=';
	int tarindex, state, ch;
	char *pos;

	state = 0;
	tarindex = 0;

	while ((ch = *src++) != '\0') {
		if (isspace(ch))	/* Skip whitespace anywhere. */
			continue;

		if (ch == pad)
			break;

		pos = strchr(base64, ch);
		if (pos == 0) 		/* A non-base64 character. */
			return (-1);

		switch (state) {
		case 0:
			if (target) {
				if (tarindex >= targsize)
					return (-1);
				target[tarindex] = (pos - base64) << 2;
			}
			state = 1;
			break;
		case 1:
			if (target) {
				if (tarindex + 1 >= targsize)
					return (-1);
				target[tarindex]   |=  (pos - base64) >> 4;
				target[tarindex+1]  = ((pos - base64) & 0x0f) << 4 ;
			}
			tarindex++;
			state = 2;
			break;
		case 2:
			if (target) {
				if (tarindex + 1 >= targsize)
					return (-1);
				target[tarindex]   |=  (pos - base64) >> 2;
				target[tarindex+1]  = ((pos - base64) & 0x03) << 6;
			}
			tarindex++;
			state = 3;
			break;
		case 3:
			if (target) {
				if (tarindex >= targsize)
					return (-1);
				target[tarindex] |= (pos - base64);
			}
			tarindex++;
			state = 0;
			break;
		}
	}

	/*
	 * We are done decoding Base-64 chars.  Let's see if we ended
	 * on a byte boundary, and/or with erroneous trailing characters.
	 */

	if (ch == pad) {		/* We got a pad char. */
		ch = *src++;		/* Skip it, get next. */
		switch (state) {
		case 0:		/* Invalid = in first position */
		case 1:		/* Invalid = in second position */
			return (-1);

		case 2:		/* Valid, means one byte of info */
			/* Skip any number of spaces. */
			for (; ch != '\0'; ch = *src++)
				if (!isspace(ch))
					break;
			/* Make sure there is another trailing = sign. */
			if (ch != pad)
				return (-1);
			ch = *src++;		/* Skip the = */
			/* Fall through to "single trailing =" case. */
			/* FALLTHROUGH */

		case 3:		/* Valid, means two bytes of info */
			/*
			 * We know this char is an =.  Is there anything but
			 * whitespace after it?
			 */
			for (; ch != '\0'; ch = *src++)
				if (!isspace(ch))
					return (-1);

			/*
			 * Now make sure for cases 2 and 3 that the "extra"
			 * bits that slopped past the last full byte were
			 * zeros.  If we don't check them, they become a
			 * subliminal channel.
			 */
			if (target && target[tarindex] != 0)
				return (-1);
		}
	} else {
		/*
		 * We ended by seeing the end of the string.  Make sure we
		 * have no partial bytes lying around.
		 */
		if (state != 0)
			return (-1);
	}

	return (tarindex);
}


// SSH2���� BASE64 �`���Ŋi�[����Ă���
static Key *parse_uudecode(char *data)
{
	int count;
	unsigned char *blob = NULL;
	int len, n;
	Key *key = NULL;
	char ch;

	// BASE64������̃T�C�Y�𓾂�
	count = eat_base64(data);
	len = 2 * count;
	blob = malloc(len);
	if (blob == NULL)
		goto error;

	// BASE64�f�R�[�h
	ch = data[count];
	data[count] = '\0';  // �����͉��s�R�[�h�̂͂��Ȃ̂ŏ����ׂ��Ă����Ȃ��͂�
	n = uudecode(data, count, blob, len);
	data[count] = ch;
	if (n < 0) {
		goto error;
	}

	key = key_from_blob(blob, n);
	if (key == NULL)
		goto error;

error:
	if (blob != NULL)
		free(blob);

	return (key);
}


static char FAR *parse_bignum(char FAR * data)
{
	uint32 digits = 0;
	BIGNUM *num = BN_new();
	BIGNUM *billion = BN_new();
	BIGNUM *digits_num = BN_new();
	BN_CTX *ctx = BN_CTX_new();
	char FAR *result;
	int ch;
	int leftover_digits = 1;

	BN_CTX_init(ctx);
	BN_set_word(num, 0);
	BN_set_word(billion, 1000000000L);

	while ((ch = *data) >= '0' && ch <= '9') {
		if (leftover_digits == 1000000000L) {
			BN_set_word(digits_num, digits);
			BN_mul(num, num, billion, ctx);
			BN_add(num, num, digits_num);
			leftover_digits = 1;
			digits = 0;
		}

		digits = digits * 10 + ch - '0';
		leftover_digits *= 10;
		data++;
	}

	BN_set_word(digits_num, digits);
	BN_set_word(billion, leftover_digits);
	BN_mul(num, num, billion, ctx);
	BN_add(num, num, digits_num);

	result = (char FAR *) malloc(2 + BN_num_bytes(num));
	set_ushort16_MSBfirst(result, BN_num_bits(num));
	BN_bn2bin(num, result + 2);

	BN_CTX_free(ctx);
	BN_free(digits_num);
	BN_free(num);
	BN_free(billion);

	return result;
}

//
// known_hosts�t�@�C���̓��e����͂��A�w�肵���z�X�g�̌��J����T���B
//
static int check_host_key(PTInstVar pvar, char FAR * hostname,
						  char FAR * data)
{
	int index = eat_spaces(data);
	int matched = 0;
	int keybits = 0;

	if (data[index] == '#') {
		return index + eat_to_end_of_line(data + index);
	}

	/* if we find an empty line, then it won't have any patterns matching the hostname
	   and so we skip it */
	index--;
	do {
		int negated;

		index++;
		negated = data[index] == '!';

		if (negated) {
			index++;
			if (match_pattern(data + index, hostname)) {
				return index + eat_to_end_of_line(data + index);
			}
		} else if (match_pattern(data + index, hostname)) {
			matched = 1;
		}

		index += eat_to_end_of_pattern(data + index);
	} while (data[index] == ',');

	if (!matched) {
		return index + eat_to_end_of_line(data + index);
	} else {
		// ���̎�ނɂ��t�H�[�}�b�g���قȂ�
		// �܂��A�ŏ��Ɉ�v�����G���g�����擾���邱�ƂɂȂ�B
		/*
		[SSH1]
		192.168.1.2 1024 35 13032....

		[SSH2]
		192.168.1.2 ssh-rsa AAAAB3NzaC1....
		192.168.1.2 ssh-dss AAAAB3NzaC1....
		192.168.1.2 rsa AAAAB3NzaC1....
		192.168.1.2 dsa AAAAB3NzaC1....
		192.168.1.2 rsa1 AAAAB3NzaC1....
		 */
		int rsa1_key_bits;

		index += eat_spaces(data + index);

		rsa1_key_bits = atoi(data + index);
		if (rsa1_key_bits > 0) { // RSA1�ł���
			if (!SSHv1(pvar)) { // SSH2�ڑ��ł���Ζ�������
				return index + eat_to_end_of_line(data + index);
			}

			pvar->hosts_state.hostkey.type = KEY_RSA1;

			pvar->hosts_state.hostkey.bits = rsa1_key_bits;
			index += eat_digits(data + index);
			index += eat_spaces(data + index);

			pvar->hosts_state.hostkey.exp = parse_bignum(data + index);
			index += eat_digits(data + index);
			index += eat_spaces(data + index);

			pvar->hosts_state.hostkey.mod = parse_bignum(data + index);

			/*
			if (pvar->hosts_state.key_bits < 0
				|| pvar->hosts_state.key_exp == NULL
				|| pvar->hosts_state.key_mod == NULL) {
				pvar->hosts_state.key_bits = 0;
				free(pvar->hosts_state.key_exp);
				free(pvar->hosts_state.key_mod);
			}*/

		} else {
			char *cp, *p;
			Key *key;

			if (!SSHv2(pvar)) { // SSH1�ڑ��ł���Ζ�������
				return index + eat_to_end_of_line(data + index);
			}

			cp = data + index;
			p = strchr(cp, ' ');
			if (p == NULL) {
				return index + eat_to_end_of_line(data + index);
			}
			index += (p - cp);  // setup index
			*p = '\0';
			pvar->hosts_state.hostkey.type = get_keytype_from_name(cp);
			*p = ' ';

			index += eat_spaces(data + index);  // update index

			// uudecode
			key = parse_uudecode(data + index);
			if (key == NULL) {
				return index + eat_to_end_of_line(data + index);
			}

			// setup
			pvar->hosts_state.hostkey.type = key->type;
			pvar->hosts_state.hostkey.dsa = key->dsa;
			pvar->hosts_state.hostkey.rsa = key->rsa;

			index += eat_base64(data + index);
			index += eat_spaces(data + index);
		}

		return index + eat_to_end_of_line(data + index);
	}
}

//
// known_hosts�t�@�C������z�X�g���ɍ��v����s��ǂ�
//
static int read_host_key(PTInstVar pvar, char FAR * hostname,
						 int suppress_errors, int return_always)
{
	int i;
	int while_flg;

	for (i = 0; hostname[i] != 0; i++) {
		int ch = hostname[i];

		if (!is_pattern_char(ch) || ch == '*' || ch == '?') {
			if (!suppress_errors) {
#ifdef I18N
				strcpy(pvar->ts->UIMsg, "The host name contains an invalid character.\n"
										"This session will be terminated.");
				UTIL_get_lang_msg("MSG_HOSTS_HOSTNAME_INVALID_ERROR", pvar);
				notify_fatal_error(pvar, pvar->ts->UIMsg);
#else
				notify_fatal_error(pvar,
								   "The host name contains an invalid character.\n"
								   "This session will be terminated.");
#endif
			}
			return 0;
		}
	}

	if (i == 0) {
		if (!suppress_errors) {
#ifdef I18N
			strcpy(pvar->ts->UIMsg, "The host name should not be empty.\n"
									"This session will be terminated.");
			UTIL_get_lang_msg("MSG_HOSTS_HOSTNAME_EMPTY_ERROR", pvar);
			notify_fatal_error(pvar, pvar->ts->UIMsg);
#else
			notify_fatal_error(pvar, "The host name should not be empty.\n"
							   "This session will be terminated.");
#endif
		}
		return 0;
	}

#if 0
	pvar->hosts_state.key_bits = 0;
	free(pvar->hosts_state.key_exp);
	pvar->hosts_state.key_exp = NULL;
	free(pvar->hosts_state.key_mod);
	pvar->hosts_state.key_mod = NULL;
#else
	// hostkey type is KEY_UNSPEC.
	init_hostkey(&pvar->hosts_state.hostkey);
#endif

	do {
		if (pvar->hosts_state.file_data == NULL
			|| pvar->hosts_state.file_data[pvar->hosts_state.
										   file_data_index] == 0) {
			char FAR *filename;
			int keep_going = 1;

			if (pvar->hosts_state.file_data != NULL) {
				end_read_file(pvar, suppress_errors);
			}

			do {
				filename =
					pvar->hosts_state.file_names[pvar->hosts_state.
												 file_num];

				if (filename == NULL) {
					return 1;
				} else {
					pvar->hosts_state.file_num++;

					if (filename[0] != 0) {
						if (begin_read_file
							(pvar, filename, suppress_errors)) {
							pvar->hosts_state.file_data_index = 0;
							keep_going = 0;
						}
					}
				}
			} while (keep_going);
		}

		pvar->hosts_state.file_data_index +=
			check_host_key(pvar, hostname,
						   pvar->hosts_state.file_data +
						   pvar->hosts_state.file_data_index);

		if (!return_always) {
			// �L���ȃL�[��������܂�
			while_flg = (pvar->hosts_state.hostkey.type == KEY_UNSPEC);
		}
		else {
			while_flg = 0;
		}
	} while (while_flg);

	return 1;
}

static void finish_read_host_files(PTInstVar pvar, int suppress_errors)
{
	if (pvar->hosts_state.file_data != NULL) {
		end_read_file(pvar, suppress_errors);
	}
}

// �T�[�o�֐ڑ�����O�ɁAknown_hosts�t�@�C������z�X�g���J�����ǂ݂��Ă����B
void HOSTS_prefetch_host_key(PTInstVar pvar, char FAR * hostname)
{
	if (!begin_read_host_files(pvar, 1)) {
		return;
	}

	if (!read_host_key(pvar, hostname, 1, 0)) {
		return;
	}

	free(pvar->hosts_state.prefetched_hostname);
	pvar->hosts_state.prefetched_hostname = _strdup(hostname);

	finish_read_host_files(pvar, 1);
}

static BOOL equal_mp_ints(unsigned char FAR * num1,
						  unsigned char FAR * num2)
{
	if (num1 == NULL || num2 == NULL) {
		return FALSE;
	} else {
		uint32 bytes = (get_ushort16_MSBfirst(num1) + 7) / 8;

		if (bytes != (get_ushort16_MSBfirst(num2) + 7) / 8) {
			return FALSE;		/* different byte lengths */
		} else {
			return memcmp(num1 + 2, num2 + 2, bytes) == 0;
		}
	}
}

// ���J�����������������؂���
static BOOL match_key(PTInstVar pvar, Key *key)
{
	int bits;
	unsigned char FAR * exp;
	unsigned char FAR * mod;

	if (key->type == KEY_RSA1) {  // SSH1 host public key
		bits = key->bits;
		exp = key->exp;
		mod = key->mod;

		/* just check for equal exponent and modulus */
		return equal_mp_ints(exp, pvar->hosts_state.hostkey.exp)
			&& equal_mp_ints(mod, pvar->hosts_state.hostkey.mod);
		/*
		return equal_mp_ints(exp, pvar->hosts_state.key_exp)
			&& equal_mp_ints(mod, pvar->hosts_state.key_mod);
			*/

	} else if (key->type == KEY_RSA) {  // SSH2 RSA host public key

		return key->rsa != NULL && pvar->hosts_state.hostkey.rsa != NULL &&
			   BN_cmp(key->rsa->e, pvar->hosts_state.hostkey.rsa->e) == 0 && 
			   BN_cmp(key->rsa->n, pvar->hosts_state.hostkey.rsa->n) == 0;

	} else { // // SSH2 DSA host public key

		return key->dsa != NULL && pvar->hosts_state.hostkey.dsa && 
			   BN_cmp(key->dsa->p, pvar->hosts_state.hostkey.dsa->p) == 0 && 
			   BN_cmp(key->dsa->q, pvar->hosts_state.hostkey.dsa->q) == 0 &&
			   BN_cmp(key->dsa->g, pvar->hosts_state.hostkey.dsa->g) == 0 &&
			   BN_cmp(key->dsa->pub_key, pvar->hosts_state.hostkey.dsa->pub_key) == 0;

	}

}

static void init_hosts_dlg(PTInstVar pvar, HWND dlg)
{
	char buf[1024];
	char buf2[2048];
	int i, j;
	int ch;
	char *fp;

	// static text�� # �������z�X�g���ɒu������
	GetDlgItemText(dlg, IDC_HOSTWARNING, buf, sizeof(buf));
	for (i = 0; (ch = buf[i]) != 0 && ch != '#'; i++) {
		buf2[i] = ch;
	}
	if (sizeof(buf2) - i - 1 > 0) {
		strncpy(buf2 + i, pvar->hosts_state.prefetched_hostname,
				sizeof(buf2) - i - 1);
	}
	j = i + strlen(buf2 + i);
	for (; buf[i] == '#'; i++) {
	}
	if (sizeof(buf2) - j - 1 > 0) {
		strncpy(buf2 + j, buf + i, sizeof(buf2) - j - 1);
	}
	buf2[sizeof(buf2) - 1] = 0;

	SetDlgItemText(dlg, IDC_HOSTWARNING, buf2);

	// fingerprint��ݒ肷��
	fp = key_fingerprint(&pvar->hosts_state.hostkey);
	SendMessage(GetDlgItem(dlg, IDC_FINGER_PRINT), WM_SETTEXT, 0, (LPARAM)fp);
}

static int print_mp_int(char FAR * buf, unsigned char FAR * mp)
{
	int i = 0, j, k;
	BIGNUM *num = BN_new();
	int ch;

	BN_bin2bn(mp + 2, (get_ushort16_MSBfirst(mp) + 7) / 8, num);

	do {
		buf[i] = (char) ((BN_div_word(num, 10)) + '0');
		i++;
	} while (!BN_is_zero(num));

	/* we need to reverse the digits */
	for (j = 0, k = i - 1; j < k; j++, k--) {
		ch = buf[j];
		buf[j] = buf[k];
		buf[k] = ch;
	}

	buf[i] = 0;
	return i;
}

//
// known_hosts �t�@�C���֕ۑ�����G���g�����쐬����B
//
static char FAR *format_host_key(PTInstVar pvar)
{
	int host_len = strlen(pvar->hosts_state.prefetched_hostname);
	char *result = NULL;
	int index;
	enum hostkey_type type = pvar->hosts_state.hostkey.type;

	if (type == KEY_RSA1) {
		result = (char FAR *) malloc(host_len
										   + 50 +
										   get_ushort16_MSBfirst(pvar->hosts_state.hostkey.exp) /
										   3 +
										   get_ushort16_MSBfirst(pvar->hosts_state.hostkey.mod) /
										   3);

		strcpy(result, pvar->hosts_state.prefetched_hostname);
		index = host_len;

		sprintf(result + index, " %d ", pvar->hosts_state.hostkey.bits);
		index += strlen(result + index);
		index += print_mp_int(result + index, pvar->hosts_state.hostkey.exp);
		result[index] = ' ';
		index++;
		index += print_mp_int(result + index, pvar->hosts_state.hostkey.mod);
		strcpy(result + index, " \r\n");

	} else if (type == KEY_RSA || type == KEY_DSA) {
		Key *key = &pvar->hosts_state.hostkey;
		char *blob = NULL;
		int blen, uulen, msize;
		char *uu = NULL;
		int n;

		key_to_blob(key, &blob, &blen);
		uulen = 2 * blen;
		uu = malloc(uulen);
		if (uu == NULL) {
			goto error;
		}
		n = uuencode(blob, blen, uu, uulen);
		if (n > 0) {
			msize = host_len + 50 + uulen;
			result = malloc(msize);
			if (result == NULL) {
				goto error;
			}

			// setup
			_snprintf(result, msize, "%s %s %s\r\n", 
				pvar->hosts_state.prefetched_hostname, 
				get_sshname_from_key(key),
				uu);
		}
error:
		if (blob != NULL)
			free(blob);
		if (uu != NULL)
			free(uu);

	} else {
		return NULL;

	}

	return result;
}

static void add_host_key(PTInstVar pvar)
{
	char FAR *name = pvar->hosts_state.file_names[0];

	if (name == NULL || name[0] == 0) {
#ifdef I18N
		strcpy(pvar->ts->UIMsg, "The host and its key cannot be added, because no known-hosts file has been specified.\n"
								"Restart Teraterm and specify a read/write known-hosts file in the TTSSH Setup dialog box.");
		UTIL_get_lang_msg("MSG_HOSTS_FILE_UNSPECIFY_ERROR", pvar);
		notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
		notify_nonfatal_error(pvar,
							  "The host and its key cannot be added, because no known-hosts file has been specified.\n"
							  "Restart Teraterm and specify a read/write known-hosts file in the TTSSH Setup dialog box.");
#endif
	} else {
		char FAR *keydata = format_host_key(pvar);
		int length = strlen(keydata);
		int fd;
		int amount_written;
		int close_result;
		char buf[FILENAME_MAX];

		get_teraterm_dir_relative_name(buf, sizeof(buf), name);
		fd = _open(buf,
			  _O_APPEND | _O_CREAT | _O_WRONLY | _O_SEQUENTIAL |
			  _O_BINARY,
			  _S_IREAD | _S_IWRITE);
		if (fd == -1) {
			if (errno == EACCES) {
#ifdef I18N
				strcpy(pvar->ts->UIMsg, "An error occurred while trying to write the host key.\n"
										"You do not have permission to write to the known-hosts file.");
				UTIL_get_lang_msg("MSG_HOSTS_WRITE_EACCES_ERROR", pvar);
				notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
				notify_nonfatal_error(pvar,
									  "An error occurred while trying to write the host key.\n"
									  "You do not have permission to write to the known-hosts file.");
#endif
			} else {
#ifdef I18N
				strcpy(pvar->ts->UIMsg, "An error occurred while trying to write the host key.\n"
										"The host key could not be written.");
				UTIL_get_lang_msg("MSG_HOSTS_WRITE_ERROR", pvar);
				notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
				notify_nonfatal_error(pvar,
									  "An error occurred while trying to write the host key.\n"
									  "The host key could not be written.");
#endif
			}
			return;
		}

		amount_written = _write(fd, keydata, length);
		free(keydata);
		close_result = _close(fd);

		if (amount_written != length || close_result == -1) {
#ifdef I18N
			strcpy(pvar->ts->UIMsg, "An error occurred while trying to write the host key.\n"
									"The host key could not be written.");
			UTIL_get_lang_msg("MSG_HOSTS_WRITE_ERROR", pvar);
			notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
			notify_nonfatal_error(pvar,
								  "An error occurred while trying to write the host key.\n"
								  "The host key could not be written.");
#endif
		}
	}
}

static char FAR *copy_mp_int(char FAR * num)
{
	int len = (get_ushort16_MSBfirst(num) + 7) / 8 + 2;
	char FAR *result = (char FAR *) malloc(len);

	if (result != NULL) {
		memcpy(result, num, len);
	}

	return result;
}

//
// �����z�X�g�œ��e�̈قȂ�L�[���폜����
// add_host_key �̂��ƂɌĂԂ���
//
static void delete_different_key(PTInstVar pvar)
{
	char FAR *name = pvar->hosts_state.file_names[0];

	if (name == NULL || name[0] == 0) {
#ifdef I18N
		strcpy(pvar->ts->UIMsg, "The host and its key cannot be added, because no known-hosts file has been specified.\n"
								"Restart Teraterm and specify a read/write known-hosts file in the TTSSH Setup dialog box.");
		UTIL_get_lang_msg("MSG_HOSTS_FILE_UNSPECIFY_ERROR", pvar);
		notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
		notify_nonfatal_error(pvar,
							  "The host and its key cannot be added, because no known-hosts file has been specified.\n"
							  "Restart Teraterm and specify a read/write known-hosts file in the TTSSH Setup dialog box.");
#endif
	}
	else {
		Key key; // �ڑ����̃z�X�g�̃L�[
		int length;
		char filename[L_tmpnam];
		int fd;
		int amount_written = 0;
		int close_result;
		int data_index = 0;
		char buf[FILENAME_MAX];

		// �������݈ꎞ�t�@�C�����J��
		tmpnam(filename);
		fd =
			_open(filename,
				  _O_CREAT | _O_WRONLY | _O_SEQUENTIAL | _O_BINARY |
				  _O_TRUNC,
				  _S_IREAD | _S_IWRITE);

		if (fd == -1) {
			if (errno == EACCES) {
#ifdef I18N
				strcpy(pvar->ts->UIMsg, "An error occurred while trying to write the host key.\n"
										"You do not have permission to write to the known-hosts file.");
				UTIL_get_lang_msg("MSG_HOSTS_WRITE_EACCES_ERROR", pvar);
				notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
				notify_nonfatal_error(pvar,
									  "An error occurred while trying to write the host key.\n"
									  "You do not have permission to write to the known-hosts file.");
#endif
			} else {
#ifdef I18N
				strcpy(pvar->ts->UIMsg, "An error occurred while trying to write the host key.\n"
										"The host key could not be written.");
				UTIL_get_lang_msg("MSG_HOSTS_WRITE_ERROR", pvar);
				notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
				notify_nonfatal_error(pvar,
									  "An error occurred while trying to write the host key.\n"
									  "The host key could not be written.");
#endif
			}
			free(filename);
			return;
		}

		// �ڑ����̃T�[�o�̃L�[��ǂݍ���
		if (pvar->hosts_state.hostkey.type == KEY_RSA1) { // SSH1
			key.type = KEY_RSA1;
			key.bits = pvar->hosts_state.hostkey.bits;
			key.exp = copy_mp_int(pvar->hosts_state.hostkey.exp);
			key.mod = copy_mp_int(pvar->hosts_state.hostkey.mod);
		} else if (pvar->hosts_state.hostkey.type == KEY_RSA) { // SSH2 RSA
			key.type = KEY_RSA;
			key.rsa = duplicate_RSA(pvar->hosts_state.hostkey.rsa);
		} else { // SSH2 DSA
			key.type = KEY_DSA;
			key.dsa = duplicate_DSA(pvar->hosts_state.hostkey.dsa);
		}

		// �t�@�C������ǂݍ���
		begin_read_host_files(pvar, 0);
		do {
			int host_index = 0;
			int matched = 0;
			int keybits = 0;
			char FAR *data;
			int do_write = 0;
			length = amount_written = 0;

			if (!read_host_key(pvar, pvar->ssh_state.hostname, 0, 1)) {
				break;
			}

			if (data_index == pvar->hosts_state.file_data_index) {
				// index ���i�܂Ȃ� == �Ō�܂œǂ�
				break;
			}

			data = pvar->hosts_state.file_data + data_index;
			host_index = eat_spaces(data);

			if (data[host_index] == '#') {
				do_write = 1;
			}
			else {
				// �z�X�g�̏ƍ�
				host_index--;
				do {
					int negated;

					host_index++;
					negated = data[host_index] == '!';

					if (negated) {
						host_index++;
						if (match_pattern(data + host_index,
										  pvar->ssh_state.hostname)) {
							matched = 0;
							// �ڑ��o�[�W�����`�F�b�N�̂��߂� host_index ��i�߂Ă��甲����
							host_index--;
							do {
								host_index++;
								host_index += eat_to_end_of_pattern(data + host_index);
							} while (data[host_index] == ',');
							break;
						}
					}
					else if (match_pattern(data + host_index,
									  pvar->ssh_state.hostname)) {
						matched = 1;
					}
					host_index += eat_to_end_of_pattern(data + host_index);
				} while (data[host_index] == ',');

				// �z�X�g���������č��v����L�[��������
				if (match_key(pvar, &key)) {
					do_write = 1;
				}
				// �z�X�g���������Ȃ�
				else if (!matched) {
					do_write = 1;
				}
				// �z�X�g�������� and �ڑ��̃o�[�W�������Ⴄ
				else {
					int rsa1_key_bits=0;
					rsa1_key_bits = atoi(data + host_index + eat_spaces(data + host_index));
					
					if (rsa1_key_bits > 0) { // �t�@�C���̃L�[�� ssh1
						if (!SSHv1(pvar)) {
							do_write = 1;
						}
					}
					else { // �t�@�C���̃L�[�� ssh2
						if (!SSHv2(pvar)) {
							do_write = 1;
						}
					}
				}
			}

			// �������ݏ���
			if (do_write) {
				length = pvar->hosts_state.file_data_index - data_index;
				amount_written =
					_write(fd, pvar->hosts_state.file_data + data_index,
						   length);

				if (amount_written != length) {
					goto error1;
				}
			}
			data_index = pvar->hosts_state.file_data_index;
		} while (1); // �Ō�܂œǂ�

error1:
		close_result = _close(fd);
		if (amount_written != length || close_result == -1) {
#ifdef I18N
			strcpy(pvar->ts->UIMsg, "An error occurred while trying to write the host key.\n"
									"The host key could not be written.");
			UTIL_get_lang_msg("MSG_HOSTS_WRITE_ERROR", pvar);
			notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
			notify_nonfatal_error(pvar,
								  "An error occurred while trying to write the host key.\n"
								  "The host key could not be written.");
#endif
			goto error2;
		}

		// �������݈ꎞ�t�@�C�����烊�l�[��
		get_teraterm_dir_relative_name(buf, sizeof(buf), name);
		_unlink(buf);
		rename(filename, buf);

error2:
		_unlink(filename);

		finish_read_host_files(pvar, 0);
	}
}

//
// Unknown host�̃z�X�g���J���� known_hosts �t�@�C���֕ۑ����邩�ǂ�����
// ���[�U�Ɋm�F������B
// TODO: finger print�̕\�����s���B
// (2006.3.25 yutaka)
//
static BOOL CALLBACK hosts_add_dlg_proc(HWND dlg, UINT msg, WPARAM wParam,
										LPARAM lParam)
{
	PTInstVar pvar;

	switch (msg) {
	case WM_INITDIALOG:
		pvar = (PTInstVar) lParam;
		pvar->hosts_state.hosts_dialog = dlg;
		SetWindowLong(dlg, DWL_USER, lParam);

#ifdef I18N
		// �ǉ��E�u�������Ƃ� init_hosts_dlg ���Ă�ł���̂ŁA���̑O�ɃZ�b�g����K�v������
		GetWindowText(dlg, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
		UTIL_get_lang_msg("DLG_UNKNONWHOST_TITLE", pvar);
		SetWindowText(dlg, pvar->ts->UIMsg);

		GetDlgItemText(dlg, IDC_HOSTWARNING, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
		UTIL_get_lang_msg("DLG_UNKNOWNHOST_WARNINIG", pvar);
		SetDlgItemText(dlg, IDC_HOSTWARNING, pvar->ts->UIMsg);

		GetDlgItemText(dlg, IDC_HOSTWARNING2, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
		UTIL_get_lang_msg("DLG_UNKNOWNHOST_WARNINIG2", pvar);
		SetDlgItemText(dlg, IDC_HOSTWARNING2, pvar->ts->UIMsg);

		GetDlgItemText(dlg, IDC_HOSTFINGERPRINT, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
		UTIL_get_lang_msg("DLG_UNKNOWNHOST_FINGERPRINT", pvar);
		SetDlgItemText(dlg, IDC_HOSTFINGERPRINT, pvar->ts->UIMsg);

		GetDlgItemText(dlg, IDC_ADDTOKNOWNHOSTS, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
		UTIL_get_lang_msg("DLG_UNKNOWNHOST_ADD", pvar);
		SetDlgItemText(dlg, IDC_ADDTOKNOWNHOSTS, pvar->ts->UIMsg);

		GetDlgItemText(dlg, IDC_CONTINUE, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
		UTIL_get_lang_msg("BTN_CONTINUE", pvar);
		SetDlgItemText(dlg, IDC_CONTINUE, pvar->ts->UIMsg);

		GetDlgItemText(dlg, IDCANCEL, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
		UTIL_get_lang_msg("BTN_DISCONNECT", pvar);
		SetDlgItemText(dlg, IDCANCEL, pvar->ts->UIMsg);
#endif

		init_hosts_dlg(pvar, dlg);

		// add host check box�Ƀ`�F�b�N���f�t�H���g�œ���Ă��� 
		SendMessage(GetDlgItem(dlg, IDC_ADDTOKNOWNHOSTS), BM_SETCHECK, BST_CHECKED, 0);

		return TRUE;			/* because we do not set the focus */

	case WM_COMMAND:
		pvar = (PTInstVar) GetWindowLong(dlg, DWL_USER);

		switch (LOWORD(wParam)) {
		case IDC_CONTINUE:
			if (IsDlgButtonChecked(dlg, IDC_ADDTOKNOWNHOSTS)) {
				add_host_key(pvar);
			}

			if (SSHv1(pvar)) {
				SSH_notify_host_OK(pvar);
			} else { // SSH2
				// SSH2�ł͂��Ƃ� SSH_notify_host_OK() ���ĂԁB
			}

			pvar->hosts_state.hosts_dialog = NULL;

			EndDialog(dlg, 1);
			return TRUE;

		case IDCANCEL:			/* kill the connection */
			pvar->hosts_state.hosts_dialog = NULL;
			notify_closed_connection(pvar);
			EndDialog(dlg, 0);
			return TRUE;

		default:
			return FALSE;
		}

	default:
		return FALSE;
	}
}

//
// �u���������̊m�F�_�C�A���O�𕪗�
//
static BOOL CALLBACK hosts_replace_dlg_proc(HWND dlg, UINT msg, WPARAM wParam,
											LPARAM lParam)
{
	PTInstVar pvar;

	switch (msg) {
	case WM_INITDIALOG:
		pvar = (PTInstVar) lParam;
		pvar->hosts_state.hosts_dialog = dlg;
		SetWindowLong(dlg, DWL_USER, lParam);

#ifdef I18N
		// �ǉ��E�u�������Ƃ� init_hosts_dlg ���Ă�ł���̂ŁA���̑O�ɃZ�b�g����K�v������
		GetWindowText(dlg, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
		UTIL_get_lang_msg("DLG_DIFFERENTHOST_TITLE", pvar);
		SetWindowText(dlg, pvar->ts->UIMsg);

		GetDlgItemText(dlg, IDC_HOSTWARNING, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
		UTIL_get_lang_msg("DLG_DIFFERENTHOST_WARNING", pvar);
		SetDlgItemText(dlg, IDC_HOSTWARNING, pvar->ts->UIMsg);

		GetDlgItemText(dlg, IDC_HOSTWARNING2, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
		UTIL_get_lang_msg("DLG_DIFFERENTHOST_WARNING2", pvar);
		SetDlgItemText(dlg, IDC_HOSTWARNING2, pvar->ts->UIMsg);

		GetDlgItemText(dlg, IDC_HOSTFINGERPRINT, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
		UTIL_get_lang_msg("DLG_DIFFERENTHOST_FINGERPRINT", pvar);
		SetDlgItemText(dlg, IDC_HOSTFINGERPRINT, pvar->ts->UIMsg);

		GetDlgItemText(dlg, IDC_ADDTOKNOWNHOSTS, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
		UTIL_get_lang_msg("DLG_DIFFERENTHOST_REPLACE", pvar);
		SetDlgItemText(dlg, IDC_ADDTOKNOWNHOSTS, pvar->ts->UIMsg);

		GetDlgItemText(dlg, IDC_CONTINUE, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
		UTIL_get_lang_msg("BTN_CONTINUE", pvar);
		SetDlgItemText(dlg, IDC_CONTINUE, pvar->ts->UIMsg);

		GetDlgItemText(dlg, IDCANCEL, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
		UTIL_get_lang_msg("BTN_DISCONNECT", pvar);
		SetDlgItemText(dlg, IDCANCEL, pvar->ts->UIMsg);
#endif

		init_hosts_dlg(pvar, dlg);

		// �f�t�H���g�Ń`�F�b�N�͓���Ȃ�
		return TRUE;			/* because we do not set the focus */

	case WM_COMMAND:
		pvar = (PTInstVar) GetWindowLong(dlg, DWL_USER);

		switch (LOWORD(wParam)) {
		case IDC_CONTINUE:
			if (IsDlgButtonChecked(dlg, IDC_ADDTOKNOWNHOSTS)) {
				add_host_key(pvar);
				delete_different_key(pvar);
			}

			if (SSHv1(pvar)) {
				SSH_notify_host_OK(pvar);
			} else { // SSH2
				// SSH2�ł͂��Ƃ� SSH_notify_host_OK() ���ĂԁB
			}

			pvar->hosts_state.hosts_dialog = NULL;

			EndDialog(dlg, 1);
			return TRUE;

		case IDCANCEL:			/* kill the connection */
			pvar->hosts_state.hosts_dialog = NULL;
			notify_closed_connection(pvar);
			EndDialog(dlg, 0);
			return TRUE;

		default:
			return FALSE;
		}

	default:
		return FALSE;
	}
}

void HOSTS_do_unknown_host_dialog(HWND wnd, PTInstVar pvar)
{
	if (pvar->hosts_state.hosts_dialog == NULL) {
		HWND cur_active = GetActiveWindow();

		DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SSHUNKNOWNHOST),
					   cur_active != NULL ? cur_active : wnd,
					   hosts_add_dlg_proc, (LPARAM) pvar);
	}
}

void HOSTS_do_different_host_dialog(HWND wnd, PTInstVar pvar)
{
	if (pvar->hosts_state.hosts_dialog == NULL) {
		HWND cur_active = GetActiveWindow();

		DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SSHDIFFERENTHOST),
					   cur_active != NULL ? cur_active : wnd,
					   hosts_replace_dlg_proc, (LPARAM) pvar);
	}
}

//
// �T�[�o���瑗���Ă����z�X�g���J���̑Ó������`�F�b�N����
//
// SSH2�Ή���ǉ� (2006.3.24 yutaka)
//
BOOL HOSTS_check_host_key(PTInstVar pvar, char FAR * hostname, Key *key)
{
	int found_different_key = 0;

	// ���ł� known_hosts �t�@�C������z�X�g���J����ǂݍ���ł���Ȃ�A����Ɣ�r����B
	if (pvar->hosts_state.prefetched_hostname != NULL
		&& _stricmp(pvar->hosts_state.prefetched_hostname, hostname) == 0
		&& match_key(pvar, key)) {

		if (SSHv1(pvar)) {
			SSH_notify_host_OK(pvar);
		} else {
			// SSH2�ł͂��Ƃ� SSH_notify_host_OK() ���ĂԁB
		}
		return TRUE;
	}

	// ��ǂ݂���Ă��Ȃ��ꍇ�́A���̎��_�Ńt�@�C������ǂݍ���
	if (begin_read_host_files(pvar, 0)) {
		do {
			if (!read_host_key(pvar, hostname, 0, 0)) {
				break;
			}

			if (pvar->hosts_state.hostkey.type != KEY_UNSPEC) {
				if (match_key(pvar, key)) {
					finish_read_host_files(pvar, 0);
					// ���ׂẴG���g�����Q�Ƃ��āA���v����L�[������������߂�B
					// SSH2�̏ꍇ�͂����ł͉������Ȃ��B(2006.3.29 yutaka)
					if (SSHv1(pvar)) {
						SSH_notify_host_OK(pvar);
					} else {
						// SSH2�ł͂��Ƃ� SSH_notify_host_OK() ���ĂԁB
					}
					return TRUE;
				} else {
					// �L�[�� known_hosts �Ɍ����������A�L�[�̓��e���قȂ�B
					found_different_key = 1;
				}
			}
		} while (pvar->hosts_state.hostkey.type != KEY_UNSPEC);  // �L�[���������Ă���Ԃ̓��[�v����

		finish_read_host_files(pvar, 0);
	}


	// known_hosts �ɑ��݂��Ȃ��L�[�͂��ƂŃt�@�C���֏������ނ��߂ɁA�����ŕۑ����Ă����B
	pvar->hosts_state.hostkey.type = key->type;
	if (key->type == KEY_RSA1) { // SSH1
		pvar->hosts_state.hostkey.bits = key->bits;
		pvar->hosts_state.hostkey.exp = copy_mp_int(key->exp);
		pvar->hosts_state.hostkey.mod = copy_mp_int(key->mod);

	} else if (key->type == KEY_RSA) { // SSH2 RSA
		pvar->hosts_state.hostkey.rsa = duplicate_RSA(key->rsa);

	} else { // SSH2 DSA
		pvar->hosts_state.hostkey.dsa = duplicate_DSA(key->dsa);

	}
	free(pvar->hosts_state.prefetched_hostname);
	pvar->hosts_state.prefetched_hostname = _strdup(hostname);

	if (found_different_key) {
		PostMessage(pvar->NotificationWindow, WM_COMMAND,
					ID_SSHDIFFERENTHOST, 0);
	} else {
		PostMessage(pvar->NotificationWindow, WM_COMMAND,
					ID_SSHUNKNOWNHOST, 0);
	}

	return TRUE;
}

void HOSTS_notify_disconnecting(PTInstVar pvar)
{
	if (pvar->hosts_state.hosts_dialog != NULL) {
		PostMessage(pvar->hosts_state.hosts_dialog, WM_COMMAND, IDCANCEL,
					0);
		/* the main window might not go away if it's not enabled. (see vtwin.cpp) */
		EnableWindow(pvar->NotificationWindow, TRUE);
	}
}

void HOSTS_end(PTInstVar pvar)
{
	int i;

	free(pvar->hosts_state.prefetched_hostname);
#if 0
	free(pvar->hosts_state.key_exp);
	free(pvar->hosts_state.key_mod);
#else
	init_hostkey(&pvar->hosts_state.hostkey);
#endif

	if (pvar->hosts_state.file_names != NULL) {
		for (i = 0; pvar->hosts_state.file_names[i] != NULL; i++) {
			free(pvar->hosts_state.file_names[i]);
		}
		free(pvar->hosts_state.file_names);
	}
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.11  2006/11/30 09:56:43  maya
 * �\�����b�Z�[�W�̓ǂݍ��ݑΉ�
 *
 * Revision 1.10  2006/11/23 02:19:30  maya
 * �\�����b�Z�[�W������t�@�C������ǂݍ��݂ރR�[�h�̍쐬���J�n�����B
 *
 * Revision 1.9  2006/07/01 00:41:02  maya
 * �������ݗp ssh_known_hosts ���w�肳��Ă��Ȃ��Ƃ��ɗ�����s����C�������B
 *
 * Revision 1.8  2006/06/29 15:27:00  yutakakn
 * ssh_known_files�t�@�C�������TeraTerm�C���X�g�[���f�B���N�g���֕ۑ�����悤�ɂ����B
 *
 * Revision 1.7  2006/04/04 13:52:52  yutakakn
 * known_hosts�t�@�C���ɂ����ăL�[��ʂ̈قȂ铯��z�X�g�̃G���g�����������ꍇ�A�Â��L�[���폜����@�\��ǉ������B
 *
 * Revision 1.6  2006/03/29 14:56:52  yutakakn
 * known_hosts�t�@�C���ɃL�[��ʂ̈قȂ铯��z�X�g�̃G���g��������ƁA�A�v���P�[�V�����G���[�ƂȂ�o�O���C�������B
 *
 * Revision 1.5  2006/03/26 17:07:17  yutakakn
 * fingerprint�\����ǉ�
 *
 * Revision 1.4  2006/03/26 15:43:58  yutakakn
 * SSH2��known_hosts�Ή���ǉ������B
 *
 * Revision 1.3  2006/02/18 07:37:02  yutakakn
 *   �E�R���p�C���� Visual Studio 2005 Standard Edition �ɐ؂�ւ����B
 *   �Estricmp()��_stricmp()�֒u������
 *   �Estrdup()��_strdup()�֒u������
 *
 * Revision 1.2  2004/12/19 15:39:42  yutakakn
 * CVS LogID�̒ǉ�
 *
 */
