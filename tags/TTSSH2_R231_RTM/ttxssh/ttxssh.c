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

/* Teraterm extension mechanism
   Robert O'Callahan (roc+tt@cs.cmu.edu)
   
   Teraterm by Takashi Teranishi (teranishi@rikaxp.riken.go.jp)
*/

#include "ttxssh.h"
#include "fwdui.h"
#include "util.h"
#include "ssh.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#include "resource.h"
#include <commctrl.h>
#include <commdlg.h>
#ifdef INET6
#include <winsock2.h>
static char FAR *ProtocolFamilyList[] = { "UNSPEC", "IPv6", "IPv4", NULL };
#else
#include <winsock.h>
#endif							/* INET6 */

#include <Lmcons.h>

// include OpenSSL header file
#include <openssl/opensslv.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/dsa.h>
#include <openssl/bn.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rc4.h>
#include <openssl/md5.h>

// include ZLib header file
#include <zlib.h>

#include "buffer.h"
#include "cipher.h"

#define MATCH_STR(s, o) _strnicmp((s), (o), NUM_ELEM(o) - 1)

/* This extension implements SSH, so we choose a load order in the
   "protocols" range. */
#define ORDER 2500

#ifdef TERATERM32
static HICON SecureIcon = NULL;
#endif

static TInstVar FAR *pvar;

#ifdef TERATERM32
  /* WIN32 allows multiple instances of a DLL */
static TInstVar InstVar;
#define GET_VAR()
#else
  /* WIN16 does not allow multiple instances of a DLL */

  /* maximum number of Tera Term instances */
#define MAXNUMINST 32
  /* list of task handles for Tera Term instances */
static HANDLE FAR TaskList[MAXNUMINST];
  /* variable sets for instances */
static TInstVar FAR *FAR InstVar[MAXNUMINST];

/* Here's how the TS settings work.
   Whenever the TS settings are read or written to the INI file, then
   the shared memory containing those settings is updated.
   When Teraterm starts, the shared memory is read to initialize the TS
   settings. */

  /* TS settings shared across instances */
static TS_SSH ts_SSH_settings;


extern void SSH2_update_cipher_myproposal(PTInstVar pvar);


static BOOL NewVar()
{
	int i = 0;
	HANDLE Task = GetCurrentTask();

	if (TaskList[0] == NULL)

		if (Task == NULL)
			return FALSE;
	while ((i < MAXNUMINST) && (TaskList[i] != NULL))
		i++;
	if (i >= MAXNUMINST)
		return FALSE;
	pvar = (TInstVar FAR *) malloc(sizeof(TInstVar));
	InstVar[i] = pvar;
	TaskList[i] = Task;
	return TRUE;
}

void DelVar()
{
	int i = 0;
	HANDLE Task = GetCurrentTask();

	if (Task == NULL)
		return;
	while ((i < MAXNUMINST) && (TaskList[i] != Task))
		i++;
	if (i >= MAXNUMINST)
		return;
	free(TaskList[i]);
	TaskList[i] = NULL;
}

BOOL GetVar()
{
	int i = 0;
	HANDLE Task = GetCurrentTask();

	if (Task == NULL)
		return FALSE;
	while ((i < MAXNUMINST) && (TaskList[i] != Task))
		i++;
	if (i >= MAXNUMINST)
		return FALSE;
	pvar = InstVar[i];
	return TRUE;
}

#define GET_VAR() if (!GetVar()) return
#endif

/*
This code makes lots of assumptions about the order in which Teraterm
does things, and how. A key assumption is that the Notification window
passed into WSAAsyncSelect is the main terminal window. We also assume
that the socket used in the first WSAconnect is the main session socket.
*/

static void init_TTSSH(PTInstVar pvar)
{
	pvar->socket = INVALID_SOCKET;
	pvar->OldLargeIcon = NULL;
	pvar->OldSmallIcon = NULL;
	pvar->NotificationWindow = NULL;
	pvar->hostdlg_activated = FALSE;
	pvar->socket = INVALID_SOCKET;
	pvar->NotificationWindow = NULL;
	pvar->protocol_major = 0;
	pvar->protocol_minor = 0;

	PKT_init(pvar);
	SSH_init(pvar);
	CRYPT_init(pvar);
	AUTH_init(pvar);
	HOSTS_init(pvar);
	FWD_init(pvar);
	FWDUI_init(pvar);

	ssh_heartbeat_lock_initialize();
}

static void uninit_TTSSH(PTInstVar pvar)
{
	halt_ssh_heartbeat_thread(pvar);

	ssh2_channel_free();

	SSH_end(pvar);
	PKT_end(pvar);
	AUTH_end(pvar);
	CRYPT_end(pvar);
	HOSTS_end(pvar);
	FWD_end(pvar);
	FWDUI_end(pvar);

	if (pvar->OldLargeIcon != NULL) {
		PostMessage(pvar->NotificationWindow, WM_SETICON, ICON_BIG,
					(LPARAM) pvar->OldLargeIcon);
		pvar->OldLargeIcon = NULL;
	}
	if (pvar->OldSmallIcon != NULL) {
		PostMessage(pvar->NotificationWindow, WM_SETICON, ICON_SMALL,
					(LPARAM) pvar->OldSmallIcon);
		pvar->OldSmallIcon = NULL;
	}

	ssh_heartbeat_lock_finalize();
}

static void PASCAL FAR TTXInit(PTTSet ts, PComVar cv)
{
#ifndef TERATERM32
	if (!NewVar())
		return;					/* should be called first */
	pvar->ts_SSH = &ts_SSH_settings;
#endif
	pvar->settings = *pvar->ts_SSH;
	pvar->ts = ts;
	pvar->cv = cv;
	pvar->fatal_error = FALSE;
	pvar->showing_err = FALSE;
	pvar->err_msg = NULL;

	init_TTSSH(pvar);
}

static void normalize_cipher_order(char FAR * buf)
{
	char ciphers_listed[SSH_CIPHER_MAX + 1];
	char ciphers_allowed[SSH_CIPHER_MAX + 1];
	int i, j;

	/* SSH_CIPHER_NONE means that all ciphers below that one are disabled.
	   We *never* allow no encryption. */
#if 0
	static char default_ciphers[] = {
		SSH_CIPHER_3DES,
		SSH_CIPHER_NONE,
		SSH_CIPHER_DES, SSH_CIPHER_BLOWFISH
	};
#else
	// for SSH2(yutaka)
	static char default_ciphers[] = {
		SSH_CIPHER_AES128,
		SSH_CIPHER_3DES_CBC,
		SSH_CIPHER_3DES,
		SSH_CIPHER_NONE,
		SSH_CIPHER_DES, SSH_CIPHER_BLOWFISH
	};
#endif

	memset(ciphers_listed, 0, sizeof(ciphers_listed));

	memset(ciphers_allowed, 0, sizeof(ciphers_allowed));
	for (i = 0; i < NUM_ELEM(default_ciphers); i++) {
		ciphers_allowed[default_ciphers[i]] = 1;
	}

	for (i = 0; buf[i] != 0; i++) {
		int cipher_num = buf[i] - '0';

		if (cipher_num < 0 || cipher_num > SSH_CIPHER_MAX
			|| !ciphers_allowed[cipher_num]
			|| ciphers_listed[cipher_num]) {
			memmove(buf + i, buf + i + 1, strlen(buf + i + 1) + 1);
			i--;
		} else {
			ciphers_listed[cipher_num] = 1;
		}
	}

	for (j = 0; j < NUM_ELEM(default_ciphers); j++) {
		int cipher_num = default_ciphers[j];

		if (!ciphers_listed[cipher_num]) {
			buf[i] = cipher_num + '0';
			i++;
		}
	}

	buf[i] = 0;
}

/* Remove local settings from the shared memory block. */
static void clear_local_settings(PTInstVar pvar)
{
	pvar->ts_SSH->TryDefaultAuth = FALSE;
}

static BOOL read_BOOL_option(PCHAR fileName, char FAR * keyName, BOOL def)
{
	char buf[1024];

	buf[0] = 0;
	GetPrivateProfileString("TTSSH", keyName, "", buf, sizeof(buf),
							fileName);
	if (buf[0] == 0) {
		return def;
	} else {
		return atoi(buf) != 0 ||
			_stricmp(buf, "yes") == 0 || _stricmp(buf, "y") == 0;
	}
}

static void read_string_option(PCHAR fileName, char FAR * keyName,
							   char FAR * def, char FAR * buf, int bufSize)
{

	buf[0] = 0;
	GetPrivateProfileString("TTSSH", keyName, def, buf, bufSize, fileName);
}

