/*
Copyright (c) 2011, TeraTerm Project
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

#include "ttxssh.h"
#include "key.h"

#include <openssl/rsa.h>
#include <openssl/dsa.h>

#define INTBLOB_LEN 20
#define SIGBLOB_LEN (2*INTBLOB_LEN)

//////////////////////////////////////////////////////////////////////////////
//
// Key verify function
//
//////////////////////////////////////////////////////////////////////////////

//
// DSS
//

int ssh_dss_verify(DSA *key,
                   u_char *signature, u_int signaturelen,
                   u_char *data, u_int datalen)
{
	DSA_SIG *sig;
	const EVP_MD *evp_md = EVP_sha1();
	EVP_MD_CTX md;
	unsigned char digest[EVP_MAX_MD_SIZE], *sigblob;
	unsigned int len, dlen;
	int ret;
	char *ptr;

	OpenSSL_add_all_digests();

	if (key == NULL) {
		return -2;
	}

	ptr = signature;

	// step1
	if (signaturelen == 0x28) {
		// workaround for SSH-2.0-2.0* and SSH-2.0-2.1* (2006.11.18 maya)
		ptr -= 4;
	}
	else {
		len = get_uint32_MSBfirst(ptr);
		ptr += 4;
		if (strncmp("ssh-dss", ptr, len) != 0) {
			return -3;
		}
		ptr += len;
	}

	// step2
	len = get_uint32_MSBfirst(ptr);
	ptr += 4;
	sigblob = ptr;
	ptr += len;

	if (len != SIGBLOB_LEN) {
		return -4;
	}

	/* parse signature */
	if ((sig = DSA_SIG_new()) == NULL)
		return -5;
	if ((sig->r = BN_new()) == NULL)
		return -6;
	if ((sig->s = BN_new()) == NULL)
		return -7;
	BN_bin2bn(sigblob, INTBLOB_LEN, sig->r);
	BN_bin2bn(sigblob+ INTBLOB_LEN, INTBLOB_LEN, sig->s);

	/* sha1 the data */
	EVP_DigestInit(&md, evp_md);
	EVP_DigestUpdate(&md, data, datalen);
	EVP_DigestFinal(&md, digest, &dlen);

	ret = DSA_do_verify(digest, dlen, sig, key);
	memset(digest, 'd', sizeof(digest));

	DSA_SIG_free(sig);

	return ret;
}


//
// RSA
//

/*
* See:
* http://www.rsasecurity.com/rsalabs/pkcs/pkcs-1/
* ftp://ftp.rsasecurity.com/pub/pkcs/pkcs-1/pkcs-1v2-1.asn
*/
/*
* id-sha1 OBJECT IDENTIFIER ::= { iso(1) identified-organization(3)
*	oiw(14) secsig(3) algorithms(2) 26 }
*/
static const u_char id_sha1[] = {
	0x30, 0x21, /* type Sequence, length 0x21 (33) */
		0x30, 0x09, /* type Sequence, length 0x09 */
		0x06, 0x05, /* type OID, length 0x05 */
		0x2b, 0x0e, 0x03, 0x02, 0x1a, /* id-sha1 OID */
		0x05, 0x00, /* NULL */
		0x04, 0x14  /* Octet string, length 0x14 (20), followed by sha1 hash */
};
/*
* id-md5 OBJECT IDENTIFIER ::= { iso(1) member-body(2) us(840)
*	rsadsi(113549) digestAlgorithm(2) 5 }
*/
static const u_char id_md5[] = {
	0x30, 0x20, /* type Sequence, length 0x20 (32) */
		0x30, 0x0c, /* type Sequence, length 0x09 */
		0x06, 0x08, /* type OID, length 0x05 */
		0x2a, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x02, 0x05, /* id-md5 */
		0x05, 0x00, /* NULL */
		0x04, 0x10  /* Octet string, length 0x10 (16), followed by md5 hash */
};

