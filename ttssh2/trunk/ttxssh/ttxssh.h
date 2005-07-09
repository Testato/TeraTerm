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

#ifndef __TTXSSH_H
#define __TTXSSH_H

#pragma warning(3 : 4035)

#ifdef INET6
#include <winsock2.h>
#include <ws2tcpip.h>
/* actual body of in6addr_any and in6addr_loopback is disappeared?? */
#undef IN6_IS_ADDR_LOOPBACK
#define IN6_IS_ADDR_LOOPBACK(a)         \
        ((*(unsigned int *)(&(a)->s6_addr[0]) == 0) &&     \
         (*(unsigned int *)(&(a)->s6_addr[4]) == 0) &&     \
         (*(unsigned int *)(&(a)->s6_addr[8]) == 0) &&     \
         (*(unsigned int *)(&(a)->s6_addr[12]) == ntohl(1)))
/* work around for MS Platform SDK Oct 2000 */
#include <malloc.h> /* prevent of conflict stdlib.h */
#endif /* INET6 */
#include <stdlib.h>
#include <crtdbg.h>


typedef struct _TInstVar FAR * PTInstVar;

#include "util.h"
#include "pkt.h"
#include "ssh.h"
#include "auth.h"
#include "crypt.h"
#include "hosts.h"
#include "fwd.h"

#include <openssl/dh.h>
#include <openssl/evp.h>
#include "buffer.h"

/* tttypes.h で定義されている EM マクロが openssl/rsa.h (OpenSSL 0.9.8)の関数プロトタイプ宣言に
 * ある引数名と重複してしまうので、ビルドエラーとなる。下記3ヘッダのinclude位置を下記に移動した。
 * (2005.7.9 yutaka)
 */
#include "teraterm.h"
#include "tttypes.h"
#include "ttplugin.h"

HANDLE hInst; /* Instance handle of TTXSSH.DLL */

#define ID_ABOUTMENU 62501
#define ID_SSHAUTH   62502
#define ID_SSHSETUPMENU 62503
#define ID_SSHUNKNOWNHOST 62504
#define ID_SSHDIFFERENTHOST 62505
#define ID_SSHAUTHSETUPMENU 62506
#define ID_SSHASYNCMESSAGEBOX 62507
#define ID_SSHFWDSETUPMENU 62508
#define ID_SSHKEYGENMENU 62509

/*
These are the fields that WOULD go in Teraterm's 'ts' structure, if
we could put them there.
*/
typedef struct _TS_SSH {
  BOOL Enabled;
  int CompressionLevel; /* 0 = NONE, else 1-9 */
  char DefaultUserName[256];

  /* this next option is a string of digits. Each digit represents a
     cipher. The first digit is the most preferred cipher, and so on.
     The digit SSH_CIPHER_NONE signifies that any ciphers after it are
     disabled. */
  char CipherOrder[16];

  char KnownHostsFiles[2048];
  int DefaultAuthMethod;
  char DefaultRhostsLocalUserName[256];
  char DefaultRhostsHostPrivateKeyFile[1024];
  char DefaultRSAPrivateKeyFile[1024];

  char DefaultForwarding[2048];
  BOOL TryDefaultAuth;

  int LogLevel;      /* 0 = NONE, 100 = Verbose */
  int WriteBufferSize;
  int LocalForwardingIdentityCheck;

  int ssh_protocol_version; // SSH version (2004.10.11 yutaka)
  int ssh_heartbeat_overtime; // SSH heartbeat(keepalive) (2004.12.11 yutaka)
  int ssh2_keyboard_interactive; // SSH2 keyboard-interactive (2005.1.23 yutaka)
} TS_SSH;

typedef struct _TInstVar {
  PTTSet ts;
  PComVar cv;

  /* shared memory for settings across instances. Basically it's
     a cache for the INI file.*/
  TS_SSH FAR * ts_SSH;

  int fatal_error;
  int showing_err;
  char FAR * err_msg;

  Tconnect Pconnect;
  Trecv Precv;
  Tsend Psend;
  TWSAAsyncSelect PWSAAsyncSelect;
  TWSAGetLastError PWSAGetLastError;

  PReadIniFile ReadIniFile;
  PWriteIniFile WriteIniFile;
  PParseParam ParseParam;

  SOCKET socket;
  HWND NotificationWindow;
  unsigned int notification_msg;
  long notification_events;
  HICON OldSmallIcon;
  HICON OldLargeIcon;

  BOOL hostdlg_activated;
  BOOL hostdlg_Enabled;

  int protocol_major;
  int protocol_minor;

  PKTState pkt_state;
  SSHState ssh_state;
  AUTHState auth_state;
  CRYPTState crypt_state;
  HOSTSState hosts_state;
  FWDState fwd_state;

/* The settings applied to the current session. The user may change
   the settings but usually we don't want that to affect the session
   in progress (race conditions). So user setup changes usually
   modify the 'settings' field below. */
  TS_SSH session_settings;

/* our copy of the global settings. This is synced up with the shared
   memory only when we do a ReadIniFile or WriteIniFile
   (i.e. the user loads or saves setup) */
  TS_SSH settings;

  // SSH2
  DH *kexdh;
  char server_version_string[128];
  char client_version_string[128];
  buffer_t *my_kex;
  buffer_t *peer_kex;
  enum kex_exchange kex_type; // KEX algorithm
  enum hostkey_type hostkey_type;
  SSHCipher ctos_cipher;
  SSHCipher stoc_cipher;
  enum hmac_type ctos_hmac;
  enum hmac_type stoc_hmac;
  int ctos_compression;
  int stoc_compression;
  int we_need;
  int key_done;
  int rekeying;
  char *session_id;
  int session_id_len;
  Newkeys ssh2_keys[MODE_MAX];
  EVP_CIPHER_CTX evpcip[MODE_MAX];
  int userauth_success;
  int shell_id;
  /*int remote_id;*/
  int session_nego_status;
  /*
  unsigned int local_window;
  unsigned int local_window_max;
  unsigned int local_consumed;
  unsigned int local_maxpacket;
  unsigned int remote_window;
  unsigned int remote_maxpacket;
  */
  int client_key_bits;
  int server_key_bits;
  int kexgex_min;
  int kexgex_bits;
  int kexgex_max;
  int ssh2_autologin;
  SSHAuthMethod ssh2_authmethod;
  char ssh2_username[MAX_PATH];
  char ssh2_password[MAX_PATH];
  char ssh2_keyfile[MAX_PATH];
  time_t ssh_heartbeat_tick;
  HANDLE ssh_heartbeat_thread;
  int keyboard_interactive_done;
  int keyboard_interactive_password_input;
  int userauth_retry_count;
  buffer_t *decomp_buffer;
} TInstVar;