static void read_ssh_options(PTInstVar pvar, PCHAR fileName)
{
	char buf[1024];
	TS_SSH FAR *settings = pvar->ts_SSH;

#define READ_STD_STRING_OPTION(name) \
  read_string_option(fileName, #name, "", settings->name, sizeof(settings->name))

	settings->Enabled = read_BOOL_option(fileName, "Enabled", FALSE);

	buf[0] = 0;
	GetPrivateProfileString("TTSSH", "Compression", "", buf, sizeof(buf),
							fileName);
	settings->CompressionLevel = atoi(buf);
	if (settings->CompressionLevel < 0 || settings->CompressionLevel > 9) {
		settings->CompressionLevel = 0;
	}

	READ_STD_STRING_OPTION(DefaultUserName);
	READ_STD_STRING_OPTION(DefaultForwarding);
	READ_STD_STRING_OPTION(DefaultRhostsLocalUserName);
	READ_STD_STRING_OPTION(DefaultRhostsHostPrivateKeyFile);
	READ_STD_STRING_OPTION(DefaultRSAPrivateKeyFile);

	READ_STD_STRING_OPTION(CipherOrder);
	normalize_cipher_order(settings->CipherOrder);

	read_string_option(fileName, "KnownHostsFiles", "ssh_known_hosts",
					   settings->KnownHostsFiles,
					   sizeof(settings->KnownHostsFiles));

	buf[0] = 0;
	GetPrivateProfileString("TTSSH", "DefaultAuthMethod", "", buf,
							sizeof(buf), fileName);
	settings->DefaultAuthMethod = atoi(buf);
	if (settings->DefaultAuthMethod != SSH_AUTH_PASSWORD
		&& settings->DefaultAuthMethod != SSH_AUTH_RSA
		&& settings->DefaultAuthMethod != SSH_AUTH_TIS  // add (2005.3.12 yutaka)
		&& settings->DefaultAuthMethod != SSH_AUTH_RHOSTS) {
		/* this default can never be SSH_AUTH_RHOSTS_RSA because that is not a
		   selection in the dialog box; SSH_AUTH_RHOSTS_RSA is automatically chosen
		   when the dialog box has rhosts selected and an host private key file
		   is supplied. */
		settings->DefaultAuthMethod = SSH_AUTH_PASSWORD;
	}

	buf[0] = 0;
	GetPrivateProfileString("TTSSH", "LogLevel", "", buf, sizeof(buf),
							fileName);
	settings->LogLevel = atoi(buf);

	buf[0] = 0;
	GetPrivateProfileString("TTSSH", "WriteBufferSize", "", buf,
							sizeof(buf), fileName);
	settings->WriteBufferSize = atoi(buf);
	if (settings->WriteBufferSize <= 0) {
		settings->WriteBufferSize = 2 * 1024 * 1024;
	}

	settings->LocalForwardingIdentityCheck =
		read_BOOL_option(fileName, "LocalForwardingIdentityCheck", TRUE);

	// SSH protocol version (2004.10.11 yutaka)
	// default is SSH2 (2004.11.30 yutaka)
	settings->ssh_protocol_version = GetPrivateProfileInt("TTSSH", "ProtocolVersion", 2, fileName);

	// SSH heartbeat time(second) (2004.12.11 yutaka)
	settings->ssh_heartbeat_overtime = GetPrivateProfileInt("TTSSH", "HeartBeat", 60, fileName);

	// SSH2 keyboard-interactive (2005.1.23 yutaka)
	// デフォルトでは無効とする。OpenSSH 4.0ではkeyboard-interactiveメソッドが定義されていない場合に、
	// 当該メソッドを使うとコネクションが切られてしまう。(2005.3.12 yutaka)
	settings->ssh2_keyboard_interactive = GetPrivateProfileInt("TTSSH", "KeyboardInteractive", 0, fileName);

	// パスワード認証および公開鍵認証に使うパスワードをメモリ上に保持しておくかどうかを
	// 表す。(2006.8.5 yutaka)
	settings->remember_password = GetPrivateProfileInt("TTSSH", "RememberPassword", 1, fileName);

	clear_local_settings(pvar);
}

static void write_ssh_options(PTInstVar pvar, PCHAR fileName,
							  TS_SSH FAR * settings)
{
	char buf[1024];

	WritePrivateProfileString("TTSSH", "Enabled",
							  settings->Enabled ? "1" : "0", fileName);

	_itoa(settings->CompressionLevel, buf, 10);
	WritePrivateProfileString("TTSSH", "Compression", buf, fileName);

	WritePrivateProfileString("TTSSH", "DefaultUserName",
							  settings->DefaultUserName, fileName);

	WritePrivateProfileString("TTSSH", "DefaultForwarding",
							  settings->DefaultForwarding, fileName);

	WritePrivateProfileString("TTSSH", "CipherOrder",
							  settings->CipherOrder, fileName);

	WritePrivateProfileString("TTSSH", "KnownHostsFiles",
							  settings->KnownHostsFiles, fileName);

	WritePrivateProfileString("TTSSH", "DefaultRhostsLocalUserName",
							  settings->DefaultRhostsLocalUserName,
							  fileName);

	WritePrivateProfileString("TTSSH", "DefaultRhostsHostPrivateKeyFile",
							  settings->DefaultRhostsHostPrivateKeyFile,
							  fileName);

	WritePrivateProfileString("TTSSH", "DefaultRSAPrivateKeyFile",
							  settings->DefaultRSAPrivateKeyFile,
							  fileName);

	_itoa(settings->DefaultAuthMethod, buf, 10);
	WritePrivateProfileString("TTSSH", "DefaultAuthMethod", buf, fileName);

	_itoa(settings->LogLevel, buf, 10);
	WritePrivateProfileString("TTSSH", "LogLevel", buf, fileName);

	_itoa(settings->WriteBufferSize, buf, 10);
	WritePrivateProfileString("TTSSH", "WriteBufferSize", buf, fileName);

	WritePrivateProfileString("TTSSH", "LocalForwardingIdentityCheck",
							  settings->
							  LocalForwardingIdentityCheck ? "1" : "0",
							  fileName);

	// SSH protocol version (2004.10.11 yutaka)
	WritePrivateProfileString("TTSSH", "ProtocolVersion",
		settings->ssh_protocol_version==2 ? "2" : "1",
		fileName);

	// SSH heartbeat time(second) (2004.12.11 yutaka)
	_snprintf(buf, sizeof(buf), "%d", settings->ssh_heartbeat_overtime);
	WritePrivateProfileString("TTSSH", "HeartBeat", buf, fileName);

	// SSH2 keyboard-interactive (2005.1.23 yutaka)
	WritePrivateProfileString("TTSSH", "KeyboardInteractive", 
		settings->ssh2_keyboard_interactive ? "1" : "0", 
		fileName);

	// Remember password (2006.8.5 yutaka)
	WritePrivateProfileString("TTSSH", "RememberPassword", 
		settings->remember_password ? "1" : "0", 
		fileName);
}


/* find free port in all protocol family */
static unsigned short find_local_port(PTInstVar pvar)
{
	int tries;
#ifdef INET6
	SOCKET connecter;
	struct addrinfo hints;
	struct addrinfo FAR *res;
	struct addrinfo FAR *res0;
	unsigned short port;
	char pname[NI_MAXHOST];
#endif							/* INET6 */

	if (pvar->session_settings.DefaultAuthMethod != SSH_AUTH_RHOSTS) {
		return 0;
	}

	/* The random numbers here are only used to try to get fresh
	   ports across runs (dangling ports can cause bind errors
	   if we're unlucky). They do not need to be (and are not)
	   cryptographically strong.
	 */
	srand((unsigned) GetTickCount());

#ifdef INET6
	for (tries = 20; tries > 0; tries--) {
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = pvar->ts->ProtocolFamily;
		hints.ai_flags = AI_PASSIVE;
		hints.ai_socktype = SOCK_STREAM;
		port = (unsigned) rand() % 512 + 512;
		_snprintf(pname, sizeof(pname), "%d", (int) port);
		if (getaddrinfo(NULL, pname, &hints, &res0)) {
			return 0;
			/* NOT REACHED */
		}

		for (res = res0; res; res = res->ai_next) {
			if (res->ai_family == AF_INET || res->ai_family == AF_INET6)
				continue;

			connecter =
				socket(res->ai_family, res->ai_socktype, res->ai_protocol);
			if (connecter == INVALID_SOCKET) {
				freeaddrinfo(res0);
				return 0;
			}

			if (bind(connecter, res->ai_addr, res->ai_addrlen) !=
				SOCKET_ERROR) {
				return port;
				freeaddrinfo(res0);
				closesocket(connecter);
			} else if (WSAGetLastError() != WSAEADDRINUSE) {
				closesocket(connecter);
				freeaddrinfo(res0);
				return 0;
			}

			closesocket(connecter);
		}
		freeaddrinfo(res0);
	}

	return 0;
#else
	for (tries = 20; tries > 0; tries--) {
		SOCKET connecter = socket(AF_INET, SOCK_STREAM, 0);
		struct sockaddr_in connecter_addr;

		connecter_addr.sin_family = AF_INET;
		connecter_addr.sin_port = (unsigned) rand() % 512 + 512;
		connecter_addr.sin_addr.s_addr = htonl(INADDR_ANY);

		if (connecter == INVALID_SOCKET) {
			return 0;
		}

		if (bind
			(connecter, (struct sockaddr FAR *) &connecter_addr,
			 sizeof(connecter_addr)) != SOCKET_ERROR) {
			closesocket(connecter);
			return connecter_addr.sin_port;
		} else if (WSAGetLastError() != WSAEADDRINUSE) {
			closesocket(connecter);
			return 0;
		}

		closesocket(connecter);
	}

	return 0;
#endif							/* INET6 */
}

static int PASCAL FAR TTXconnect(SOCKET s,
								 const struct sockaddr FAR * name,
								 int namelen)
{
	GET_VAR();

#ifdef INET6
	if (pvar->socket == INVALID_SOCKET) {
		struct sockaddr_storage ss;
		int len;

		pvar->socket = s;

		memset(&ss, 0, sizeof(ss));
		switch (pvar->ts->ProtocolFamily) {
		case AF_INET:
			len = sizeof(struct sockaddr_in);
			((struct sockaddr_in FAR *) &ss)->sin_family = AF_INET;
			((struct sockaddr_in FAR *) &ss)->sin_addr.s_addr = INADDR_ANY;
			((struct sockaddr_in FAR *) &ss)->sin_port =
				htons(find_local_port(pvar));
			break;
		case AF_INET6:
			len = sizeof(struct sockaddr_in6);
			((struct sockaddr_in6 FAR *) &ss)->sin6_family = AF_INET6;
#if 0							/* symbol "in6addr_any" is not included in wsock32.lib */
			/* if wsock32.lib will be linked, we can't refer "in6addr_any" */
			((struct sockaddr_in6 FAR *) &ss)->sin6_addr = in6addr_any;
#eles
			memset(&((struct sockaddr_in6 FAR *) &ss)->sin6_addr, 0,
				   sizeof(struct in_addr6));
#endif							/* 0 */
			((struct sockaddr_in6 FAR *) &ss)->sin6_port =
				htons(find_local_port(pvar));
			break;
		default:
			/* NOT REACHED */
			break;
		}

		bind(s, (struct sockaddr FAR *) &ss, len);
	}
#else
	if (pvar->socket == INVALID_SOCKET) {
		struct sockaddr_in addr;

		pvar->socket = s;

		addr.sin_family = AF_INET;
		addr.sin_port = htons(find_local_port(pvar));
		addr.sin_addr.s_addr = INADDR_ANY;
		memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

		bind(s, (struct sockaddr FAR *) &addr, sizeof(addr));
	}
#endif							/* INET6 */

	return (pvar->Pconnect) (s, name, namelen);
}

static int PASCAL FAR TTXWSAAsyncSelect(SOCKET s, HWND hWnd, u_int wMsg,
										long lEvent)
{
	GET_VAR();

	if (s == pvar->socket) {
		pvar->notification_events = lEvent;
		pvar->notification_msg = wMsg;

		if (pvar->NotificationWindow == NULL) {
			pvar->NotificationWindow = hWnd;
			AUTH_advance_to_next_cred(pvar);
		}
	}

	return (pvar->PWSAAsyncSelect) (s, hWnd, wMsg, lEvent);
}

static int PASCAL FAR TTXrecv(SOCKET s, char FAR * buf, int len, int flags)
{
	GET_VAR();

	if (s == pvar->socket) {
		int ret;

		ssh_heartbeat_lock();
		ret = PKT_recv(pvar, buf, len);
		ssh_heartbeat_unlock();
		return (ret);

	} else {
		return (pvar->Precv) (s, buf, len, flags);
	}
}

static int PASCAL FAR TTXsend(SOCKET s, char const FAR * buf, int len,
							  int flags)
{
	GET_VAR();

	if (s == pvar->socket) {
		ssh_heartbeat_lock();
		SSH_send(pvar, buf, len);
		ssh_heartbeat_unlock();
		return len;
	} else {
		return (pvar->Psend) (s, buf, len, flags);
	}
}

void notify_established_secure_connection(PTInstVar pvar)
{
#ifdef TERATERM32
	if (SecureIcon == NULL) {
		SecureIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SECURETT));
	}

	if (SecureIcon != NULL) {
		pvar->OldSmallIcon =
			(HICON) SendMessage(pvar->NotificationWindow, WM_GETICON,
								ICON_SMALL, 0);
		pvar->OldLargeIcon =
			(HICON) SendMessage(pvar->NotificationWindow, WM_GETICON,
								ICON_BIG, 0);
		PostMessage(pvar->NotificationWindow, WM_SETICON, ICON_BIG,
					(LPARAM) SecureIcon);
		PostMessage(pvar->NotificationWindow, WM_SETICON, ICON_SMALL,
					(LPARAM) SecureIcon);
	}
#endif

	notify_verbose_message(pvar, "Entering secure mode",
						   LOG_LEVEL_VERBOSE);
}

void notify_closed_connection(PTInstVar pvar)
{
	SSH_notify_disconnecting(pvar, NULL);
	AUTH_notify_disconnecting(pvar);
	HOSTS_notify_disconnecting(pvar);

	PostMessage(pvar->NotificationWindow, WM_USER_COMMNOTIFY,
				pvar->socket, MAKELPARAM(FD_CLOSE, 0));

}

static void add_err_msg(PTInstVar pvar, char FAR * msg)
{
	if (pvar->err_msg != NULL) {
		char FAR *buf =
			(char FAR *) malloc(strlen(pvar->err_msg) + 3 + strlen(msg));

		strcpy(buf, pvar->err_msg);
		strcat(buf, "\n\n");
		strcat(buf, msg);
		free(pvar->err_msg);
		pvar->err_msg = buf;
	} else {
		pvar->err_msg = _strdup(msg);
	}
}

void notify_nonfatal_error(PTInstVar pvar, char FAR * msg)
{
	if (!pvar->showing_err) {
		// 未接続の状態では通知先ウィンドウがないので、デスクトップをオーナーとして
		// メッセージボックスを出現させる。(2006.6.11 yutaka)
		if (pvar->NotificationWindow == NULL) {
			MessageBox(NULL, msg, "Tera Term: not fatal error", MB_OK|MB_ICONINFORMATION);
			msg[0] = '\0';

		} else {
			PostMessage(pvar->NotificationWindow, WM_COMMAND,
						ID_SSHASYNCMESSAGEBOX, 0);
		}
	}
	if (msg[0] != 0) {
		notify_verbose_message(pvar, msg, LOG_LEVEL_ERROR);
		add_err_msg(pvar, msg);
	}
}

void notify_fatal_error(PTInstVar pvar, char FAR * msg)
{
	if (msg[0] != 0) {
		notify_verbose_message(pvar, msg, LOG_LEVEL_FATAL);
		add_err_msg(pvar, msg);
	}

	if (!pvar->fatal_error) {
		pvar->fatal_error = TRUE;

		SSH_notify_disconnecting(pvar, msg);
		AUTH_notify_disconnecting(pvar);
		HOSTS_notify_disconnecting(pvar);

		PostMessage(pvar->NotificationWindow, WM_USER_COMMNOTIFY,
					pvar->socket, MAKELPARAM(FD_CLOSE,
											 (pvar->PWSAGetLastError) ()));
	}
}

void notify_verbose_message(PTInstVar pvar, char FAR * msg, int level)
{
	if (level <= pvar->session_settings.LogLevel) {
		char buf[1024];
		int file;

		get_teraterm_dir_relative_name(buf, NUM_ELEM(buf), "TTSSH.LOG");
		file = _open(buf, _O_RDWR | _O_APPEND | _O_CREAT | _O_TEXT,
					 _S_IREAD | _S_IWRITE);

		if (file >= 0) {
			_write(file, msg, strlen(msg));
			_write(file, "\n", 1);
			_close(file);
		}
	}
}

static void PASCAL FAR TTXOpenTCP(TTXSockHooks FAR * hooks)
{
	GET_VAR();

	if (pvar->settings.Enabled) {
		char buf[1024] = "\n---------------------------------------------------------------------\nInitiating SSH session at ";
		struct tm FAR *newtime;
		time_t long_time;

		pvar->session_settings = pvar->settings;

		time(&long_time);
		newtime = localtime(&long_time);
		strcat(buf, asctime(newtime));
		buf[strlen(buf) - 1] = 0;
		notify_verbose_message(pvar, buf, LOG_LEVEL_VERBOSE);

		FWDUI_load_settings(pvar);

		pvar->cv->TelAutoDetect = FALSE;
		/* This next line should not be needed because Teraterm's
		   CommLib should find ts->Telnet == 0 ... but we'll do this
		   just to be on the safe side. */
		pvar->cv->TelFlag = FALSE;

		pvar->Precv = *hooks->Precv;
		pvar->Psend = *hooks->Psend;
		pvar->PWSAAsyncSelect = *hooks->PWSAAsyncSelect;
		pvar->Pconnect = *hooks->Pconnect;
		pvar->PWSAGetLastError = *hooks->PWSAGetLastError;

		*hooks->Precv = TTXrecv;
		*hooks->Psend = TTXsend;
		*hooks->PWSAAsyncSelect = TTXWSAAsyncSelect;
		*hooks->Pconnect = TTXconnect;

		SSH_open(pvar);
		HOSTS_open(pvar);
		FWDUI_open(pvar);

		// 設定を myproposal に反映するのは、接続直前のここだけ。 (2006.6.26 maya)
		SSH2_update_cipher_myproposal(pvar);
		SSH2_update_compression_myproposal(pvar);
	}
}

static void PASCAL FAR TTXCloseTCP(TTXSockHooks FAR * hooks)
{
	GET_VAR();

	if (pvar->session_settings.Enabled) {
		pvar->socket = INVALID_SOCKET;

		notify_verbose_message(pvar, "Terminating SSH session...",
							   LOG_LEVEL_VERBOSE);

		*hooks->Precv = pvar->Precv;
		*hooks->Psend = pvar->Psend;
		*hooks->PWSAAsyncSelect = pvar->PWSAAsyncSelect;
		*hooks->Pconnect = pvar->Pconnect;
	}

	uninit_TTSSH(pvar);
	init_TTSSH(pvar);
}

static void enable_dlg_items(HWND dlg, int from, int to, BOOL enabled)
{
	for (; from <= to; from++) {
		EnableWindow(GetDlgItem(dlg, from), enabled);
	}
}