static int openssh_RSA_verify(int type, u_char *hash, u_int hashlen,
                              u_char *sigbuf, u_int siglen, RSA *rsa)
{
	u_int ret, rsasize, oidlen = 0, hlen = 0;
	int len;
	const u_char *oid = NULL;
	u_char *decrypted = NULL;

	ret = 0;
	switch (type) {
	case NID_sha1:
		oid = id_sha1;
		oidlen = sizeof(id_sha1);
		hlen = 20;
		break;
	case NID_md5:
		oid = id_md5;
		oidlen = sizeof(id_md5);
		hlen = 16;
		break;
	default:
		goto done;
		break;
	}
	if (hashlen != hlen) {
		//error("bad hashlen");
		goto done;
	}
	rsasize = RSA_size(rsa);
	if (siglen == 0 || siglen > rsasize) {
		//error("bad siglen");
		goto done;
	}
	decrypted = malloc(rsasize);
	if (decrypted == NULL)
		return 1; // error

	if ((len = RSA_public_decrypt(siglen, sigbuf, decrypted, rsa,
	                              RSA_PKCS1_PADDING)) < 0) {
			//error("RSA_public_decrypt failed: %s",
			//    ERR_error_string(ERR_get_error(), NULL));
			goto done;
		}
		if (len != hlen + oidlen) {
			//error("bad decrypted len: %d != %d + %d", len, hlen, oidlen);
			goto done;
		}
		if (memcmp(decrypted, oid, oidlen) != 0) {
			//error("oid mismatch");
			goto done;
		}
		if (memcmp(decrypted + oidlen, hash, hlen) != 0) {
			//error("hash mismatch");
			goto done;
		}
		ret = 1;
done:
		if (decrypted)
			free(decrypted);
		return ret;
}

int ssh_rsa_verify(RSA *key,
                   u_char *signature, u_int signaturelen,
                   u_char *data, u_int datalen)
{
	const EVP_MD *evp_md;
	EVP_MD_CTX md;
	//	char *ktype;
	u_char digest[EVP_MAX_MD_SIZE], *sigblob;
	u_int len, dlen, modlen;
//	int rlen, ret, nid;
	int ret, nid;
	char *ptr;

	OpenSSL_add_all_digests();

	if (key == NULL) {
		return -2;
	}
	if (BN_num_bits(key->n) < SSH_RSA_MINIMUM_MODULUS_SIZE) {
		return -3;
	}
	//debug_print(41, signature, signaturelen);
	ptr = signature;

	// step1
	len = get_uint32_MSBfirst(ptr);
	ptr += 4;
	if (strncmp("ssh-rsa", ptr, len) != 0) {
		return -4;
	}
	ptr += len;

	// step2
	len = get_uint32_MSBfirst(ptr);
	ptr += 4;
	sigblob = ptr;
	ptr += len;
#if 0
	rlen = get_uint32_MSBfirst(ptr);
	if (rlen != 0) {
		return -1;
	}
#endif

	/* RSA_verify expects a signature of RSA_size */
	modlen = RSA_size(key);
	if (len > modlen) {
		return -5;

	} else if (len < modlen) {
		u_int diff = modlen - len;
		sigblob = realloc(sigblob, modlen);
		memmove(sigblob + diff, sigblob, len);
		memset(sigblob, 0, diff);
		len = modlen;
	}
	
	/* sha1 the data */
	//	nid = (datafellows & SSH_BUG_RSASIGMD5) ? NID_md5 : NID_sha1;
	nid = NID_sha1;
	if ((evp_md = EVP_get_digestbynid(nid)) == NULL) {
		//error("ssh_rsa_verify: EVP_get_digestbynid %d failed", nid);
		return -6;
	}
	EVP_DigestInit(&md, evp_md);
	EVP_DigestUpdate(&md, data, datalen);
	EVP_DigestFinal(&md, digest, &dlen);

	ret = openssh_RSA_verify(nid, digest, dlen, sigblob, len, key);

	memset(digest, 'd', sizeof(digest));
	memset(sigblob, 's', len);
	//free(sigblob);
	//debug("ssh_rsa_verify: signature %scorrect", (ret==0) ? "in" : "");

	return ret;
}

