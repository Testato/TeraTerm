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

#include "ttxssh.h"
#include "util.h"
#include "ssh.h"

#include <io.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

#include "resource.h"
#include "keyfiles.h"

#define AUTH_START_USER_AUTH_ON_ERROR_END 1

#define MAX_AUTH_CONTROL IDC_SSHUSETIS

#ifndef NO_I18N
static HFONT DlgAuthFont;
static HFONT DlgTisFont;
static HFONT DlgAuthSetupFont;
#endif

void destroy_malloced_string(char FAR * FAR * str)
{
	if (*str != NULL) {
		memset(*str, 0, strlen(*str));
		free(*str);
		*str = NULL;
	}
}

static int auth_types_to_control_IDs[] = {
	-1, IDC_SSHUSERHOSTS, IDC_SSHUSERSA, IDC_SSHUSEPASSWORD,
	IDC_SSHUSERHOSTS, IDC_SSHUSETIS, -1
};

static LRESULT CALLBACK password_wnd_proc(HWND control, UINT msg,
										  WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CHAR:
		if ((GetKeyState(VK_CONTROL) & 0x8000) != 0) {
			char chars[] = { (char) wParam, 0 };

			SendMessage(control, EM_REPLACESEL, (WPARAM) TRUE,
						(LPARAM) (char FAR *) chars);
			return 0;
		}
	}

	return CallWindowProc((WNDPROC) GetWindowLong(control, GWL_USERDATA),
						  control, msg, wParam, lParam);
}

static void init_password_control(HWND dlg)
{
	HWND passwordControl = GetDlgItem(dlg, IDC_SSHPASSWORD);

	SetWindowLong(passwordControl, GWL_USERDATA,
				  SetWindowLong(passwordControl, GWL_WNDPROC,
								(LONG) password_wnd_proc));

	SetFocus(passwordControl);
}

static void set_auth_options_status(HWND dlg, int controlID)
{
	BOOL RSA_enabled = controlID == IDC_SSHUSERSA;
	BOOL rhosts_enabled = controlID == IDC_SSHUSERHOSTS;
	BOOL TIS_enabled = controlID == IDC_SSHUSETIS;
	int i;

	CheckRadioButton(dlg, IDC_SSHUSEPASSWORD, MAX_AUTH_CONTROL, controlID);

	EnableWindow(GetDlgItem(dlg, IDC_SSHPASSWORDCAPTION), !TIS_enabled);
	EnableWindow(GetDlgItem(dlg, IDC_SSHPASSWORD), !TIS_enabled);

	for (i = IDC_CHOOSERSAFILE; i <= IDC_RSAFILENAME; i++) {
		EnableWindow(GetDlgItem(dlg, i), RSA_enabled);
	}

	for (i = IDC_LOCALUSERNAMELABEL; i <= IDC_HOSTRSAFILENAME; i++) {
		EnableWindow(GetDlgItem(dlg, i), rhosts_enabled);
	}
}

static void init_auth_machine_banner(PTInstVar pvar, HWND dlg)
{
	char buf[1024] = "Logging in to ";
#ifndef NO_I18N
	char buf2[1024];
	GetDlgItemText(dlg, IDC_SSHAUTHBANNER, buf2, sizeof(buf2));
	_snprintf(buf, sizeof(buf), buf2, SSH_get_host_name(pvar));
#else
	if (strlen(buf) + strlen(SSH_get_host_name(pvar)) < sizeof(buf) - 2) {
		strcat(buf, SSH_get_host_name(pvar));
	}
#endif
	SetDlgItemText(dlg, IDC_SSHAUTHBANNER, buf);
}

static void update_server_supported_types(PTInstVar pvar, HWND dlg)
{
	int supported_methods = pvar->auth_state.supported_types;
	int cur_control = -1;
	int control;
	HWND focus = GetFocus();

	if (supported_methods == 0) {
		return;
	}

	for (control = IDC_SSHUSEPASSWORD; control <= MAX_AUTH_CONTROL;
		 control++) {
		BOOL enabled = FALSE;
		int method;
		HWND item = GetDlgItem(dlg, control);

		if (item != NULL) {
			for (method = 0; method <= SSH_AUTH_MAX; method++) {
				if (auth_types_to_control_IDs[method] == control
					&& (supported_methods & (1 << method)) != 0) {
					enabled = TRUE;
				}
			}

			EnableWindow(item, enabled);

			if (IsDlgButtonChecked(dlg, control)) {
				cur_control = control;
			}
		}
	}

	if (cur_control >= 0) {
		if (!IsWindowEnabled(GetDlgItem(dlg, cur_control))) {
			do {
				cur_control++;
				if (cur_control > MAX_AUTH_CONTROL) {
					cur_control = IDC_SSHUSEPASSWORD;
				}
			} while (!IsWindowEnabled(GetDlgItem(dlg, cur_control)));

			set_auth_options_status(dlg, cur_control);

			if (focus != NULL && !IsWindowEnabled(focus)) {
				SetFocus(GetDlgItem(dlg, cur_control));
			}
		}
	}
}