static BOOL CALLBACK TTXHostDlg(HWND dlg, UINT msg, WPARAM wParam,
								LPARAM lParam)
{
	static char *ssh_version[] = {"SSH1", "SSH2", NULL};
	PGetHNRec GetHNRec;
	char EntName[7];
	char TempHost[HostNameMaxLength + 1];
	WORD i, j, w;
	BOOL Ok;

	GET_VAR();

	switch (msg) {
	case WM_INITDIALOG:
		GetHNRec = (PGetHNRec) lParam;
		SetWindowLong(dlg, DWL_USER, lParam);

		// ホストヒストリのチェックボックスを追加 (2005.10.21 yutaka)
		if (pvar->ts->HistoryList > 0) {
			SendMessage(GetDlgItem(dlg, IDC_HISTORY), BM_SETCHECK, BST_CHECKED, 0);
		} else {
			SendMessage(GetDlgItem(dlg, IDC_HISTORY), BM_SETCHECK, BST_UNCHECKED, 0);
		}

		if (GetHNRec->PortType == IdFile)
			GetHNRec->PortType = IdTCPIP;
		CheckRadioButton(dlg, IDC_HOSTTCPIP, IDC_HOSTSERIAL,
						 IDC_HOSTTCPIP + GetHNRec->PortType - 1);

		strcpy(EntName, "Host");

		i = 1;
		do {
			sprintf(&EntName[4], "%d", i);
			GetPrivateProfileString("Hosts", EntName, "",
									TempHost, sizeof(TempHost),
									GetHNRec->SetupFN);
			if (strlen(TempHost) > 0)
				SendDlgItemMessage(dlg, IDC_HOSTNAME, CB_ADDSTRING,
								   0, (LPARAM) TempHost);
			i++;
		} while ((i <= 99) && (strlen(TempHost) > 0));

		SendDlgItemMessage(dlg, IDC_HOSTNAME, EM_LIMITTEXT,
						   HostNameMaxLength - 1, 0);

		SendDlgItemMessage(dlg, IDC_HOSTNAME, CB_SETCURSEL, 0, 0);

		CheckRadioButton(dlg, IDC_HOSTTELNET, IDC_HOSTOTHER,
						 pvar->settings.Enabled ? IDC_HOSTSSH : GetHNRec->
						 Telnet ? IDC_HOSTTELNET : IDC_HOSTOTHER);
		SendDlgItemMessage(dlg, IDC_HOSTTCPPORT, EM_LIMITTEXT, 5, 0);
		SetDlgItemInt(dlg, IDC_HOSTTCPPORT, GetHNRec->TCPPort, FALSE);
#ifdef INET6
		for (i = 0; ProtocolFamilyList[i]; ++i) {
			SendDlgItemMessage(dlg, IDC_HOSTTCPPROTOCOL, CB_ADDSTRING,
							   0, (LPARAM) ProtocolFamilyList[i]);
		}
		SendDlgItemMessage(dlg, IDC_HOSTTCPPROTOCOL, EM_LIMITTEXT,
						   ProtocolFamilyMaxLength - 1, 0);
		SendDlgItemMessage(dlg, IDC_HOSTTCPPROTOCOL, CB_SETCURSEL, 0, 0);
#endif							/* INET6 */

		/////// SSH version
		for (i = 0; ssh_version[i]; ++i) {
			SendDlgItemMessage(dlg, IDC_SSH_VERSION, CB_ADDSTRING,
							   0, (LPARAM) ssh_version[i]);
		}
		SendDlgItemMessage(dlg, IDC_SSH_VERSION, EM_LIMITTEXT,
						   NUM_ELEM(ssh_version) - 1, 0);

		if (pvar->settings.ssh_protocol_version == 1) {
			SendDlgItemMessage(dlg, IDC_SSH_VERSION, CB_SETCURSEL, 0, 0); // SSH1
		} else {
			SendDlgItemMessage(dlg, IDC_SSH_VERSION, CB_SETCURSEL, 1, 0); // SSH2
		}

		if (IsDlgButtonChecked(dlg, IDC_HOSTSSH)) {
			enable_dlg_items(dlg, IDC_SSH_VERSION, IDC_SSH_VERSION, TRUE); // enabled
		} else {
			enable_dlg_items(dlg, IDC_SSH_VERSION, IDC_SSH_VERSION, FALSE); // disabled
		}
		/////// SSH version


		j = 0;
		w = 1;
		strcpy(EntName, "COM");
		for (i = 1; i <= GetHNRec->MaxComPort; i++) {
			sprintf(&EntName[3], "%d", i);
			SendDlgItemMessage(dlg, IDC_HOSTCOM, CB_ADDSTRING,
							   0, (LPARAM) EntName);
			j++;
			if (GetHNRec->ComPort == i)
				w = j;
		}
		if (j > 0)
			SendDlgItemMessage(dlg, IDC_HOSTCOM, CB_SETCURSEL, w - 1, 0);
		else					/* All com ports are already used */
			GetHNRec->PortType = IdTCPIP;

		if (GetHNRec->PortType == IdTCPIP) {
			enable_dlg_items(dlg, IDC_HOSTCOMLABEL, IDC_HOSTCOM, FALSE);

			enable_dlg_items(dlg, IDC_SSH_VERSION, IDC_SSH_VERSION, TRUE); 
			enable_dlg_items(dlg, IDC_SSH_VERSION_LABEL, IDC_SSH_VERSION_LABEL, TRUE); 
		}
#ifdef INET6
		else {
			enable_dlg_items(dlg, IDC_HOSTNAMELABEL, IDC_HOSTTCPPORT,
							 FALSE);
			enable_dlg_items(dlg, IDC_HOSTTCPPROTOCOLLABEL,
							 IDC_HOSTTCPPROTOCOL, FALSE);

			enable_dlg_items(dlg, IDC_SSH_VERSION, IDC_SSH_VERSION, FALSE); // disabled
			enable_dlg_items(dlg, IDC_SSH_VERSION_LABEL, IDC_SSH_VERSION_LABEL, FALSE); // disabled (2004.11.23 yutaka)
		}
#else
		else
			enable_dlg_items(dlg, IDC_HOSTNAMELABEL, IDC_HOSTTCPPORT,
							 FALSE);
#endif							/* INET6 */

		// Host dialogにフォーカスをあてる (2004.10.2 yutaka)
		if (GetHNRec->PortType == IdTCPIP) {
			HWND hwnd = GetDlgItem(dlg, IDC_HOSTNAME);
			SetFocus(hwnd);
		} else {
			HWND hwnd = GetDlgItem(dlg, IDC_HOSTCOM);
			SetFocus(hwnd);
		}

		// SetFocus()でフォーカスをあわせた場合、FALSEを返す必要がある。
		// TRUEを返すと、TABSTOP対象の一番はじめのコントロールが選ばれる。
		// (2004.11.23 yutaka)
		return FALSE;
		//return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			GetHNRec = (PGetHNRec) GetWindowLong(dlg, DWL_USER);
			if (GetHNRec != NULL) {
				if (IsDlgButtonChecked(dlg, IDC_HOSTTCPIP)) {
#ifdef INET6
					char afstr[BUFSIZ];
#endif							/* INET6 */
					i = GetDlgItemInt(dlg, IDC_HOSTTCPPORT, &Ok, FALSE);
					if (Ok) {
						GetHNRec->TCPPort = i;
					} else {
						MessageBox(dlg, "Teraterm",
								   "The TCP port must be a number.",
								   MB_OK | MB_ICONEXCLAMATION);
						return TRUE;
					}
#ifdef INET6
#define getaf(str) \
((strcmp((str), "IPv6") == 0) ? AF_INET6 : \
 ((strcmp((str), "IPv4") == 0) ? AF_INET : AF_UNSPEC))
					memset(afstr, 0, sizeof(afstr));
					GetDlgItemText(dlg, IDC_HOSTTCPPROTOCOL, afstr,
								   sizeof(afstr));
					GetHNRec->ProtocolFamily = getaf(afstr);
#endif							/* INET6 */
					GetHNRec->PortType = IdTCPIP;
					GetDlgItemText(dlg, IDC_HOSTNAME, GetHNRec->HostName,
								   HostNameMaxLength);
					GetHNRec->Telnet = FALSE;
					pvar->hostdlg_activated = TRUE;
					pvar->hostdlg_Enabled = FALSE;
					if (IsDlgButtonChecked(dlg, IDC_HOSTTELNET)) {
						GetHNRec->Telnet = TRUE;
					} else if (IsDlgButtonChecked(dlg, IDC_HOSTSSH)) {
						pvar->hostdlg_Enabled = TRUE;

						// check SSH protocol version 
						memset(afstr, 0, sizeof(afstr));
						GetDlgItemText(dlg, IDC_SSH_VERSION, afstr, sizeof(afstr));
						if (_stricmp(afstr, "SSH1") == 0) {
							pvar->settings.ssh_protocol_version = 1;
						} else {
							pvar->settings.ssh_protocol_version = 2;
						}
					}

					// host history check button
					if (SendMessage(GetDlgItem(dlg, IDC_HISTORY), BM_GETCHECK, 0, 0) == BST_CHECKED) {
						pvar->ts->HistoryList = 1;
					} else {
						pvar->ts->HistoryList = 0;
					}

				} else {
					GetHNRec->PortType = IdSerial;
					GetHNRec->HostName[0] = 0;
					memset(EntName, 0, sizeof(EntName));
					GetDlgItemText(dlg, IDC_HOSTCOM, EntName,
								   sizeof(EntName) - 1);
					GetHNRec->ComPort = (BYTE) (EntName[3]) - 0x30;
					if (strlen(EntName) > 4)
						GetHNRec->ComPort =
							GetHNRec->ComPort * 10 + (BYTE) (EntName[4]) -
							0x30;
				}
			}
			EndDialog(dlg, 1);
			return TRUE;

		case IDCANCEL:
			EndDialog(dlg, 0);
			return TRUE;

		case IDC_HOSTTCPIP:
			enable_dlg_items(dlg, IDC_HOSTNAMELABEL, IDC_HOSTTCPPORT,
							 TRUE);
#ifdef INET6
			enable_dlg_items(dlg, IDC_HOSTTCPPROTOCOLLABEL,
							 IDC_HOSTTCPPROTOCOL, TRUE);
#endif							/* INET6 */
			enable_dlg_items(dlg, IDC_HOSTCOMLABEL, IDC_HOSTCOM, FALSE);

			enable_dlg_items(dlg, IDC_SSH_VERSION_LABEL, IDC_SSH_VERSION_LABEL, TRUE); // disabled (2004.11.23 yutaka)
			if (IsDlgButtonChecked(dlg, IDC_HOSTSSH)) {
				enable_dlg_items(dlg, IDC_SSH_VERSION, IDC_SSH_VERSION, TRUE);
			} else {
				enable_dlg_items(dlg, IDC_SSH_VERSION, IDC_SSH_VERSION, FALSE); // disabled
			}

			enable_dlg_items(dlg, IDC_HISTORY, IDC_HISTORY, TRUE); // disabled

			return TRUE;

		case IDC_HOSTSERIAL:
			enable_dlg_items(dlg, IDC_HOSTCOMLABEL, IDC_HOSTCOM, TRUE);
			enable_dlg_items(dlg, IDC_HOSTNAMELABEL, IDC_HOSTTCPPORT,
							 FALSE);
#ifdef INET6
			enable_dlg_items(dlg, IDC_HOSTTCPPROTOCOLLABEL,
							 IDC_HOSTTCPPROTOCOL, FALSE);
#endif							/* INET6 */
			enable_dlg_items(dlg, IDC_SSH_VERSION, IDC_SSH_VERSION, FALSE); // disabled
			enable_dlg_items(dlg, IDC_SSH_VERSION_LABEL, IDC_SSH_VERSION_LABEL, FALSE); // disabled (2004.11.23 yutaka)

			enable_dlg_items(dlg, IDC_HISTORY, IDC_HISTORY, FALSE); // disabled

			return TRUE;

		case IDC_HOSTSSH:
			enable_dlg_items(dlg, IDC_SSH_VERSION,
							 IDC_SSH_VERSION, TRUE);
			goto hostssh_enabled;

		case IDC_HOSTTELNET:
		case IDC_HOSTOTHER:
			enable_dlg_items(dlg, IDC_SSH_VERSION, IDC_SSH_VERSION, FALSE); // disabled
hostssh_enabled:

			GetHNRec = (PGetHNRec) GetWindowLong(dlg, DWL_USER);

			if (IsDlgButtonChecked(dlg, IDC_HOSTTELNET)) {
				if (GetHNRec != NULL)
					SetDlgItemInt(dlg, IDC_HOSTTCPPORT, GetHNRec->TelPort,
								  FALSE);
			} else if (IsDlgButtonChecked(dlg, IDC_HOSTSSH)) {
				SetDlgItemInt(dlg, IDC_HOSTTCPPORT, 22, FALSE);
			}
			return TRUE;

		case IDC_HOSTHELP:
			PostMessage(GetParent(dlg), WM_USER_DLGHELP2, 0, 0);
		}
	}
	return FALSE;
}

static BOOL FAR PASCAL TTXGetHostName(HWND parent, PGetHNRec rec)
{
	return (BOOL) DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_HOSTDLG),
								 parent, TTXHostDlg, (LONG) rec);
}

static void PASCAL FAR TTXGetUIHooks(TTXUIHooks FAR * hooks)
{
	GET_VAR();

	*hooks->GetHostName = TTXGetHostName;
}

static void FAR PASCAL TTXReadINIFile(PCHAR fileName, PTTSet ts)
{
	GET_VAR();

	(pvar->ReadIniFile) (fileName, ts);
	read_ssh_options(pvar, fileName);
	pvar->settings = *pvar->ts_SSH;
	notify_verbose_message(pvar, "Reading INI file", LOG_LEVEL_VERBOSE);
	FWDUI_load_settings(pvar);
}

static void FAR PASCAL TTXWriteINIFile(PCHAR fileName, PTTSet ts)
{
	GET_VAR();

	(pvar->WriteIniFile) (fileName, ts);
	*pvar->ts_SSH = pvar->settings;
	clear_local_settings(pvar);
	notify_verbose_message(pvar, "Writing INI file", LOG_LEVEL_VERBOSE);
	write_ssh_options(pvar, fileName, pvar->ts_SSH);
}

static void read_ssh_options_from_user_file(PTInstVar pvar,
											char FAR * user_file_name)
{
	if (user_file_name[0] == '.') {
		read_ssh_options(pvar, user_file_name);
	} else {
		char buf[1024];

		get_teraterm_dir_relative_name(buf, sizeof(buf), user_file_name);
		read_ssh_options(pvar, buf);
	}

	pvar->settings = *pvar->ts_SSH;
	FWDUI_load_settings(pvar);
}


// @をブランクに置換する。 (2005.1.26 yutaka)
static void replace_to_blank(char *src, char *dst, int dst_len)
{
	int len, i;

	len = strlen(src);
	if (dst_len < len) // buffer overflow check
		return;

	for (i = 0 ; i < len ; i++) {
		if (src[i] == '@') { // @ が登場したら
			if (i < len - 1 && src[i + 1] == '@') { // その次も @ ならアットマークと認識する
				*dst++ = '@';
				i++;
			} else {
				*dst++ = ' '; // 空白に置き換える
			}
		} else {
			*dst++ = src[i];
		}
	}
	*dst = '\0';
}