int key_verify(RSA *rsa_key, DSA *dsa_key,
               unsigned char *signature, unsigned int signaturelen,
               unsigned char *data, unsigned int datalen)
{
	int ret = 0;

	if (rsa_key != NULL) {
		ret = ssh_rsa_verify(rsa_key, signature, signaturelen, data, datalen);

	} else if (dsa_key != NULL) {
		ret = ssh_dss_verify(dsa_key, signature, signaturelen, data, datalen);

	} else {
		return -1;
	}

	return (ret);   // success
}

//
// RSA�\���̂̕���
//
RSA *duplicate_RSA(RSA *src)
{
	RSA *rsa = NULL;

	rsa = RSA_new();
	if (rsa == NULL)
		goto error;
	rsa->n = BN_new();
	rsa->e = BN_new();
	if (rsa->n == NULL || rsa->e == NULL) {
		RSA_free(rsa);
		goto error;
	}

	// �[���R�s�[(deep copy)���s���B�󂢃R�s�[(shallow copy)��NG�B
	BN_copy(rsa->n, src->n);
	BN_copy(rsa->e, src->e);

error:
	return (rsa);
}


//
// DSA�\���̂̕���
//
DSA *duplicate_DSA(DSA *src)
{
	DSA *dsa = NULL;

	dsa = DSA_new();
	if (dsa == NULL)
		goto error;
	dsa->p = BN_new();
	dsa->q = BN_new();
	dsa->g = BN_new();
	dsa->pub_key = BN_new();
	if (dsa->p == NULL ||
	    dsa->q == NULL ||
	    dsa->g == NULL ||
	    dsa->pub_key == NULL) {
		DSA_free(dsa);
		goto error;
	}

	// �[���R�s�[(deep copy)���s���B�󂢃R�s�[(shallow copy)��NG�B
	BN_copy(dsa->p, src->p);
	BN_copy(dsa->q, src->q);
	BN_copy(dsa->g, src->g);
	BN_copy(dsa->pub_key, src->pub_key);

error:
	return (dsa);
}


char* key_fingerprint_raw(Key *k, int *dgst_raw_length)
{
	const EVP_MD *md = NULL;
	EVP_MD_CTX ctx;
	char *blob = NULL;
	char *retval = NULL;
	int len = 0;
	int nlen, elen;
	RSA *rsa;

	*dgst_raw_length = 0;

	// MD5�A���S���Y�����g�p����
	md = EVP_md5();

	switch (k->type) {
	case KEY_RSA1:
		rsa = make_key(NULL, k->bits, k->exp, k->mod);
		nlen = BN_num_bytes(rsa->n);
		elen = BN_num_bytes(rsa->e);
		len = nlen + elen;
		blob = malloc(len);
		if (blob == NULL) {
			// TODO:
		}
		BN_bn2bin(rsa->n, blob);
		BN_bn2bin(rsa->e, blob + nlen);
		RSA_free(rsa);
		break;

	case KEY_DSA:
	case KEY_RSA:
		key_to_blob(k, &blob, &len);
		break;

	case KEY_UNSPEC:
		return retval;
		break;

	default:
		//fatal("key_fingerprint_raw: bad key type %d", k->type);
		break;
	}

	if (blob != NULL) {
		retval = malloc(EVP_MAX_MD_SIZE);
		if (retval == NULL) {
			// TODO:
		}
		EVP_DigestInit(&ctx, md);
		EVP_DigestUpdate(&ctx, blob, len);
		EVP_DigestFinal(&ctx, retval, dgst_raw_length);
		memset(blob, 0, len);
		free(blob);
	} else {
		//fatal("key_fingerprint_raw: blob is null");
	}
	return retval;
}


