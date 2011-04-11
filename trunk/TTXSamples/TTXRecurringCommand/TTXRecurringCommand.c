#include <ctype.h>
#include <stdio.h>
#include "teraterm.h"
#include "tttypes.h"
#include "ttplugin.h"
#include "ttlib.h"
#include "tt_res.h"
#include "resource.h"
#include "i18n.h"

#include "compat_w95.h"

#define ORDER 4000

#define MINIMUM_INTERVAL 1
#define DEFAULT_INTERVAL 300

#define IdRecurringTimer 3001

#define ID_MENU_SETUP 55500
#define ID_MENU_CONTROL 55501

#define SECTION "TTXRecurringCommand"

static HANDLE hInst;

typedef struct {
	PTTSet ts;
	PComVar cv;
	Tsend origPsend;
	TWriteFile origPWriteFile;
	PReadIniFile origReadIniFile;
	PWriteIniFile origWriteIniFile;
	HMENU SetupMenu;
	HMENU ControlMenu;
	int interval;
	BOOL enable;
	int cmdLen;
	unsigned char command[50];
	unsigned char orgCommand[50];
} TInstVar;

typedef TInstVar FAR * PTInstVar;
PTInstVar pvar;
static TInstVar InstVar;

#define GetFileMenu(menu)       GetSubMenuByChildID(menu, ID_FILE_NEWCONNECTION)
#define GetEditMenu(menu)       GetSubMenuByChildID(menu, ID_EDIT_COPY2)
#define GetSetupMenu(menu)      GetSubMenuByChildID(menu, ID_SETUP_TERMINAL)
#define GetControlMenu(menu)    GetSubMenuByChildID(menu, ID_CONTROL_RESETTERMINAL)
#define GetHelpMenu(menu)       GetSubMenuByChildID(menu, ID_HELP_ABOUT)

HMENU GetSubMenuByChildID(HMENU menu, UINT id) {
	int i, j, items, subitems, cur_id;
	HMENU m;

	items = GetMenuItemCount(menu);

	for (i=0; i<items; i++) {
		if (m = GetSubMenu(menu, i)) {
			subitems = GetMenuItemCount(m);
			for (j=0; j<subitems; j++) {
				cur_id = GetMenuItemID(m, j);
				if (cur_id == id) {
					return m;
				}
			}
		}
	}
	return NULL;
}

WORD GetOnOff(PCHAR Sect, PCHAR Key, PCHAR FName, BOOL Default)
{
	char Temp[4];
	GetPrivateProfileString(Sect, Key, "", Temp, sizeof(Temp), FName);
	if (Default) {
		if (_stricmp(Temp, "off") == 0)
			return 0;
		else
			return 1;
	}
	else {
		if (_stricmp(Temp, "on") == 0)
			return 1;
		else
			return 0;
	}
}

//
// \n, \t����W�J����B
// common/ttlib.c:RestoreNewLine()���x�[�X�B
//
void UnEscapeStr(BYTE *Text)
{
	int i;
	unsigned char *src, *dst;

	src = dst = Text;
	while (*src && *src != '\\') {
		src++; dst++;
	}

	while (*src) {
		if (*src == '\\') {
			switch (*++src) {
				case '\\': *dst = '\\';   break;
				case 'n':  *dst = '\n';   break;
				case 't':  *dst = '\t';   break;
				case 'a':  *dst = '\a';   break;
				case 'b':  *dst = '\b';   break;
				case 'f':  *dst = '\f';   break;
				case 'r':  *dst = '\r';   break;
				case 'v':  *dst = '\v';   break;
				case 'e':  *dst = '\033'; break;
//				case '"':  *dst = '"';    break;
//				case '\'': *dst = '\'';   break;
				case 'x':
					if (isxdigit(src[1]) && isxdigit(src[2])) {
						src++;
						if (isalpha(*src)) {
							*dst = (*src|0x20) - 'a' + 10;
						}
						else {
							*dst = *src - '0';
						}
						*dst <<= 4;

						src++;
						if (isalpha(*src)) {
							*dst += (*src|0x20) - 'a' + 10;
						}
						else {
							*dst += *src - '0';
						}
					}
					else {
						*dst++ = '\\';
						*dst = 'x';
					}
					break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
					*dst = 0;
					for (i=0; i<3; i++) {
						if (*src < '0' || *src > '7')
							break;
						*dst = *dst << 3 + *src - '0';
					}
					src--;
				default:
					*dst = '\\';
					src--;
			}
		}
		else {
			*dst = *src;
		}
		src++; dst++;
	}

	return (dst - Text);
}