/* returns 1 if the option text must be deleted */
static int parse_option(PTInstVar pvar, char FAR * option)
{
	if ((option[0] == '-' || option[0] == '/')) {
		if (MATCH_STR(option + 1, "ssh") == 0) {
			if (option[4] == 0) {
				pvar->settings.Enabled = 1;
			} else if (MATCH_STR(option + 4, "-L") == 0
					   || MATCH_STR(option + 4, "-R") == 0
					   || _stricmp(option + 4, "-X") == 0) {
				if (pvar->settings.DefaultForwarding[0] == 0) {
					strcpy(pvar->settings.DefaultForwarding, option + 5);
				} else {
					strcat(pvar->settings.DefaultForwarding, ";");
					strcat(pvar->settings.DefaultForwarding, option + 5);
				}
			} else if (MATCH_STR(option + 4, "-f=") == 0) {
				read_ssh_options_from_user_file(pvar, option + 7);
			} else if (MATCH_STR(option + 4, "-v") == 0) {
				pvar->settings.LogLevel = LOG_LEVEL_VERBOSE;
			} else if (_stricmp(option + 4, "-autologin") == 0
					   || _stricmp(option + 4, "-autologon") == 0) {
				pvar->settings.TryDefaultAuth = TRUE;

			} else if (MATCH_STR(option + 4, "-consume=") == 0) {
				read_ssh_options_from_user_file(pvar, option + 13);
				DeleteFile(option + 13);
			} else {
				char buf[1024];

				_snprintf(buf, sizeof(buf),
						  "Unrecognized command-line option: %s", option);
				buf[sizeof(buf) - 1] = 0;

				MessageBox(NULL, buf, "TTSSH", MB_OK | MB_ICONEXCLAMATION);
			}

			return 1;
		} else if (MATCH_STR(option + 1, "t=") == 0) {
			if (strcmp(option + 3, "2") == 0) {
				pvar->settings.Enabled = 1;
				return 1;
			} else {
				pvar->settings.Enabled = 0;
			}
		} else if (MATCH_STR(option + 1, "f=") == 0) {
			read_ssh_options_from_user_file(pvar, option + 3);

		// /1 および /2 オプションの新規追加 (2004.10.3 yutaka)
		} else if (MATCH_STR(option + 1, "1") == 0) {
			// command line: /ssh /1 is SSH1 only
			pvar->settings.ssh_protocol_version = 1;

		} else if (MATCH_STR(option + 1, "2") == 0) {
			// command line: /ssh /2 is SSH2 & SSH1
			pvar->settings.ssh_protocol_version = 2;

		} else if (MATCH_STR(option + 1, "nossh") == 0) {
			// '/nossh' オプションの追加。
			// TERATERM.INI でSSHが有効になっている場合、うまくCygtermが起動しないことが
			// あることへの対処。(2004.10.11 yutaka)
			pvar->settings.Enabled = 0;

		} else if (MATCH_STR(option + 1, "auth") == 0) {
			// SSH2自動ログインオプションの追加 
			//
			// SYNOPSIS: /ssh /auth=passowrd /user=ユーザ名 /passwd=パスワード
			//           /ssh /auth=publickey /user=ユーザ名 /passwd=パスワード /keyfile=パス
			// EXAMPLE: /ssh /auth=password /user=nike /passwd=a@bc
			//          /ssh /auth=publickey /user=foo /passwd=bar /keyfile=d:\tmp\id_rsa
			// NOTICE: パスワードやパスに空白が含む場合は、ブランクの代わりに @ を使うこと。
			//
			// (2004.11.30 yutaka)
			// (2005.1.26 yutaka) 空白対応。公開鍵認証サポート。
			//
			pvar->ssh2_autologin = 1; // for SSH2 (2004.11.30 yutaka)

			if (MATCH_STR(option + 5, "=password") == 0) { // パスワード/keyboard-interactive認証
				//pvar->auth_state.cur_cred.method = SSH_AUTH_PASSWORD;
				pvar->ssh2_authmethod = SSH_AUTH_PASSWORD;

			} else if (MATCH_STR(option + 5, "=publickey") == 0) { // 公開鍵認証
				//pvar->auth_state.cur_cred.method = SSH_AUTH_RSA;
				pvar->ssh2_authmethod = SSH_AUTH_RSA;

			} else {
				// TODO:

			}

		} else if (MATCH_STR(option + 1, "user=") == 0) {
			replace_to_blank(option + 6, pvar->ssh2_username, sizeof(pvar->ssh2_username));
			//_snprintf(pvar->ssh2_username, sizeof(pvar->ssh2_username), "%s", option + 6);

		} else if (MATCH_STR(option + 1, "passwd=") == 0) {
			replace_to_blank(option + 8, pvar->ssh2_password, sizeof(pvar->ssh2_password));
			//_snprintf(pvar->ssh2_password, sizeof(pvar->ssh2_password), "%s", option + 8);

		} else if (MATCH_STR(option + 1, "keyfile=") == 0) {
			replace_to_blank(option + 9, pvar->ssh2_keyfile, sizeof(pvar->ssh2_keyfile));

		}

	}

	return 0;
}

static void FAR PASCAL TTXParseParam(PCHAR param, PTTSet ts,
									 PCHAR DDETopic)
{
	int i;
	BOOL inParam = FALSE;
	BOOL inQuotes = FALSE;
	PCHAR option = NULL;
	GET_VAR();

	if (pvar->hostdlg_activated) {
		pvar->settings.Enabled = pvar->hostdlg_Enabled;
	}

	for (i = 0; param[i] != 0; i++) {
		if (inQuotes ? param[i] ==
			'"' : (param[i] == ' ' || param[i] == '\t')) {
			if (option != NULL) {
				char ch = param[i];

				param[i] = 0;
				if (parse_option
					(pvar, *option == '"' ? option + 1 : option)) {
					memset(option, ' ', i + 1 - (option - param));
				} else {
					param[i] = ch;
				}
				option = NULL;
			}
			inParam = FALSE;
			inQuotes = FALSE;
		} else if (!inParam) {
			if (param[i] == '"') {
				inQuotes = TRUE;
				inParam = TRUE;
				option = param + i;
			} else if (param[i] != ' ' && param[i] != '\t') {
				inParam = TRUE;
				option = param + i;
			}
		}
	}

	if (option != NULL) {
		if (parse_option(pvar, option)) {
			memset(option, ' ', i - (option - param));
		}
	}

	FWDUI_load_settings(pvar);

	(pvar->ParseParam) (param, ts, DDETopic);

}

static void PASCAL FAR TTXGetSetupHooks(TTXSetupHooks FAR * hooks)
{
	GET_VAR();

	pvar->ReadIniFile = *hooks->ReadIniFile;
	pvar->WriteIniFile = *hooks->WriteIniFile;
	pvar->ParseParam = *hooks->ParseParam;

	*hooks->ReadIniFile = TTXReadINIFile;
	*hooks->WriteIniFile = TTXWriteINIFile;
	*hooks->ParseParam = TTXParseParam;
}

static void PASCAL FAR TTXSetWinSize(int rows, int cols)
{
	GET_VAR();

	SSH_notify_win_size(pvar, cols, rows);
}

static void insertMenuBeforeItem(HMENU menu, WORD beforeItemID, WORD flags,
								 WORD newItemID, char FAR * text)
{
	int i, j;

	for (i = GetMenuItemCount(menu) - 1; i >= 0; i--) {
		HMENU submenu = GetSubMenu(menu, i);

		for (j = GetMenuItemCount(submenu) - 1; j >= 0; j--) {
			if (GetMenuItemID(submenu, j) == beforeItemID) {
				InsertMenu(submenu, j, MF_BYPOSITION | flags, newItemID,
						   text);
				return;
			}
		}
	}
}

static void PASCAL FAR TTXModifyMenu(HMENU menu)
{
	GET_VAR();

	/* inserts before ID_HELP_ABOUT */
	insertMenuBeforeItem(menu, 50990, MF_ENABLED, ID_ABOUTMENU,
						 "About &TTSSH...");

	/* inserts before ID_SETUP_TCPIP */
	insertMenuBeforeItem(menu, 50360, MF_ENABLED, ID_SSHSETUPMENU,
						 "SS&H...");
	/* inserts before ID_SETUP_TCPIP */
	insertMenuBeforeItem(menu, 50360, MF_ENABLED, ID_SSHAUTHSETUPMENU,
						 "SSH &Authentication...");
	/* inserts before ID_SETUP_TCPIP */
	insertMenuBeforeItem(menu, 50360, MF_ENABLED, ID_SSHFWDSETUPMENU,
						 "SSH F&orwarding...");

	insertMenuBeforeItem(menu, 50360, MF_ENABLED, ID_SSHKEYGENMENU,
						 "SSH KeyGenerator...");
}

static void append_about_text(HWND dlg, char FAR * prefix, char FAR * msg)
{
	SendDlgItemMessage(dlg, IDC_ABOUTTEXT, EM_REPLACESEL, 0,
					   (LPARAM) prefix);
	SendDlgItemMessage(dlg, IDC_ABOUTTEXT, EM_REPLACESEL, 0, (LPARAM) msg);
	SendDlgItemMessage(dlg, IDC_ABOUTTEXT, EM_REPLACESEL, 0,
					   (LPARAM) (char FAR *) "\r\n");
}

// 実行ファイルからバージョン情報を得る (2005.2.28 yutaka)
void get_file_version(char *exefile, int *major, int *minor, int *release, int *build)
{
	typedef struct {
		WORD wLanguage;
		WORD wCodePage;
	} LANGANDCODEPAGE, *LPLANGANDCODEPAGE;
	LPLANGANDCODEPAGE lplgcode;
	UINT unLen;
	DWORD size;
	char *buf = NULL;
	BOOL ret;
	int i;
	char fmt[80];
	char *pbuf;

	size = GetFileVersionInfoSize(exefile, NULL);
	if (size == 0) {
		goto error;
	}
	buf = malloc(size);
	ZeroMemory(buf, size);

	if (GetFileVersionInfo(exefile, 0, size, buf) == FALSE) {
		goto error;
	}

	ret = VerQueryValue(buf,
			"\\VarFileInfo\\Translation", 
			(LPVOID *)&lplgcode, &unLen);
	if (ret == FALSE)
		goto error;

	for (i = 0 ; i < (int)(unLen / sizeof(LANGANDCODEPAGE)) ; i++) {
		_snprintf(fmt, sizeof(fmt), "\\StringFileInfo\\%04x%04x\\FileVersion", 
			lplgcode[i].wLanguage, lplgcode[i].wCodePage);
		VerQueryValue(buf, fmt, &pbuf, &unLen);
		if (unLen > 0) { // get success
			int n, a, b, c, d;

			n = sscanf(pbuf, "%d, %d, %d, %d", &a, &b, &c, &d);
			if (n == 4) { // convert success
				*major = a;
				*minor = b;
				*release = c;
				*build = d;
				break;
			}
		}
	}

	free(buf);
	return;

error:
	free(buf);
	*major = *minor = *release = *build = 0;
}

static void init_about_dlg(PTInstVar pvar, HWND dlg)
{
	char buf[1024];
	int a, b, c, d;

	// TTSSHのバージョンを設定する (2005.2.28 yutaka)
	get_file_version("ttxssh.dll", &a, &b, &c, &d);
	_snprintf(buf, sizeof(buf), "TTSSH\r\nTeraterm Secure Shell extension, %d.%d", a, b);
	SendMessage(GetDlgItem(dlg, IDC_TTSSH_VERSION), WM_SETTEXT, 0, (LPARAM)buf);

	// OpenSSLのバージョンを設定する (2005.1.24 yutaka)
	// 条件文追加 (2005.5.11 yutaka)
#ifdef OPENSSL_VERSION_TEXT
	SendMessage(GetDlgItem(dlg, IDC_OPENSSL_VERSION), WM_SETTEXT, 0, (LPARAM)OPENSSL_VERSION_TEXT);
#else
	SendMessage(GetDlgItem(dlg, IDC_OPENSSL_VERSION), WM_SETTEXT, 0, (LPARAM)"Unknown");
#endif

	// zlibのバージョンを設定する (2005.5.11 yutaka)
#ifdef ZLIB_VERSION
	_snprintf(buf, sizeof(buf), "ZLib %s", ZLIB_VERSION);
#else
	_snprintf(buf, sizeof(buf), "ZLib Unknown");
#endif
	SendMessage(GetDlgItem(dlg, IDC_ZLIB_VERSION), WM_SETTEXT, 0, (LPARAM)buf);


	// TTSSHダイアログに表示するSSHに関する情報 (2004.10.30 yutaka)
	if (pvar->socket != INVALID_SOCKET) {
		if (SSHv1(pvar)) {
			SSH_get_server_ID_info(pvar, buf, sizeof(buf));
			append_about_text(dlg, "Server ID: ", buf);
			SSH_get_protocol_version_info(pvar, buf, sizeof(buf));
			append_about_text(dlg, "Using protocol: ", buf);
			CRYPT_get_cipher_info(pvar, buf, sizeof(buf));
			append_about_text(dlg, "Encryption: ", buf);
			CRYPT_get_server_key_info(pvar, buf, sizeof(buf));
			append_about_text(dlg, "Server keys: ", buf);
			AUTH_get_auth_info(pvar, buf, sizeof(buf));
			append_about_text(dlg, "Authentication: ", buf);
			SSH_get_compression_info(pvar, buf, sizeof(buf));
			append_about_text(dlg, "Compression: ", buf);

		} else { // SSH2
			SSH_get_server_ID_info(pvar, buf, sizeof(buf));
			append_about_text(dlg, "Server ID: ", buf);

			append_about_text(dlg, "Client ID: ", pvar->client_version_string);

			SSH_get_protocol_version_info(pvar, buf, sizeof(buf));
			append_about_text(dlg, "Using protocol: ", buf);

			if (pvar->kex_type == KEX_DH_GRP1_SHA1) {
				strcpy(buf, KEX_DH1);
			} else if (pvar->kex_type == KEX_DH_GRP14_SHA1) {
				strcpy(buf, KEX_DH14);
			} else {
				strcpy(buf, KEX_DHGEX);
			}
			append_about_text(dlg, "KEX: ", buf);

			if (pvar->hostkey_type == KEY_DSA) {
				strcpy(buf, "ssh-dss");
			} else {
				strcpy(buf, "ssh-rsa");
			}
			append_about_text(dlg, "Host Key: ", buf);

			// add HMAC algorithm (2004.12.17 yutaka)
			buf[0] = '\0';
			if (pvar->ctos_hmac == HMAC_SHA1) {
				strcat(buf, "hmac-sha1");
			} else if (pvar->ctos_hmac == HMAC_MD5) {
				strcat(buf, "hmac-md5");
			}
			strcat(buf, " to server, ");
			if (pvar->stoc_hmac == HMAC_SHA1) {
				strcat(buf, "hmac-sha1");
			} else if (pvar->stoc_hmac == HMAC_MD5) {
				strcat(buf, "hmac-md5");
			}
			strcat(buf, " from server");
			append_about_text(dlg, "HMAC: ", buf);

			CRYPT_get_cipher_info(pvar, buf, sizeof(buf));
			append_about_text(dlg, "Encryption: ", buf);
			CRYPT_get_server_key_info(pvar, buf, sizeof(buf));
			append_about_text(dlg, "Server keys: ", buf);
			AUTH_get_auth_info(pvar, buf, sizeof(buf));
			append_about_text(dlg, "Authentication: ", buf);

			SSH_get_compression_info(pvar, buf, sizeof(buf));
			if (pvar->ctos_compression == COMP_DELAYED) { // 遅延パケット圧縮の場合 (2006.6.23 yutaka)
				append_about_text(dlg, "Delayed Compression: ", buf);
			} else {
				append_about_text(dlg, "Compression: ", buf);
			}

		}
	}
}