const char *
key_type(const Key *k)
{
	switch (k->type) {
	case KEY_RSA1:
		return "RSA1";
	case KEY_RSA:
		return "RSA";
	case KEY_DSA:
		return "DSA";
	}
	return "unknown";
}

unsigned int
key_size(const Key *k)
{
	switch (k->type) {
	case KEY_RSA1:
		// SSH1�̏ꍇ�� key->rsa �� key->dsa �� NULL �ł���̂ŁA�g��Ȃ��B
		return k->bits;
	case KEY_RSA:
		return BN_num_bits(k->rsa->n);
	case KEY_DSA:
		return BN_num_bits(k->dsa->p);
	}
	return 0;
}

// based on OpenSSH 5.1
#define	FLDBASE		8
#define	FLDSIZE_Y	(FLDBASE + 1)
#define	FLDSIZE_X	(FLDBASE * 2 + 1)
static char *
key_fingerprint_randomart(u_char *dgst_raw, u_int dgst_raw_len, const Key *k)
{
	/*
	 * Chars to be used after each other every time the worm
	 * intersects with itself.  Matter of taste.
	 */
	char	*augmentation_string = " .o+=*BOX@%&#/^SE";
	char	*retval, *p;
	unsigned char	 field[FLDSIZE_X][FLDSIZE_Y];
	unsigned int	 i, b;
	int	 x, y;
	size_t	 len = strlen(augmentation_string) - 1;

	retval = calloc(1, (FLDSIZE_X + 3 + 1) * (FLDSIZE_Y + 2));

	/* initialize field */
	memset(field, 0, FLDSIZE_X * FLDSIZE_Y * sizeof(char));
	x = FLDSIZE_X / 2;
	y = FLDSIZE_Y / 2;

	/* process raw key */
	for (i = 0; i < dgst_raw_len; i++) {
		int input;
		/* each byte conveys four 2-bit move commands */
		input = dgst_raw[i];
		for (b = 0; b < 4; b++) {
			/* evaluate 2 bit, rest is shifted later */
			x += (input & 0x1) ? 1 : -1;
			y += (input & 0x2) ? 1 : -1;

			/* assure we are still in bounds */
			x = max(x, 0);
			y = max(y, 0);
			x = min(x, FLDSIZE_X - 1);
			y = min(y, FLDSIZE_Y - 1);

			/* augment the field */
			field[x][y]++;
			input = input >> 2;
		}
	}

	/* mark starting point and end point*/
	field[FLDSIZE_X / 2][FLDSIZE_Y / 2] = len - 1;
	field[x][y] = len;

	/* fill in retval */
	_snprintf_s(retval, FLDSIZE_X, _TRUNCATE, "+--[%4s %4u]", key_type(k), key_size(k));
	p = strchr(retval, '\0');

	/* output upper border */
	for (i = p - retval - 1; i < FLDSIZE_X; i++)
		*p++ = '-';
	*p++ = '+';
	*p++ = '\r';
	*p++ = '\n';

	/* output content */
	for (y = 0; y < FLDSIZE_Y; y++) {
		*p++ = '|';
		for (x = 0; x < FLDSIZE_X; x++)
			*p++ = augmentation_string[min(field[x][y], len)];
		*p++ = '|';
		*p++ = '\r';
		*p++ = '\n';
	}

	/* output lower border */
	*p++ = '+';
	for (i = 0; i < FLDSIZE_X; i++)
		*p++ = '-';
	*p++ = '+';

	return retval;
}
#undef	FLDBASE	
#undef	FLDSIZE_Y
#undef	FLDSIZE_X