#define LOG_LEVEL_FATAL      5
#define LOG_LEVEL_ERROR      10
#define LOG_LEVEL_URGENT     20
#define LOG_LEVEL_WARNING    30
#define LOG_LEVEL_VERBOSE    100

#define SSHv1(pvar) ((pvar)->protocol_major == 1)
#define SSHv2(pvar) ((pvar)->protocol_major == 2)

void notify_established_secure_connection(PTInstVar pvar);
void notify_closed_connection(PTInstVar pvar);
void notify_nonfatal_error(PTInstVar pvar, char FAR * msg);
void notify_fatal_error(PTInstVar pvar, char FAR * msg);
void notify_verbose_message(PTInstVar pvar, char FAR * msg, int level);

void get_teraterm_dir_relative_name(char FAR * buf, int bufsize, char FAR * basename);
int copy_teraterm_dir_relative_path(char FAR * dest, int destsize, char FAR * basename);
void get_file_version(char *exefile, int *major, int *minor, int *release, int *build);

#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.13  2005/07/09 05:16:06  yutakakn
 * OpenSSL 0.9.8でビルドできるようにした。
 *
 * Revision 1.12  2005/04/23 17:26:57  yutakakn
 * キー作成ダイアログの追加。
 *
 * Revision 1.11  2005/04/03 14:39:48  yutakakn
 * SSH2 channel lookup機構の追加（ポートフォワーディングのため）。
 * TTSSH 2.10で追加したlog dump機構において、DH鍵再作成時にbuffer freeで
 * アプリケーションが落ちてしまうバグを修正。
 *
 * Revision 1.10  2005/03/12 15:07:34  yutakakn
 * SSH2 keyboard-interactive認証をTISダイアログに実装した。
 *
 * Revision 1.9  2005/03/10 13:40:39  yutakakn
 * すでにログイン処理を行っている場合は、SSH2_MSG_SERVICE_REQUESTの送信は
 * しないことにする。OpenSSHでは支障ないが、Tru64 UNIXではサーバエラーとなってしまうため。
 *
 * Revision 1.8  2005/03/03 13:28:23  yutakakn
 * クライアントのSSHバージョンを ttxssh.dll から取得して、サーバへ送るようにした。
 *
 * Revision 1.7  2005/01/27 13:30:33  yutakakn
 * 公開鍵認証自動ログインをサポート。
 * /auth=publickey, /keyfile オプションを新規追加した。
 * また、空白を含む引数をサポート。
 *
 * Revision 1.6  2005/01/24 14:07:07  yutakakn
 * ・keyboard-interactive認証をサポートした。
 * 　それに伴い、teraterm.iniに "KeyboardInteractive" エントリを追加した。
 * ・バージョンダイアログに OpenSSLバージョン を追加
 *
 * Revision 1.5  2004/12/27 14:05:08  yutakakn
 * 'Auto window close'が有効の場合、切断後の接続ができない問題を修正した。
 * 　・スレッドの終了待ち合わせ処理の追加
 * 　・確保済みSSHリソースの解放
 *
 * Revision 1.4  2004/12/17 14:05:55  yutakakn
 * パケット受信時のHMACチェックを追加。
 * KEXにおけるHMACアルゴリズムチェックを追加。
 *
 * Revision 1.3  2004/12/11 07:31:00  yutakakn
 * SSH heartbeatスレッドの追加した。これにより、IPマスカレード環境において、ルータの
 * NATテーブルクリアにより、SSHコネクションが切断される現象が回避される。
 * それに合わせて、teraterm.iniのTTSSHセクションに、HeartBeat エントリを追加。
 *
 * Revision 1.2  2004/12/01 15:37:49  yutakakn
 * SSH2自動ログイン機能を追加。
 * 現状、パスワード認証のみに対応。
 * ・コマンドライン
 *   /ssh /auth=認証メソッド /user=ユーザ名 /passwd=パスワード
 *
 */
