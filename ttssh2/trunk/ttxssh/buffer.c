//
// buffer.c
//

#include <winsock2.h>
#include <malloc.h>
#include "buffer.h"
#include "ttxssh.h"
#include "util.h"
#include <openssl/bn.h>

void buffer_clear(buffer_t *buf)
{
	buf->offset = 0;
	buf->len = 0;
}

buffer_t *buffer_init(void)
{
	void *ptr;
	buffer_t *buf;
	int size = 4096;

	buf = malloc(sizeof(buffer_t));
	ptr = malloc(size);
	if (buf && ptr) {
		memset(buf, 0, sizeof(buffer_t));
		memset(ptr, 0, size);
		buf->buf = ptr;
		buf->maxlen = size;
		buf->len = 0;
		buf->offset = 0;

	} else {
		ptr = NULL; *(char *)ptr = 0;
	}

	return (buf);
}

void buffer_free(buffer_t * buf)
{
	if (buf != NULL) {
		free(buf->buf);
		free(buf);
	}
}

int buffer_append(buffer_t * buf, char *ptr, int size)
{
	int n;
	int ret = -1;

	n = buf->offset + size;
	if (n < buf->maxlen) {
		memcpy(buf->buf + buf->offset, ptr, size);
		buf->offset += size;
		buf->len = buf->offset;
		ret = 0;

	} else {
		char *p = NULL;
		// TODO: realloc
		*p = 0;

	}

	return (ret);
}

int buffer_append_length(buffer_t * msg, char *ptr, int size)
{
	char buf[4];
	int val;
	int ret = -1;

	val = htonl(size);
	memcpy(buf, &val, sizeof(val));
	ret = buffer_append(msg, buf, sizeof(buf));
	if (ptr != NULL) {
		ret = buffer_append(msg, ptr, size);
	}

	return (ret);
}

// getting string buffer.
// NOTE: You should free the return pointer if it's unused.
// (2005.6.26 yutaka)
char *buffer_get_string(char **data_ptr, int *buflen_ptr)
{
	char *data = *data_ptr;
	char *ptr;
	int buflen;

	buflen = get_uint32_MSBfirst(data);
	data += 4;
	if (buflen <= 0)
		return NULL;

	ptr = malloc(buflen + 1);
	if (ptr == NULL) {
		if (buflen_ptr != NULL)
			*buflen_ptr = 0;
		return NULL;
	}
	memcpy(ptr, data, buflen);
	ptr[buflen] = '\0'; // null-terminate
	data += buflen;

	*data_ptr = data;
	if (buflen_ptr != NULL)
		*buflen_ptr = buflen;

	return(ptr);
}

void buffer_put_string(buffer_t *msg, char *ptr, int size)
{
	char buf[4];
	int val;
	int ret = -1;

	// 「サイズ＋文字列」で書き込む。サイズは4byteのbig-endian。
	val = htonl(size);
	memcpy(buf, &val, sizeof(val));
	ret = buffer_append(msg, buf, sizeof(buf));
	if (ptr != NULL) {
		ret = buffer_append(msg, ptr, size);
	}
}

void buffer_put_char(buffer_t *msg, int value)
{
	char ch = value;

	buffer_append(msg, &ch, 1);
}

void buffer_put_padding(buffer_t *msg, int size)
{
	char ch = ' ';
	int i;

	for (i = 0 ; i < size ; i++) {
		buffer_append(msg, &ch, 1);
	}
}

void buffer_put_int(buffer_t *msg, int value)
{
	char buf[4];

	set_uint32_MSBfirst(buf, value);
	buffer_append(msg, buf, sizeof(buf));
}

int buffer_len(buffer_t *msg)
{
	return (msg->len);
}

char *buffer_ptr(buffer_t *msg)
{
	return (msg->buf);
}

char *buffer_tail_ptr(buffer_t *msg)
{
	return (char *)(msg->buf + msg->offset);
}

int buffer_overflow_verify(buffer_t *msg, int len)
{
	if (msg->offset + len > msg->maxlen) {
		return -1;  // error
	}
	return 0; // no problem
}

// for SSH1
void buffer_put_bignum(buffer_t *buffer, BIGNUM *value)
{
    unsigned int bits, bin_size;
    unsigned char *buf;
    int oi;
    char msg[2];
	
    bits = BN_num_bits(value);
	bin_size = (bits + 7) / 8;
    buf = malloc(bin_size);
	if (buf == NULL) {
		*buf = 0;
		goto error;
	}

    buf[0] = '\0';
    /* Get the value of in binary */
    oi = BN_bn2bin(value, buf);
	if (oi != bin_size) {
		goto error;
	}

    /* Store the number of bits in the buffer in two bytes, msb first. */
	set_ushort16_MSBfirst(msg, bits);
	buffer_append(buffer, msg, 2);

    /* Store the binary data. */
    buffer_append(buffer, (char *)buf, oi);

error:
    free(buf);
}

// for SSH2
void buffer_put_bignum2(buffer_t *msg, BIGNUM *value)
{
    unsigned int bytes;
    unsigned char *buf;
    int oi;
    unsigned int hasnohigh = 0;
	
    bytes = BN_num_bytes(value) + 1; /* extra padding byte */
    buf = malloc(bytes);
	if (buf == NULL) {
		*buf = 0;
		goto error;
	}

    buf[0] = '\0';
    /* Get the value of in binary */
    oi = BN_bn2bin(value, buf+1);
    hasnohigh = (buf[1] & 0x80) ? 0 : 1;
    buffer_put_string(msg, buf+hasnohigh, bytes-hasnohigh);
    //memset(buf, 0, bytes);

error:
    free(buf);
}

void buffer_get_bignum2(char **data, BIGNUM *value)
{
	char *buf = *data;
	int len;

	len = get_uint32_MSBfirst(buf);
	buf += 4;
	BN_bin2bn(buf, len, value);
	buf += len;

	*data = buf;
}

void buffer_dump(FILE *fp, buffer_t *buf)
{
	int i;
	char *ch = buffer_ptr(buf);

	for (i = 0 ; i < buffer_len(buf) ; i++) {
		fprintf(fp, "%02x", ch[i] & 0xff);
		if (i % 16 == 15)
			fprintf(fp, "\n");
		else if (i % 2 == 1)
			fprintf(fp, " ");
	}
	fprintf(fp, "\n");
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2005/06/26 14:26:24  yutakakn
 * update: SSH2 port-forwarding (remote to local)
 *
 * Revision 1.3  2005/04/23 17:26:57  yutakakn
 * キー作成ダイアログの追加。
 *
 * Revision 1.2  2004/12/19 15:37:37  yutakakn
 * CVS LogIDの追加
 *
 */