//
// fingerprint�i�w��F�z�X�g���J���̃n�b�V���j�𐶐�����
//
char *key_fingerprint(Key *key, enum fp_rep dgst_rep)
{
	char *retval = NULL;
	unsigned char *dgst_raw;
	int dgst_raw_len;
	int i, retval_len;

	// fingerprint�̃n�b�V���l�i�o�C�i���j�����߂�
	dgst_raw = key_fingerprint_raw(key, &dgst_raw_len);

	if (dgst_rep == SSH_FP_HEX) {
		// 16�i�\�L�֕ϊ�����
		retval_len = dgst_raw_len * 3 + 1;
		retval = malloc(retval_len);
		retval[0] = '\0';
		for (i = 0; i < dgst_raw_len; i++) {
			char hex[4];
			_snprintf_s(hex, sizeof(hex), _TRUNCATE, "%02x:", dgst_raw[i]);
			strncat_s(retval, retval_len, hex, _TRUNCATE);
		}

		/* Remove the trailing ':' character */
		retval[(dgst_raw_len * 3) - 1] = '\0';

	} else if (dgst_rep == SSH_FP_RANDOMART) {
		retval = key_fingerprint_randomart(dgst_raw, dgst_raw_len, key);

	} else {

	}

	memset(dgst_raw, 0, dgst_raw_len);
	free(dgst_raw);

	return (retval);
}


//
// �L�[�̃������̈���
//
void key_free(Key *key)
{
	switch (key->type) {
		case KEY_RSA1:
		case KEY_RSA:
			if (key->rsa != NULL)
				RSA_free(key->rsa);
			key->rsa = NULL;
			break;

		case KEY_DSA:
			if (key->dsa != NULL)
				DSA_free(key->dsa);
			key->dsa = NULL;
			break;
	}
	free(key);
}

//
// �L�[���當�����ԋp����
//
char *get_sshname_from_key(Key *key)
{
	if (key->type == KEY_RSA) {
		return "ssh-rsa";
	} else if (key->type == KEY_DSA) {
		return "ssh-dss";
	} else {
		return "ssh-unknown";
	}
}


//
// �L�[�����񂩂��ʂ𔻒肷��
//
enum hostkey_type get_keytype_from_name(char *name)
{
	if (strcmp(name, "rsa1") == 0) {
		return KEY_RSA1;
	} else if (strcmp(name, "rsa") == 0) {
		return KEY_RSA;
	} else if (strcmp(name, "dsa") == 0) {
		return KEY_DSA;
	} else if (strcmp(name, "ssh-rsa") == 0) {
		return KEY_RSA;
	} else if (strcmp(name, "ssh-dss") == 0) {
		return KEY_DSA;
	}
	return KEY_UNSPEC;
}


//
// �L�[��񂩂�o�b�t�@�֕ϊ����� (for SSH2)
// NOTE:
//
int key_to_blob(Key *key, char **blobp, int *lenp)
{
	buffer_t *b;
	char *sshname;
	int len;
	int ret = 1;  // success

	b = buffer_init();
	sshname = get_sshname_from_key(key);

	if (key->type == KEY_RSA) {
		buffer_put_string(b, sshname, strlen(sshname));
		buffer_put_bignum2(b, key->rsa->e);
		buffer_put_bignum2(b, key->rsa->n);

	} else if (key->type == KEY_DSA) {
		buffer_put_string(b, sshname, strlen(sshname));
		buffer_put_bignum2(b, key->dsa->p);
		buffer_put_bignum2(b, key->dsa->q);
		buffer_put_bignum2(b, key->dsa->g);
		buffer_put_bignum2(b, key->dsa->pub_key);

	} else {
		ret = 0;
		goto error;

	}

	len = buffer_len(b);
	if (lenp != NULL)
		*lenp = len;
	if (blobp != NULL) {
		*blobp = malloc(len);
		if (*blobp == NULL) {
			ret = 0;
			goto error;
		}
		memcpy(*blobp, buffer_ptr(b), len);
	}

error:
	buffer_free(b);

	return (ret);
}


