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

void buffer_put_string(buffer_t *msg, char *ptr, int size)
{
	char buf[4];
	int val;
	int ret = -1;

	// �u�T�C�Y�{������v�ŏ������ށB�T�C�Y��4byte��big-endian�B
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

