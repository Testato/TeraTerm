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

#ifndef __SSH_H
#define __SSH_H

#include "zlib.h"
#include <openssl/evp.h>


// yutaka
#define SSH2_USE


/* Some of this code has been adapted from Ian Goldberg's Pilot SSH */

typedef enum {
    SSH_MSG_NONE, SSH_MSG_DISCONNECT, SSH_SMSG_PUBLIC_KEY,
    SSH_CMSG_SESSION_KEY, SSH_CMSG_USER, SSH_CMSG_AUTH_RHOSTS,
    SSH_CMSG_AUTH_RSA, SSH_SMSG_AUTH_RSA_CHALLENGE,
    SSH_CMSG_AUTH_RSA_RESPONSE, SSH_CMSG_AUTH_PASSWORD,
    SSH_CMSG_REQUEST_PTY, SSH_CMSG_WINDOW_SIZE, SSH_CMSG_EXEC_SHELL,
    SSH_CMSG_EXEC_CMD, SSH_SMSG_SUCCESS, SSH_SMSG_FAILURE,
    SSH_CMSG_STDIN_DATA, SSH_SMSG_STDOUT_DATA, SSH_SMSG_STDERR_DATA,
    SSH_CMSG_EOF, SSH_SMSG_EXITSTATUS,
    SSH_MSG_CHANNEL_OPEN_CONFIRMATION, SSH_MSG_CHANNEL_OPEN_FAILURE,
    SSH_MSG_CHANNEL_DATA, SSH_MSG_CHANNEL_INPUT_EOF,
    SSH_MSG_CHANNEL_OUTPUT_CLOSED, SSH_MSG_OBSOLETED0,
    SSH_SMSG_X11_OPEN, SSH_CMSG_PORT_FORWARD_REQUEST, SSH_MSG_PORT_OPEN,
    SSH_CMSG_AGENT_REQUEST_FORWARDING, SSH_SMSG_AGENT_OPEN,
    SSH_MSG_IGNORE, SSH_CMSG_EXIT_CONFIRMATION,
    SSH_CMSG_X11_REQUEST_FORWARDING, SSH_CMSG_AUTH_RHOSTS_RSA,
    SSH_MSG_DEBUG, SSH_CMSG_REQUEST_COMPRESSION,
    SSH_CMSG_MAX_PACKET_SIZE, SSH_CMSG_AUTH_TIS,
    SSH_SMSG_AUTH_TIS_CHALLENGE, SSH_CMSG_AUTH_TIS_RESPONSE,
    SSH_CMSG_AUTH_KERBEROS, SSH_SMSG_AUTH_KERBEROS_RESPONSE
} SSHMessage;

typedef enum {
    SSH_CIPHER_NONE, SSH_CIPHER_IDEA, SSH_CIPHER_DES, SSH_CIPHER_3DES,
    SSH_CIPHER_TSS, SSH_CIPHER_RC4, SSH_CIPHER_BLOWFISH,
	// for SSH2
	SSH_CIPHER_3DES_CBC, SSH_CIPHER_AES128,
} SSHCipher;

//#define SSH_CIPHER_MAX SSH_CIPHER_BLOWFISH
#define SSH_CIPHER_MAX SSH_CIPHER_AES128

typedef enum {
    SSH_AUTH_NONE, SSH_AUTH_RHOSTS, SSH_AUTH_RSA, SSH_AUTH_PASSWORD,
    SSH_AUTH_RHOSTS_RSA, SSH_AUTH_TIS, SSH_AUTH_KERBEROS,
	// for SSH2
	SSH_AUTH_DSA,
} SSHAuthMethod;

/* we don't support Kerberos at this time */
//#define SSH_AUTH_MAX SSH_AUTH_TIS
#define SSH_AUTH_MAX SSH_AUTH_DSA

typedef enum {
    SSH_GENERIC_AUTHENTICATION, SSH_TIS_AUTHENTICATION
} SSHAuthMode;

#define SSH_PROTOFLAG_SCREEN_NUMBER 1
#define SSH_PROTOFLAG_HOST_IN_FWD_OPEN 2

#define SSH_MAX_SEND_PACKET_SIZE   250000


/* SSH2 constants */