static BOOL CALLBACK TTXAboutDlg(HWND dlg, UINT msg, WPARAM wParam,
								 LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		init_about_dlg((PTInstVar) lParam, dlg);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			EndDialog(dlg, 1);
			return TRUE;
		case IDCANCEL:			/* there isn't a cancel button, but other Windows
								   UI things can send this message */
			EndDialog(dlg, 0);
			return TRUE;
		}
		break;
	}

	return FALSE;
}

static char FAR *get_cipher_name(int cipher)
{
	switch (cipher) {
	case SSH_CIPHER_NONE:
		return "<ciphers below this line are disabled>";
	case SSH_CIPHER_RC4:
		return "RC4";
	case SSH_CIPHER_3DES:
		return "3DES";
	case SSH_CIPHER_DES:
		return "DES";
	case SSH_CIPHER_IDEA:
		return "IDEA";
	case SSH_CIPHER_TSS:
		return "TSS";
	case SSH_CIPHER_BLOWFISH:
		return "Blowfish";

	// for SSH2(yutaka)
	case SSH_CIPHER_AES128:
		return "AES128(SSH2)";
	case SSH_CIPHER_3DES_CBC:
		return "3DES-CBC(SSH2)";

	default:
		return NULL;
	}
}

static void set_move_button_status(HWND dlg)
{
	HWND cipherControl = GetDlgItem(dlg, IDC_SSHCIPHERPREFS);
	int curPos = (int) SendMessage(cipherControl, LB_GETCURSEL, 0, 0);
	int maxPos = (int) SendMessage(cipherControl, LB_GETCOUNT, 0, 0) - 1;

	EnableWindow(GetDlgItem(dlg, IDC_SSHMOVECIPHERUP), curPos > 0
				 && curPos <= maxPos);
	EnableWindow(GetDlgItem(dlg, IDC_SSHMOVECIPHERDOWN), curPos >= 0
				 && curPos < maxPos);
}

static void init_setup_dlg(PTInstVar pvar, HWND dlg)
{
	HWND compressionControl = GetDlgItem(dlg, IDC_SSHCOMPRESSIONLEVEL);
	HWND cipherControl = GetDlgItem(dlg, IDC_SSHCIPHERPREFS);
	int i;
	int ch;

	SendMessage(compressionControl, TBM_SETRANGE, TRUE, MAKELONG(0, 9));
	SendMessage(compressionControl, TBM_SETPOS, TRUE,
				pvar->settings.CompressionLevel);

	normalize_cipher_order(pvar->settings.CipherOrder);

	for (i = 0; pvar->settings.CipherOrder[i] != 0; i++) {
		int cipher = pvar->settings.CipherOrder[i] - '0';
		char FAR *name = get_cipher_name(cipher);

		if (name != NULL) {
			SendMessage(cipherControl, LB_ADDSTRING, 0, (LPARAM) name);
		}
	}

	SendMessage(cipherControl, LB_SETCURSEL, 0, 0);
	set_move_button_status(dlg);

	for (i = 0; (ch = pvar->settings.KnownHostsFiles[i]) != 0 && ch != ';';
		 i++) {
	}
	if (ch != 0) {
		pvar->settings.KnownHostsFiles[i] = 0;
		SetDlgItemText(dlg, IDC_READWRITEFILENAME,
					   pvar->settings.KnownHostsFiles);
		pvar->settings.KnownHostsFiles[i] = ch;
		SetDlgItemText(dlg, IDC_READONLYFILENAME,
					   pvar->settings.KnownHostsFiles + i + 1);
	} else {
		SetDlgItemText(dlg, IDC_READWRITEFILENAME,
					   pvar->settings.KnownHostsFiles);
	}

	// SSH2 HeartBeat(keep-alive)を追加 (2005.2.22 yutaka)
	{
		char buf[10];
		_snprintf(buf, sizeof(buf), "%d", pvar->settings.ssh_heartbeat_overtime);
		SetDlgItemText(dlg, IDC_HEARTBEAT_EDIT, buf);
	}

}

void get_teraterm_dir_relative_name(char FAR * buf, int bufsize,
									char FAR * basename)
{
	int filename_start = 0;
	int i;
	int ch;

	if (basename[0] == '\\' || basename[0] == '/'
		|| (basename[0] != 0 && basename[1] == ':')) {
		strncpy(buf, basename, bufsize);
		buf[bufsize - 1] = 0;
		return;
	}

	GetModuleFileName(NULL, buf, bufsize);
	for (i = 0; (ch = buf[i]) != 0; i++) {
		if (ch == '\\' || ch == '/' || ch == ':') {
			filename_start = i + 1;
		}
	}

	if (bufsize > filename_start) {
		strncpy(buf + filename_start, basename, bufsize - filename_start);
	}
	buf[bufsize - 1] = 0;
}

int copy_teraterm_dir_relative_path(char FAR * dest, int destsize,
									char FAR * basename)
{
	char buf[1024];
	int filename_start = 0;
	int i;
	int ch, ch2;

	if (basename[0] != '\\' && basename[0] != '/'
		&& (basename[0] == 0 || basename[1] != ':')) {
		strncpy(dest, basename, destsize);
		dest[destsize - 1] = 0;
		return strlen(dest);
	}

	GetModuleFileName(NULL, buf, sizeof(buf));
	for (i = 0; (ch = buf[i]) != 0; i++) {
		if (ch == '\\' || ch == '/' || ch == ':') {
			filename_start = i + 1;
		}
	}

	for (i = 0; i < filename_start; i++) {
		ch = toupper(buf[i]);
		ch2 = toupper(basename[i]);

		if (ch == ch2
			|| ((ch == '\\' || ch == '/')
				&& (ch2 == '\\' || ch2 == '/'))) {
		} else {
			break;
		}
	}

	if (i == filename_start) {
		strncpy(dest, basename + i, destsize);
	} else {
		strncpy(dest, basename, destsize);
	}
	dest[destsize - 1] = 0;
	return strlen(dest);
}

static void complete_setup_dlg(PTInstVar pvar, HWND dlg)
{
	char buf[4096];
	char buf2[1024];
	HWND compressionControl = GetDlgItem(dlg, IDC_SSHCOMPRESSIONLEVEL);
	HWND cipherControl = GetDlgItem(dlg, IDC_SSHCIPHERPREFS);
	int i, j, buf2index, bufindex;
	int count = (int) SendMessage(cipherControl, LB_GETCOUNT, 0, 0);

	pvar->settings.CompressionLevel =
		(int) SendMessage(compressionControl, TBM_GETPOS, 0, 0);

	buf2index = 0;
	for (i = 0; i < count; i++) {
		int len = SendMessage(cipherControl, LB_GETTEXTLEN, i, 0);

		if (len > 0 && len < sizeof(buf)) {	/* should always be true */
			buf[0] = 0;
			SendMessage(cipherControl, LB_GETTEXT, i, (LPARAM) buf);
			for (j = 0;
				 j <= SSH_CIPHER_MAX
				 && strcmp(buf, get_cipher_name(j)) != 0; j++) {
			}
			if (j <= SSH_CIPHER_MAX) {
				buf2[buf2index] = '0' + j;
				buf2index++;
			}
		}
	}
	buf2[buf2index] = 0;
	normalize_cipher_order(buf2);
	strcpy(pvar->settings.CipherOrder, buf2);

	buf[0] = 0;
	GetDlgItemText(dlg, IDC_READWRITEFILENAME, buf, sizeof(buf));
	j = copy_teraterm_dir_relative_path(pvar->settings.KnownHostsFiles,
										sizeof(pvar->settings.
											   KnownHostsFiles), buf);
	buf[0] = 0;
	bufindex = 0;
	GetDlgItemText(dlg, IDC_READONLYFILENAME, buf, sizeof(buf));
	for (i = 0; buf[i] != 0; i++) {
		if (buf[i] == ';') {
			buf[i] = 0;
			if (j < sizeof(pvar->settings.KnownHostsFiles) - 1) {
				pvar->settings.KnownHostsFiles[j] = ';';
				j++;
				j += copy_teraterm_dir_relative_path(pvar->settings.
													 KnownHostsFiles + j,
													 sizeof(pvar->settings.
															KnownHostsFiles)
													 - j, buf + bufindex);
			}
			bufindex = i + 1;
		}
	}
	if (bufindex < i && j < sizeof(pvar->settings.KnownHostsFiles) - 1) {
		pvar->settings.KnownHostsFiles[j] = ';';
		j++;
		copy_teraterm_dir_relative_path(pvar->settings.KnownHostsFiles + j,
										sizeof(pvar->settings.
											   KnownHostsFiles) - j,
										buf + bufindex);
	}

	// get SSH HeartBeat(keep-alive)
	SendMessage(GetDlgItem(dlg, IDC_HEARTBEAT_EDIT), WM_GETTEXT, sizeof(buf), (LPARAM)buf);
	i = atoi(buf);
	if (i < 0)
		i = 60;
	pvar->settings.ssh_heartbeat_overtime = i;

}

static void move_cur_sel_delta(HWND listbox, int delta)
{
	int curPos = (int) SendMessage(listbox, LB_GETCURSEL, 0, 0);
	int maxPos = (int) SendMessage(listbox, LB_GETCOUNT, 0, 0) - 1;
	int newPos = curPos + delta;
	char buf[1024];

	if (curPos >= 0 && newPos >= 0 && newPos <= maxPos) {
		int len = SendMessage(listbox, LB_GETTEXTLEN, curPos, 0);

		if (len > 0 && len < sizeof(buf)) {	/* should always be true */
			buf[0] = 0;
			SendMessage(listbox, LB_GETTEXT, curPos, (LPARAM) buf);
			SendMessage(listbox, LB_DELETESTRING, curPos, 0);
			SendMessage(listbox, LB_INSERTSTRING, newPos,
						(LPARAM) (char FAR *) buf);
			SendMessage(listbox, LB_SETCURSEL, newPos, 0);
		}
	}
}

static int get_keys_file_name(HWND parent, char FAR * buf, int bufsize,
							  int readonly)
{
#ifdef TERATERM32
	OPENFILENAME params;
	char fullname_buf[2048] = "ssh_known_hosts";

	params.lStructSize = sizeof(OPENFILENAME);
	params.hwndOwner = parent;
	params.lpstrFilter = NULL;
	params.lpstrCustomFilter = NULL;
	params.nFilterIndex = 0;
	buf[0] = 0;
	params.lpstrFile = fullname_buf;
	params.nMaxFile = sizeof(fullname_buf);
	params.lpstrFileTitle = NULL;
	params.lpstrInitialDir = NULL;
	params.lpstrTitle =
		readonly ? "Choose a read-only known-hosts file to add" :
		"Choose a read/write known-hosts file";
	params.Flags = (readonly ? OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST : 0)
		| OFN_HIDEREADONLY | (!readonly ? OFN_NOREADONLYRETURN : 0);
	params.lpstrDefExt = NULL;

	if (GetOpenFileName(&params) != 0) {
		copy_teraterm_dir_relative_path(buf, bufsize, fullname_buf);
		return 1;
	} else {
		int err = CommDlgExtendedError();

		if (err != 0) {
			char buf[1024];

			_snprintf(buf, sizeof(buf),
					  "Cannot show file dialog box: error %d", err);
			buf[sizeof(buf) - 1] = 0;
			MessageBox(parent, buf, "TTSSH Error",
					   MB_OK | MB_ICONEXCLAMATION);
		}

		return 0;
	}
#else
	return 0;
#endif
}

static void choose_read_write_file(HWND dlg)
{
	char buf[1024];

	if (get_keys_file_name(dlg, buf, sizeof(buf), 0)) {
		SetDlgItemText(dlg, IDC_READWRITEFILENAME, buf);
	}
}

static void choose_read_only_file(HWND dlg)
{
	char buf[1024];
	char buf2[4096];

	if (get_keys_file_name(dlg, buf, sizeof(buf), 1)) {
		buf2[0] = 0;
		GetDlgItemText(dlg, IDC_READONLYFILENAME, buf2, sizeof(buf2));
		if (buf2[0] != 0 && buf2[strlen(buf2) - 1] != ';') {
			strncat(buf2, ";", sizeof(buf2));
		}
		strncat(buf2, buf, sizeof(buf2));
		SetDlgItemText(dlg, IDC_READONLYFILENAME, buf2);
	}
}

static BOOL CALLBACK TTXSetupDlg(HWND dlg, UINT msg, WPARAM wParam,
								 LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		SetWindowLong(dlg, DWL_USER, lParam);
		init_setup_dlg((PTInstVar) lParam, dlg);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			complete_setup_dlg((PTInstVar) GetWindowLong(dlg, DWL_USER),
							   dlg);
			EndDialog(dlg, 1);
			return TRUE;
		case IDCANCEL:			/* there isn't a cancel button, but other Windows
								   UI things can send this message */
			EndDialog(dlg, 0);
			return TRUE;
		case IDC_SSHMOVECIPHERUP:
			move_cur_sel_delta(GetDlgItem(dlg, IDC_SSHCIPHERPREFS), -1);
			set_move_button_status(dlg);
			SetFocus(GetDlgItem(dlg, IDC_SSHCIPHERPREFS));
			return TRUE;
		case IDC_SSHMOVECIPHERDOWN:
			move_cur_sel_delta(GetDlgItem(dlg, IDC_SSHCIPHERPREFS), 1);
			set_move_button_status(dlg);
			SetFocus(GetDlgItem(dlg, IDC_SSHCIPHERPREFS));
			return TRUE;
		case IDC_SSHCIPHERPREFS:
			set_move_button_status(dlg);
			return TRUE;
		case IDC_CHOOSEREADWRITEFILE:
			choose_read_write_file(dlg);
			return TRUE;
		case IDC_CHOOSEREADONLYFILE:
			choose_read_only_file(dlg);
			return TRUE;
		}
		break;
	}

	return FALSE;
}


