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

#include <io.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

#include "resource.h"
#include "keyfiles.h"

#define AUTH_START_USER_AUTH_ON_ERROR_END 1

#define MAX_AUTH_CONTROL IDC_SSHUSETIS

static void destroy_malloced_string(char FAR * FAR * str)
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

	if (strlen(buf) + strlen(SSH_get_host_name(pvar)) < sizeof(buf) - 2) {
		strcat(buf, SSH_get_host_name(pvar));
	}
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

	init_auth_machine_banner(pvar, dlg);
	init_password_control(dlg);

	if (pvar->auth_state.failed_method != SSH_AUTH_NONE) {
		/* must be retrying a failed attempt */
		SetDlgItemText(dlg, IDC_SSHAUTHBANNER2,
					   "Authentication failed. Please retry.");
		SetWindowText(dlg, "Retrying SSH Authentication");
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
	if (pvar->ssh2_autologin == 1) {
		SetDlgItemText(dlg, IDC_SSHUSERNAME, pvar->ssh2_username);
		EnableWindow(GetDlgItem(dlg, IDC_SSHUSERNAME), FALSE);
		EnableWindow(GetDlgItem(dlg, IDC_SSHUSERNAMELABEL), FALSE);

		SetDlgItemText(dlg, IDC_SSHPASSWORD, pvar->ssh2_password);
		EnableWindow(GetDlgItem(dlg, IDC_SSHPASSWORD), FALSE);
		EnableWindow(GetDlgItem(dlg, IDC_SSHPASSWORDCAPTION), FALSE);

		if (pvar->ssh2_authmethod == SSH_AUTH_PASSWORD) {
			CheckRadioButton(dlg, IDC_SSHUSEPASSWORD, MAX_AUTH_CONTROL, IDC_SSHUSEPASSWORD);

		} else if (pvar->ssh2_authmethod == SSH_AUTH_RSA) {
			CheckRadioButton(dlg, IDC_SSHUSEPASSWORD, MAX_AUTH_CONTROL, IDC_SSHUSERSA);

			SetDlgItemText(dlg, IDC_RSAFILENAME, pvar->ssh2_keyfile);
			EnableWindow(GetDlgItem(dlg, IDC_CHOOSERSAFILE), FALSE);
			EnableWindow(GetDlgItem(dlg, IDC_RSAFILENAME), FALSE);

		} else {
			// TODO

		}
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

static int get_key_file_name(HWND parent, char FAR * buf, int bufsize)
{
#ifdef TERATERM32
	OPENFILENAME params;
	char fullname_buf[2048] = "identity";

	ZeroMemory(&params, sizeof(params));
	params.lStructSize = sizeof(OPENFILENAME);
	params.hwndOwner = parent;
	// フィルタの追加 (2004.12.19 yutaka)
	params.lpstrFilter = "identity(RSA1)\0identity\0id_rsa(SSH2)\0id_rsa\0id_dsa(SSH2)\0id_dsa\0all(*.*)\0*.*\0\0";
	params.lpstrCustomFilter = NULL;
	params.nFilterIndex = 0;
	buf[0] = 0;
	params.lpstrFile = fullname_buf;
	params.nMaxFile = sizeof(fullname_buf);
	params.lpstrFileTitle = NULL;
	params.lpstrInitialDir = NULL;
	params.lpstrTitle = "Choose a file with the RSA private key";
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

static void choose_RSA_key_file(HWND dlg)
{
	char buf[1024];

	if (get_key_file_name(dlg, buf, sizeof(buf))) {
		SetDlgItemText(dlg, IDC_RSAFILENAME, buf);
	}
}

static void choose_host_RSA_key_file(HWND dlg)
{
	char buf[1024];

	if (get_key_file_name(dlg, buf, sizeof(buf))) {
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
			notify_nonfatal_error(pvar,
								  "You must specify a file containing the RSA/DSA private key.");
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
				_snprintf(buf, sizeof(buf), "read error SSH2 private key file\r\n%s", errmsg);
				notify_nonfatal_error(pvar, buf);
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
	if (method == SSH_AUTH_PASSWORD) {
		pvar->auth_state.cur_cred.password = password;
	} else {
		destroy_malloced_string(&password);
	}
	if (method == SSH_AUTH_RHOSTS || method == SSH_AUTH_RHOSTS_RSA) {
		if (pvar->session_settings.DefaultAuthMethod != SSH_AUTH_RHOSTS) {
			notify_nonfatal_error(pvar,
								  "Rhosts authentication will probably fail because it was not "
								  "the default authentication method.\n"
								  "To use Rhosts authentication "
								  "in TTSSH, you need to set it to be the default by restarting\n"
								  "TTSSH and selecting \"SSH Authentication...\" from the Setup menu"
								  "before connecting.");
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
	return TRUE;
}

static BOOL CALLBACK auth_dlg_proc(HWND dlg, UINT msg, WPARAM wParam,
								   LPARAM lParam)
{
	const int IDC_TIMER1 = 300;
	const int autologin_timeout = 10; // ミリ秒
	PTInstVar pvar;

	switch (msg) {
	case WM_INITDIALOG:
		pvar = (PTInstVar) lParam;
		pvar->auth_state.auth_dialog = dlg;
		SetWindowLong(dlg, DWL_USER, lParam);

		init_auth_dlg(pvar, dlg);

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
			if ((pvar->ssh_state.status_flags & STATUS_DONT_SEND_USER_NAME)) {
				return FALSE;
			}
			return end_auth_dlg(pvar, dlg);

		case IDCANCEL:			/* kill the connection */
			pvar->auth_state.auth_dialog = NULL;
			notify_closed_connection(pvar);
			EndDialog(dlg, 0);
			return TRUE;

		case IDC_SSHUSEPASSWORD:
		case IDC_SSHUSERSA:
		case IDC_SSHUSERHOSTS:
		case IDC_SSHUSETIS:
			set_auth_options_status(dlg, LOWORD(wParam));
			return TRUE;

		case IDC_CHOOSERSAFILE:
			choose_RSA_key_file(dlg);
			return TRUE;

		case IDC_CHOOSEHOSTRSAFILE:
			choose_host_RSA_key_file(dlg);
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
		types &= (1 << SSH_AUTH_PASSWORD) | (1 << SSH_AUTH_RSA)
			| (1 << SSH_AUTH_DSA);
	}
	pvar->auth_state.supported_types = types;

	if (types == 0) {
		notify_fatal_error(pvar,
						   "Server does not support any of the authentication options\n"
						   "provided by TTSSH. This connection will now close.");
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

	SSH_notify_cred(pvar);

	EndDialog(dlg, 1);
	return TRUE;
}

static BOOL CALLBACK TIS_dlg_proc(HWND dlg, UINT msg, WPARAM wParam,
								  LPARAM lParam)
{
	PTInstVar pvar;

	switch (msg) {
	case WM_INITDIALOG:
		pvar = (PTInstVar) lParam;
		pvar->auth_state.auth_dialog = dlg;
		SetWindowLong(dlg, DWL_USER, lParam);

		init_TIS_dlg(pvar, dlg);
		return FALSE;			/* because we set the focus */

	case WM_COMMAND:
		pvar = (PTInstVar) GetWindowLong(dlg, DWL_USER);

		switch (LOWORD(wParam)) {
		case IDOK:
			return end_TIS_dlg(pvar, dlg);

		case IDCANCEL:			/* kill the connection */
			pvar->auth_state.auth_dialog = NULL;
			notify_closed_connection(pvar);
			EndDialog(dlg, 0);
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
			notify_fatal_error(pvar,
							   "Unable to display authentication dialog box.\n"
							   "Connection terminated.");
		}
	}
}

static void init_default_auth_dlg(PTInstVar pvar, HWND dlg)
{
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

	switch (msg) {
	case WM_INITDIALOG:
		pvar = (PTInstVar) lParam;
		SetWindowLong(dlg, DWL_USER, lParam);

		init_default_auth_dlg(pvar, dlg);
		return TRUE;			/* because we do not set the focus */

	case WM_COMMAND:
		pvar = (PTInstVar) GetWindowLong(dlg, DWL_USER);

		switch (LOWORD(wParam)) {
		case IDOK:
			return end_default_auth_dlg(pvar, dlg);

		case IDCANCEL:
			EndDialog(dlg, 0);
			return TRUE;

		case IDC_CHOOSERSAFILE:
			choose_RSA_key_file(dlg);
			return TRUE;

		case IDC_CHOOSEHOSTRSAFILE:
			choose_host_RSA_key_file(dlg);
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
		notify_nonfatal_error(pvar,
							  "Unable to display authentication setup dialog box.");
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
			_snprintf(dest, len, "User '%s', using %s", pvar->auth_state.user,
					get_auth_method_name(pvar->auth_state.cur_cred.method));

		} else { // SSH2:認証メソッドの判別 (2004.12.23 yutaka)
			if (pvar->auth_state.cur_cred.method == SSH_AUTH_PASSWORD) {
				// keyboard-interactiveメソッドを追加 (2005.1.24 yutaka)
				if (pvar->keyboard_interactive_done == 1) {
					method = "keyboard-interactive";
				} else {
					method = get_auth_method_name(pvar->auth_state.cur_cred.method);
				}
				_snprintf(dest, len, "User '%s', using %s", pvar->auth_state.user, method);

			} else {
				if (pvar->auth_state.cur_cred.key_pair->RSA_key != NULL) {
					method = "RSA";
				} else if (pvar->auth_state.cur_cred.key_pair->DSA_key != NULL) {
					method = "DSA";
				}
				_snprintf(dest, len, "User '%s', using %s", pvar->auth_state.user, method);
			}

		}

	} else {
		_snprintf(dest, len, "User '%s', using %s", pvar->auth_state.user,
				  get_auth_method_name(pvar->auth_state.failed_method));
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