static void init_auth_dlg(PTInstVar pvar, HWND dlg)
{
	int default_method = pvar->session_settings.DefaultAuthMethod;

#ifndef NO_I18N
	GetWindowText(dlg, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("DLG_AUTH_TITLE", pvar);
	SetWindowText(dlg, pvar->ts->UIMsg);

	GetDlgItemText(dlg, IDC_SSHAUTHBANNER, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("DLG_AUTH_BANNER", pvar);
	SetDlgItemText(dlg, IDC_SSHAUTHBANNER, pvar->ts->UIMsg);

	GetDlgItemText(dlg, IDC_SSHAUTHBANNER2, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("DLG_AUTH_BANNER2", pvar);
	SetDlgItemText(dlg, IDC_SSHAUTHBANNER2, pvar->ts->UIMsg);

	GetDlgItemText(dlg, IDC_SSHUSERNAMELABEL, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("DLG_AUTH_USERNAME", pvar);
	SetDlgItemText(dlg, IDC_SSHUSERNAMELABEL, pvar->ts->UIMsg);

	GetDlgItemText(dlg, IDC_SSHPASSWORDCAPTION, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("DLG_AUTH_PASSWORD", pvar);
	SetDlgItemText(dlg, IDC_SSHPASSWORDCAPTION, pvar->ts->UIMsg);

	GetDlgItemText(dlg, IDC_REMEMBER_PASSWORD, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("DLG_AUTH_REMEMBER_PASSWORD", pvar);
	SetDlgItemText(dlg, IDC_REMEMBER_PASSWORD, pvar->ts->UIMsg);

	GetDlgItemText(dlg, IDC_SSHUSEPASSWORD, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("DLG_AUTH_METHOD_PASSWORD", pvar);
	SetDlgItemText(dlg, IDC_SSHUSEPASSWORD, pvar->ts->UIMsg);

	GetDlgItemText(dlg, IDC_SSHUSERSA, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("DLG_AUTH_METHOD_RSA", pvar);
	SetDlgItemText(dlg, IDC_SSHUSERSA, pvar->ts->UIMsg);

	GetDlgItemText(dlg, IDC_SSHUSERHOSTS, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("DLG_AUTH_METHOD_RHOST", pvar);
	SetDlgItemText(dlg, IDC_SSHUSERHOSTS, pvar->ts->UIMsg);

	GetDlgItemText(dlg, IDC_CHOOSERSAFILE, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("DLG_AUTH_PRIVATEKEY", pvar);
	SetDlgItemText(dlg, IDC_CHOOSERSAFILE, pvar->ts->UIMsg);

	GetDlgItemText(dlg, IDC_LOCALUSERNAMELABEL, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("DLG_AUTH_LOCALUSER", pvar);
	SetDlgItemText(dlg, IDC_LOCALUSERNAMELABEL, pvar->ts->UIMsg);

	GetDlgItemText(dlg, IDC_CHOOSEHOSTRSAFILE, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("DLG_AUTH_HOST_PRIVATEKEY", pvar);
	SetDlgItemText(dlg, IDC_CHOOSEHOSTRSAFILE, pvar->ts->UIMsg);

	GetDlgItemText(dlg, IDOK, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("BTN_OK", pvar);
	SetDlgItemText(dlg, IDOK, pvar->ts->UIMsg);

	GetDlgItemText(dlg, IDCANCEL, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("BTN_DISCONNECT", pvar);
	SetDlgItemText(dlg, IDCANCEL, pvar->ts->UIMsg);
#endif

	init_auth_machine_banner(pvar, dlg);
	init_password_control(dlg);

	if (pvar->auth_state.failed_method != SSH_AUTH_NONE) {
		/* must be retrying a failed attempt */
#ifndef NO_I18N
		strcpy(pvar->ts->UIMsg, "Authentication failed. Please retry.");
		UTIL_get_lang_msg("DLG_AUTH_BANNER2_FAILED", pvar);
		SetDlgItemText(dlg, IDC_SSHAUTHBANNER2, "Retrying SSH Authentication");

		strcpy(pvar->ts->UIMsg, "Retrying SSH Authentication");
		UTIL_get_lang_msg("DLG_AUTH_TITLE_FAILED", pvar);
		SetWindowText(dlg, pvar->ts->UIMsg);
#else
		SetDlgItemText(dlg, IDC_SSHAUTHBANNER2,
					   "Authentication failed. Please retry.");
		SetWindowText(dlg, "Retrying SSH Authentication");
#endif
		default_method = pvar->auth_state.failed_method;
	}

	set_auth_options_status(dlg,
							auth_types_to_control_IDs[default_method]);

	if (default_method == SSH_AUTH_TIS) {
		/* we disabled the password control, so fix the focus */
		SetFocus(GetDlgItem(dlg, IDC_SSHUSETIS));
	}

	if (pvar->auth_state.user != NULL) {
		SetDlgItemText(dlg, IDC_SSHUSERNAME, pvar->auth_state.user);
		EnableWindow(GetDlgItem(dlg, IDC_SSHUSERNAME), FALSE);
		EnableWindow(GetDlgItem(dlg, IDC_SSHUSERNAMELABEL), FALSE);
	} else if (pvar->session_settings.DefaultUserName[0] != 0) {
		SetDlgItemText(dlg, IDC_SSHUSERNAME,
					   pvar->session_settings.DefaultUserName);
	} else {
		SetFocus(GetDlgItem(dlg, IDC_SSHUSERNAME));
	}

	SetDlgItemText(dlg, IDC_RSAFILENAME,
				   pvar->session_settings.DefaultRSAPrivateKeyFile);
	SetDlgItemText(dlg, IDC_HOSTRSAFILENAME,
				   pvar->session_settings.DefaultRhostsHostPrivateKeyFile);
	SetDlgItemText(dlg, IDC_LOCALUSERNAME,
				   pvar->session_settings.DefaultRhostsLocalUserName);

	update_server_supported_types(pvar, dlg);

	// SSH2 autologin 
	// ユーザ、パスワード、認証メソッドを自動設定して、一定時間後にOKボタンを押下する。
	// 
	// (2004.12.1 yutaka)
	// (2005.1.26 yutaka) 公開鍵認証サポート
	// 自動ログインでないときは、自動設定はするが変更可能 (2006.9.18 maya)
#if 0
	if (pvar->ssh2_autologin == 1) {
#endif
		if (strlen(pvar->ssh2_username) > 0) {
			SetDlgItemText(dlg, IDC_SSHUSERNAME, pvar->ssh2_username);
		}
		if (pvar->ssh2_autologin == 1) {
			EnableWindow(GetDlgItem(dlg, IDC_SSHUSERNAME), FALSE);
			EnableWindow(GetDlgItem(dlg, IDC_SSHUSERNAMELABEL), FALSE);
		}

		SetDlgItemText(dlg, IDC_SSHPASSWORD, pvar->ssh2_password);
		if (pvar->ssh2_autologin == 1) {
			EnableWindow(GetDlgItem(dlg, IDC_SSHPASSWORD), FALSE);
			EnableWindow(GetDlgItem(dlg, IDC_SSHPASSWORDCAPTION), FALSE);
		}

		// '/I' 指定があるときのみ最小化する (2005.9.5 yutaka)
		if (pvar->ts->Minimize) {
			//20050822追加 start T.Takahashi
			ShowWindow(dlg,SW_MINIMIZE);
			//20050822追加 end T.Takahashi
		}

		if (pvar->ssh2_authmethod == SSH_AUTH_PASSWORD) {
			CheckRadioButton(dlg, IDC_SSHUSEPASSWORD, MAX_AUTH_CONTROL, IDC_SSHUSEPASSWORD);

		} else if (pvar->ssh2_authmethod == SSH_AUTH_RSA) {
			CheckRadioButton(dlg, IDC_SSHUSEPASSWORD, MAX_AUTH_CONTROL, IDC_SSHUSERSA);

			SetDlgItemText(dlg, IDC_RSAFILENAME, pvar->ssh2_keyfile);
			if (pvar->ssh2_autologin == 1) {
				EnableWindow(GetDlgItem(dlg, IDC_CHOOSERSAFILE), FALSE);
				EnableWindow(GetDlgItem(dlg, IDC_RSAFILENAME), FALSE);
			}

		} else {
			// TODO

		}
		
		if (pvar->ask4passwd == 1) {
			SetFocus(GetDlgItem(dlg, IDC_SSHPASSWORD));
		}
#if 0
	}
#endif

#if 1
	// パスワード認証を試す前に、keyboard-interactiveメソッドを試す場合は、ラベル名を
	// 変更する。(2005.3.12 yutaka)
	if (pvar->settings.ssh2_keyboard_interactive == 1) {
#ifndef NO_I18N
		strcpy(pvar->ts->UIMsg, "Use p&lain password to log in (with keyboard-interactive)");
		UTIL_get_lang_msg("DLG_AUTH_METHOD_PASSWORD_KBDINT", pvar);
		SetDlgItemText(dlg, IDC_SSHUSEPASSWORD, pvar->ts->UIMsg);
#else
		SetDlgItemText(dlg, IDC_SSHUSEPASSWORD, "Use p&lain password to log in (with keyboard-interactive)");
#endif
	}

	if (pvar->settings.ssh_protocol_version == 1) {
#ifndef NO_I18N
		strcpy(pvar->ts->UIMsg, "Use challenge/response to log in(&TIS)");
		UTIL_get_lang_msg("DLG_AUTH_METHOD_CHALLENGE1", pvar);
		SetDlgItemText(dlg, IDC_SSHUSETIS, pvar->ts->UIMsg);
#else
		SetDlgItemText(dlg, IDC_SSHUSETIS, "Use challenge/response to log in(&TIS)");
#endif
	} else {
#ifndef NO_I18N
		strcpy(pvar->ts->UIMsg, "Use &challenge/response to log in(keyboard-interactive)");
		UTIL_get_lang_msg("DLG_AUTH_METHOD_CHALLENGE2", pvar);
		SetDlgItemText(dlg, IDC_SSHUSETIS, pvar->ts->UIMsg);
#else
		SetDlgItemText(dlg, IDC_SSHUSETIS, "Use challenge/response to log in(&keyboard-interactive)");
#endif
	}
#endif

	// パスワードを覚えておくチェックボックスにはデフォルトで有効とする (2006.8.3 yutaka)
	if (pvar->ts_SSH->remember_password) {
		SendMessage(GetDlgItem(dlg, IDC_REMEMBER_PASSWORD), BM_SETCHECK, BST_CHECKED, 0);
	} else {
		SendMessage(GetDlgItem(dlg, IDC_REMEMBER_PASSWORD), BM_SETCHECK, BST_UNCHECKED, 0);
	}

}

static char FAR *alloc_control_text(HWND ctl)
{
	int len = GetWindowTextLength(ctl);
	char FAR *result = malloc(len + 1);

	if (result != NULL) {
		GetWindowText(ctl, result, len + 1);
		result[len] = 0;
	}

	return result;
}

#ifndef NO_I18N
static int get_key_file_name(HWND parent, char FAR * buf, int bufsize, PTInstVar pvar)
#else
static int get_key_file_name(HWND parent, char FAR * buf, int bufsize)
#endif
{
#ifdef TERATERM32
	OPENFILENAME params;
	char fullname_buf[2048] = "identity";
#ifndef NO_I18N
	char filter[MAX_UIMSG];
#endif

	ZeroMemory(&params, sizeof(params));
	params.lStructSize = sizeof(OPENFILENAME);
	params.hwndOwner = parent;
	// フィルタの追加 (2004.12.19 yutaka)
	// 3ファイルフィルタの追加 (2005.4.26 yutaka)
#ifndef NO_I18N
	strncpy(pvar->ts->UIMsg, "identity files\\0identity;id_rsa;id_dsa\\0identity(RSA1)\\0identity\\0id_rsa(SSH2)\\0id_rsa\\0id_dsa(SSH2)\\0id_dsa\\0all(*.*)\\0*.*\\0\\0", sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("FILEDLG_OPEN_PRIVATEKEY_FILTER", pvar);
	memcpy(filter, pvar->ts->UIMsg, sizeof(filter));
	params.lpstrFilter = filter;
#else
	params.lpstrFilter = "identity files\0identity;id_rsa;id_dsa\0identity(RSA1)\0identity\0id_rsa(SSH2)\0id_rsa\0id_dsa(SSH2)\0id_dsa\0all(*.*)\0*.*\0\0";
#endif
	params.lpstrCustomFilter = NULL;
	params.nFilterIndex = 0;
	buf[0] = 0;
	params.lpstrFile = fullname_buf;
	params.nMaxFile = sizeof(fullname_buf);
	params.lpstrFileTitle = NULL;
	params.lpstrInitialDir = NULL;
#ifndef NO_I18N
	strcpy(pvar->ts->UIMsg, "Choose a file with the RSA/DSA private key");
	UTIL_get_lang_msg("FILEDLG_OPEN_PRIVATEKEY_TITLE", pvar);
	params.lpstrTitle = pvar->ts->UIMsg;
#else
	params.lpstrTitle = "Choose a file with the RSA/DSA private key";
#endif
	params.Flags =
		OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	params.lpstrDefExt = NULL;

	if (GetOpenFileName(&params) != 0) {
		copy_teraterm_dir_relative_path(buf, bufsize, fullname_buf);
		return 1;
	} else {
		return 0;
	}
#else
	return 0;
#endif
}

#ifndef NO_I18N
static void choose_RSA_key_file(HWND dlg, PTInstVar pvar)
#else
static void choose_RSA_key_file(HWND dlg)
#endif
{
	char buf[1024];

#ifndef NO_I18N
	if (get_key_file_name(dlg, buf, sizeof(buf), pvar)) {
#else
	if (get_key_file_name(dlg, buf, sizeof(buf))) {
#endif
		SetDlgItemText(dlg, IDC_RSAFILENAME, buf);
	}
}

#ifndef NO_I18N
static void choose_host_RSA_key_file(HWND dlg, PTInstVar pvar)
#else
static void choose_host_RSA_key_file(HWND dlg)
#endif
{
	char buf[1024];

#ifndef NO_I18N
	if (get_key_file_name(dlg, buf, sizeof(buf), pvar)) {
#else
	if (get_key_file_name(dlg, buf, sizeof(buf))) {
#endif
		SetDlgItemText(dlg, IDC_HOSTRSAFILENAME, buf);
	}
}

static BOOL end_auth_dlg(PTInstVar pvar, HWND dlg)
{
	int method = SSH_AUTH_PASSWORD;
	char FAR *password =
		alloc_control_text(GetDlgItem(dlg, IDC_SSHPASSWORD));
	CRYPTKeyPair FAR *key_pair = NULL;

	if (IsDlgButtonChecked(dlg, IDC_SSHUSERSA)) {
		method = SSH_AUTH_RSA;
	} else if (IsDlgButtonChecked(dlg, IDC_SSHUSERHOSTS)) {
		if (GetWindowTextLength(GetDlgItem(dlg, IDC_HOSTRSAFILENAME)) > 0) {
			method = SSH_AUTH_RHOSTS_RSA;
		} else {
			method = SSH_AUTH_RHOSTS;
		}
	} else if (IsDlgButtonChecked(dlg, IDC_SSHUSETIS)) {
		method = SSH_AUTH_TIS;
	}

	if (method == SSH_AUTH_RSA || method == SSH_AUTH_RHOSTS_RSA) {
		char buf[2048];
		int file_ctl_ID =
			method == SSH_AUTH_RSA ? IDC_RSAFILENAME : IDC_HOSTRSAFILENAME;

		buf[0] = 0;
		GetDlgItemText(dlg, file_ctl_ID, buf, sizeof(buf));
		if (buf[0] == 0) {
#ifndef NO_I18N
			strcpy(pvar->ts->UIMsg, "You must specify a file containing the RSA/DSA private key.");
			UTIL_get_lang_msg("MSG_KEYSPECIFY_ERROR", pvar);
			notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
			notify_nonfatal_error(pvar,
								  "You must specify a file containing the RSA/DSA private key.");
#endif
			SetFocus(GetDlgItem(dlg, file_ctl_ID));
			destroy_malloced_string(&password);
			return FALSE;
		} 
		
		if (SSHv1(pvar)) {
			BOOL invalid_passphrase = FALSE;

			key_pair = KEYFILES_read_private_key(pvar, buf, password,
												 &invalid_passphrase,
												 FALSE);

			if (key_pair == NULL) {
				if (invalid_passphrase) {
					HWND passwordCtl = GetDlgItem(dlg, IDC_SSHPASSWORD);

					SetFocus(passwordCtl);
					SendMessage(passwordCtl, EM_SETSEL, 0, -1);
				} else {
					SetFocus(GetDlgItem(dlg, file_ctl_ID));
				}
				destroy_malloced_string(&password);
				return FALSE;
			}

		} else { // SSH2(yutaka)
			BOOL invalid_passphrase = FALSE;
			char errmsg[256];

			memset(errmsg, 0, sizeof(errmsg));
			//GetCurrentDirectory(sizeof(errmsg), errmsg);

			key_pair = read_SSH2_private_key(pvar, buf, password,
									&invalid_passphrase,
									FALSE,
									errmsg,
									sizeof(errmsg)
									);

			if (key_pair == NULL) { // read error
				char buf[1024];
#ifndef NO_I18N
				strcpy(pvar->ts->UIMsg, "read error SSH2 private key file\r\n%s");
				UTIL_get_lang_msg("MSG_READKEY_ERROR", pvar);
				_snprintf(buf, sizeof(buf), pvar->ts->UIMsg, errmsg);
#else
				_snprintf(buf, sizeof(buf), "read error SSH2 private key file\r\n%s", errmsg);
#endif
				notify_nonfatal_error(pvar, buf);
				// パスフレーズが鍵と一致しなかった場合はIDC_SSHPASSWORDにフォーカスを移す (2006.10.29 yasuhide)
				if (invalid_passphrase) {
					HWND passwordCtl = GetDlgItem(dlg, IDC_SSHPASSWORD);

					SetFocus(passwordCtl);
					SendMessage(passwordCtl, EM_SETSEL, 0, -1);
				} else {
					SetFocus(GetDlgItem(dlg, file_ctl_ID));
				}
				destroy_malloced_string(&password);
				return FALSE;
			}

		}

	}

	/* from here on, we cannot fail, so just munge cur_cred in place */
	pvar->auth_state.cur_cred.method = method;
	pvar->auth_state.cur_cred.key_pair = key_pair;
	/* we can't change the user name once it's set. It may already have
	   been sent to the server, and it can only be sent once. */
	if (pvar->auth_state.user == NULL) {
		pvar->auth_state.user =
			alloc_control_text(GetDlgItem(dlg, IDC_SSHUSERNAME));
	}

	// パスワードの保存をするかどうかを決める (2006.8.3 yutaka)
	if (SendMessage(GetDlgItem(dlg, IDC_REMEMBER_PASSWORD), BM_GETCHECK, 0,0) == BST_CHECKED) {
		pvar->settings.remember_password = 1;  // 覚えておく
		pvar->ts_SSH->remember_password = 1;
	} else {
		pvar->settings.remember_password = 0;  // ここですっかり忘れる
		pvar->ts_SSH->remember_password = 0;
	}

	// 公開鍵認証の場合、セッション複製時にパスワードを使い回したいので解放しないようにする。
	// (2005.4.8 yutaka)
	if (method == SSH_AUTH_PASSWORD || method == SSH_AUTH_RSA) {
		pvar->auth_state.cur_cred.password = password;
	} else {
		destroy_malloced_string(&password);
	}
	if (method == SSH_AUTH_RHOSTS || method == SSH_AUTH_RHOSTS_RSA) {
		if (pvar->session_settings.DefaultAuthMethod != SSH_AUTH_RHOSTS) {
#ifndef NO_I18N
			strcpy(pvar->ts->UIMsg, "Rhosts authentication will probably fail because it was not "
									"the default authentication method.\n"
									"To use Rhosts authentication "
									"in TTSSH, you need to set it to be the default by restarting\n"
									"TTSSH and selecting \"SSH Authentication...\" from the Setup menu"
									"before connecting.");
			UTIL_get_lang_msg("MSG_RHOSTS_NOTDEFAULT_ERROR", pvar);
			notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
			notify_nonfatal_error(pvar,
								  "Rhosts authentication will probably fail because it was not "
								  "the default authentication method.\n"
								  "To use Rhosts authentication "
								  "in TTSSH, you need to set it to be the default by restarting\n"
								  "TTSSH and selecting \"SSH Authentication...\" from the Setup menu"
								  "before connecting.");
#endif
		}

		pvar->auth_state.cur_cred.rhosts_client_user =
			alloc_control_text(GetDlgItem(dlg, IDC_LOCALUSERNAME));
	}
	pvar->auth_state.auth_dialog = NULL;

	GetDlgItemText(dlg, IDC_RSAFILENAME,
				   pvar->session_settings.DefaultRSAPrivateKeyFile,
				   sizeof(pvar->session_settings.
						  DefaultRSAPrivateKeyFile));
	GetDlgItemText(dlg, IDC_HOSTRSAFILENAME,
				   pvar->session_settings.DefaultRhostsHostPrivateKeyFile,
				   sizeof(pvar->session_settings.
						  DefaultRhostsHostPrivateKeyFile));
	GetDlgItemText(dlg, IDC_LOCALUSERNAME,
				   pvar->session_settings.DefaultRhostsLocalUserName,
				   sizeof(pvar->session_settings.
						  DefaultRhostsLocalUserName));

	if (SSHv1(pvar)) {
		SSH_notify_user_name(pvar);
		SSH_notify_cred(pvar);
	} else {
		// for SSH2(yutaka)
		do_SSH2_userauth(pvar);
	}

	EndDialog(dlg, 1);
#ifndef NO_I18N
			if (DlgAuthFont != NULL) {
				DeleteObject(DlgAuthFont);
			}
#endif

	return TRUE;
}

static BOOL CALLBACK auth_dlg_proc(HWND dlg, UINT msg, WPARAM wParam,
								   LPARAM lParam)
{
	const int IDC_TIMER1 = 300;
	const int autologin_timeout = 10; // ミリ秒
	PTInstVar pvar;
#ifndef NO_I18N
	LOGFONT logfont;
	HFONT font;
#endif

	switch (msg) {
	case WM_INITDIALOG:
		pvar = (PTInstVar) lParam;
		pvar->auth_state.auth_dialog = dlg;
		SetWindowLong(dlg, DWL_USER, lParam);

		init_auth_dlg(pvar, dlg);

#ifndef NO_I18N
		font = (HFONT)SendMessage(dlg, WM_GETFONT, 0, 0);
		GetObject(font, sizeof(LOGFONT), &logfont);
		if (UTIL_get_lang_font("DLG_TAHOMA_FONT", dlg, &logfont, &DlgAuthFont, pvar)) {
			SendDlgItemMessage(dlg, IDC_SSHAUTHBANNER, WM_SETFONT, (WPARAM)DlgAuthFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_SSHAUTHBANNER2, WM_SETFONT, (WPARAM)DlgAuthFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_SSHUSERNAMELABEL, WM_SETFONT, (WPARAM)DlgAuthFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_SSHUSERNAME, WM_SETFONT, (WPARAM)DlgAuthFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_SSHPASSWORDCAPTION, WM_SETFONT, (WPARAM)DlgAuthFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_SSHPASSWORD, WM_SETFONT, (WPARAM)DlgAuthFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_REMEMBER_PASSWORD, WM_SETFONT, (WPARAM)DlgAuthFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_SSHUSEPASSWORD, WM_SETFONT, (WPARAM)DlgAuthFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_SSHUSERSA, WM_SETFONT, (WPARAM)DlgAuthFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_CHOOSERSAFILE, WM_SETFONT, (WPARAM)DlgAuthFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_RSAFILENAME, WM_SETFONT, (WPARAM)DlgAuthFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_SSHUSERHOSTS, WM_SETFONT, (WPARAM)DlgAuthFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_LOCALUSERNAMELABEL, WM_SETFONT, (WPARAM)DlgAuthFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_LOCALUSERNAME, WM_SETFONT, (WPARAM)DlgAuthFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_CHOOSEHOSTRSAFILE, WM_SETFONT, (WPARAM)DlgAuthFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_HOSTRSAFILENAME, WM_SETFONT, (WPARAM)DlgAuthFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_SSHUSETIS, WM_SETFONT, (WPARAM)DlgAuthFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDOK, WM_SETFONT, (WPARAM)DlgAuthFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDCANCEL, WM_SETFONT, (WPARAM)DlgAuthFont, MAKELPARAM(TRUE,0));
		}
		else {
			DlgAuthFont = NULL;
		}
#endif

		// SSH2 autologinが有効の場合は、タイマを仕掛ける。 (2004.12.1 yutaka)
		if (pvar->ssh2_autologin == 1) {
			SetTimer(dlg, IDC_TIMER1, autologin_timeout, 0);
		}
		return FALSE;			/* because we set the focus */

	case WM_TIMER:
		pvar = (PTInstVar) GetWindowLong(dlg, DWL_USER);
		// 認証準備ができてから、認証データを送信する。早すぎると、落ちる。(2004.12.16 yutaka)
		if (!(pvar->ssh_state.status_flags & STATUS_DONT_SEND_USER_NAME)) {
			KillTimer(dlg, IDC_TIMER1);
			SendMessage(dlg, WM_COMMAND, IDOK, 0);
		}
		return TRUE;

	case WM_COMMAND:
		pvar = (PTInstVar) GetWindowLong(dlg, DWL_USER);

		switch (LOWORD(wParam)) {
		case IDOK:
			// 認証準備ができてから、認証データを送信する。早すぎると、落ちる。(2001.1.25 yutaka)
			if (pvar->userauth_retry_count == 0 && (pvar->ssh_state.status_flags & STATUS_DONT_SEND_USER_NAME)) {
				return FALSE;
			}

			return end_auth_dlg(pvar, dlg);

		case IDCANCEL:			/* kill the connection */
			pvar->auth_state.auth_dialog = NULL;
			notify_closed_connection(pvar);
			EndDialog(dlg, 0);

#ifndef NO_I18N
			if (DlgAuthFont != NULL) {
				DeleteObject(DlgAuthFont);
			}
#endif

			return TRUE;

		case IDC_SSHUSEPASSWORD:
		case IDC_SSHUSERSA:
		case IDC_SSHUSERHOSTS:
		case IDC_SSHUSETIS:
			set_auth_options_status(dlg, LOWORD(wParam));
			return TRUE;

		case IDC_CHOOSERSAFILE:
#ifndef NO_I18N
			choose_RSA_key_file(dlg, pvar);
#else
			choose_RSA_key_file(dlg);
#endif
			return TRUE;

		case IDC_CHOOSEHOSTRSAFILE:
#ifndef NO_I18N
			choose_host_RSA_key_file(dlg, pvar);
#else
			choose_host_RSA_key_file(dlg);
#endif
			return TRUE;

		default:
			return FALSE;
		}

	default:
		return FALSE;
	}
}

char FAR *AUTH_get_user_name(PTInstVar pvar)
{
	return pvar->auth_state.user;
}

int AUTH_set_supported_auth_types(PTInstVar pvar, int types)
{
	char buf[1024];

	_snprintf(buf, sizeof(buf),
			  "Server reports supported authentication method mask = %d",
			  types);
	buf[sizeof(buf) - 1] = 0;
	notify_verbose_message(pvar, buf, LOG_LEVEL_VERBOSE);

	if (SSHv1(pvar)) {
		types &= (1 << SSH_AUTH_PASSWORD) | (1 << SSH_AUTH_RSA)
			| (1 << SSH_AUTH_RHOSTS_RSA) | (1 << SSH_AUTH_RHOSTS)
			| (1 << SSH_AUTH_TIS);
	} else {
		// for SSH2(yutaka)
//		types &= (1 << SSH_AUTH_PASSWORD);
		// 公開鍵認証を有効にする (2004.12.18 yutaka)
		// TISを追加。SSH2ではkeyboard-interactiveとして扱う。(2005.3.12 yutaka)
		types &= (1 << SSH_AUTH_PASSWORD) | (1 << SSH_AUTH_RSA)
			| (1 << SSH_AUTH_DSA)
			| (1 << SSH_AUTH_TIS);
	}
	pvar->auth_state.supported_types = types;

	if (types == 0) {
#ifndef NO_I18N
		strcpy(pvar->ts->UIMsg,
			   "Server does not support any of the authentication options\n"
			   "provided by TTSSH. This connection will now close.");
		UTIL_get_lang_msg("MSG_NOAUTHMETHOD_ERROR", pvar);
		notify_fatal_error(pvar, pvar->ts->UIMsg);
#else
		notify_fatal_error(pvar,
						   "Server does not support any of the authentication options\n"
						   "provided by TTSSH. This connection will now close.");
#endif
		return 0;
	} else {
		if (pvar->auth_state.auth_dialog != NULL) {
			update_server_supported_types(pvar,
										  pvar->auth_state.auth_dialog);
		}

		return 1;
	}
}

static void start_user_auth(PTInstVar pvar)
{
	// 認証ダイアログを表示させる (2004.12.1 yutaka)
	PostMessage(pvar->NotificationWindow, WM_COMMAND, (WPARAM) ID_SSHAUTH,
				(LPARAM) NULL);
	pvar->auth_state.cur_cred.method = SSH_AUTH_NONE;
}

static void try_default_auth(PTInstVar pvar)
{
	if (pvar->session_settings.TryDefaultAuth) {
		switch (pvar->session_settings.DefaultAuthMethod) {
		case SSH_AUTH_RSA:{
				BOOL invalid_passphrase;
				char password[] = "";

				pvar->auth_state.cur_cred.key_pair
					=
					KEYFILES_read_private_key(pvar,
											  pvar->session_settings.
											  DefaultRSAPrivateKeyFile,
											  password,
											  &invalid_passphrase, TRUE);
				if (pvar->auth_state.cur_cred.key_pair == NULL) {
					return;
				} else {
					pvar->auth_state.cur_cred.method = SSH_AUTH_RSA;
				}
				break;
			}

		case SSH_AUTH_RHOSTS:
			if (pvar->session_settings.
				DefaultRhostsHostPrivateKeyFile[0] != 0) {
				BOOL invalid_passphrase;
				char password[] = "";

				pvar->auth_state.cur_cred.key_pair
					=
					KEYFILES_read_private_key(pvar,
											  pvar->session_settings.
											  DefaultRhostsHostPrivateKeyFile,
											  password,
											  &invalid_passphrase, TRUE);
				if (pvar->auth_state.cur_cred.key_pair == NULL) {
					return;
				} else {
					pvar->auth_state.cur_cred.method = SSH_AUTH_RHOSTS_RSA;
				}
			} else {
				pvar->auth_state.cur_cred.method = SSH_AUTH_RHOSTS;
			}

			pvar->auth_state.cur_cred.rhosts_client_user =
				_strdup(pvar->session_settings.DefaultRhostsLocalUserName);
			break;

		case SSH_AUTH_PASSWORD:
			pvar->auth_state.cur_cred.password = _strdup("");
			pvar->auth_state.cur_cred.method = SSH_AUTH_PASSWORD;
			break;

		case SSH_AUTH_TIS:
		default:
			return;
		}

		pvar->auth_state.user =
			_strdup(pvar->session_settings.DefaultUserName);
	}
}

void AUTH_notify_end_error(PTInstVar pvar)
{
	if ((pvar->auth_state.flags & AUTH_START_USER_AUTH_ON_ERROR_END) != 0) {
		start_user_auth(pvar);
		pvar->auth_state.flags &= ~AUTH_START_USER_AUTH_ON_ERROR_END;
	}
}

void AUTH_advance_to_next_cred(PTInstVar pvar)
{
	pvar->auth_state.failed_method = pvar->auth_state.cur_cred.method;

	if (pvar->auth_state.cur_cred.method == SSH_AUTH_NONE) {
		try_default_auth(pvar);

		if (pvar->auth_state.cur_cred.method == SSH_AUTH_NONE) {
			if (pvar->err_msg != NULL) {
				pvar->auth_state.flags |=
					AUTH_START_USER_AUTH_ON_ERROR_END;
			} else {
				// ここで認証ダイアログを出現させる (2004.12.1 yutaka)
				// コマンドライン指定なしの場合
				start_user_auth(pvar);
			}
		}
	} else {
		// ここで認証ダイアログを出現させる (2004.12.1 yutaka)
		// コマンドライン指定あり(/auth=xxxx)の場合
		start_user_auth(pvar);
	}
}

static void init_TIS_dlg(PTInstVar pvar, HWND dlg)
{
#ifndef NO_I18N
	GetWindowText(dlg, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("DLG_TIS_TITLE", pvar);
	SetWindowText(dlg, pvar->ts->UIMsg);

	GetDlgItemText(dlg, IDC_SSHAUTHBANNER, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("DLG_TIS_BANNER", pvar);
	SetDlgItemText(dlg, IDC_SSHAUTHBANNER, pvar->ts->UIMsg);

	GetDlgItemText(dlg, IDOK, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("BTN_OK", pvar);
	SetDlgItemText(dlg, IDOK, pvar->ts->UIMsg);

	GetDlgItemText(dlg, IDCANCEL, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("BTN_DISCONNECT", pvar);
	SetDlgItemText(dlg, IDCANCEL, pvar->ts->UIMsg);
#endif

	init_auth_machine_banner(pvar, dlg);
	init_password_control(dlg);

	if (pvar->auth_state.TIS_prompt != NULL) {
		if (strlen(pvar->auth_state.TIS_prompt) > 10000) {
			pvar->auth_state.TIS_prompt[10000] = 0;
		}
		SetDlgItemText(dlg, IDC_SSHAUTHBANNER2,
					   pvar->auth_state.TIS_prompt);
		destroy_malloced_string(&pvar->auth_state.TIS_prompt);
	}
}

static BOOL end_TIS_dlg(PTInstVar pvar, HWND dlg)
{
	char FAR *password =
		alloc_control_text(GetDlgItem(dlg, IDC_SSHPASSWORD));

	pvar->auth_state.cur_cred.method = SSH_AUTH_TIS;
	pvar->auth_state.cur_cred.password = password;
	pvar->auth_state.auth_dialog = NULL;

	// add
	if (SSHv2(pvar)) {
		pvar->keyboard_interactive_password_input = 1;
		handle_SSH2_userauth_inforeq(pvar);
	} 

	SSH_notify_cred(pvar);

	EndDialog(dlg, 1);
	return TRUE;
}

static BOOL CALLBACK TIS_dlg_proc(HWND dlg, UINT msg, WPARAM wParam,
								  LPARAM lParam)
{
	PTInstVar pvar;
#ifndef NO_I18N
	LOGFONT logfont;
	HFONT font;
#endif

	switch (msg) {
	case WM_INITDIALOG:
		pvar = (PTInstVar) lParam;
		pvar->auth_state.auth_dialog = dlg;
		SetWindowLong(dlg, DWL_USER, lParam);

		init_TIS_dlg(pvar, dlg);

#ifndef NO_I18N
		font = (HFONT)SendMessage(dlg, WM_GETFONT, 0, 0);
		GetObject(font, sizeof(LOGFONT), &logfont);
		if (UTIL_get_lang_font("DLG_TAHOMA_FONT", dlg, &logfont, &DlgTisFont, pvar)) {
			SendDlgItemMessage(dlg, IDC_SSHAUTHBANNER, WM_SETFONT, (WPARAM)DlgTisFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_SSHAUTHBANNER2, WM_SETFONT, (WPARAM)DlgTisFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_SSHPASSWORD, WM_SETFONT, (WPARAM)DlgTisFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDOK, WM_SETFONT, (WPARAM)DlgTisFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDCANCEL, WM_SETFONT, (WPARAM)DlgTisFont, MAKELPARAM(TRUE,0));
		}
		else {
			DlgTisFont = NULL;
		}
#endif

		return FALSE;			/* because we set the focus */

	case WM_COMMAND:
		pvar = (PTInstVar) GetWindowLong(dlg, DWL_USER);

		switch (LOWORD(wParam)) {
		case IDOK:
#ifndef NO_I18N
			if (DlgTisFont != NULL) {
				DeleteObject(DlgTisFont);
			}
#endif

			return end_TIS_dlg(pvar, dlg);

		case IDCANCEL:			/* kill the connection */
			pvar->auth_state.auth_dialog = NULL;
			notify_closed_connection(pvar);
			EndDialog(dlg, 0);

#ifndef NO_I18N
			if (DlgTisFont != NULL) {
				DeleteObject(DlgTisFont);
			}
#endif

			return TRUE;

		default:
			return FALSE;
		}

	default:
		return FALSE;
	}
}

void AUTH_do_cred_dialog(PTInstVar pvar)
{
	if (pvar->auth_state.auth_dialog == NULL) {
		HWND cur_active = GetActiveWindow();
		DLGPROC dlg_proc;
		LPCTSTR dialog_template;

		switch (pvar->auth_state.mode) {
		case TIS_AUTH_MODE:
			dialog_template = MAKEINTRESOURCE(IDD_SSHTISAUTH);
			dlg_proc = TIS_dlg_proc;
			break;
		case GENERIC_AUTH_MODE:
		default:
			dialog_template = MAKEINTRESOURCE(IDD_SSHAUTH);
			dlg_proc = auth_dlg_proc;
		}

		if (!DialogBoxParam(hInst, dialog_template,
							cur_active !=
							NULL ? cur_active : pvar->NotificationWindow,
							dlg_proc, (LPARAM) pvar) == -1) {
#ifndef NO_I18N
			strcpy(pvar->ts->UIMsg,
				   "Unable to display authentication dialog box.\n"
				   "Connection terminated.");
			UTIL_get_lang_msg("MSG_CREATEWINDOW_AUTH_ERROR", pvar);
			notify_fatal_error(pvar, pvar->ts->UIMsg);
#else
			notify_fatal_error(pvar,
							   "Unable to display authentication dialog box.\n"
							   "Connection terminated.");
#endif
		}
	}
}

static void init_default_auth_dlg(PTInstVar pvar, HWND dlg)
{
#ifndef NO_I18N
	GetWindowText(dlg, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("DLG_AUTHSETUP_TITLE", pvar);
	SetWindowText(dlg, pvar->ts->UIMsg);

	GetDlgItemText(dlg, IDC_SSHAUTHBANNER, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("DLG_AUTHSETUP_BANNER", pvar);
	SetDlgItemText(dlg, IDC_SSHAUTHBANNER, pvar->ts->UIMsg);

	GetDlgItemText(dlg, IDC_SSHUSERNAMELABEL, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("DLG_AUTHSETUP_USERNAME", pvar);
	SetDlgItemText(dlg, IDC_SSHUSERNAMELABEL, pvar->ts->UIMsg);

	GetDlgItemText(dlg, IDC_SSHUSEPASSWORD, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("DLG_AUTHSETUP_METHOD_PASSWORD", pvar);
	SetDlgItemText(dlg, IDC_SSHUSEPASSWORD, pvar->ts->UIMsg);

	GetDlgItemText(dlg, IDC_KEYBOARD_INTERACTIVE_CHECK, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("DLG_AUTHSETUP_METHOD_PASSWORD_KBDINT", pvar);
	SetDlgItemText(dlg, IDC_KEYBOARD_INTERACTIVE_CHECK, pvar->ts->UIMsg);

	GetDlgItemText(dlg, IDC_SSHUSERSA, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("DLG_AUTHSETUP_METHOD_RSA", pvar);
	SetDlgItemText(dlg, IDC_SSHUSERSA, pvar->ts->UIMsg);

	GetDlgItemText(dlg, IDC_SSHUSERHOSTS, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("DLG_AUTHSETUP_METHOD_RHOST", pvar);
	SetDlgItemText(dlg, IDC_SSHUSERHOSTS, pvar->ts->UIMsg);

	GetDlgItemText(dlg, IDC_SSHUSETIS, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("DLG_AUTHSETUP_METHOD_CHALLENGE", pvar);
	SetDlgItemText(dlg, IDC_SSHUSETIS, pvar->ts->UIMsg);

	GetDlgItemText(dlg, IDC_CHOOSERSAFILE, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("DLG_AUTH_PRIVATEKEY", pvar);
	SetDlgItemText(dlg, IDC_CHOOSERSAFILE, pvar->ts->UIMsg);

	GetDlgItemText(dlg, IDC_LOCALUSERNAMELABEL, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("DLG_AUTH_LOCALUSER", pvar);
	SetDlgItemText(dlg, IDC_LOCALUSERNAMELABEL, pvar->ts->UIMsg);

	GetDlgItemText(dlg, IDC_CHOOSEHOSTRSAFILE, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("DLG_AUTH_HOST_PRIVATEKEY", pvar);
	SetDlgItemText(dlg, IDC_CHOOSEHOSTRSAFILE, pvar->ts->UIMsg);

	GetDlgItemText(dlg, IDOK, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("BTN_OK", pvar);
	SetDlgItemText(dlg, IDOK, pvar->ts->UIMsg);

	GetDlgItemText(dlg, IDCANCEL, pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg));
	UTIL_get_lang_msg("BTN_CANCEL", pvar);
	SetDlgItemText(dlg, IDCANCEL, pvar->ts->UIMsg);
#endif

	switch (pvar->settings.DefaultAuthMethod) {
	case SSH_AUTH_RSA:
		CheckRadioButton(dlg, IDC_SSHUSEPASSWORD, MAX_AUTH_CONTROL,
						 IDC_SSHUSERSA);
		break;
	case SSH_AUTH_RHOSTS:
	case SSH_AUTH_RHOSTS_RSA:
		CheckRadioButton(dlg, IDC_SSHUSEPASSWORD, MAX_AUTH_CONTROL,
						 IDC_SSHUSERHOSTS);
		break;
	case SSH_AUTH_TIS:
		CheckRadioButton(dlg, IDC_SSHUSEPASSWORD, MAX_AUTH_CONTROL,
						 IDC_SSHUSETIS);
		break;
	case SSH_AUTH_PASSWORD:
	default:
		CheckRadioButton(dlg, IDC_SSHUSEPASSWORD, MAX_AUTH_CONTROL,
						 IDC_SSHUSEPASSWORD);
	}

	SetDlgItemText(dlg, IDC_SSHUSERNAME, pvar->settings.DefaultUserName);
	SetDlgItemText(dlg, IDC_RSAFILENAME,
				   pvar->settings.DefaultRSAPrivateKeyFile);
	SetDlgItemText(dlg, IDC_HOSTRSAFILENAME,
				   pvar->settings.DefaultRhostsHostPrivateKeyFile);
	SetDlgItemText(dlg, IDC_LOCALUSERNAME,
				   pvar->settings.DefaultRhostsLocalUserName);

	// SSH2 keyboard-interactive method (2005.2.22 yutaka)
	if (pvar->settings.ssh2_keyboard_interactive) {
		SendMessage(GetDlgItem(dlg, IDC_KEYBOARD_INTERACTIVE_CHECK), BM_SETCHECK, BST_CHECKED, 0);
	}

}

static BOOL end_default_auth_dlg(PTInstVar pvar, HWND dlg)
{
	if (IsDlgButtonChecked(dlg, IDC_SSHUSERSA)) {
		pvar->settings.DefaultAuthMethod = SSH_AUTH_RSA;
	} else if (IsDlgButtonChecked(dlg, IDC_SSHUSERHOSTS)) {
		if (GetWindowTextLength(GetDlgItem(dlg, IDC_HOSTRSAFILENAME)) > 0) {
			pvar->settings.DefaultAuthMethod = SSH_AUTH_RHOSTS_RSA;
		} else {
			pvar->settings.DefaultAuthMethod = SSH_AUTH_RHOSTS;
		}
	} else if (IsDlgButtonChecked(dlg, IDC_SSHUSETIS)) {
		pvar->settings.DefaultAuthMethod = SSH_AUTH_TIS;
	} else {
		pvar->settings.DefaultAuthMethod = SSH_AUTH_PASSWORD;
	}

	GetDlgItemText(dlg, IDC_SSHUSERNAME, pvar->settings.DefaultUserName,
				   sizeof(pvar->settings.DefaultUserName));
	GetDlgItemText(dlg, IDC_RSAFILENAME,
				   pvar->settings.DefaultRSAPrivateKeyFile,
				   sizeof(pvar->settings.DefaultRSAPrivateKeyFile));
	GetDlgItemText(dlg, IDC_HOSTRSAFILENAME,
				   pvar->settings.DefaultRhostsHostPrivateKeyFile,
				   sizeof(pvar->settings.DefaultRhostsHostPrivateKeyFile));
	GetDlgItemText(dlg, IDC_LOCALUSERNAME,
				   pvar->settings.DefaultRhostsLocalUserName,
				   sizeof(pvar->settings.DefaultRhostsLocalUserName));

	// SSH2 keyboard-interactive method (2005.2.22 yutaka)
	{
		LRESULT ret;
		ret = SendMessage(GetDlgItem(dlg, IDC_KEYBOARD_INTERACTIVE_CHECK), BM_GETCHECK, 0, 0);
		if (ret & BST_CHECKED) {
			pvar->settings.ssh2_keyboard_interactive = 1;
		} else {
			pvar->settings.ssh2_keyboard_interactive = 0;
		}
	}

	EndDialog(dlg, 1);
	return TRUE;
}

static BOOL CALLBACK default_auth_dlg_proc(HWND dlg, UINT msg,
										   WPARAM wParam, LPARAM lParam)
{
	PTInstVar pvar;
#ifndef NO_I18N
	LOGFONT logfont;
	HFONT font;
#endif

	switch (msg) {
	case WM_INITDIALOG:
		pvar = (PTInstVar) lParam;
		SetWindowLong(dlg, DWL_USER, lParam);

		init_default_auth_dlg(pvar, dlg);

#ifndef NO_I18N
		font = (HFONT)SendMessage(dlg, WM_GETFONT, 0, 0);
		GetObject(font, sizeof(LOGFONT), &logfont);
		if (UTIL_get_lang_font("DLG_TAHOMA_FONT", dlg, &logfont, &DlgAuthSetupFont, pvar)) {
			SendDlgItemMessage(dlg, IDC_SSHAUTHBANNER, WM_SETFONT, (WPARAM)DlgAuthSetupFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_SSHUSERNAMELABEL, WM_SETFONT, (WPARAM)DlgAuthSetupFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_SSHUSERNAME, WM_SETFONT, (WPARAM)DlgAuthSetupFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_SSHUSEPASSWORD, WM_SETFONT, (WPARAM)DlgAuthSetupFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_KEYBOARD_INTERACTIVE_CHECK, WM_SETFONT, (WPARAM)DlgAuthSetupFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_SSHUSERSA, WM_SETFONT, (WPARAM)DlgAuthSetupFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_CHOOSERSAFILE, WM_SETFONT, (WPARAM)DlgAuthSetupFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_RSAFILENAME, WM_SETFONT, (WPARAM)DlgAuthSetupFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_SSHUSERHOSTS, WM_SETFONT, (WPARAM)DlgAuthSetupFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_LOCALUSERNAMELABEL, WM_SETFONT, (WPARAM)DlgAuthSetupFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_LOCALUSERNAME, WM_SETFONT, (WPARAM)DlgAuthSetupFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_CHOOSEHOSTRSAFILE, WM_SETFONT, (WPARAM)DlgAuthSetupFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_HOSTRSAFILENAME, WM_SETFONT, (WPARAM)DlgAuthSetupFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_SSHUSETIS, WM_SETFONT, (WPARAM)DlgAuthSetupFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDOK, WM_SETFONT, (WPARAM)DlgAuthSetupFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDCANCEL, WM_SETFONT, (WPARAM)DlgAuthSetupFont, MAKELPARAM(TRUE,0));
		}
		else {
			DlgAuthSetupFont = NULL;
		}
#endif

		return TRUE;			/* because we do not set the focus */

	case WM_COMMAND:
		pvar = (PTInstVar) GetWindowLong(dlg, DWL_USER);

		switch (LOWORD(wParam)) {
		case IDOK:

#ifndef NO_I18N
			if (DlgAuthSetupFont != NULL) {
				DeleteObject(DlgAuthSetupFont);
			}
#endif

			return end_default_auth_dlg(pvar, dlg);

		case IDCANCEL:
			EndDialog(dlg, 0);

#ifndef NO_I18N
			if (DlgAuthSetupFont != NULL) {
				DeleteObject(DlgAuthSetupFont);
			}
#endif

			return TRUE;

		case IDC_CHOOSERSAFILE:
#ifndef NO_I18N
			choose_RSA_key_file(dlg, pvar);
#else
			choose_RSA_key_file(dlg);
#endif
			return TRUE;

		case IDC_CHOOSEHOSTRSAFILE:
#ifndef NO_I18N
			choose_host_RSA_key_file(dlg, pvar);
#else
			choose_host_RSA_key_file(dlg);
#endif
			return TRUE;

		default:
			return FALSE;
		}

	default:
		return FALSE;
	}
}

void AUTH_init(PTInstVar pvar)
{
	pvar->auth_state.failed_method = SSH_AUTH_NONE;
	pvar->auth_state.auth_dialog = NULL;
	pvar->auth_state.user = NULL;
	pvar->auth_state.flags = 0;
	pvar->auth_state.TIS_prompt = NULL;
	pvar->auth_state.supported_types = 0;
	pvar->auth_state.cur_cred.method = SSH_AUTH_NONE;
	pvar->auth_state.cur_cred.password = NULL;
	pvar->auth_state.cur_cred.rhosts_client_user = NULL;
	pvar->auth_state.cur_cred.key_pair = NULL;
	AUTH_set_generic_mode(pvar);
}

void AUTH_set_generic_mode(PTInstVar pvar)
{
	pvar->auth_state.mode = GENERIC_AUTH_MODE;
	destroy_malloced_string(&pvar->auth_state.TIS_prompt);
}

void AUTH_set_TIS_mode(PTInstVar pvar, char FAR * prompt, int len)
{
	if (pvar->auth_state.cur_cred.method == SSH_AUTH_TIS) {
		pvar->auth_state.mode = TIS_AUTH_MODE;

		destroy_malloced_string(&pvar->auth_state.TIS_prompt);
		pvar->auth_state.TIS_prompt = malloc(len + 1);
		memcpy(pvar->auth_state.TIS_prompt, prompt, len);
		pvar->auth_state.TIS_prompt[len] = 0;
	} else {
		AUTH_set_generic_mode(pvar);
	}
}

void AUTH_do_default_cred_dialog(PTInstVar pvar)
{
	HWND cur_active = GetActiveWindow();

	if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SSHAUTHSETUP),
					   cur_active !=
					   NULL ? cur_active : pvar->NotificationWindow,
					   default_auth_dlg_proc, (LPARAM) pvar) == -1) {
#ifndef NO_I18N
		strcpy(pvar->ts->UIMsg, "Unable to display authentication setup dialog box.");
		UTIL_get_lang_msg("MSG_CREATEWINDOW_AUTHSETUP_ERROR", pvar);
		notify_nonfatal_error(pvar, pvar->ts->UIMsg);
#else
		notify_nonfatal_error(pvar,
							  "Unable to display authentication setup dialog box.");
#endif
	}
}

void AUTH_destroy_cur_cred(PTInstVar pvar)
{
	destroy_malloced_string(&pvar->auth_state.cur_cred.password);
	destroy_malloced_string(&pvar->auth_state.cur_cred.rhosts_client_user);
	if (pvar->auth_state.cur_cred.key_pair != NULL) {
		CRYPT_free_key_pair(pvar->auth_state.cur_cred.key_pair);
		pvar->auth_state.cur_cred.key_pair = NULL;
	}
}

static char FAR *get_auth_method_name(SSHAuthMethod auth)
{
	switch (auth) {
	case SSH_AUTH_PASSWORD:
		return "password";
	case SSH_AUTH_RSA:
		return "RSA";
	case SSH_AUTH_RHOSTS:
		return "rhosts";
	case SSH_AUTH_RHOSTS_RSA:
		return "rhosts with RSA";
	case SSH_AUTH_TIS:
		return "challenge/response (TIS)";
	default:
		return "unknown method";
	}
}

void AUTH_get_auth_info(PTInstVar pvar, char FAR * dest, int len)
{
	char *method = "unknown";

	if (pvar->auth_state.user == NULL) {
		strncpy(dest, "None", len);
	} else if (pvar->auth_state.cur_cred.method != SSH_AUTH_NONE) {
		if (SSHv1(pvar)) {
#ifndef NO_I18N
			strcpy(pvar->ts->UIMsg, "User '%s', using %s");
			UTIL_get_lang_msg("DLG_ABOUT_AUTH_INFO", pvar);
			_snprintf(dest, len, pvar->ts->UIMsg, pvar->auth_state.user,
					get_auth_method_name(pvar->auth_state.cur_cred.method));
#else
			_snprintf(dest, len, "User '%s', using %s", pvar->auth_state.user,
					get_auth_method_name(pvar->auth_state.cur_cred.method));
#endif

		} else { 
			// SSH2:認証メソッドの判別 (2004.12.23 yutaka)
			// keyboard-interactiveメソッドを追加 (2005.3.12 yutaka)
			if (pvar->auth_state.cur_cred.method == SSH_AUTH_PASSWORD ||
				pvar->auth_state.cur_cred.method == SSH_AUTH_TIS) {
				// keyboard-interactiveメソッドを追加 (2005.1.24 yutaka)
				if (pvar->keyboard_interactive_done == 1 ||
					pvar->auth_state.cur_cred.method == SSH_AUTH_TIS) {
					method = "keyboard-interactive";
				} else {
					method = get_auth_method_name(pvar->auth_state.cur_cred.method);
				}
#ifndef NO_I18N
				strcpy(pvar->ts->UIMsg, "User '%s', using %s");
				UTIL_get_lang_msg("DLG_ABOUT_AUTH_INFO", pvar);
				_snprintf(dest, len, pvar->ts->UIMsg, pvar->auth_state.user, method);
#else
				_snprintf(dest, len, "User '%s', using %s", pvar->auth_state.user, method);
#endif

			} else {
				if (pvar->auth_state.cur_cred.key_pair->RSA_key != NULL) {
					method = "RSA";
				} else if (pvar->auth_state.cur_cred.key_pair->DSA_key != NULL) {
					method = "DSA";
				}
#ifndef NO_I18N
				strcpy(pvar->ts->UIMsg, "User '%s', using %s");
				UTIL_get_lang_msg("DLG_ABOUT_AUTH_INFO", pvar);
				_snprintf(dest, len, pvar->ts->UIMsg, pvar->auth_state.user, method);
#else
				_snprintf(dest, len, "User '%s', using %s", pvar->auth_state.user, method);
#endif
			}

		}

	} else {
#ifndef NO_I18N
		strcpy(pvar->ts->UIMsg, "User '%s', using %s");
		UTIL_get_lang_msg("DLG_ABOUT_AUTH_INFO", pvar);
		_snprintf(dest, len, pvar->ts->UIMsg, pvar->auth_state.user,
				  get_auth_method_name(pvar->auth_state.failed_method));
#else
		_snprintf(dest, len, "User '%s', using %s", pvar->auth_state.user,
				  get_auth_method_name(pvar->auth_state.failed_method));
#endif
	}

	dest[len - 1] = 0;
}

void AUTH_notify_disconnecting(PTInstVar pvar)
{
	if (pvar->auth_state.auth_dialog != NULL) {
		PostMessage(pvar->auth_state.auth_dialog, WM_COMMAND, IDCANCEL, 0);
		/* the main window might not go away if it's not enabled. (see vtwin.cpp) */
		EnableWindow(pvar->NotificationWindow, TRUE);
	}
}

void AUTH_end(PTInstVar pvar)
{
	destroy_malloced_string(&pvar->auth_state.user);
	destroy_malloced_string(&pvar->auth_state.TIS_prompt);

	AUTH_destroy_cur_cred(pvar);
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.32  2007/03/17 11:53:29  doda
 * プレインテキスト認証と一緒にキーボードインタラクティブ認証を使用するように設定している時、SSH認証ダイアログでプレインテキスト認証がrhosts認証と表示されるのを修正した。
 *
 * Revision 1.31  2007/02/17 17:31:55  yasuhide
 * コマンドラインでask4passwd が指定された時、ログインダイアログのパスフレーズ入力にフォーカスする
 *
 * Revision 1.30  2007/02/17 16:20:21  yasuhide
 * SSH2 鍵を用いた認証でパスフレーズを間違えた際、パスフレーズダイアログにフォーカスを移す
 *
 * Revision 1.29  2007/02/17 14:01:03  maya
 * 表示メッセージの読み込み対応
 *
 * Revision 1.28  2007/01/31 13:15:08  maya
 * 言語ファイルがないときに \0 が正しく認識されないバグを修正した。
 *
 * Revision 1.27  2007/01/27 14:29:59  maya
 * 英語版と日本語版のアクセラレータキーをあわせた。
 *
 * Revision 1.26  2007/01/22 13:45:19  maya
 * 表示メッセージの読み込み対応
 *
 * Revision 1.25  2007/01/04 08:36:42  maya
 * フォントを変更する部分を追加した。
 *
 * Revision 1.24  2006/12/06 14:31:13  maya
 * 表示メッセージの読み込み対応
 *
 * Revision 1.23  2006/12/06 14:25:40  maya
 * 表示メッセージの読み込み対応
 *
 * Revision 1.22  2006/11/29 16:58:52  maya
 * 表示メッセージの読み込み対応
 *
 * Revision 1.21  2006/11/23 02:19:30  maya
 * 表示メッセージを言語ファイルから読み込みむコードの作成を開始した。
 *
 * Revision 1.20  2006/09/18 05:08:04  maya
 * コマンドラインパラメータ '/ask4passwd' を追加した。
 *
 * Revision 1.19  2006/08/05 03:47:49  yutakakn
 * パスワードをメモリ上に覚えておくかどうかの設定は teraterm.ini に反映させるようにした。
 *
 * Revision 1.18  2006/08/03 15:04:37  yutakakn
 * パスワードをメモリ上に保持するかどうかを決めるチェックボックスを認証ダイアログに追加した。
 *
 * Revision 1.17  2005/09/05 10:46:22  yutakakn
 * '/I' 指定があるときのみ認証ダイアログを最小化するようにした。
 *
 * Revision 1.16  2005/08/26 16:26:02  yutakakn
 * 自動ログイン時にSSH認証ダイアログを最小化するようにした。
 *
 * Revision 1.15  2005/07/15 14:58:04  yutakakn
 * SSH1接続で一度ユーザ認証に失敗すると、その後認証ができなくなるバグを修正。
 *
 * Revision 1.14  2005/04/26 13:57:57  yutakakn
 * private keyファイルダイアログに3ファイルフィルタを追加した。
 *
 * Revision 1.13  2005/04/08 14:55:03  yutakakn
 * "Duplicate session"においてSSH自動ログインを行うようにした。
 *
 * Revision 1.12  2005/03/23 12:39:35  yutakakn
 * SSH2認証ダイアログの Use challenge/response to log in にアクセラレータキーを割り当てた。
 *
 * Revision 1.11  2005/03/12 15:07:33  yutakakn
 * SSH2 keyboard-interactive認証をTISダイアログに実装した。
 *
 * Revision 1.10  2005/03/12 12:08:05  yutakakn
 * パスワード認証の前に行うkeyboard-interactiveメソッドで、デフォルト設定値を無効(0)にした。
 * また、認証ダイアログのラベル名を設定の有無により変更するようにした。
 *
 * Revision 1.9  2005/02/22 08:48:11  yutakakn
 * TTSSH setupダイアログに HeartBeat 設定を追加。
 * TTSSH authentication setupダイアログに keyboard-interactive 設定を追加。
 *
 * Revision 1.8  2005/01/27 13:30:33  yutakakn
 * 公開鍵認証自動ログインをサポート。
 * /auth=publickey, /keyfile オプションを新規追加した。
 * また、空白を含む引数をサポート。
 *
 * Revision 1.7  2005/01/25 13:38:22  yutakakn
 * SSH認証ダイアログで、Rhosts/TISがグレーになる前に、Enterキーを押下すると、
 * アプリケーションエラーとなる現象に対処した。
 *
 * Revision 1.6  2005/01/24 14:07:07  yutakakn
 * ・keyboard-interactive認証をサポートした。
 * 　それに伴い、teraterm.iniに "KeyboardInteractive" エントリを追加した。
 * ・バージョンダイアログに OpenSSLバージョン を追加
 *
 * Revision 1.5  2004/12/27 14:35:41  yutakakn
 * SSH2秘密鍵読み込み失敗時のエラーメッセージを強化した。
 *
 * Revision 1.4  2004/12/22 17:28:14  yutakakn
 * SSH2公開鍵認証(RSA/DSA)をサポートした。
 *
 * Revision 1.3  2004/12/16 13:01:09  yutakakn
 * SSH自動ログインでアプリケーションエラーとなる現象を修正した。
 *
 * Revision 1.2  2004/12/01 15:37:49  yutakakn
 * SSH2自動ログイン機能を追加。
 * 現状、パスワード認証のみに対応。
 * ・コマンドライン
 *   /ssh /auth=認証メソッド /user=ユーザ名 /passwd=パスワード
 *
 */