//
//  �o�̓o�b�t�@�ɕ��������������
//  ttpcmn/ttcmn.c:CommTextOut()���x�[�X
//
void CommOut(char *str, int len) {
	int outlen, c;
	char *p;

	if (len > OutBuffSize - pvar->cv->OutBuffCount)
		outlen = OutBuffSize - pvar->cv->OutBuffCount;
	else
		outlen = len;

	if (pvar->cv->OutPtr > 0) {
		memmove(pvar->cv->OutBuff, &(pvar->cv->OutBuff[pvar->cv->OutPtr]), pvar->cv->OutBuffCount);
		pvar->cv->OutPtr = 0;
	}

	c = pvar->cv->OutBuffCount;
	for (p=str; outlen>0; p++, outlen--) {
		switch (*p) {
		  case 0x0d:
			switch (pvar->cv->CRSend) {
			  case IdCR:
				pvar->cv->OutBuff[c++] = 0x0d;
				if (c < OutBuffSize && pvar->cv->TelFlag && ! pvar->cv->TelBinSend) {
					pvar->cv->OutBuff[c++] = 0;
				}
				break;
			  case IdLF:
				pvar->cv->OutBuff[c++] = 0x0a;
				break;
			  case IdCRLF:
				pvar->cv->OutBuff[c++] = 0x0d;
				if (c < OutBuffSize) {
					pvar->cv->OutBuff[c++] = 0x0a;
				}
				break;
			}
			if (c + outlen > OutBuffSize) {
				outlen--;
			}
			break;
		  case 0xff:
			if (pvar->cv->TelFlag) {
				if (c < OutBuffSize - 1) {
					pvar->cv->OutBuff[c++] = 0xff;
					pvar->cv->OutBuff[c++] = 0xff;
				}
			}
			else {
				pvar->cv->OutBuff[c++] = 0xff;
			}
			if (c + outlen > OutBuffSize) {
				outlen--;
			}
			break;
		  default:
			pvar->cv->OutBuff[c++] = *p;
		}
	}

	pvar->cv->OutBuffCount = c;
}

//
// �^�C�}���^�C���A�E�g�������̃R�[���o�b�N�֐�
//	�L���ɂ���Ă���Ȃ�΁A�o�̓o�b�t�@�ɃR�}���h����������
//
void CALLBACK RecurringTimerProc(HWND hwnd, UINT msg, UINT_PTR ev, DWORD now) {
	if (pvar->enable && pvar->cmdLen > 0 && pvar->cv->Ready && pvar->cv->OutBuffCount == 0) {
		CommOut(pvar->command, pvar->cmdLen);
	}
//	SendMessage(hwnd, WM_IDLE, 0, 0);
	return;
}

//
//  TTXInit -- �N��������
//
static void PASCAL FAR TTXInit(PTTSet ts, PComVar cv) {
	pvar->ts = ts;
	pvar->cv = cv;
	pvar->origPsend = NULL;
	pvar->origPWriteFile = NULL;
	pvar->origReadIniFile = NULL;
	pvar->origWriteIniFile = NULL;
	pvar->interval = DEFAULT_INTERVAL;
}

//
//  TTXSend, TTXWriteFile -- �L�[���͏���
//	�L�[���͂���������A�^�C�}����������
//
static int PASCAL FAR TTXsend(SOCKET s, const char FAR *buf, int len, int flags) {
	if (pvar->enable && len > 0) {
		SetTimer(pvar->cv->HWin, IdRecurringTimer, pvar->interval * 1000, RecurringTimerProc);
	}
	return pvar->origPsend(s, buf, len, flags);
}

static BOOL PASCAL FAR TTXWriteFile(HANDLE fh, LPCVOID buff, DWORD len, LPDWORD wbytes, LPOVERLAPPED wol) {
	if (pvar->enable && len > 0) {
		SetTimer(pvar->cv->HWin, IdRecurringTimer, pvar->interval * 1000, RecurringTimerProc);
	}
	return pvar->origPWriteFile(fh, buff, len, wbytes, wol);
}

