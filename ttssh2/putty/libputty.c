// PuTTY is copyright 1997-2007 Simon Tatham.
/*
 * Copyright (C) 1994-1998 T. Teranishi
 * (C) 2004-2017 TeraTerm Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <windows.h>
#include <assert.h>

#include "sshbn.h"

// from SSHBN.C (ver 0.60)
#define BIGNUM_INTERNAL
typedef BignumInt *Bignum;

#include "ssh.h"

#include "libputty.h"


/*
 * for SSH2
 *   ���̈ꗗ�𓾂�
 */
int putty_get_ssh2_keylist(unsigned char **keylist)
{
	int keylistlen;

	*keylist = get_keylist2(&keylistlen);
	if (*keylist == NULL){
		// �擾�Ɏ��s
		return 0;
	}
	return keylistlen;
}

/*
 * for SSH2
 *   ���J���ƃf�[�^(���������J��)��n���A
 *   ���J���ɂ���ď������ꂽ�f�[�^�𓾂�
 */
void *putty_sign_ssh2_key(unsigned char *pubkey,
                          unsigned char *data,
                          int *outlen)
{
	void *ret;

	unsigned char *request, *response;
	void *vresponse;
	int resplen;
	int pubkeylen, datalen, reqlen;

	pubkeylen = GET_32BIT(pubkey);
	datalen = GET_32BIT(data);
	reqlen = 4 + 1 + (4 + pubkeylen) + (4 + datalen);
	request = (unsigned char *)malloc(reqlen);

	// request length
	PUT_32BIT(request, reqlen);
	// request type
	request[4] = SSH2_AGENTC_SIGN_REQUEST;
	// public key (length + data)
	memcpy(request + 5, pubkey, 4 + pubkeylen);
	// sign data (length + data)
	memcpy(request + 5 + 4 + pubkeylen, data, 4 + datalen);

	agent_query(request, reqlen, &vresponse, &resplen, NULL, NULL);

	response = vresponse;
	if (resplen < 5 || response[4] != SSH2_AGENT_SIGN_RESPONSE) {
		sfree(response);
		return NULL;
	}

	ret = snewn(resplen-5, unsigned char);
	memcpy(ret, response+5, resplen-5);
	sfree(response);

	if (outlen)
		*outlen = resplen-5;

	return ret;
}

/*
 * for SSH1
 *   ���̈ꗗ�𓾂�
 */
int putty_get_ssh1_keylist(unsigned char **keylist)
{
	int keylistlen;

	*keylist = get_keylist1(&keylistlen);
	if (*keylist == NULL){
		// �擾�Ɏ��s
		return 0;
	}
	return keylistlen;
}

/*
 * for SSH1
 *   ���J���ƈÍ����f�[�^��n��
 *   �����f�[�^�̃n�b�V���𓾂�
 */
void *putty_hash_ssh1_challenge(unsigned char *pubkey,
                                int pubkeylen,
                                unsigned char *data,
                                int datalen,
                                unsigned char *session_id,
                                int *outlen)
{
	void *ret;

	unsigned char *request, *response, *p;
	void *vresponse;
	int resplen;
	int reqlen;

	reqlen = 4 + 1 + pubkeylen + datalen + 16 + 4;
	request = (unsigned char *)malloc(reqlen);
	p = request;

	// request length
	PUT_32BIT(request, reqlen);
	// request type
	request[4] = SSH1_AGENTC_RSA_CHALLENGE;
	p += 5;

	// public key
	memcpy(p, pubkey, pubkeylen);
	p += pubkeylen;
	// challange from server
	memcpy(p, data, datalen);
	p += datalen;
	// session_id
	memcpy(p, session_id, 16);
	p += 16;
	// terminator?
	PUT_32BIT(p, 1);

	agent_query(request, reqlen, &vresponse, &resplen, NULL, NULL);

	response = vresponse;
	if (resplen < 5 || response[4] != SSH1_AGENT_RSA_RESPONSE) {
		sfree(response);
		return NULL;
	}

	ret = snewn(resplen-5, unsigned char);
	memcpy(ret, response+5, resplen-5);
	sfree(response);

	if (outlen)
		*outlen = resplen-5;

	return ret;
}

int putty_get_ssh1_keylen(unsigned char *key,
                          int maxlen)
{
	return rsa_public_blob_len(key, maxlen);
}

const char *putty_get_version()
{
	extern const char ver[]; /* in version.c */
	return ver;
}

/*
 * Following functions are copied from putty source.
 */

// from SSHBN.C (ver 0.63)
static Bignum newbn(int length)
{
	Bignum b;

	assert(length >= 0 && length < INT_MAX / BIGNUM_INT_BITS);

	b = snewn(length + 1, BignumInt);
	if (!b)
		abort();		       /* FIXME */
	memset(b, 0, (length + 1) * sizeof(*b));
	b[0] = length;
	return b;
}