/* SSH2 messages */
#define SSH2_MSG_DISCONNECT             1
#define SSH2_MSG_IGNORE                 2
#define SSH2_MSG_UNIMPLEMENTED          3
#define SSH2_MSG_DEBUG                  4
#define SSH2_MSG_SERVICE_REQUEST        5
#define SSH2_MSG_SERVICE_ACCEPT         6

#define SSH2_MSG_KEXINIT                20
#define SSH2_MSG_NEWKEYS                21

#define SSH2_MSG_KEXDH_INIT             30
#define SSH2_MSG_KEXDH_REPLY            31

#define SSH2_MSG_KEX_DH_GEX_GROUP           31
#define SSH2_MSG_KEX_DH_GEX_INIT            32
#define SSH2_MSG_KEX_DH_GEX_REPLY           33
#define SSH2_MSG_KEX_DH_GEX_REQUEST         34

#define SSH2_MSG_USERAUTH_REQUEST            50
#define SSH2_MSG_USERAUTH_FAILURE            51
#define SSH2_MSG_USERAUTH_SUCCESS            52
#define SSH2_MSG_USERAUTH_BANNER             53

#define SSH2_MSG_USERAUTH_PK_OK              60
#define SSH2_MSG_USERAUTH_PASSWD_CHANGEREQ   60

#define SSH2_MSG_GLOBAL_REQUEST                  80
#define SSH2_MSG_REQUEST_SUCCESS                 81
#define SSH2_MSG_REQUEST_FAILURE                 82
#define SSH2_MSG_CHANNEL_OPEN                    90
#define SSH2_MSG_CHANNEL_OPEN_CONFIRMATION       91
#define SSH2_MSG_CHANNEL_OPEN_FAILURE            92
#define SSH2_MSG_CHANNEL_WINDOW_ADJUST           93
#define SSH2_MSG_CHANNEL_DATA                    94
#define SSH2_MSG_CHANNEL_EXTENDED_DATA           95
#define SSH2_MSG_CHANNEL_EOF                     96
#define SSH2_MSG_CHANNEL_CLOSE                   97
#define SSH2_MSG_CHANNEL_REQUEST                 98
#define SSH2_MSG_CHANNEL_SUCCESS                 99
#define SSH2_MSG_CHANNEL_FAILURE                 100

/* SSH2 miscellaneous constants */
#define SSH2_DISCONNECT_HOST_NOT_ALLOWED_TO_CONNECT      1
#define SSH2_DISCONNECT_PROTOCOL_ERROR                   2
#define SSH2_DISCONNECT_KEY_EXCHANGE_FAILED              3
#define SSH2_DISCONNECT_HOST_AUTHENTICATION_FAILED       4
#define SSH2_DISCONNECT_MAC_ERROR                        5
#define SSH2_DISCONNECT_COMPRESSION_ERROR                6
#define SSH2_DISCONNECT_SERVICE_NOT_AVAILABLE            7
#define SSH2_DISCONNECT_PROTOCOL_VERSION_NOT_SUPPORTED   8
#define SSH2_DISCONNECT_HOST_KEY_NOT_VERIFIABLE          9
#define SSH2_DISCONNECT_CONNECTION_LOST                  10
#define SSH2_DISCONNECT_BY_APPLICATION                   11

#define SSH2_OPEN_ADMINISTRATIVELY_PROHIBITED    1
#define SSH2_OPEN_CONNECT_FAILED                 2
#define SSH2_OPEN_UNKNOWN_CHANNEL_TYPE           3
#define SSH2_OPEN_RESOURCE_SHORTAGE              4

// �L�[�����A���S���Y��
#define KEX_DH1     "diffie-hellman-group1-sha1"
#define KEX_DH14    "diffie-hellman-group14-sha1"
#define KEX_DHGEX   "diffie-hellman-group-exchange-sha1"

enum kex_exchange {
    KEX_DH_GRP1_SHA1,
    KEX_DH_GRP14_SHA1,
    KEX_DH_GEX_SHA1,
    KEX_MAX
};

enum hostkey_type {
	KEY_RSA1,
	KEY_RSA,
	KEY_DSA,
	KEY_UNSPEC,
};