//
// TTXOpenTCP, TTXOpenFile -- �Z�b�V�����J�n����
//	Psend, WriteFile���t�b�N���A�L���Ȃ�΃^�C�}���Z�b�g����B
//
static void PASCAL FAR TTXOpenTCP(TTXSockHooks FAR * hooks) {
	pvar->origPsend = *hooks->Psend;
	*hooks->Psend = TTXsend;

	if (pvar->enable && pvar->cmdLen > 0) {
		SetTimer(pvar->cv->HWin, IdRecurringTimer, pvar->interval * 1000, RecurringTimerProc);
	}
}

static void PASCAL FAR TTXOpenFile(TTXFileHooks FAR * hooks) {
	pvar->origPWriteFile = *hooks->PWriteFile;
	*hooks->PWriteFile = TTXWriteFile;

	if (pvar->enable && pvar->cmdLen > 0) {
		SetTimer(pvar->cv->HWin, IdRecurringTimer, pvar->interval * 1000, RecurringTimerProc);
	}
}

//
// TTXCloseTCP, TTXCloseFile -- �Z�b�V�����I��������
//	Psend, WriteFile�̃t�b�N���������A�^�C�}���~�߂�B
//
static void PASCAL FAR TTXCloseTCP(TTXSockHooks FAR * hooks) {
	if (pvar->origPsend) {
		*hooks->Psend = pvar->origPsend;
	}
	KillTimer(pvar->cv->HWin, IdRecurringTimer);
}

static void PASCAL FAR TTXCloseFile(TTXFileHooks FAR * hooks) {
	if (pvar->origPWriteFile) {
		*hooks->PWriteFile = pvar->origPWriteFile;
	}
	KillTimer(pvar->cv->HWin, IdRecurringTimer);
}

//
// TTXReadIniFile, TTXWriteIniFile -- �ݒ�t�@�C���̓ǂݏ���
//
static void PASCAL FAR TTXReadIniFile(PCHAR fn, PTTSet ts) {
	pvar->origReadIniFile(fn, ts);

	GetPrivateProfileString(SECTION, "Command", "", pvar->orgCommand, sizeof(pvar->orgCommand), fn);
	strncpy_s(pvar->command, sizeof(pvar->command), pvar->orgCommand, _TRUNCATE);
	UnEscapeStr(pvar->command);
	pvar->cmdLen = (int)strlen(pvar->command);

	pvar->interval = GetPrivateProfileInt(SECTION, "Interval", DEFAULT_INTERVAL, fn);
	if (pvar->interval < MINIMUM_INTERVAL) {
		pvar->interval = MINIMUM_INTERVAL;
	}

	pvar->enable = GetOnOff(SECTION, "Enable", fn, FALSE);

	return;
}

static void PASCAL FAR TTXWriteIniFile(PCHAR fn, PTTSet ts) {
	char buff[20];

	pvar->origWriteIniFile(fn, ts);

	WritePrivateProfileString(SECTION, "Enable", pvar->enable?"on":"off", fn);

	WritePrivateProfileString(SECTION, "Command", pvar->orgCommand, fn);

	_snprintf_s(buff, sizeof(buff), _TRUNCATE, "%d", pvar->interval);
	WritePrivateProfileString(SECTION, "Interval", buff, fn);

	return;
}

static void PASCAL FAR TTXGetSetupHooks(TTXSetupHooks FAR * hooks) {
	pvar->origReadIniFile = *hooks->ReadIniFile;
	*hooks->ReadIniFile = TTXReadIniFile;

	pvar->origWriteIniFile = *hooks->WriteIniFile;
	*hooks->WriteIniFile = TTXWriteIniFile;
}

