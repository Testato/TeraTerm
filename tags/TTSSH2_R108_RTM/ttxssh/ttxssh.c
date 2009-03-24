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

#include <openssl/opensslv.h>

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
			stricmp(buf, "yes") == 0 || stricmp(buf, "y") == 0;
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
	extern void SSH2_update_cipher_myproposal(PTInstVar pvar);
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
	SSH2_update_cipher_myproposal(pvar); // yutaka

	read_string_option(fileName, "KnownHostsFiles", "ssh_known_hosts",
					   settings->KnownHostsFiles,
					   sizeof(settings->KnownHostsFiles));

	buf[0] = 0;
	GetPrivateProfileString("TTSSH", "DefaultAuthMethod", "", buf,
							sizeof(buf), fileName);
	settings->DefaultAuthMethod = atoi(buf);
	if (settings->DefaultAuthMethod != SSH_AUTH_PASSWORD
		&& settings->DefaultAuthMethod != SSH_AUTH_RSA
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
	settings->ssh2_keyboard_interactive = GetPrivateProfileInt("TTSSH", "KeyboardInteractive", 1, fileName);

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
		PostMessage(pvar->NotificationWindow, WM_COMMAND,
					ID_SSHASYNCMESSAGEBOX, 0);
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
		char buf[1024] = "\nInitiating SSH session at ";
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

		if (GetHNRec->PortType == IdTCPIP)
			enable_dlg_items(dlg, IDC_HOSTCOMLABEL, IDC_HOSTCOM, FALSE);
#ifdef INET6
		else {
			enable_dlg_items(dlg, IDC_HOSTNAMELABEL, IDC_HOSTTCPPORT,
							 FALSE);
			enable_dlg_items(dlg, IDC_HOSTTCPPROTOCOLLABEL,
							 IDC_HOSTTCPPROTOCOL, FALSE);
		}
#else
		else
			enable_dlg_items(dlg, IDC_HOSTNAMELABEL, IDC_HOSTTCPPORT,
							 FALSE);
#endif							/* INET6 */

		// Host dialogにフォーカスをあてる (2004.10.2 yutaka)
		{
		HWND hwnd = GetDlgItem(dlg, IDC_HOSTNAME);

		SetFocus(hwnd);
		//SendMessage(hwnd, BM_SETCHECK, BST_CHECKED, 0);
		//style = GetClassLongPtr(hwnd, GCL_STYLE);
		//SetClassLongPtr(hwnd, GCL_STYLE, style | WS_TABSTOP);
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
						if (stricmp(afstr, "SSH1") == 0) {
							pvar->settings.ssh_protocol_version = 1;
						} else {
							pvar->settings.ssh_protocol_version = 2;
						}
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
					   || stricmp(option + 4, "-X") == 0) {
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
			} else if (stricmp(option + 4, "-autologin") == 0
					   || stricmp(option + 4, "-autologon") == 0) {
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
}

static void append_about_text(HWND dlg, char FAR * prefix, char FAR * msg)
{
	SendDlgItemMessage(dlg, IDC_ABOUTTEXT, EM_REPLACESEL, 0,
					   (LPARAM) prefix);
	SendDlgItemMessage(dlg, IDC_ABOUTTEXT, EM_REPLACESEL, 0, (LPARAM) msg);
	SendDlgItemMessage(dlg, IDC_ABOUTTEXT, EM_REPLACESEL, 0,
					   (LPARAM) (char FAR *) "\r\n");
}

static void init_about_dlg(PTInstVar pvar, HWND dlg)
{
	char buf[1024];

	// OpenSSLのバージョンを設定する (2005.1.24 yutaka)
	SendMessage(GetDlgItem(dlg, IDC_OPENSSL_VERSION), WM_SETTEXT, 0, (LPARAM)OPENSSL_VERSION_TEXT);

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
			append_about_text(dlg, "Compression: ", buf);

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
	SSH2_update_cipher_myproposal(pvar); // yutaka

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
	SSH2_update_cipher_myproposal(pvar); // yutaka

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

static int PASCAL FAR TTXProcessCommand(HWND hWin, WORD cmd)
{
	GET_VAR();

	if (pvar->fatal_error) {
		return 0;
	}

	switch (cmd) {
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