// ���L�̃C���f�b�N�X�� ssh2_macs[] �ƍ��킹�邱�ƁB
enum hmac_type {
	HMAC_SHA1,
	HMAC_MD5,
	HMAC_UNKNOWN
};

#define KEX_DEFAULT_KEX     "diffie-hellman-group-exchange-sha1,diffie-hellman-group1-sha1"

#define	KEX_DEFAULT_PK_ALG	"ssh-rsa,ssh-dss"
#define	KEX_DEFAULT_ENCRYPT \
	"aes128-cbc,3des-cbc,blowfish-cbc,cast128-cbc,arcfour," \
	"aes192-cbc,aes256-cbc,rijndael-cbc@lysator.liu.se," \
	"aes128-ctr,aes192-ctr,aes256-ctr"
#define	KEX_DEFAULT_MAC \
	"hmac-md5,hmac-sha1,hmac-ripemd160," \
	"hmac-ripemd160@openssh.com," \
	"hmac-sha1-96,hmac-md5-96"
#define	KEX_DEFAULT_COMP	"none,zlib"
#define	KEX_DEFAULT_LANG	""

/* Minimum modulus size (n) for RSA keys. */
#define SSH_RSA_MINIMUM_MODULUS_SIZE    768


enum kex_init_proposals {
	PROPOSAL_KEX_ALGS,
	PROPOSAL_SERVER_HOST_KEY_ALGS,
	PROPOSAL_ENC_ALGS_CTOS,
	PROPOSAL_ENC_ALGS_STOC,
	PROPOSAL_MAC_ALGS_CTOS,
	PROPOSAL_MAC_ALGS_STOC,
	PROPOSAL_COMP_ALGS_CTOS,
	PROPOSAL_COMP_ALGS_STOC,
	PROPOSAL_LANG_CTOS,
	PROPOSAL_LANG_STOC,
	PROPOSAL_MAX
};

struct Enc {
	u_char	*key;
	u_char	*iv;
	unsigned int key_len;
	unsigned int block_size;
};

struct Mac {
    char    *name; 
    int enabled; 
    const EVP_MD    *md;
    int mac_len; 
    u_char  *key;
    int key_len;
};

struct Comp {
	int	type;
	int	enabled;
	char	*name;
};

typedef struct {
	struct Enc	enc;
	struct Mac	mac;
	struct Comp	comp;
} Newkeys;

#define roundup(x, y)   ((((x)+((y)-1))/(y))*(y))

enum kex_modes {
	MODE_IN,
	MODE_OUT,
	MODE_MAX
};



/* The packet handler returns TRUE to keep the handler in place,
   FALSE to remove the handler. */
typedef BOOL (* SSHPacketHandler)(PTInstVar pvar);

typedef struct _SSHPacketHandlerItem SSHPacketHandlerItem;
struct _SSHPacketHandlerItem {
  SSHPacketHandler handler;
  /* Circular list of handlers for given message */
  SSHPacketHandlerItem FAR * next_for_message;
  SSHPacketHandlerItem FAR * last_for_message;
  /* Circular list of handlers in set */
  SSHPacketHandlerItem FAR * next_in_set;
  int active_for_message;
};

typedef struct {
  char FAR * hostname;

  int server_protocol_flags;
  char FAR * server_ID;

  /* This buffer is used to hold the outgoing data, and encrypted in-place
     here if necessary. */
  unsigned char FAR * outbuf;
  long outbuflen;
  /* This buffer is used by the SSH protocol processing to store uncompressed
     packet data for compression. User data is never streamed through here;
     it is compressed directly from the user's buffer. */
  unsigned char FAR * precompress_outbuf;
  long precompress_outbuflen;
  /* this is the length of the packet data, including the type header */
  long outgoing_packet_len;

  /* This buffer is used by the SSH protocol processing to store decompressed
     packet data. User data is never streamed through here; it is decompressed
     directly to the user's buffer. */
  unsigned char FAR * postdecompress_inbuf;
  long postdecompress_inbuflen;

  unsigned char FAR * payload;
  long payload_grabbed;
  long payloadlen;
  long payload_datastart;
  long payload_datalen;

  uint32 receiver_sequence_number;
  uint32 sender_sequence_number;

  z_stream compress_stream;
  z_stream decompress_stream;
  BOOL compressing;
  BOOL decompressing;
  int compression_level;

  SSHPacketHandlerItem FAR * packet_handlers[256];
  int status_flags;

  int win_cols;
  int win_rows;
} SSHState;