//
// SSH key generator dialog (2005.4.10 yutaka)
//

typedef struct {
	RSA *rsa;
	DSA *dsa;
} ssh_private_key_t;

static ssh_private_key_t private_key = {NULL, NULL};

typedef struct {
	RSA *rsa;
	DSA *dsa;
} ssh_public_key_t;

static ssh_public_key_t public_key = {NULL, NULL};;

static void free_ssh_key(void)
{
	// DSA_free(), RSA_free()にNULLを渡しても問題はなし。
	DSA_free(private_key.dsa);
	private_key.dsa = NULL;
	DSA_free(public_key.dsa);
	public_key.dsa = NULL;

	RSA_free(private_key.rsa);
	private_key.rsa = NULL;
	RSA_free(public_key.rsa);
	public_key.rsa = NULL;
}


static BOOL generate_ssh_key(enum hostkey_type type)
{
	int bits = 1024;

	// if SSH key already is generated, should free the resource.
	free_ssh_key();

	if (type == KEY_RSA1 || type == KEY_RSA) {
		RSA *priv = NULL;
		RSA *pub = NULL;

		// private key
		priv =  RSA_generate_key(bits, 35, NULL, NULL);
		if (priv == NULL)
			goto error;
		private_key.rsa = priv;

		// public key
		pub = RSA_new();
		pub->n = BN_new();
		pub->e = BN_new();
		if (pub->n == NULL || pub->e == NULL) {
			RSA_free(pub);
			goto error;
		}

		BN_copy(pub->n, priv->n);
		BN_copy(pub->e, priv->e);
		public_key.rsa = pub;

	} else if (type == KEY_DSA) {
		DSA *priv = NULL;
		DSA *pub = NULL;

		// private key
		priv = DSA_generate_parameters(bits, NULL, 0, NULL, NULL, NULL, NULL);
		if (priv == NULL)
			goto error;
		if (!DSA_generate_key(priv)) {
			// TODO: free 'priv'?
			goto error;
		}
		private_key.dsa = priv;

		// public key
		pub = DSA_new();
		if (pub == NULL)
			goto error;
		pub->p = BN_new();
		pub->q = BN_new();
		pub->g = BN_new();
		pub->pub_key = BN_new();
		if (pub->p == NULL || pub->q == NULL || pub->g == NULL || pub->pub_key == NULL) {
			DSA_free(pub);
			goto error;
		}

		BN_copy(pub->p, priv->p);
		BN_copy(pub->q, priv->q);
		BN_copy(pub->g, priv->g);
		BN_copy(pub->pub_key, priv->pub_key);
		public_key.dsa = pub;

	} else {
		goto error;
	}

	return TRUE;

error:
	free_ssh_key();
	return FALSE;
}


//
// RC4 
//

/* Size of key to use */
#define SEED_SIZE 20

/* Number of bytes to reseed after */
#define REKEY_BYTES (1 << 24)

static int rc4_ready = 0;
static RC4_KEY rc4;

static void seed_rng(void)
{
    if (RAND_status() != 1)
		return;
}

static void arc4random_stir(void)
{
    unsigned char rand_buf[SEED_SIZE];
    int i;

    memset(&rc4, 0, sizeof(rc4));
	if (RAND_bytes(rand_buf, sizeof(rand_buf)) <= 0) {
        //fatal("Couldn't obtain random bytes (error %ld)",
        //    ERR_get_error());
	}
    RC4_set_key(&rc4, sizeof(rand_buf), rand_buf);

    /*
     * Discard early keystream, as per recommendations in:
     * http://www.wisdom.weizmann.ac.il/~itsik/RC4/Papers/Rc4_ksa.ps
     */
    for(i = 0; i <= 256; i += sizeof(rand_buf))
        RC4(&rc4, sizeof(rand_buf), rand_buf, rand_buf);

    memset(rand_buf, 0, sizeof(rand_buf));

    rc4_ready = REKEY_BYTES;
}

static unsigned int arc4random(void)
{
    unsigned int r = 0;
    static int first_time = 1;

    if (rc4_ready <= 0) {
		if (first_time) {
            seed_rng();
		}
        first_time = 0;
        arc4random_stir();
    }

    RC4(&rc4, sizeof(r), (unsigned char *)&r, (unsigned char *)&r);

    rc4_ready -= sizeof(r);

    return(r);
}

//
// SSH1 3DES
//
/*
 * This is used by SSH1:
 *
 * What kind of triple DES are these 2 routines?
 *
 * Why is there a redundant initialization vector?
 *
 * If only iv3 was used, then, this would till effect have been
 * outer-cbc. However, there is also a private iv1 == iv2 which
 * perhaps makes differential analysis easier. On the other hand, the
 * private iv1 probably makes the CRC-32 attack ineffective. This is a
 * result of that there is no longer any known iv1 to use when
 * choosing the X block.
 */
struct ssh1_3des_ctx
{
        EVP_CIPHER_CTX  k1, k2, k3;
};

static int ssh1_3des_init(EVP_CIPHER_CTX *ctx, const u_char *key, const u_char *iv, int enc)
{
	struct ssh1_3des_ctx *c;
	u_char *k1, *k2, *k3;

	if ((c = EVP_CIPHER_CTX_get_app_data(ctx)) == NULL) {
		c = malloc(sizeof(*c));
		EVP_CIPHER_CTX_set_app_data(ctx, c);
	}
	if (key == NULL)
		return (1);
	if (enc == -1)
		enc = ctx->encrypt;
	k1 = k2 = k3 = (u_char *) key;
	k2 += 8;
	if (EVP_CIPHER_CTX_key_length(ctx) >= 16+8) {
		if (enc)
			k3 += 16;
		else
			k1 += 16;
	}
	EVP_CIPHER_CTX_init(&c->k1);
	EVP_CIPHER_CTX_init(&c->k2);
	EVP_CIPHER_CTX_init(&c->k3);
	if (EVP_CipherInit(&c->k1, EVP_des_cbc(), k1, NULL, enc) == 0 ||
		EVP_CipherInit(&c->k2, EVP_des_cbc(), k2, NULL, !enc) == 0 ||
		EVP_CipherInit(&c->k3, EVP_des_cbc(), k3, NULL, enc) == 0) {
			memset(c, 0, sizeof(*c));
			free(c);
			EVP_CIPHER_CTX_set_app_data(ctx, NULL);
			return (0);
	}
	return (1);
}

static int ssh1_3des_cbc(EVP_CIPHER_CTX *ctx, u_char *dest, const u_char *src, u_int len)
{
	struct ssh1_3des_ctx *c;

	if ((c = EVP_CIPHER_CTX_get_app_data(ctx)) == NULL) {
		//error("ssh1_3des_cbc: no context");
		return (0);
	}
	if (EVP_Cipher(&c->k1, dest, (u_char *)src, len) == 0 ||
		EVP_Cipher(&c->k2, dest, dest, len) == 0 ||
		EVP_Cipher(&c->k3, dest, dest, len) == 0)
		return (0);
	return (1);
}

static int ssh1_3des_cleanup(EVP_CIPHER_CTX *ctx)
{
	struct ssh1_3des_ctx *c;

	if ((c = EVP_CIPHER_CTX_get_app_data(ctx)) != NULL) {
		EVP_CIPHER_CTX_cleanup(&c->k1);
		EVP_CIPHER_CTX_cleanup(&c->k2);
		EVP_CIPHER_CTX_cleanup(&c->k3);
		memset(c, 0, sizeof(*c));
		free(c);
		EVP_CIPHER_CTX_set_app_data(ctx, NULL);
	}
	return (1);
}

void ssh1_3des_iv(EVP_CIPHER_CTX *evp, int doset, u_char *iv, int len)
{
	struct ssh1_3des_ctx *c;

	if (len != 24)
		//fatal("%s: bad 3des iv length: %d", __func__, len);
		;

	if ((c = EVP_CIPHER_CTX_get_app_data(evp)) == NULL)
		//fatal("%s: no 3des context", __func__);
		;

	if (doset) {
		//debug3("%s: Installed 3DES IV", __func__);
		memcpy(c->k1.iv, iv, 8);
		memcpy(c->k2.iv, iv + 8, 8);
		memcpy(c->k3.iv, iv + 16, 8);
	} else {
		//debug3("%s: Copying 3DES IV", __func__);
		memcpy(iv, c->k1.iv, 8);
		memcpy(iv + 8, c->k2.iv, 8);
		memcpy(iv + 16, c->k3.iv, 8);
	}
}

const EVP_CIPHER *evp_ssh1_3des(void)
{
	static EVP_CIPHER ssh1_3des;

	memset(&ssh1_3des, 0, sizeof(EVP_CIPHER));
	ssh1_3des.nid = NID_undef;
	ssh1_3des.block_size = 8;
	ssh1_3des.iv_len = 0;
	ssh1_3des.key_len = 16;
	ssh1_3des.init = ssh1_3des_init;
	ssh1_3des.cleanup = ssh1_3des_cleanup;
	ssh1_3des.do_cipher = ssh1_3des_cbc;
	ssh1_3des.flags = EVP_CIPH_CBC_MODE | EVP_CIPH_VARIABLE_LENGTH;
	return (&ssh1_3des);
}

static void ssh_make_comment(char *comment, int maxlen)
{
	char user[UNLEN + 1], host[128];
	DWORD dwSize;
	WSADATA wsaData;
	int ret;

	// get Windows logon user name
	dwSize = sizeof(user);
	if (GetUserName(user, &dwSize) == 0) {
		strcpy(user, "yutaka");
	}

	// get local hostname (by WinSock)
	ret = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (ret == 0) {
		if (gethostname(host, sizeof(host)) != 0) {
			ret = WSAGetLastError();
		}
		WSACleanup();
	}
	if (ret != 0) {
		strcpy(host, "sai");
	}

	_snprintf(comment, maxlen, "%s@%s", user, host);
}

// uuencode (rfc1521)
int uuencode(unsigned char *src, int srclen, unsigned char *target, int targsize)
{
	char base64[] ="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	char pad = '=';
    int datalength = 0;
    unsigned char input[3];
    unsigned char output[4];
    int i;

    while (srclen > 2) {
        input[0] = *src++;
        input[1] = *src++;
        input[2] = *src++;
        srclen -= 3;

        output[0] = input[0] >> 2;
        output[1] = ((input[0] & 0x03) << 4) + (input[1] >> 4);
        output[2] = ((input[1] & 0x0f) << 2) + (input[2] >> 6);
        output[3] = input[2] & 0x3f;
		if (output[0] >= 64 || 
			output[1] >= 64 ||
			output[2] >= 64 ||
			output[3] >= 64)
			return -1;

        if (datalength + 4 > targsize)
            return (-1);
        target[datalength++] = base64[output[0]];
        target[datalength++] = base64[output[1]];
        target[datalength++] = base64[output[2]];
        target[datalength++] = base64[output[3]];
    }

    if (srclen != 0) {
        /* Get what's left. */
        input[0] = input[1] = input[2] = '\0';
        for (i = 0; i < srclen; i++)
            input[i] = *src++;

        output[0] = input[0] >> 2;
        output[1] = ((input[0] & 0x03) << 4) + (input[1] >> 4);
        output[2] = ((input[1] & 0x0f) << 2) + (input[2] >> 6);
		if (output[0] >= 64 || 
			output[1] >= 64 ||
			output[2] >= 64)
			return -1;

        if (datalength + 4 > targsize)
            return (-1);
        target[datalength++] = base64[output[0]];
        target[datalength++] = base64[output[1]];
        if (srclen == 1)
            target[datalength++] = pad;
        else
            target[datalength++] = base64[output[2]];
        target[datalength++] = pad;
    }
    if (datalength >= targsize)
        return (-1);
    target[datalength] = '\0';  /* Returned value doesn't count \0. */

	return (datalength); // success
}