//
// ���j���[����
//	�R���g���[�����j���[��RecurringCommand��ǉ��B
//
static void PASCAL FAR TTXModifyMenu(HMENU menu) {
	UINT flag = MF_BYCOMMAND | MF_STRING | MF_ENABLED;

	pvar->SetupMenu = GetSetupMenu(menu);
	pvar->ControlMenu = GetControlMenu(menu);

	GetI18nStr(SECTION, "MENU_SETUP_RECURRING", pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg),
		"Rec&urring command", pvar->ts->UILanguageFile);
	InsertMenu(pvar->SetupMenu, ID_SETUP_ADDITIONALSETTINGS, flag, ID_MENU_SETUP, pvar->ts->UIMsg);

	if (pvar->enable) {
		flag |= MF_CHECKED;
	}


	GetI18nStr(SECTION, "MENU_CONTROL_RECURRING", pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg),
		"Rec&urring command", pvar->ts->UILanguageFile);
	InsertMenu(pvar->ControlMenu, ID_CONTROL_MACRO, flag, ID_MENU_CONTROL, pvar->ts->UIMsg);
	InsertMenu(pvar->ControlMenu, ID_CONTROL_MACRO, MF_BYCOMMAND | MF_SEPARATOR, 0, NULL);
}

static void PASCAL FAR TTXModifyPopupMenu(HMENU menu) {
	if (menu==pvar->ControlMenu) {
		if (pvar->enable) {
			CheckMenuItem(pvar->ControlMenu, ID_MENU_CONTROL, MF_BYCOMMAND | MF_CHECKED);
		}
		else {
			CheckMenuItem(pvar->ControlMenu, ID_MENU_CONTROL, MF_BYCOMMAND | MF_UNCHECKED);
		}
	}
}

//
// RecurringCommand�ݒ�_�C�A���O�̃R�[���o�b�N�֐��B
//
static LRESULT CALLBACK RecurringCommandSetting(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	char uimsg[MAX_UIMSG];
	static HFONT DlgFont;
	LOGFONT logfont;
	HFONT font;

	switch (msg) {
	  case WM_INITDIALOG:
	  	font = (HFONT)SendMessage(dlg, WM_GETFONT, 0, 0);
		GetObject(font, sizeof(LOGFONT), &logfont);

		if ((GetI18nLogfont(SECTION, "DLG_TAHOMA_FONT", &logfont, GetDeviceCaps(GetDC(dlg), LOGPIXELSY),
		                   pvar->ts->UILanguageFile) != FALSE) &&
		   ((DlgFont = CreateFontIndirect(&logfont)) != NULL)) {
			SendDlgItemMessage(dlg, IDC_ENABLE, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_INTERVAL, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_INTERVAL_LABEL, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_COMMAND, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
			SendDlgItemMessage(dlg, IDC_COMMAND_LABEL, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
		}
		else {
			DlgFont = NULL;
		}

		GetWindowText(dlg, uimsg, sizeof(uimsg));
		GetI18nStr(SECTION, "DLG_TITLE", pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg), uimsg, pvar->ts->UILanguageFile);
		SetWindowText(dlg, pvar->ts->UIMsg);

		GetDlgItemText(dlg, IDC_ENABLE, uimsg, sizeof(uimsg));
		GetI18nStr(SECTION, "DLG_ENABLE", pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg), uimsg, pvar->ts->UILanguageFile);
		SetDlgItemText(dlg, IDC_ENABLE, pvar->ts->UIMsg);

		GetDlgItemText(dlg, IDC_INTERVAL_LABEL, uimsg, sizeof(uimsg));
		GetI18nStr(SECTION, "DLG_INTERVAL", pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg), uimsg, pvar->ts->UILanguageFile);
		SetDlgItemText(dlg, IDC_INTERVAL_LABEL, pvar->ts->UIMsg);

		GetDlgItemText(dlg, IDC_COMMAND_LABEL, uimsg, sizeof(uimsg));
		GetI18nStr(SECTION, "DLG_COMMAND", pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg), uimsg, pvar->ts->UILanguageFile);
		SetDlgItemText(dlg, IDC_COMMAND_LABEL, pvar->ts->UIMsg);

		SendMessage(GetDlgItem(dlg, IDC_ENABLE), BM_SETCHECK,
		            pvar->enable?BST_CHECKED:BST_UNCHECKED, 0);
		SetDlgItemInt(dlg, IDC_INTERVAL, pvar->interval, FALSE);
		SetDlgItemText(dlg, IDC_COMMAND, pvar->orgCommand);

		return TRUE;
	  case WM_COMMAND:
		switch (LOWORD(wParam)) {
		  case IDOK:
			pvar->enable = IsDlgButtonChecked(dlg, IDC_ENABLE) == BST_CHECKED;

			pvar->interval = GetDlgItemInt(dlg, IDC_INTERVAL, NULL, FALSE);
			if (pvar->interval < MINIMUM_INTERVAL) {
				pvar->interval = MINIMUM_INTERVAL;
			}

			GetDlgItemText(dlg, IDC_COMMAND, pvar->orgCommand, sizeof(pvar->orgCommand));
			strncpy_s(pvar->command, sizeof(pvar->command), pvar->orgCommand, _TRUNCATE);
			UnEscapeStr(pvar->command);
			pvar->cmdLen = (int)strlen(pvar->command);

			if (pvar->cv->Ready) {
				if (pvar->enable) {
					SetTimer(pvar->cv->HWin, IdRecurringTimer,
					         pvar->interval * 1000, RecurringTimerProc);
				}
				else {
					KillTimer(pvar->cv->HWin, IdRecurringTimer);
				}
			}

			EndDialog(dlg, IDOK);
			if (DlgFont != NULL) {
				DeleteObject(DlgFont);
			}
			return TRUE;

		  case IDCANCEL:
			EndDialog(dlg, IDCANCEL);
			if (DlgFont != NULL) {
				DeleteObject(DlgFont);
			}
			return TRUE;
		}
		break;
	}
	return FALSE;
}