#define STATUS_DONT_SEND_USER_NAME            0x01
#define STATUS_EXPECTING_COMPRESSION_RESPONSE 0x02
#define STATUS_DONT_SEND_CREDENTIALS          0x04
#define STATUS_HOST_OK                        0x08
#define STATUS_INTERACTIVE                    0x10
#define STATUS_IN_PARTIAL_ID_STRING           0x20

void SSH_init(PTInstVar pvar);
void SSH_open(PTInstVar pvar);
void SSH_notify_disconnecting(PTInstVar pvar, char FAR * reason);
/* SSH_handle_server_ID returns TRUE iff a valid ID string has been
   received. If it returns FALSE, we need to keep looking for another
   ID string. */
BOOL SSH_handle_server_ID(PTInstVar pvar, char FAR * ID, int ID_len);
/* SSH_handle_packet requires NO PAYLOAD on entry.
   'len' is the size of the packet: payload + padding (+ CRC for SSHv1)
   'padding' is the size of the padding.
   'data' points to the start of the packet data (the length field)
*/
void SSH_handle_packet(PTInstVar pvar, char FAR * data, int len, int padding);
void SSH_notify_win_size(PTInstVar pvar, int cols, int rows);
void SSH_notify_user_name(PTInstVar pvar);
void SSH_notify_cred(PTInstVar pvar);
void SSH_notify_host_OK(PTInstVar pvar);
void SSH_send(PTInstVar pvar, unsigned char const FAR * buf, int buflen);
/* SSH_extract_payload returns number of bytes extracted */
int SSH_extract_payload(PTInstVar pvar, unsigned char FAR * dest, int len);
void SSH_end(PTInstVar pvar);

void SSH_get_server_ID_info(PTInstVar pvar, char FAR * dest, int len);
void SSH_get_protocol_version_info(PTInstVar pvar, char FAR * dest, int len);
void SSH_get_compression_info(PTInstVar pvar, char FAR * dest, int len);

/* len must be <= SSH_MAX_SEND_PACKET_SIZE */
void SSH_channel_send(PTInstVar pvar, uint32 remote_channel_num,
                      unsigned char FAR * buf, int len);
void SSH_fail_channel_open(PTInstVar pvar, uint32 remote_channel_num);
void SSH_confirm_channel_open(PTInstVar pvar, uint32 remote_channel_num, uint32 local_channel_num);
void SSH_channel_output_eof(PTInstVar pvar, uint32 remote_channel_num);
void SSH_channel_input_eof(PTInstVar pvar, uint32 remote_channel_num);
void SSH_request_forwarding(PTInstVar pvar, int from_server_port,
  char FAR * to_local_host, int to_local_port);
void SSH_request_X11_forwarding(PTInstVar pvar,
  char FAR * auth_protocol, unsigned char FAR * auth_data, int auth_data_len, int screen_num);
void SSH_open_channel(PTInstVar pvar, uint32 local_channel_num,
  char FAR * to_remote_host, int to_remote_port, char FAR * originator);

/* auxiliary SSH2 interfaces for pkt.c */
int SSH_get_min_packet_size(PTInstVar pvar);
/* data is guaranteed to be at least SSH_get_min_packet_size bytes long
   at least 5 bytes must be decrypted */
void SSH_predecrpyt_packet(PTInstVar pvar, char FAR * data);
int SSH_get_clear_MAC_size(PTInstVar pvar);

#define SSH_is_any_payload(pvar) ((pvar)->ssh_state.payload_datalen > 0)
#define SSH_get_host_name(pvar) ((pvar)->ssh_state.hostname)
#define SSH_get_compression_level(pvar) ((pvar)->ssh_state.compressing ? (pvar)->ts_SSH_CompressionLevel : 0)

void SSH2_send_kexinit(PTInstVar pvar);
BOOL do_SSH2_userauth(PTInstVar pvar);
void debug_print(int no, char *msg, int len);
void ssh_heartbeat_lock_initialize(void);
void ssh_heartbeat_lock(void);
void ssh_heartbeat_unlock(void);

#endif