static BOOL CALLBACK TTXKeyGenerator(HWND dlg, UINT msg, WPARAM wParam,
								 LPARAM lParam)
{
	static enum hostkey_type key_type;

	switch (msg) {
	case WM_INITDIALOG: 
		{
		// default key type
		SendMessage(GetDlgItem(dlg, IDC_RSA_TYPE), BM_SETCHECK, BST_CHECKED, 0);
		key_type = KEY_RSA;

		// passphrase edit box disabled(default)
		EnableWindow(GetDlgItem(dlg, IDC_KEY_EDIT), FALSE);
		EnableWindow(GetDlgItem(dlg, IDC_CONFIRM_EDIT), FALSE);

		// file saving dialog disabled(default)
		EnableWindow(GetDlgItem(dlg, IDC_SAVE_PUBLIC_KEY), FALSE);
		EnableWindow(GetDlgItem(dlg, IDC_SAVE_PRIBATE_KEY), FALSE);

		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK: // key generate button pressed
			// passphrase edit box disabled(default)
			EnableWindow(GetDlgItem(dlg, IDC_KEY_EDIT), FALSE);
			EnableWindow(GetDlgItem(dlg, IDC_CONFIRM_EDIT), FALSE);

			// file saving dialog disabled(default)
			EnableWindow(GetDlgItem(dlg, IDC_SAVE_PUBLIC_KEY), FALSE);
			EnableWindow(GetDlgItem(dlg, IDC_SAVE_PRIBATE_KEY), FALSE);

			if (generate_ssh_key(key_type)) {
				// passphrase edit box disabled(default)
				EnableWindow(GetDlgItem(dlg, IDC_KEY_EDIT), TRUE);
				EnableWindow(GetDlgItem(dlg, IDC_CONFIRM_EDIT), TRUE);

				// file saving dialog disabled(default)
				EnableWindow(GetDlgItem(dlg, IDC_SAVE_PUBLIC_KEY), TRUE);
				EnableWindow(GetDlgItem(dlg, IDC_SAVE_PRIBATE_KEY), TRUE);
			}
			return TRUE;

		case IDCANCEL:			
			// don't forget to free SSH resource!
			free_ssh_key();
			EndDialog(dlg, 0); // dialog close
			return TRUE;

		// if radio button pressed...
		case IDC_RSA1_TYPE | (BN_CLICKED << 16):
			key_type = KEY_RSA1;
			break;

		case IDC_RSA_TYPE | (BN_CLICKED << 16):
			key_type = KEY_RSA;
			break;

		case IDC_DSA_TYPE | (BN_CLICKED << 16):
			key_type = KEY_DSA;
			break;

		// saving public key file
		case IDC_SAVE_PUBLIC_KEY:
			{
			int ret;
			OPENFILENAME ofn;
			char filename[MAX_PATH];
			FILE *fp;
			char comment[1024]; // comment string in private key

			arc4random_stir();

			// saving file dialog
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = dlg;
			if (key_type == KEY_RSA1) {
				ofn.lpstrFilter = "SSH1 RSA key(identity.pub)\0identity.pub\0All Files(*.*)\0*.*\0\0";
				_snprintf(filename, sizeof(filename), "identity.pub");
			} else if (key_type == KEY_RSA) {
				ofn.lpstrFilter = "SSH2 RSA key(id_rsa.pub)\0id_rsa.pub\0All Files(*.*)\0*.*\0\0";
				_snprintf(filename, sizeof(filename), "id_rsa.pub");
			} else {
				ofn.lpstrFilter = "SSH2 DSA key(id_dsa.pub)\0id_dsa.pub\0All Files(*.*)\0*.*\0\0";
				_snprintf(filename, sizeof(filename), "id_dsa.pub");
			}
			ofn.lpstrFile = filename;
			ofn.nMaxFile = sizeof(filename);
			ofn.lpstrTitle = "Save public key as:";
			if (GetSaveFileName(&ofn) == 0) { // failure
				ret = CommDlgExtendedError();
				break;
			}

			ssh_make_comment(comment, sizeof(comment));

			// saving public key file
			fp = fopen(filename, "wb");
			if (fp == NULL) {
				MessageBox(dlg, "Can't open key file", "ERROR", MB_OK | MB_ICONEXCLAMATION);
				break;
			}

			if (key_type == KEY_RSA1) { // SSH1 RSA
				RSA *rsa = public_key.rsa;
				int bits;
				char *buf;

				bits = BN_num_bits(rsa->n);
				fprintf(fp, "%u", bits);

				buf = BN_bn2dec(rsa->e);
				fprintf(fp, " %s", buf);
				OPENSSL_free(buf);

				buf = BN_bn2dec(rsa->n);
				fprintf(fp, " %s", buf);
				OPENSSL_free(buf);

			} else { // SSH2 RSA, DSA
				buffer_t *b;
				char *keyname;
				DSA *dsa = public_key.dsa;
				RSA *rsa = public_key.rsa;
				int len;
				char *blob;
				char *uuenc; // uuencode data
				int uulen;

				b = buffer_init();
				if (b == NULL)
					goto public_error;

				if (key_type == KEY_DSA) { // DSA
					keyname = "ssh-dss";
					buffer_put_string(b, keyname, strlen(keyname));
					buffer_put_bignum2(b, dsa->p);
					buffer_put_bignum2(b, dsa->q);
					buffer_put_bignum2(b, dsa->g);
					buffer_put_bignum2(b, dsa->pub_key);

				} else { // RSA
					keyname = "ssh-rsa";
					buffer_put_string(b, keyname, strlen(keyname));
					buffer_put_bignum2(b, rsa->e);
					buffer_put_bignum2(b, rsa->n);
				}
				
				blob = buffer_ptr(b);
				len = buffer_len(b);
				uuenc = malloc(len * 2);
				if (uuenc == NULL) {
					buffer_free(b);
					goto public_error;
				}
				uulen = uuencode(blob, len, uuenc, len * 2);
				if (uulen > 0) {
					fprintf(fp, "%s %s", keyname, uuenc);
				}
				free(uuenc);
				buffer_free(b);
			}

			// writing a comment(+LF)
			fprintf(fp, " %s", comment);
			fputc(0x0a, fp);

public_error:
			fclose(fp);

			}
			break;

		// saving private key file
		case IDC_SAVE_PRIVATE_KEY: 
			{
			char buf[1024], buf_conf[1024];  // passphrase 
			int ret;
			OPENFILENAME ofn;
			char filename[MAX_PATH];
			char comment[1024]; // comment string in private key

			// パスフレーズのチェックを行う。パスフレーズは秘密鍵ファイルに付ける。
			SendMessage(GetDlgItem(dlg, IDC_KEY_EDIT), WM_GETTEXT, sizeof(buf), (LPARAM)buf);
			SendMessage(GetDlgItem(dlg, IDC_CONFIRM_EDIT), WM_GETTEXT, sizeof(buf_conf), (LPARAM)buf_conf);

			// check matching
			if (strcmp(buf, buf_conf) != 0) {
				MessageBox(dlg, "Two passphrases don't match.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
				break;
			}

			// check empty-passphrase (this is warning level)
			if (buf[0] == '\0') {
				ret = MessageBox(dlg, "Are you sure that you want to use a empty passphrase?", "WARNING", MB_YESNO | MB_ICONWARNING);
				if (ret == IDNO)
					break;
			}

			ssh_make_comment(comment, sizeof(comment));

			// saving file dialog
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = dlg;
			if (key_type == KEY_RSA1) {
				ofn.lpstrFilter = "SSH1 RSA key(identity)\0identity\0All Files(*.*)\0*.*\0\0";
				_snprintf(filename, sizeof(filename), "identity");
			} else if (key_type == KEY_RSA) {
				ofn.lpstrFilter = "SSH2 RSA key(id_rsa)\0id_rsa\0All Files(*.*)\0*.*\0\0";
				_snprintf(filename, sizeof(filename), "id_rsa");
			} else {
				ofn.lpstrFilter = "SSH2 DSA key(id_dsa)\0id_dsa\0All Files(*.*)\0*.*\0\0";
				_snprintf(filename, sizeof(filename), "id_dsa");
			}
			ofn.lpstrFile = filename;
			ofn.nMaxFile = sizeof(filename);
			ofn.lpstrTitle = "Save private key as:";
			if (GetSaveFileName(&ofn) == 0) { // failure
				ret = CommDlgExtendedError();
				break;
			}

			// saving private key file
			if (key_type == KEY_RSA1) { // SSH1 RSA
				int cipher_num;
				buffer_t *b, *enc;
				unsigned int rnd;
				unsigned char tmp[128];
				RSA *rsa;
				int i, len;
				char authfile_id_string[] = "SSH PRIVATE KEY FILE FORMAT 1.1";
				MD5_CTX md;
				unsigned char digest[16];
				char *passphrase = buf;
				EVP_CIPHER_CTX cipher_ctx;
				FILE *fp;
				char wrapped[4096];

				if (passphrase[0] == '\0') { // passphrase is empty
					cipher_num = SSH_CIPHER_NONE;
				} else {
					cipher_num = SSH_CIPHER_3DES; // 3DES-CBC
				}

				b = buffer_init();
				if (b == NULL)
					break;
				enc = buffer_init();
				if (enc == NULL) {
					buffer_free(b);
					break;
				}

				// set random value
				rnd = arc4random();
				tmp[0] = rnd & 0xff;
				tmp[1] = (rnd >> 8) & 0xff;
				tmp[2] = tmp[0];
				tmp[3] = tmp[1];
				buffer_append(b, tmp, 4);

				// set private key
				rsa = private_key.rsa;
				buffer_put_bignum(b, rsa->d);
				buffer_put_bignum(b, rsa->iqmp);
				buffer_put_bignum(b, rsa->q);
				buffer_put_bignum(b, rsa->p);

				// padding with 8byte align
				while (buffer_len(b) % 8) {
					buffer_put_char(b, 0);
				} 

				//
				// step(2)
				//
				// encrypted buffer
			    /* First store keyfile id string. */
				for (i = 0 ; authfile_id_string[i] ; i++) {
					buffer_put_char(enc, authfile_id_string[i]);
				}
				buffer_put_char(enc, 0x0a); // LF
				buffer_put_char(enc, 0);

				/* Store cipher type. */
				buffer_put_char(enc, cipher_num);
				buffer_put_int(enc, 0);  // type is 'int'!! (For future extension)

				/* Store public key.  This will be in plain text. */
				buffer_put_int(enc, BN_num_bits(rsa->n));
				buffer_put_bignum(enc, rsa->n);
				buffer_put_bignum(enc, rsa->e);
				buffer_put_string(enc, comment, strlen(comment));

				// setup the MD5ed passphrase to cipher encryption key
				MD5_Init(&md);
				MD5_Update(&md, (const unsigned char *)passphrase, strlen(passphrase));
				MD5_Final(digest, &md);
				if (cipher_num == SSH_CIPHER_NONE) {
					cipher_init_SSH2(&cipher_ctx, digest, 16, NULL, 0, CIPHER_ENCRYPT, EVP_enc_null);
				} else {
					cipher_init_SSH2(&cipher_ctx, digest, 16, NULL, 0, CIPHER_ENCRYPT, evp_ssh1_3des);
				}
				len = buffer_len(b);
				if (len % 8) { // fatal error
					goto error;
				}

				// check buffer overflow
				if (buffer_overflow_verify(enc, len) && (sizeof(wrapped) < len)) {
					goto error;
				}

				if (EVP_Cipher(&cipher_ctx, wrapped, buffer_ptr(b), len) == 0) {
					goto error;
				}
				if (EVP_CIPHER_CTX_cleanup(&cipher_ctx) == 0) {
					goto error;
				}

				buffer_append(enc, wrapped, len);

				// saving private key file (binary mode)
				fp = fopen(filename, "wb");
				if (fp == NULL) {
					MessageBox(dlg, "Can't open key file", "ERROR", MB_OK | MB_ICONEXCLAMATION);
					break;
				}
				fwrite(buffer_ptr(enc), buffer_len(enc), 1, fp);

				fclose(fp);

error:;
				buffer_free(b);
				buffer_free(enc);

			} else { // SSH2 RSA, DSA
				int len;
				FILE *fp;
				const EVP_CIPHER *cipher;

				len = strlen(buf);
				// TODO: range check (len >= 4)

				cipher = NULL;
				if (len > 0) {
					cipher = EVP_des_ede3_cbc();
				}

				fp = fopen(filename, "w");
				if (fp == NULL) {
					MessageBox(dlg, "Can't open key file", "ERROR", MB_OK | MB_ICONEXCLAMATION);
					break;
				}

				if (key_type == KEY_RSA) { // RSA
					ret = PEM_write_RSAPrivateKey(fp, private_key.rsa, cipher, buf, len, NULL, NULL);
				} else { // DSA
					ret = PEM_write_DSAPrivateKey(fp, private_key.dsa, cipher, buf, len, NULL, NULL);
				}
				if (ret == 0) {
					MessageBox(dlg, "Can't write key file", "ERROR", MB_OK | MB_ICONEXCLAMATION);
				}
				fclose(fp);
			}


			}
			break;

		}
		break;
	}

	return FALSE;
}


static int PASCAL FAR TTXProcessCommand(HWND hWin, WORD cmd)
{
	GET_VAR();

	if (pvar->fatal_error) {
		return 0;
	}

	switch (cmd) {
	case ID_SSHKEYGENMENU: 
		if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SSHKEYGEN), hWin, TTXKeyGenerator,
			(LPARAM) pvar) == -1) {
			MessageBox(hWin, "Cannot create Key Generator window.",
				"TTSSH Error", MB_OK | MB_ICONEXCLAMATION);
		}
		return 1;

	case ID_ABOUTMENU:
		if (DialogBoxParam
			(hInst, MAKEINTRESOURCE(IDD_ABOUTDIALOG), hWin, TTXAboutDlg,
			 (LPARAM) pvar)
			== -1) {
			MessageBox(hWin, "Cannot create About box window.",
					   "TTSSH Error", MB_OK | MB_ICONEXCLAMATION);
		}
		return 1;
	case ID_SSHAUTH:
		AUTH_do_cred_dialog(pvar);
		return 1;
	case ID_SSHSETUPMENU:
		if (DialogBoxParam
			(hInst, MAKEINTRESOURCE(IDD_SSHSETUP), hWin, TTXSetupDlg,
			 (LPARAM) pvar)
			== -1) {
			MessageBox(hWin, "Cannot create TTSSH Setup window.",
					   "TTSSH Error", MB_OK | MB_ICONEXCLAMATION);
		}
		return 1;
	case ID_SSHAUTHSETUPMENU:
		AUTH_do_default_cred_dialog(pvar);
		return 1;
	case ID_SSHFWDSETUPMENU:
		FWDUI_do_forwarding_dialog(pvar);
		return 1;
	case ID_SSHUNKNOWNHOST:
		HOSTS_do_unknown_host_dialog(hWin, pvar);
		return 1;
	case ID_SSHDIFFERENTHOST:
		HOSTS_do_different_host_dialog(hWin, pvar);
		return 1;
	case ID_SSHASYNCMESSAGEBOX:
		if (pvar->err_msg != NULL) {
			char FAR *msg = pvar->err_msg;

			/* Could there be a buffer overrun bug anywhere in Win32
			   MessageBox? Who knows? I'm paranoid. */
			if (strlen(msg) > 2048) {
				msg[2048] = 0;
			}

			pvar->showing_err = TRUE;
			pvar->err_msg = NULL;
#if 1
			// XXX: "SECURITY WARINIG" dialogで ESC キーを押下すると、
			// なぜかアプリケーションエラーとなるため、下記APIは削除。(2004.12.16 yutaka)
			if (!SSHv1(pvar)) {
				MessageBox(NULL, msg, "TTSSH",
						MB_TASKMODAL | MB_ICONEXCLAMATION);
			}
#else
			MessageBox(NULL, msg, "TTSSH",
					   MB_TASKMODAL | MB_ICONEXCLAMATION);
#endif
			free(msg);
			pvar->showing_err = FALSE;

			if (pvar->err_msg != NULL) {
				PostMessage(hWin, WM_COMMAND, ID_SSHASYNCMESSAGEBOX, 0);
			} else {
				AUTH_notify_end_error(pvar);
			}
		}
		return 1;
	default:
		return 0;
	}
}