//
// �o�b�t�@����L�[�������o��(for SSH2)
// NOTE: �Ԓl�̓A���P�[�g�̈�ɂȂ�̂ŁA�Ăяo�����ŉ�����邱�ƁB
//
Key *key_from_blob(char *data, int blen)
{
	int keynamelen;
	char key[128];
	RSA *rsa = NULL;
	DSA *dsa = NULL;
	Key *hostkey;  // hostkey
	enum hostkey_type type;

	hostkey = malloc(sizeof(Key));
	if (hostkey == NULL)
		goto error;

	memset(hostkey, 0, sizeof(Key));

	keynamelen = get_uint32_MSBfirst(data);
	if (keynamelen >= sizeof(key)) {
		goto error;
	}
	data += 4;
	memcpy(key, data, keynamelen);
	key[keynamelen] = 0;
	data += keynamelen;

	type = get_keytype_from_name(key);

		// RSA key
	if (type == KEY_RSA) {
		rsa = RSA_new();
		if (rsa == NULL) {
			goto error;
		}
		rsa->n = BN_new();
		rsa->e = BN_new();
		if (rsa->n == NULL || rsa->e == NULL) {
			goto error;
		}

		buffer_get_bignum2(&data, rsa->e);
		buffer_get_bignum2(&data, rsa->n);

		hostkey->type = type;
		hostkey->rsa = rsa;

	} else if (type == KEY_DSA) { // DSA key
		dsa = DSA_new();
		if (dsa == NULL) {
			goto error;
		}
		dsa->p = BN_new();
		dsa->q = BN_new();
		dsa->g = BN_new();
		dsa->pub_key = BN_new();
		if (dsa->p == NULL ||
		    dsa->q == NULL ||
		    dsa->g == NULL ||
		    dsa->pub_key == NULL) {
			goto error;
		}

		buffer_get_bignum2(&data, dsa->p);
		buffer_get_bignum2(&data, dsa->q);
		buffer_get_bignum2(&data, dsa->g);
		buffer_get_bignum2(&data, dsa->pub_key);

		hostkey->type = type;
		hostkey->dsa = dsa;

	} else {
		// unknown key
		goto error;

	}

	return (hostkey);

error:
	if (rsa != NULL)
		RSA_free(rsa);
	if (dsa != NULL)
		DSA_free(dsa);

	return NULL;
}

char *get_SSH2_keyname(CRYPTKeyPair *keypair)
{
	char *s;

	if (keypair->RSA_key != NULL) {
		s = "ssh-rsa";
	} else {
		s = "ssh-dss";
	}

	return (s);
}