// from SSHBN.C (ver 0.65)
Bignum bignum_from_bytes(const unsigned char *data, int nbytes)
{
	Bignum result;
	int w, i;

	assert(nbytes >= 0 && nbytes < INT_MAX / 8);

	w = (nbytes + BIGNUM_INT_BYTES - 1) / BIGNUM_INT_BYTES; /* bytes->words */

	result = newbn(w);
	for (i = 1; i <= w; i++)
		result[i] = 0;
	for (i = nbytes; i--;) {
		unsigned char byte = *data++;
		result[1 + i / BIGNUM_INT_BYTES] |=
			(BignumInt)byte << (8 * i % BIGNUM_INT_BITS);
	}

	while (result[0] > 1 && result[result[0]] == 0)
		result[0]--;
	return result;
}

// from SSHBN.C (ver 0.60)
/*
* Read an SSH-1-format bignum from a data buffer. Return the number
* of bytes consumed, or -1 if there wasn't enough data.
*/
int ssh1_read_bignum(const unsigned char *data, int len, Bignum * result)
{
	const unsigned char *p = data;
	int i;
	int w, b;

	if (len < 2)
		return -1;

	w = 0;
	for (i = 0; i < 2; i++)
		w = (w << 8) + *p++;
	b = (w + 7) / 8;		       /* bits -> bytes */

	if (len < b + 2)
		return -1;

	if (!result)		       /* just return length */
		return b + 2;

	*result = bignum_from_bytes(p, b);

	return p + b - data;
}

// from SSHRSA.C (putty 0.60)
/* Given a public blob, determine its length. */
int rsa_public_blob_len(void *data, int maxlen)
{
	unsigned char *p = (unsigned char *)data;
	int n;

	if (maxlen < 4)
		return -1;
	p += 4;			       /* length word */
	maxlen -= 4;

	n = ssh1_read_bignum(p, maxlen, NULL);    /* exponent */
	if (n < 0)
		return -1;
	p += n;

	n = ssh1_read_bignum(p, maxlen, NULL);    /* modulus */
	if (n < 0)
		return -1;
	p += n;

	return p - (unsigned char *)data;
}

// from WINDOWS\WINPGNT.C (putty 0.63)
/*
 * Acquire a keylist1 from the primary Pageant; this means either
 * calling make_keylist1 (if that's us) or sending a message to the
 * primary Pageant (if it's not).
 */
static void *get_keylist1(int *length)
{
	void *ret;

	unsigned char request[5], *response;
	void *vresponse;
	int resplen;
	request[4] = SSH1_AGENTC_REQUEST_RSA_IDENTITIES;
	PUT_32BIT(request, 1);

	agent_query(request, 5, &vresponse, &resplen, NULL, NULL);

	response = vresponse;
	if (resplen < 5 || response[4] != SSH1_AGENT_RSA_IDENTITIES_ANSWER) {
		sfree(response);
		return NULL;
	}

	ret = snewn(resplen-5, unsigned char);
	memcpy(ret, response+5, resplen-5);
	sfree(response);

	if (length)
		*length = resplen-5;

	return ret;
}

// from WINDOWS\WINPGNT.C (putty 0.63)
/*
 * Acquire a keylist2 from the primary Pageant; this means either
 * calling make_keylist2 (if that's us) or sending a message to the
 * primary Pageant (if it's not).
 */
static void *get_keylist2(int *length)
{
	void *ret;

	unsigned char request[5], *response;
	void *vresponse;
	int resplen;

	request[4] = SSH2_AGENTC_REQUEST_IDENTITIES;
	PUT_32BIT(request, 1);

	agent_query(request, 5, &vresponse, &resplen, NULL, NULL);

	response = vresponse;
	if (resplen < 5 || response[4] != SSH2_AGENT_IDENTITIES_ANSWER) {
		sfree(response);
		return NULL;
	}

	ret = snewn(resplen-5, unsigned char);
	memcpy(ret, response+5, resplen-5);
	sfree(response);

	if (length)
		*length = resplen-5;

	return ret;
}

// from WINDOWS\WINDOW.C (putty 0.60)
/*
 * Print a modal (Really Bad) message box and perform a fatal exit.
 */
void modalfatalbox(char *fmt, ...)
{
	va_list ap;
	char *stuff, morestuff[100];

	va_start(ap, fmt);
	stuff = dupvprintf(fmt, ap);
	va_end(ap);
	sprintf(morestuff, "%.70s Fatal Error", "TTSSH");
	MessageBox(NULL, stuff, morestuff,
	           MB_SYSTEMMODAL | MB_ICONERROR | MB_OK);
	sfree(stuff);
}