// 以下はTeraTerm Menuのコード(ttpmenu.cpp)と同一。
// 空白を @ に置き換える。@自身は@@にする。(2005.1.28 yutaka)
static void replace_blank_to_mark(char *str, char *dst, int dst_len)
{
	int i, len, n;

	len = strlen(str);
	n = 0;
	for (i = 0 ; i < len ; i++) {
		if (str[i] == '@')
			n++;
	}
	if (dst_len < (len + 2*n)) 
		return;

	for (i = 0 ; i < len ; i++) {
		if (str[i] == '@') {
			*dst++ = '@';
			*dst++ = '@';

		} else if (str[i] == ' ') {
			*dst++ = '@';

		} else {
			*dst++ = str[i];

		}
	}
	*dst = '\0';

}

static void PASCAL FAR TTXSetCommandLine(PCHAR cmd, int cmdlen,
										 PGetHNRec rec)
{
	char tmpFile[MAX_PATH];
	char tmpPath[1024];
	char buf[1024];
	int i;
	GET_VAR();

	GetTempPath(sizeof(tmpPath), tmpPath);
	GetTempFileName(tmpPath, "TTX", 0, tmpFile);

	for (i = 0; cmd[i] != ' ' && cmd[i] != 0; i++) {
	}

	if (i < cmdlen) {
		strncpy(buf, cmd + i, sizeof(buf));
		cmd[i] = 0;

		write_ssh_options(pvar, tmpFile, &pvar->settings);

		strncat(cmd, " /ssh-consume=", cmdlen);
		strncat(cmd, tmpFile, cmdlen);

		strncat(cmd, buf, cmdlen);

		if (pvar->hostdlg_Enabled) {
			strncat(cmd, " /ssh", cmdlen);

			// add option of SSH protcol version (2004.10.11 yutaka)
			if (pvar->settings.ssh_protocol_version == 2) {
				strncat(cmd, " /2", cmdlen);
			} else {
				strncat(cmd, " /1", cmdlen);
			}

		}

		// セッション複製の場合は、自動ログイン用パラメータを付ける。(2005.4.8 yutaka)
		if (strstr(buf, "DUPLICATE")) {
			char mark[MAX_PATH];
			char tmp[MAX_PATH*2];

			// 自動ログインの場合は下記フラグが0のため、必要なコマンドを付加する。
			if (!pvar->hostdlg_Enabled) {
				_snprintf(tmp, sizeof(tmp), " /ssh /%d", pvar->settings.ssh_protocol_version);
				strncat(cmd, tmp, cmdlen);
			}

			// パスワードを覚えている場合のみ、コマンドラインに渡す。(2006.8.3 yutaka)
			if (pvar->settings.remember_password &&
				pvar->auth_state.cur_cred.method == SSH_AUTH_PASSWORD) {
				replace_blank_to_mark(pvar->auth_state.cur_cred.password, mark, sizeof(mark));
				_snprintf(tmp, sizeof(tmp), " /auth=password /user=%s /passwd=%s", pvar->auth_state.user, mark);
				strncat(cmd, tmp, cmdlen);

			} else if (pvar->settings.remember_password &&
						pvar->auth_state.cur_cred.method == SSH_AUTH_RSA) {
				replace_blank_to_mark(pvar->auth_state.cur_cred.password, mark, sizeof(mark));
				_snprintf(tmp, sizeof(tmp), " /auth=publickey /user=%s /passwd=%s", pvar->auth_state.user, mark);
				strncat(cmd, tmp, cmdlen);

				replace_blank_to_mark(pvar->session_settings.DefaultRSAPrivateKeyFile, mark, sizeof(mark));
				_snprintf(tmp, sizeof(tmp), " /keyfile=%s", mark);
				strncat(cmd, tmp, cmdlen);

			} else if (pvar->auth_state.cur_cred.method == SSH_AUTH_TIS) {
				// keyboard-interactive認証の場合は何もしない。

			} else {
				// don't come here

			}

		}

	}
}

/* This function is called when Teraterm is quitting. You can use it to clean
   up.

   This function is called for each extension, in reverse load order (see
   below).
*/
static void PASCAL FAR TTXEnd(void)
{
	GET_VAR();

	uninit_TTSSH(pvar);

	if (pvar->err_msg != NULL) {
		/* Could there be a buffer overrun bug anywhere in Win32
		   MessageBox? Who knows? I'm paranoid. */
		if (strlen(pvar->err_msg) > 2048) {
			pvar->err_msg[2048] = 0;
		}

		MessageBox(NULL, pvar->err_msg, "TTSSH",
				   MB_TASKMODAL | MB_ICONEXCLAMATION);

		free(pvar->err_msg);
		pvar->err_msg = NULL;
	}
#ifndef TERATERM32
	DelVar();
#endif
}

/* This record contains all the information that the extension forwards to the
   main Teraterm code. It mostly consists of pointers to the above functions.
   Any of the function pointers can be replaced with NULL, in which case
   Teraterm will just ignore that function and assume default behaviour, which
   means "do nothing".
*/
static TTXExports Exports = {
/* This must contain the size of the structure. See below for its usage. */
	sizeof(TTXExports),
	ORDER,

/* Now we just list the functions that we've implemented. */
	TTXInit,
	TTXGetUIHooks,
	TTXGetSetupHooks,
	TTXOpenTCP,
	TTXCloseTCP,
	TTXSetWinSize,
	TTXModifyMenu,
	NULL,
	TTXProcessCommand,
	TTXEnd,
	TTXSetCommandLine
};

#ifdef TERATERM32
BOOL __declspec(dllexport)
PASCAL FAR TTXBind(WORD Version, TTXExports FAR * exports)
{
#else
BOOL __export PASCAL FAR TTXBind(WORD Version, TTXExports FAR * exports)
{
#endif
	int size = sizeof(Exports) - sizeof(exports->size);
	/* do version checking if necessary */
	/* if (Version!=TTVERSION) return FALSE; */

	if (size > exports->size) {
		size = exports->size;
	}
	memcpy((char FAR *) exports + sizeof(exports->size),
		   (char FAR *) &Exports + sizeof(exports->size), size);
	return TRUE;
}

#ifdef TERATERM32
static HANDLE __mem_mapping = NULL;

BOOL WINAPI DllMain(HANDLE hInstance,
					ULONG ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call) {
	case DLL_THREAD_ATTACH:
		/* do thread initialization */
		break;
	case DLL_THREAD_DETACH:
		/* do thread cleanup */
		break;
	case DLL_PROCESS_ATTACH:
		/* do process initialization */
		DisableThreadLibraryCalls(hInstance);
		hInst = hInstance;
		pvar = &InstVar;
		__mem_mapping =
			CreateFileMapping((HANDLE) 0xFFFFFFFF, NULL, PAGE_READWRITE, 0,
							  sizeof(TS_SSH), "TTSSH_1-4_TS_data");
		if (__mem_mapping == NULL) {
			/* fake it. The settings won't be shared, but what the heck. */
			pvar->ts_SSH = NULL;
		} else {
			pvar->ts_SSH =
				(TS_SSH *) MapViewOfFile(__mem_mapping, FILE_MAP_WRITE, 0,
										 0, 0);
		}
		if (pvar->ts_SSH == NULL) {
			/* fake it. The settings won't be shared, but what the heck. */
			pvar->ts_SSH = (TS_SSH *) malloc(sizeof(TS_SSH));
			if (__mem_mapping != NULL) {
				CloseHandle(__mem_mapping);
			}
		}
		break;
	case DLL_PROCESS_DETACH:
		/* do process cleanup */
		if (__mem_mapping == NULL) {
			free(pvar->ts_SSH);
		} else {
			CloseHandle(__mem_mapping);
			UnmapViewOfFile(pvar->ts_SSH);
		}
		break;
	}
	return TRUE;
}
#else
#ifdef WATCOM
#pragma off (unreferenced);
#endif
int CALLBACK LibMain(HANDLE hInstance, WORD wDataSegment,
					 WORD wHeapSize, LPSTR lpszCmdLine)
#ifdef WATCOM
#pragma on (unreferenced);
#endif
{
	int i;
	for (i = 0; i < MAXNUMINST; i++)
		TaskList[i] = NULL;
	hInst = hInstance;
	return (1);
}
#endif


/*
 * $Log: not supported by cvs2svn $
 * Revision 1.34  2006/08/05 03:47:49  yutakakn
 * パスワードをメモリ上に覚えておくかどうかの設定は teraterm.ini に反映させるようにした。
 *
 * Revision 1.33  2006/08/03 15:04:37  yutakakn
 * パスワードをメモリ上に保持するかどうかを決めるチェックボックスを認証ダイアログに追加した。
 *
 * Revision 1.32  2006/06/26 13:26:49  yutakakn
 * TTSSHのsetupダイアログの変更内容が次回接続時から反映されるようにした。
 *
 * Revision 1.31  2006/06/23 13:57:24  yutakakn
 * TTSSH 2.28にて遅延パケット圧縮をサポートした。
 *
 * Revision 1.30  2006/06/11 14:26:30  yutakakn
 * SSH Port Forward の編集画面で、TeraTermが未接続状態の場合、ポート番号不正を即座にメッセージボックスが表示されないバグを修正した。
 *
 * Revision 1.29  2006/03/26 15:43:58  yutakakn
 * SSH2のknown_hosts対応を追加した。
 *
 * Revision 1.28  2006/02/18 07:37:02  yutakakn
 *   ・コンパイラを Visual Studio 2005 Standard Edition に切り替えた。
 *   ・stricmp()を_stricmp()へ置換した
 *   ・strdup()を_strdup()へ置換した
 *
 * Revision 1.27  2005/10/21 13:43:08  yutakakn
 * Historyチェックボックスのenable / disable追加。
 *
 * Revision 1.26  2005/10/21 13:36:46  yutakakn
 * 接続ダイアログに History チェックボックスを追加した。
 * 2.18へアップデート。
 *
 * Revision 1.25  2005/07/09 17:08:47  yutakakn
 * SSH2 packet compressionをサポートした。
 *
 * Revision 1.24  2005/07/09 05:16:06  yutakakn
 * OpenSSL 0.9.8でビルドできるようにした。
 *
 * Revision 1.23  2005/06/19 09:17:47  yutakakn
 * SSH2 port-fowarding(local to remote)をサポートした。
 *
 * Revision 1.22  2005/05/15 09:14:04  yutakakn
 * zlib versionの位置調整。
 *
 * Revision 1.21  2005/05/10 16:44:08  yutakakn
 * zlibのバージョンをバージョン情報に追加した。
 *
 * Revision 1.20  2005/04/23 17:26:57  yutakakn
 * キー作成ダイアログの追加。
 *
 * Revision 1.19  2005/04/08 14:55:03  yutakakn
 * "Duplicate session"においてSSH自動ログインを行うようにした。
 *
 * Revision 1.18  2005/04/03 14:39:48  yutakakn
 * SSH2 channel lookup機構の追加（ポートフォワーディングのため）。
 * TTSSH 2.10で追加したlog dump機構において、DH鍵再作成時にbuffer freeで
 * アプリケーションが落ちてしまうバグを修正。
 *
 * Revision 1.17  2005/03/27 04:39:55  yutakakn
 * SSH2のログ採取(verbose)のデータを追加した。
 *
 * Revision 1.16  2005/03/23 12:39:20  yutakakn
 * シリアルポートを開いた状態からAlt-Nで新規接続を開こうとしたとき、フォーカスを当てるようにした。
 *
 * Revision 1.15  2005/03/12 15:07:34  yutakakn
 * SSH2 keyboard-interactive認証をTISダイアログに実装した。
 *
 * Revision 1.14  2005/03/12 12:08:05  yutakakn
 * パスワード認証の前に行うkeyboard-interactiveメソッドで、デフォルト設定値を無効(0)にした。
 * また、認証ダイアログのラベル名を設定の有無により変更するようにした。
 *
 * Revision 1.13  2005/03/03 13:28:23  yutakakn
 * クライアントのSSHバージョンを ttxssh.dll から取得して、サーバへ送るようにした。
 *
 * Revision 1.12  2005/02/28 14:51:44  yutakakn
 * バージョンダイアログに表示するTTSSHのバージョンを、ttxssh.dllの
 * バージョン情報から取得するようにした。
 *
 * Revision 1.11  2005/02/22 08:48:11  yutakakn
 * TTSSH setupダイアログに HeartBeat 設定を追加。
 * TTSSH authentication setupダイアログに keyboard-interactive 設定を追加。
 *
 * Revision 1.10  2005/01/27 13:30:33  yutakakn
 * 公開鍵認証自動ログインをサポート。
 * /auth=publickey, /keyfile オプションを新規追加した。
 * また、空白を含む引数をサポート。
 *
 * Revision 1.9  2005/01/24 14:07:07  yutakakn
 * ・keyboard-interactive認証をサポートした。
 * 　それに伴い、teraterm.iniに "KeyboardInteractive" エントリを追加した。
 * ・バージョンダイアログに OpenSSLバージョン を追加
 *
 * Revision 1.8  2004/12/27 14:05:08  yutakakn
 * 'Auto window close'が有効の場合、切断後の接続ができない問題を修正した。
 * 　・スレッドの終了待ち合わせ処理の追加
 * 　・確保済みSSHリソースの解放
 *
 * Revision 1.7  2004/12/17 14:28:36  yutakakn
 * メッセージ認証アルゴリズムに HMAC-MD5 を追加。
 * TTSSHバージョンダイアログにHMACアルゴリズム表示を追加。
 *
 * Revision 1.6  2004/12/16 13:57:43  yutakakn
 * "SECURITY WARINIG" dialogで ESC キーを押下すると、
 * アプリケーションエラーとなる現象への暫定対処。
 *
 * Revision 1.5  2004/12/11 07:31:00  yutakakn
 * SSH heartbeatスレッドの追加した。これにより、IPマスカレード環境において、ルータの
 * NATテーブルクリアにより、SSHコネクションが切断される現象が回避される。
 * それに合わせて、teraterm.iniのTTSSHセクションに、HeartBeat エントリを追加。
 *
 * Revision 1.4  2004/12/01 15:37:49  yutakakn
 * SSH2自動ログイン機能を追加。
 * 現状、パスワード認証のみに対応。
 * ・コマンドライン
 *   /ssh /auth=認証メソッド /user=ユーザ名 /passwd=パスワード
 *
 * Revision 1.3  2004/11/29 15:52:37  yutakakn
 * SSHのdefault protocolをSSH2にした。
 *
 * Revision 1.2  2004/11/23 14:32:26  yutakakn
 * 接続ダイアログの起動時に、TCP/IPの「ホスト名」にフォーカスが当たるようにした。
 *
 *
 */