BOOL generate_SSH2_keysign(CRYPTKeyPair *keypair, char **sigptr, int *siglen, char *data, int datalen)
{
	buffer_t *msg = NULL;
	char *s;

	msg = buffer_init();
	if (msg == NULL) {
		// TODO: error check
		return FALSE;
	}

	if (keypair->RSA_key != NULL) { // RSA
		const EVP_MD *evp_md;
		EVP_MD_CTX md;
		u_char digest[EVP_MAX_MD_SIZE], *sig;
		u_int slen, dlen, len;
		int ok, nid;

		nid = NID_sha1;
		if ((evp_md = EVP_get_digestbynid(nid)) == NULL) {
			goto error;
		}

		// �_�C�W�F�X�g�l�̌v�Z
		EVP_DigestInit(&md, evp_md);
		EVP_DigestUpdate(&md, data, datalen);
		EVP_DigestFinal(&md, digest, &dlen);

		slen = RSA_size(keypair->RSA_key);
		sig = malloc(slen);
		if (sig == NULL)
			goto error;

		// �d�q�������v�Z
		ok = RSA_sign(nid, digest, dlen, sig, &len, keypair->RSA_key);
		memset(digest, 'd', sizeof(digest));
		if (ok != 1) { // error
			free(sig);
			goto error;
		}
		// �����̃T�C�Y���o�b�t�@��菬�����ꍇ�A���ւ��炷�B�擪�̓[���Ŗ��߂�B
		if (len < slen) {
			u_int diff = slen - len;
			memmove(sig + diff, sig, len);
			memset(sig, 0, diff);

		} else if (len > slen) {
			free(sig);
			goto error;

		} else {
			// do nothing

		}

		s = get_SSH2_keyname(keypair);
		buffer_put_string(msg, s, strlen(s));
		buffer_append_length(msg, sig, slen);
		len = buffer_len(msg);

		// setting
		*siglen = len;
		*sigptr = malloc(len);
		if (*sigptr == NULL) {
			free(sig);
			goto error;
		}
		memcpy(*sigptr, buffer_ptr(msg), len);
		free(sig);

	} else { // DSA
		DSA_SIG *sig;
		const EVP_MD *evp_md = EVP_sha1();
		EVP_MD_CTX md;
		u_char digest[EVP_MAX_MD_SIZE], sigblob[SIGBLOB_LEN];
		u_int rlen, slen, len, dlen;

		// �_�C�W�F�X�g�̌v�Z
		EVP_DigestInit(&md, evp_md);
		EVP_DigestUpdate(&md, data, datalen);
		EVP_DigestFinal(&md, digest, &dlen);

		// DSA�d�q�������v�Z
		sig = DSA_do_sign(digest, dlen, keypair->DSA_key);
		memset(digest, 'd', sizeof(digest));
		if (sig == NULL) {
			goto error;
		}

		// BIGNUM����o�C�i���l�ւ̕ϊ�
		rlen = BN_num_bytes(sig->r);
		slen = BN_num_bytes(sig->s);
		if (rlen > INTBLOB_LEN || slen > INTBLOB_LEN) {
			DSA_SIG_free(sig);
			goto error;
		}
		memset(sigblob, 0, SIGBLOB_LEN);
		BN_bn2bin(sig->r, sigblob+ SIGBLOB_LEN - INTBLOB_LEN - rlen);
		BN_bn2bin(sig->s, sigblob+ SIGBLOB_LEN - slen);
		DSA_SIG_free(sig);

		// setting
		s = get_SSH2_keyname(keypair);
		buffer_put_string(msg, s, strlen(s));
		buffer_append_length(msg, sigblob, sizeof(sigblob));
		len = buffer_len(msg);

		// setting
		*siglen = len;
		*sigptr = malloc(len);
		if (*sigptr == NULL) {
			goto error;
		}
		memcpy(*sigptr, buffer_ptr(msg), len);

	}

	buffer_free(msg);
	return TRUE;

error:
	buffer_free(msg);

	return FALSE;
}


BOOL get_SSH2_publickey_blob(PTInstVar pvar, buffer_t **blobptr, int *bloblen)
{
	buffer_t *msg = NULL;
	CRYPTKeyPair *keypair;
	char *s;

	msg = buffer_init();
	if (msg == NULL) {
		// TODO: error check
		return FALSE;
	}

	keypair = pvar->auth_state.cur_cred.key_pair;

	if (keypair->RSA_key != NULL) { // RSA
		s = get_SSH2_keyname(keypair);
		buffer_put_string(msg, s, strlen(s));
		buffer_put_bignum2(msg, keypair->RSA_key->e); // ���J�w��
		buffer_put_bignum2(msg, keypair->RSA_key->n); // p�~q

	} else { // DSA
		s = get_SSH2_keyname(keypair);
		buffer_put_string(msg, s, strlen(s));
		buffer_put_bignum2(msg, keypair->DSA_key->p); // �f��
		buffer_put_bignum2(msg, keypair->DSA_key->q); // (p-1)�̑f����
		buffer_put_bignum2(msg, keypair->DSA_key->g); // ����
		buffer_put_bignum2(msg, keypair->DSA_key->pub_key); // ���J��

	}

	*blobptr = msg;
	*bloblen = buffer_len(msg);

	return TRUE;
}