//
// ���j���[�R�}���h����
//
static int PASCAL FAR TTXProcessCommand(HWND hWin, WORD cmd) {
	switch (cmd) {
	  case ID_MENU_SETUP:
		switch (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SETUP_RECURRINGCOMMAND),
		                       hWin, RecurringCommandSetting, (LPARAM)NULL)) {
		  case IDOK:
			break;
		  case IDCANCEL:
			break;
		  case -1:
			MessageBox(hWin, "TTXRecurringCommand: Error", "Can't display dialog box.",
			           MB_OK | MB_ICONEXCLAMATION);
			break;
		}
		return 1;

	  case ID_MENU_CONTROL:
		pvar->enable = !pvar->enable;
		if (pvar->enable) {
			SetTimer(pvar->cv->HWin, IdRecurringTimer,
			         pvar->interval * 1000, RecurringTimerProc);
		}
		else {
			KillTimer(pvar->cv->HWin, IdRecurringTimer);
		}
		return 1;
	}
	return 0;
}

static TTXExports Exports = {
	sizeof(TTXExports),
	ORDER,

	TTXInit,
	NULL, // TTXGetUIHooks,
	TTXGetSetupHooks,
	TTXOpenTCP,
	TTXCloseTCP,
	NULL, // TTXSetWinSize,
	TTXModifyMenu,
	TTXModifyPopupMenu,
	TTXProcessCommand,
	NULL, // TTXEnd,
	NULL, // TTXSetCommandLine,
	TTXOpenFile,
	TTXCloseFile
};

BOOL __declspec(dllexport) PASCAL FAR TTXBind(WORD Version, TTXExports FAR * exports) {
	int size = sizeof(Exports) - sizeof(exports->size);
	/* do version checking if necessary */
	/* if (Version!=TTVERSION) return FALSE; */

	if (size > exports->size) {
		size = exports->size;
	}
	memcpy((char FAR *)exports + sizeof(exports->size),
	       (char FAR *)&Exports + sizeof(exports->size), size);
	return TRUE;
}

BOOL WINAPI DllMain(HANDLE hInstance,
		    ULONG ul_reason_for_call,
		    LPVOID lpReserved)
{
	switch( ul_reason_for_call ) {
	  case DLL_THREAD_ATTACH:
		/* do thread initialization */
		break;
	  case DLL_THREAD_DETACH:
		/* do thread cleanup */
		break;
	  case DLL_PROCESS_ATTACH:
		/* do process initialization */
		DoCover_IsDebuggerPresent();
		hInst = hInstance;
		pvar = &InstVar;
		break;
	  case DLL_PROCESS_DETACH:
		/* do process cleanup */
		break;
	}
	return TRUE;
}
