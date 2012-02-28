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

int key_verify(Key *key,
               unsigned char *signature, unsigned int signaturelen,
               unsigned char *data, unsigned int datalen);
RSA *duplicate_RSA(RSA *src);
DSA *duplicate_DSA(DSA *src);

char *key_fingerprint_raw(Key *k, enum fp_type dgst_type, int *dgst_raw_length);
char *key_fingerprint(Key *key, enum fp_rep dgst_rep);

const char *key_type(const Key *k);
char *get_sshname_from_key(Key *key);
enum hostkey_type get_keytype_from_name(char *name);
char *curve_keytype_to_name(ssh_keytype type);

void key_free(Key *key);
int key_to_blob(Key *key, char **blobp, int *lenp);
Key *key_from_blob(char *data, int blen);
BOOL get_SSH2_publickey_blob(PTInstVar pvar, buffer_t **blobptr, int *bloblen);
BOOL generate_SSH2_keysign(Key *keypair, char **sigptr, int *siglen, char *data, int datalen);

int kextype_to_cipher_nid(kex_algorithm type);
int keytype_to_hash_nid(ssh_keytype type);
int keytype_to_cipher_nid(ssh_keytype type);
ssh_keytype nid_to_keytype(int nid);
