/* ==========================================================================
	Project Name		: TeraTerm Menu
	Outline				: TeraTerm Menu Function
	Version				: 0.94
	Create				: 1998-11-22(Sun)
	Update				: 2002-10-02(Wed)
	Reference			: Copyright (C) S.Hayakawa 1997-2002
   ======1=========2=========3=========4=========5=========6=========7======= */
#define		STRICT

#include	<windows.h>
#include	<commctrl.h>

#include	"ttpmenu.h"
#include	"registry.h"
#include	"winmisc.h"
#include	"resource.h"

// UTF-8 TeraTerm�ł́A�f�t�H���g�C���X�g�[��������L�ɕύX�����B(2004.12.2 yutaka)
// ����ɁA�f�t�H���g�C���X�g�[����̓J�����g�f�B���N�g���ɕύX�B(2004.12.14 yutaka)
#define DEFAULT_PATH "."

// �O���[�o���ϐ�
HWND		g_hWnd;				// ���C���̃n���h��
HWND		g_hWndMenu = NULL;	// �ݒ�_�C�A���O�̃n���h��
HWND		g_hWndTip;			// �ݒ�_�C�A���O���c�[���`�b�v�̃n���h��
HICON		g_hIcon;			// �A�v���P�[�V�����A�C�R���̃n���h��
HMENU		g_hMenu;			// ���j���[�i��\���j�̃n���h��
HMENU		g_hSubMenu;			// �|�b�v�A�b�v���j���[�̃n���h��
HMENU		g_hListMenu;		// �ݒ�ꗗ�|�b�v�A�b�v���j���[�̃n���h��
HMENU		g_hConfigMenu;		// �\���ݒ�|�b�v�A�b�v���j���[�̃n���h��
HHOOK		g_hHook = NULL;		// �c�[���`�b�v�֘A�t�b�N�̃n���h��
HINSTANCE	g_hI;				// �A�v���P�[�V�����C���X�^���X

JobInfo		g_JobInfo;			// �J�����g�̐ݒ���\���́i�ݒ�_�C�A���O�j
MenuData	g_MenuData;			// TeraTerm Menu�̕\���ݒ蓙�̍\����


/* ==========================================================================
	Function Name	: (BOOL) ExecStartup()
	Outline			: �X�^�[�g�A�b�v�ݒ�̃W���u�����s����B
	Arguments		: HWND		hWnd		(In) �_�C�A���O�̃n���h��
	Return Value	: ���� TRUE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL ExecStartup(HWND hWnd)
{
	char	szEntryName[MAX_PATH];
	char	szJobName[MAXJOBNUM][MAX_PATH];
	HKEY	hKey;
	DWORD	dwCnt;
	DWORD	dwIndex = 0;
	DWORD	dwSize = MAX_PATH;

	if ((hKey = RegOpen(HKEY_CURRENT_USER, TTERM_KEY)) != INVALID_HANDLE_VALUE) {
		while (::RegEnumKeyEx(hKey, dwIndex, szEntryName, &dwSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
			::lstrcpy(szJobName[dwIndex++], szEntryName);
			dwSize = MAX_PATH;
		}
		::lstrcpy(szJobName[dwIndex], "");
		::RegCloseKey(hKey);

		for (dwCnt = 0; dwCnt < dwIndex; dwCnt++)
			ConnectHost(hWnd, 0, szJobName[dwCnt]);
	}

	return TRUE;
}

/* ==========================================================================
	Function Name	: (BOOL) ErrorMessage()
	Outline			: �w�胁�b�Z�[�W�{�V�X�e���̃G���[���b�Z�[�W��\������B
	Arguments		: HWND			hWnd		(In) �e�E�C���h�E�̃n���h��
					: LPTSTR		msg,...		(In) �C�Ӄ��b�Z�[�W������
	Return Value	: ���� TRUE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL ErrorMessage(HWND hWnd, LPTSTR msg,...)
{
	char	szBuffer[MAX_PATH] = "";

	va_list ap;
	va_start(ap, msg);
	vsprintf(szBuffer + ::lstrlen(szBuffer), msg, ap);
	va_end(ap);

	::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
					NULL,
					::GetLastError(),
					LANG_NEUTRAL,
					szBuffer + ::lstrlen(szBuffer),
					MAX_PATH,
					NULL);

	MessageBox(hWnd, szBuffer, "TeraTerm Menu", MB_ICONSTOP | MB_OK);

	return TRUE;
}

/* ==========================================================================
	Function Name	: (BOOL) SetMenuFont()
	Outline			: �t�H���g�w��_�C�A���O��\�����A�w�肳�ꂽ�t�H���g��
					: �ݒ肷��B
	Arguments		: HWND			hWnd		(In) �e�E�C���h�E�̃n���h��
	Return Value	: ���� TRUE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL SetMenuFont(HWND hWnd)
{
	HWND		hFontWnd;
	DWORD		rgbColors;
	LOGFONT		lfFont;
	CHOOSEFONT	chooseFont;
	static int	open = 0;

	if (open == 1) {
		while ((hFontWnd = ::FindWindow(NULL, "Font")) != NULL) {
			if (hWnd == ::GetParent(hFontWnd)) {
				::SetForceForegroundWindow(hFontWnd);
				break;
			}
		}
		return TRUE;
	}
	open = 1;

	lfFont		= g_MenuData.lfFont;
	rgbColors	= g_MenuData.crMenuTxt;

	memset((void *) &chooseFont, 0, sizeof(CHOOSEFONT));
	chooseFont.lStructSize	= sizeof(CHOOSEFONT);
	chooseFont.hwndOwner	= hWnd;
	chooseFont.lpLogFont	= &lfFont;
	chooseFont.Flags		= CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_EFFECTS;
	chooseFont.rgbColors	= rgbColors;
	chooseFont.nFontType	= SCREEN_FONTTYPE;

	if (::ChooseFont(&chooseFont) == TRUE) {
		if (g_MenuData.hFont != NULL)
			::DeleteObject((HGDIOBJ) g_MenuData.hFont);
		g_MenuData.crMenuTxt	= chooseFont.rgbColors;
		g_MenuData.lfFont		= lfFont;
		g_MenuData.hFont		= ::CreateFontIndirect(&lfFont);
		RedrawMenu(hWnd);
	}

	open = 0;

	return TRUE;
}

/* ==========================================================================
	Function Name	: (BOOL) ExtractAssociatedIconEx()
	Outline			: �A�v���P�[�V�����Ɋ֘A�t����ꂽ�A�C�R�����擾����B
					: �ݒ肷��B
	Arguments		: char			*szPath		(In) �A�v���P�[�V������
					: HICON			*hLargeIcon	(Out) �傫���A�C�R���̃n���h��
					: HICON			*hSmallIcon	(Out) �������A�C�R���̃n���h��
	Return Value	: ���� TRUE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL ExtractAssociatedIconEx(char *szPath, HICON *hLargeIcon, HICON *hSmallIcon)
{
	SHFILEINFO	sfi;

	::SHGetFileInfo(szPath, 0, &sfi, sizeof(sfi), SHGFI_LARGEICON | SHGFI_ICON);
	*hLargeIcon = ::CopyIcon(sfi.hIcon);

	::SHGetFileInfo(szPath, 0, &sfi, sizeof(sfi), SHGFI_SMALLICON | SHGFI_ICON);
	*hSmallIcon = ::CopyIcon(sfi.hIcon);

	return TRUE;
}

/* ==========================================================================
	Function Name	: (BOOL) GetApplicationFilename()
	Outline			: ���W�X�g�����w�肳�ꂽ�ݒ�̃A�v���P�[�V���������擾
					: ����B
	Arguments		: char			*szName		(In) �ݒ薼
					: char			*szPath		(Out) �A�v���P�[�V������
	Return Value	: ���� TRUE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL GetApplicationFilename(char *szName, char *szPath)
{
	char	szSubKey[MAX_PATH];
	char	szDefault[MAX_PATH] = DEFAULT_PATH;

	char	szTTermPath[MAX_PATH];
	BOOL	bRet;
	BOOL	bTtssh = FALSE;
	HKEY	hKey;

	::wsprintf(szSubKey, "%s\\%s", TTERM_KEY, szName);
	if ((hKey = RegOpen(HKEY_CURRENT_USER, szSubKey)) == INVALID_HANDLE_VALUE)
		return FALSE;

	bRet = RegGetStr(hKey, KEY_TERATERM, szPath, MAX_PATH);
	if (bRet == FALSE || ::lstrlen(szPath) == 0) {
		RegGetDword(hKey, KEY_TTSSH, (LPDWORD) &bTtssh);
		::GetProfileString("Tera Term Pro", "Path", szDefault, szTTermPath, MAX_PATH);
		::wsprintf(szPath, "%s\\%s", szTTermPath, bTtssh ? TTSSH : TERATERM);
	}

	RegClose(hKey);

	return TRUE;
}

/* ==========================================================================
	Function Name	: (BOOL) AddTooltip()
	Outline			: �w�肳�ꂽ�R���g���[���Ƀc�[���`�b�v���֘A�t����
	Arguments		: int			idControl	(In) �R���g���[��ID
	Return Value	: ���� TRUE / ���s FALSE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL AddTooltip(int idControl)
{
	TOOLINFO	ti;

	ti.cbSize	= sizeof(TOOLINFO);
	ti.uFlags	= TTF_IDISHWND; 
	ti.hwnd		= g_hWndMenu; 
	ti.uId		= (UINT) ::GetDlgItem(g_hWndMenu, idControl); 
	ti.hinst	= 0; 
	ti.lpszText	= LPSTR_TEXTCALLBACK;

	return ::SendMessage(g_hWndTip, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
}

/* ==========================================================================
	Function Name	: (BOOL) LoadConfig()
	Outline			: ���W�X�g�����TeraTerm Menu�̕\���ݒ蓙���擾����
	Arguments		: �Ȃ�
	Return Value	: ���� TRUE / ���s FALSE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL LoadConfig(void)
{
	HKEY	hKey;

	if ((hKey = RegCreate(HKEY_CURRENT_USER, TTERM_KEY)) == INVALID_HANDLE_VALUE)
		return FALSE;
	
	if (RegGetDword(hKey, KEY_ICONMODE, &(g_MenuData.dwIconMode)) == TRUE) {
		if (g_MenuData.dwIconMode == MODE_LARGEICON)
			::ModifyMenu(g_hConfigMenu, ID_ICON, MF_CHECKED | MF_BYCOMMAND, ID_ICON, STR_ICONMODE);
	} else
		g_MenuData.dwIconMode = MODE_SMALLICON;
	
	if (RegGetDword(hKey, KEY_LEFTBUTTONPOPUP, (LPDWORD) &(g_MenuData.bLeftButtonPopup)) == FALSE)
		g_MenuData.bLeftButtonPopup = TRUE;
	if (g_MenuData.bLeftButtonPopup == TRUE)
		::ModifyMenu(g_hConfigMenu, ID_LEFTPOPUP, MF_CHECKED | MF_BYCOMMAND, ID_LEFTPOPUP, STR_LEFTBUTTONPOPUP);

	if (RegGetDword(hKey, KEY_MENUTEXTCOLOR, &(g_MenuData.crMenuTxt)) == FALSE)
		g_MenuData.crMenuTxt = ::GetSysColor(COLOR_MENUTEXT);

	if (RegGetDword(hKey, KEY_HOTKEY, (LPDWORD) &(g_MenuData.bHotkey)) == FALSE)
		g_MenuData.bHotkey	= FALSE;
	if (g_MenuData.bHotkey == TRUE) {
		::ModifyMenu(g_hConfigMenu, ID_HOTKEY, MF_CHECKED | MF_BYCOMMAND, ID_HOTKEY, STR_HOTKEY);
		::RegisterHotKey(g_hWnd, WM_MENUOPEN, MOD_CONTROL | MOD_ALT, 'M');
	}

	if (RegGetDword(hKey, KEY_LF_HEIGHT, (DWORD *) &(g_MenuData.lfFont.lfHeight)) == TRUE) {
		RegGetDword(hKey, KEY_LF_WIDTH, (DWORD *) &(g_MenuData.lfFont.lfWidth));
		RegGetDword(hKey, KEY_LF_ESCAPEMENT, (DWORD *) &(g_MenuData.lfFont.lfEscapement));
		RegGetDword(hKey, KEY_LF_ORIENTATION, (DWORD *) &(g_MenuData.lfFont.lfOrientation));
		RegGetDword(hKey, KEY_LF_WEIGHT, (DWORD *) &(g_MenuData.lfFont.lfWeight));
		RegGetDword(hKey, KEY_LF_ITALIC, (DWORD *) &(g_MenuData.lfFont.lfItalic));
		RegGetDword(hKey, KEY_LF_UNDERLINE, (DWORD *) &(g_MenuData.lfFont.lfUnderline));
		RegGetDword(hKey, KEY_LF_STRIKEOUT, (DWORD *) &(g_MenuData.lfFont.lfStrikeOut));
		RegGetDword(hKey, KEY_LF_CHARSET, (DWORD *) &(g_MenuData.lfFont.lfCharSet));
		RegGetDword(hKey, KEY_LF_OUTPRECISION, (DWORD *) &(g_MenuData.lfFont.lfOutPrecision));
		RegGetDword(hKey, KEY_LF_CLIPPRECISION, (DWORD *) &(g_MenuData.lfFont.lfClipPrecision));
		RegGetDword(hKey, KEY_LF_QUALITY, (DWORD *) &(g_MenuData.lfFont.lfQuality));
		RegGetDword(hKey, KEY_LF_PITCHANDFAMILY, (DWORD *) &(g_MenuData.lfFont.lfPitchAndFamily));
		RegGetStr(hKey, KEY_LF_FACENAME, g_MenuData.lfFont.lfFaceName, LF_FACESIZE);
	} else
		::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &(g_MenuData.lfFont));

	RegClose(hKey);

	g_MenuData.crMenuBg		= ::GetSysColor(COLOR_MENU);
	g_MenuData.crSelMenuBg	= ::GetSysColor(COLOR_HIGHLIGHT);
	g_MenuData.crSelMenuTxt	= ::GetSysColor(COLOR_HIGHLIGHTTEXT);
	g_MenuData.hFont		= ::CreateFontIndirect(&(g_MenuData.lfFont));

	return TRUE;
}

/* ==========================================================================
	Function Name	: (BOOL) SaveConfig()
	Outline			: ���W�X�g����TeraTerm Menu�̕\���ݒ蓙��ۑ�����
	Arguments		: �Ȃ�
	Return Value	: ���� TRUE / ���s FALSE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL SaveConfig(void)
{
	HKEY	hKey;

	if ((hKey = RegOpen(HKEY_CURRENT_USER, TTERM_KEY)) == INVALID_HANDLE_VALUE)
		return FALSE;

	RegSetDword(hKey, KEY_ICONMODE, g_MenuData.dwIconMode);
	RegSetDword(hKey, KEY_LEFTBUTTONPOPUP, g_MenuData.bLeftButtonPopup);
	RegSetDword(hKey, KEY_HOTKEY, g_MenuData.bHotkey);
	RegSetDword(hKey, KEY_MENUTEXTCOLOR, g_MenuData.crMenuTxt);
	RegSetDword(hKey, KEY_LF_HEIGHT, g_MenuData.lfFont.lfHeight);
	RegSetDword(hKey, KEY_LF_WIDTH, g_MenuData.lfFont.lfWidth);
	RegSetDword(hKey, KEY_LF_ESCAPEMENT, g_MenuData.lfFont.lfEscapement);
	RegSetDword(hKey, KEY_LF_ORIENTATION, g_MenuData.lfFont.lfOrientation);
	RegSetDword(hKey, KEY_LF_WEIGHT, g_MenuData.lfFont.lfWeight);
	RegSetDword(hKey, KEY_LF_ITALIC, g_MenuData.lfFont.lfItalic);
	RegSetDword(hKey, KEY_LF_UNDERLINE, g_MenuData.lfFont.lfUnderline);
	RegSetDword(hKey, KEY_LF_STRIKEOUT, g_MenuData.lfFont.lfStrikeOut);
	RegSetDword(hKey, KEY_LF_CHARSET, g_MenuData.lfFont.lfCharSet);
	RegSetDword(hKey, KEY_LF_OUTPRECISION, g_MenuData.lfFont.lfOutPrecision);
	RegSetDword(hKey, KEY_LF_CLIPPRECISION, g_MenuData.lfFont.lfClipPrecision);
	RegSetDword(hKey, KEY_LF_QUALITY, g_MenuData.lfFont.lfQuality);
	RegSetDword(hKey, KEY_LF_PITCHANDFAMILY, g_MenuData.lfFont.lfPitchAndFamily);
	RegSetStr(hKey, KEY_LF_FACENAME, g_MenuData.lfFont.lfFaceName);

	RegClose(hKey);

	return TRUE;
}

/* ==========================================================================
	Function Name	: (LRESULT CALLBACK) GetMsgProc()
	Outline			: �t�b�N �v���V�[�W���iGetMsgProc�̃w���v�Q�Ɓj
	Arguments		: 
	Return Value	: 
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	MSG	*lpMsg;

	lpMsg	= (MSG *) lParam;
	if (nCode < 0 || !(::IsChild(g_hWndMenu, lpMsg->hwnd)))
		return ::CallNextHookEx(g_hHook, nCode, wParam, lParam);

	switch (lpMsg->message) {
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		if (g_hWndTip != NULL) {
			MSG	msg;
			msg.lParam	= lpMsg->lParam;
			msg.wParam	= lpMsg->wParam;
			msg.message	= lpMsg->message;
			msg.hwnd	= lpMsg->hwnd;
			::SendMessage(g_hWndTip, TTM_RELAYEVENT, 0, (LPARAM) (LPMSG) &msg);
		}
		break;
	default:
		break;
    }

    return ::CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

/* ==========================================================================
	Function Name	: (BOOL) CreateTooltip()
	Outline			: �c�[���`�b�v���쐬����
	Arguments		: �Ȃ�
	Return Value	: ���� TRUE / ���s FALSE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL CreateTooltip(void)
{
	::InitCommonControls(); 

	g_hWndTip = ::CreateWindowEx(0,
								TOOLTIPS_CLASS,
								(LPSTR) NULL,
								TTS_ALWAYSTIP,
								CW_USEDEFAULT,
								CW_USEDEFAULT,
								CW_USEDEFAULT,
								CW_USEDEFAULT,
								g_hWndMenu,
								(HMENU) NULL,
								g_hI,
								NULL);

	if (g_hWndTip == NULL)
		return FALSE;

	AddTooltip(BUTTON_SET);
	AddTooltip(BUTTON_DELETE);
	AddTooltip(BUTTON_ETC);
	AddTooltip(CHECK_TTSSH);

	g_hHook = ::SetWindowsHookEx(WH_GETMESSAGE,
								GetMsgProc,
								(HINSTANCE) NULL,
								::GetCurrentThreadId()); 

	if (g_hHook == (HHOOK) NULL)
		return FALSE; 

	return TRUE; 
}

/* ==========================================================================
	Function Name	: (BOOL) ManageWMNotify_Config()
	Outline			: �ݒ�_�C�A���O��WM_NOTIFY����������
	Arguments		: LPARAM	lParam
	Return Value	: ���� TRUE / ������ FALSE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL ManageWMNotify_Config(LPARAM lParam)
{
	int				idCtrl;
	LPTOOLTIPTEXT	lpttt;

	if ((((LPNMHDR) lParam)->code) == TTN_NEEDTEXT) {
		idCtrl	= ::GetDlgCtrlID((HWND) ((LPNMHDR) lParam)->idFrom);
		lpttt	= (LPTOOLTIPTEXT) lParam;
		switch (idCtrl) {
		case BUTTON_SET:
			lpttt->lpszText	= "Regist";
			return TRUE; 
		case BUTTON_DELETE:
			lpttt->lpszText	= "Delete";
			return TRUE; 
		case BUTTON_ETC:
			lpttt->lpszText	= "Configure";
			return TRUE; 
		case CHECK_TTSSH:
			lpttt->lpszText	= "use SSH";
			return TRUE; 
		}
    }

	return FALSE; 
}

/* ==========================================================================
	Function Name	: (void) PopupMenu()
	Outline			: ���C���̃|�b�v�A�b�v���j���[��\������B
	Arguments		: HWND		hWnd		(In) �e�E�C���h�E�̃n���h��
	Return Value	: �Ȃ�
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
void PopupMenu(HWND hWnd)
{
	DWORD	dwPos	= ::GetMessagePos();

	::SetForceForegroundWindow(hWnd);

	::TrackPopupMenu(g_hSubMenu,
						TPM_LEFTALIGN | TPM_RIGHTBUTTON,
						LOWORD(dwPos),
						HIWORD(dwPos),
						0,
						hWnd,
						NULL);
}

/* ==========================================================================
	Function Name	: (void) PopupListMenu()
	Outline			: �ݒ�ꗗ�̃|�b�v�A�b�v���j���[��\������B
	Arguments		: HWND		hWnd		(In) �e�E�C���h�E�̃n���h��
	Return Value	: �Ȃ�
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
void PopupListMenu(HWND hWnd)
{
	DWORD	dwPos	= ::GetMessagePos();

	::SetForceForegroundWindow(hWnd);

	::TrackPopupMenu(g_hListMenu,
						TPM_LEFTALIGN | TPM_RIGHTBUTTON,
						LOWORD(dwPos),
						HIWORD(dwPos),
						0,
						hWnd,
						NULL);
}

/* ==========================================================================
	Function Name	: (BOOL) InitListBox()
	Outline			: �ݒ�_�C�A���O���̐ݒ�ꗗ���X�g�{�b�N�X������������B
	Arguments		: HWND		hWnd		(In) �_�C�A���O�̃n���h��
	Return Value	: ���� TRUE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL InitListBox(HWND hWnd)
{
	char	szPath[MAX_PATH];
	DWORD	dwCnt = 0;
	DWORD	dwIndex = 0;

	::SendDlgItemMessage(hWnd, LIST_HOST, LB_RESETCONTENT, 0, 0);

	while (::lstrlen(g_MenuData.szName[dwIndex]) != 0) {
		if (GetApplicationFilename(g_MenuData.szName[dwIndex], szPath) == TRUE) {
			::SendDlgItemMessage(hWnd, LIST_HOST, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR) g_MenuData.szName[dwIndex]);
			::SendDlgItemMessage(hWnd, LIST_HOST, LB_SETITEMDATA, (WPARAM) dwCnt, (LPARAM) dwIndex);
			dwCnt++;
		}
		dwIndex++;
	}

	return TRUE;
}

/* ==========================================================================
	Function Name	: (BOOL) InitConfigDlg()
	Outline			: �ݒ�_�C�A���O������������B
	Arguments		: HWND		hWnd		(In) �_�C�A���O�̃n���h��
	Return Value	: ���� TRUE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL InitConfigDlg(HWND hWnd)
{
	HICON	g_hIconLeft;
	HICON	g_hIconRight;

	memset(&g_JobInfo, 0, sizeof(JobInfo));

	::DeleteMenu(::GetSystemMenu(hWnd, FALSE), SC_MAXIMIZE, MF_BYCOMMAND);
	::DeleteMenu(::GetSystemMenu(hWnd, FALSE), SC_SIZE, MF_BYCOMMAND);

	g_hIconLeft		= ::LoadIcon(g_hI, (LPCSTR)ICON_LEFT);
	g_hIconRight	= ::LoadIcon(g_hI, (LPCSTR)ICON_RIGHT);
	::SendDlgItemMessage(hWnd, BUTTON_SET, BM_SETIMAGE, (WPARAM) IMAGE_ICON, (LPARAM)(HANDLE) g_hIconLeft);
	::SendDlgItemMessage(hWnd, BUTTON_DELETE, BM_SETIMAGE, (WPARAM) IMAGE_ICON, (LPARAM)(HANDLE) g_hIconRight);

	::CheckRadioButton(hWnd, RADIO_LOGIN, RADIO_MACRO, RADIO_LOGIN);
	EnableItem(hWnd, EDIT_MACRO, FALSE);
	EnableItem(hWnd, BUTTON_MACRO, FALSE);
	::CheckDlgButton(hWnd, CHECK_USER, 1);
	::CheckDlgButton(hWnd, CHECK_PASSWORD, 1);
	::CheckDlgButton(hWnd, CHECK_INI_FILE, 1);

	InitListBox(hWnd);

	return TRUE;
}

/* ==========================================================================
	Function Name	: (BOOL) InitEtcDlg()
	Outline			: �ڍאݒ�_�C�A���O������������B
	Arguments		: HWND		hWnd		(In) �_�C�A���O�̃n���h��
	Return Value	: ���� TRUE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL InitEtcDlg(HWND hWnd)
{
	char	szDefault[MAX_PATH] = DEFAULT_PATH;
	char	szTTermPath[MAX_PATH];

	if (::lstrlen(g_JobInfo.szTeraTerm) == 0) {
		::GetProfileString("Tera Term Pro", "Path", szDefault, szTTermPath, MAX_PATH);
		::wsprintf(g_JobInfo.szTeraTerm, "%s\\%s", szTTermPath, g_JobInfo.bTtssh ? TTSSH : TERATERM);
	}
	if (g_JobInfo.bTtssh == TRUE && lstrstri(g_JobInfo.szTeraTerm, TTSSH) == NULL)
		::wsprintf(g_JobInfo.szTeraTerm, "%s\\%s", szTTermPath, TTSSH);
	if (::lstrlen(g_JobInfo.szLoginPrompt) == 0) {
		::lstrcpy(g_JobInfo.szLoginPrompt, LOGIN_PROMPT);
	}
	if (::lstrlen(g_JobInfo.szPasswdPrompt) == 0) {
		::lstrcpy(g_JobInfo.szPasswdPrompt, PASSWORD_PROMPT);
	}

	::SetDlgItemText(hWnd, EDIT_TTMPATH, g_JobInfo.szTeraTerm);
	::SetDlgItemText(hWnd, EDIT_INITFILE, g_JobInfo.szInitFile);
	::SetDlgItemText(hWnd, EDIT_OPTION, g_JobInfo.szOption);
	::SetDlgItemText(hWnd, EDIT_PROMPT_USER, g_JobInfo.szLoginPrompt);
	::SetDlgItemText(hWnd, EDIT_PROMPT_PASS, g_JobInfo.szPasswdPrompt);

	::SetDlgItemText(hWnd, EDIT_LOG, g_JobInfo.szLog);

	return TRUE;
}

/* ==========================================================================
	Function Name	: (BOOL) InitVersionDlg()
	Outline			: �u�o�[�W�������v�_�C�A���O������������B
	Arguments		: HWND		hWnd		(In) �_�C�A���O�̃n���h��
	Return Value	: ���� TRUE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL InitVersionDlg(HWND hWnd)
{
	::DeleteMenu(::GetSystemMenu(hWnd, FALSE), SC_MAXIMIZE, MF_BYCOMMAND);
	::DeleteMenu(::GetSystemMenu(hWnd, FALSE), SC_SIZE, MF_BYCOMMAND);
	::DeleteMenu(::GetSystemMenu(hWnd, FALSE), SC_MINIMIZE, MF_BYCOMMAND);
	::DeleteMenu(::GetSystemMenu(hWnd, FALSE), SC_RESTORE, MF_BYCOMMAND);

	return TRUE;
}

/* ==========================================================================
	Function Name	: (BOOL) SetDefaultEtcDlg()
	Outline			: �ڍאݒ�_�C�A���O�̊e���ڂɃf�t�H���g�l��ݒ肷��B
	Arguments		: HWND		hWnd		(In) �_�C�A���O�̃n���h��
	Return Value	: ���� TRUE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL SetDefaultEtcDlg(HWND hWnd)
{
	char	szDefault[MAX_PATH] = DEFAULT_PATH;
	char	szTTermPath[MAX_PATH];

	::GetProfileString("Tera Term Pro", "Path", szDefault, szTTermPath, MAX_PATH);
	::wsprintf(szTTermPath, "%s\\%s", szTTermPath, g_JobInfo.bTtssh ? TTSSH : TERATERM);

	::SetDlgItemText(hWnd, EDIT_TTMPATH, szTTermPath);
	::SetDlgItemText(hWnd, EDIT_INITFILE, "");
	// �f�t�H���g�I�v�V������ /KT , /KR ��ǉ� (2005.1.25 yutaka)
	::SetDlgItemText(hWnd, EDIT_OPTION, "/KT=UTF8 /KR=UTF8");
//	::SetDlgItemText(hWnd, EDIT_OPTION, "");
	::SetDlgItemText(hWnd, EDIT_PROMPT_USER, LOGIN_PROMPT);
	::SetDlgItemText(hWnd, EDIT_PROMPT_PASS, PASSWORD_PROMPT);

	return TRUE;
}

/* ==========================================================================
	Function Name	: (BOOL) SetTaskTray()
	Outline			: �^�X�N�g���C�ɃA�C�R����o�^�^�폜����B
	Arguments		: HWND		hWnd		(In) �E�C���h�E�̃n���h��
					: DWORD		dwMessage	(In) Shell_NotifyIcon�̑�����
	Return Value	: ���� TRUE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL SetTaskTray(HWND hWnd, DWORD dwMessage)
{
	NOTIFYICONDATA	nid;

	memset(&nid, 0, sizeof(nid));
	nid.cbSize				= sizeof(nid);
	nid.hWnd				= hWnd;
	nid.uID					= TTERM_ICON;
	nid.uFlags				= NIF_ICON | NIF_TIP | NIF_MESSAGE;
	nid.uCallbackMessage	= WM_TMENU_NOTIFY;
	nid.hIcon				= g_hIcon;
	lstrcpy(nid.szTip, "TeraTerm Menu");

	::Shell_NotifyIcon(dwMessage, &nid);

	return TRUE;
}

/* ==========================================================================
	Function Name	: (BOOL) MakeTTL()
	Outline			: �������O�C���p�}�N���t�@�C���𐶐�����B
	Arguments		: char		*TTLName	(In) �}�N���t�@�C����
					: JobInfo	JobInfo		(In) �ݒ���\����
	Return Value	: ���� TRUE / ���s FALSE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL MakeTTL(char *TTLName, JobInfo *jobInfo)
{
	char	buf[1024];
	DWORD	dwWrite;
	HANDLE	hFile;

	hFile = ::CreateFile(TTLName,
						GENERIC_WRITE, 
						FILE_SHARE_WRITE | FILE_SHARE_READ, 
						NULL,
						CREATE_ALWAYS, 
						FILE_ATTRIBUTE_NORMAL, 
						NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	::wsprintf(buf, "filedelete '%s'\r\n", TTLName);
	::WriteFile(hFile, buf, ::lstrlen(buf), &dwWrite, NULL);

	if (::lstrlen(jobInfo->szLog) != 0) {
		::wsprintf(buf, "logopen '%s' 0 1\r\n", jobInfo->szLog);
		::WriteFile(hFile, buf, ::lstrlen(buf), &dwWrite, NULL);
	}

	// telnet�|�[�g�ԍ���t������ (2004.12.3 yutaka)
	::wsprintf(buf, "connect '%s:23'\r\n", jobInfo->szHostName);
	::WriteFile(hFile, buf, ::lstrlen(buf), &dwWrite, NULL);

	if (jobInfo->bUsername == TRUE) {
		if (::lstrlen(jobInfo->szLoginPrompt) == 0)
			::lstrcpy(jobInfo->szLoginPrompt, LOGIN_PROMPT);
		::wsprintf(buf, "UsernamePrompt = '%s'\r\nUsername = '%s'\r\n", jobInfo->szLoginPrompt, jobInfo->szUsername);
		::WriteFile(hFile, buf, ::lstrlen(buf), &dwWrite, NULL);
	}

	if (jobInfo->bPassword == TRUE) {
		if (::lstrlen(jobInfo->szPasswdPrompt) == 0)
			::lstrcpy(jobInfo->szPasswdPrompt, PASSWORD_PROMPT);
		::wsprintf(buf, "PasswordPrompt = '%s'\r\nPassword = '%s'\r\n", jobInfo->szPasswdPrompt, jobInfo->szPassword);
		::WriteFile(hFile, buf, ::lstrlen(buf), &dwWrite, NULL);
	}

	if (jobInfo->bUsername == TRUE) {
		::wsprintf(buf, "wait   UsernamePrompt\r\nsendln Username\r\n");
		::WriteFile(hFile, buf, ::lstrlen(buf), &dwWrite, NULL);
	}

	if (jobInfo->bPassword == TRUE) {
		::wsprintf(buf, "wait   PasswordPrompt\r\nsendln Password\r\n");
		::WriteFile(hFile, buf, ::lstrlen(buf), &dwWrite, NULL);
	}

	::CloseHandle(hFile);

	return TRUE;
}


// �󔒂� @ �ɒu��������B@���g��@@�ɂ���B(2005.1.28 yutaka)
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


/* ==========================================================================
	Function Name	: (BOOL) ConnectHost()
	Outline			: �������O�C���܂��̓A�v���P�[�V�����̎��s������B
	Arguments		: HWND		hWnd		(In) �E�C���h�E�̃n���h��
					: UINT		idItem		(In) �I�����ꂽ�R���g���[��ID
					: char		*szJobName	(In) ���s����W���u
	Return Value	: ���� TRUE / ���s FALSE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL ConnectHost(HWND hWnd, UINT idItem, char *szJobName)
{
	char	szName[MAX_PATH];
	char	szDefault[MAX_PATH] = DEFAULT_PATH;

	char	szDirectory[MAX_PATH];
	char	szHostName[MAX_PATH];
	char	szTempPath[MAX_PATH];
	char	szMacroFile[MAX_PATH];
	char	szArgment[MAX_PATH] = "";
	char	*pHostName;
	TCHAR	*pt;
	JobInfo	jobInfo;

	::lstrcpy(szName, (szJobName == NULL) ? g_MenuData.szName[idItem - ID_MENU_MIN] : szJobName);

	if (RegLoadLoginHostInformation(szName, &jobInfo) == FALSE) {
		ErrorMessage(hWnd, "���W�X�g������̃f�[�^�ǂݏo���Ɏ��s���܂����B\r\n");
		return FALSE;
	}

	if (szJobName != NULL && jobInfo.bStartup == FALSE)
		return TRUE;

	if (::lstrlen(jobInfo.szTeraTerm) == 0) {
		::GetProfileString("Tera Term Pro", "Path", szDefault, jobInfo.szTeraTerm, MAX_PATH);
		::wsprintf(jobInfo.szTeraTerm, "%s\\%s", jobInfo.szTeraTerm, jobInfo.bTtssh ? TTSSH : TERATERM);
	}

	::lstrcpy(szHostName, jobInfo.szHostName);
	if ((pHostName = _tcstok(szHostName, _T(" ([{'\"|*"))) != NULL)
		pHostName = szHostName;

	if (jobInfo.dwMode != MODE_DIRECT)
		if (::lstrlen(jobInfo.szInitFile) != 0)
			::wsprintf(szArgment, "/F=\"%s\"", jobInfo.szInitFile);

	switch (jobInfo.dwMode) {
	case MODE_AUTOLOGIN:
		::GetTempPath(MAX_PATH, szTempPath);
		::GetTempFileName(szTempPath, "ttm", 0, szMacroFile);
		if (MakeTTL(szMacroFile, &jobInfo) == FALSE) {
			ErrorMessage(hWnd, "Could not make 'ttpmenu.TTL'\r\n");
			return FALSE;
		}
		break;
	case MODE_MACRO:
		::lstrcpy(szMacroFile, jobInfo.szMacroFile);
		break;
	}

	// SSH�������O�C���̏ꍇ�̓}�N���͕s�v (2005.1.25 yutaka)
	if (jobInfo.bTtssh != TRUE) {
		if (jobInfo.dwMode != MODE_DIRECT)
			::wsprintf(szArgment, "%s /M=\"%s\"", szArgment, szMacroFile);
	}

	if (::lstrlen(jobInfo.szOption) != 0)
		::wsprintf(szArgment, "%s %s", szArgment, jobInfo.szOption);

	// TTSSH���L���̏ꍇ�́A�������O�C���̂��߂̃R�}���h���C����t������B(2004.12.3 yutaka)
	// ���[�U�̃p�����[�^���w��ł���悤�ɂ��� (2005.1.25 yutaka)
	// ���J���F�؂��T�|�[�g (2005.1.27 yutaka)
	if (jobInfo.dwMode == MODE_AUTOLOGIN) {
		if (jobInfo.bTtssh == TRUE) {
			char tmp[MAX_PATH];
			char passwd[MAX_PATH], keyfile[MAX_PATH];

			strcpy(tmp, szArgment);
			replace_blank_to_mark(jobInfo.szPassword, passwd, sizeof(passwd));
			replace_blank_to_mark(jobInfo.PrivateKeyFile, keyfile, sizeof(keyfile));

			if (jobInfo.PrivateKeyFile[0] == NULL) { // password authentication
				_snprintf(szArgment, sizeof(szArgment), "%s:22 /ssh /auth=password /user=%s /passwd=%s %s", 
					jobInfo.szHostName,
					jobInfo.szUsername,
					passwd,
					tmp
					);

			} else { // publickey
				_snprintf(szArgment, sizeof(szArgment), "%s:22 /ssh /auth=publickey /user=%s /passwd=%s /keyfile=%s %s", 
					jobInfo.szHostName,
					jobInfo.szUsername,
					passwd,
					keyfile,
					tmp
					);

			}

		} else {
			// SSH���g��Ȃ��ꍇ�A/nossh �I�v�V������t���Ă����B
			::wsprintf(szArgment, "%s /nossh", szArgment);
		}
	}

	::lstrcpy(szDirectory, jobInfo.szTeraTerm);
	if ((::GetFileAttributes(jobInfo.szTeraTerm) & FILE_ATTRIBUTE_DIRECTORY) == 0)
		if ((pt = _tcsrchr(szDirectory, '\\')) != NULL)
			*pt	= '\0';

	SHELLEXECUTEINFO	ExecInfo;
	memset((void *) &ExecInfo, 0, sizeof(SHELLEXECUTEINFO));
	ExecInfo.cbSize			= sizeof(SHELLEXECUTEINFO);
	ExecInfo.fMask			= SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
	ExecInfo.hwnd			= hWnd;
	ExecInfo.lpVerb			= (LPCSTR) NULL;
	ExecInfo.lpFile			= (LPCSTR) jobInfo.szTeraTerm;
	ExecInfo.lpParameters	= (LPCSTR) szArgment;
	ExecInfo.lpDirectory	= (LPCSTR) szDirectory;
	ExecInfo.nShow			= SW_SHOWNORMAL;
	ExecInfo.hInstApp		= g_hI;

	if (::ShellExecuteEx(&ExecInfo) == FALSE) {
		ErrorMessage(hWnd, "Launching the application was failure.\r\n");
		::DeleteFile(szTempPath);
	}

	if (::lstrlen(jobInfo.szLog) != 0) {
		Sleep(500);
		HWND hLog = ::FindWindow(NULL, "Tera Term: Log");
		if (hLog != NULL)
			ShowWindow(hLog, SW_HIDE);
	}

	return TRUE;
}

/* ==========================================================================
	Function Name	: (BOOL) InitMenu()
	Outline			: �e�|�b�v�A�b�v���j���[���쐬����B
	Arguments		: �Ȃ�
	Return Value	: ���� TRUE / ���s FALSE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL InitMenu(void)
{
	for (int cnt = 0; cnt < MAXJOBNUM; cnt++) {
		g_MenuData.hLargeIcon[cnt] = NULL;
		g_MenuData.hSmallIcon[cnt] = NULL;
	}

	if (g_hListMenu == NULL) {
		g_hMenu			= ::LoadMenu(g_hI, (LPCSTR) TTERM_MENU);
		g_hSubMenu		= ::GetSubMenu(g_hMenu, 0);
		g_hListMenu		= ::CreateMenu();
		g_hConfigMenu	= ::GetSubMenu(g_hSubMenu, 1);
		if (g_hMenu == NULL || g_hSubMenu == NULL || g_hListMenu == NULL)
			return FALSE;
		::ModifyMenu(g_hSubMenu, ID_EXEC, MF_BYCOMMAND | MF_POPUP, (UINT) g_hListMenu, (LPCTSTR) "Execute");
	}

	return TRUE;
}

/* ==========================================================================
	Function Name	: (BOOL) InitListMenu()
	Outline			: �ݒ�ꗗ�|�b�v�A�b�v���j���[������������B
	Arguments		: HWND		hWnd		(In) �E�C���h�E�̃n���h��
	Return Value	: ���� TRUE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL InitListMenu(HWND hWnd)
{
	char	szPath[MAX_PATH];
	char	szEntryName[MAX_PATH];
	HKEY	hKey;
	DWORD	dwCnt;
	DWORD	dwIndex = 0;
	DWORD	dwSize = MAX_PATH;

	for (int cnt = 0; cnt < MAXJOBNUM; cnt++) {
		memset(g_MenuData.szName, 0, MAX_PATH);
		if (g_MenuData.hLargeIcon[cnt] != NULL) {
			::DestroyIcon(g_MenuData.hLargeIcon[cnt]);
			g_MenuData.hLargeIcon[cnt] = NULL;
		}
		if (g_MenuData.hSmallIcon[cnt] != NULL) {
			::DestroyIcon(g_MenuData.hSmallIcon[cnt]);
			g_MenuData.hSmallIcon[cnt] = NULL;
		}
	}

	if ((hKey = RegOpen(HKEY_CURRENT_USER, TTERM_KEY)) != INVALID_HANDLE_VALUE) {
		while (::RegEnumKeyEx(hKey, dwIndex, szEntryName, &dwSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
			::lstrcpy(g_MenuData.szName[dwIndex++], szEntryName);
			dwSize = MAX_PATH;
		}
		::lstrcpy(g_MenuData.szName[dwIndex], "");
		::RegCloseKey(hKey);

		for (dwCnt = 0; dwCnt < dwIndex; dwCnt++)
			if (GetApplicationFilename(g_MenuData.szName[dwCnt], szPath) == TRUE)
				ExtractAssociatedIconEx(szPath, &g_MenuData.hLargeIcon[dwCnt], &g_MenuData.hSmallIcon[dwCnt]);
	}

	RedrawMenu(hWnd);

	return TRUE;
}

/* ==========================================================================
	Function Name	: (BOOL) RedrawMenu()
	Outline			: �ݒ�ꗗ�|�b�v�A�b�v���j���[��`�悷��B
	Arguments		: HWND		hWnd		(In) �E�C���h�E�̃n���h��
	Return Value	: ���� TRUE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL RedrawMenu(HWND hWnd)
{
	int			num;
	char		szPath[MAX_PATH];
	HDC			hDC;
	HWND		hWndItem;
	DWORD		itemNum;
	DWORD		desktopHeight;
	DWORD		dwCnt = 0;
	DWORD		dwValidCnt = 0;
	TEXTMETRIC	textMetric;

	::DeleteMenu(g_hListMenu, ID_NOENTRY, MF_BYCOMMAND);
	num = ::GetMenuItemCount(g_hListMenu);
	for (dwCnt = 0; dwCnt < (DWORD) num; dwCnt++)
		if (::DeleteMenu(g_hListMenu, ID_MENU_MIN + dwCnt, MF_BYCOMMAND) == FALSE)
			num++;
	
	hWndItem	= ::GetDlgItem((HWND) g_hListMenu, ID_MENU_MIN);
	hDC			= ::GetWindowDC(hWndItem);
	if (g_MenuData.hFont != NULL)
		::SelectObject(hDC, (HGDIOBJ) g_MenuData.hFont);
	::GetTextMetrics(hDC, &textMetric);
	if (g_MenuData.dwIconMode == MODE_SMALLICON)
		g_MenuData.dwMenuHeight	= (ICONSPACE_SMALL > textMetric.tmHeight) ? ICONSPACE_SMALL : textMetric.tmHeight;
	else
		g_MenuData.dwMenuHeight	= (ICONSPACE_LARGE > textMetric.tmHeight) ? ICONSPACE_LARGE : textMetric.tmHeight;
	ReleaseDC(hWndItem, hDC);

	desktopHeight	= ::GetSystemMetrics(SM_CYSCREEN);
	itemNum			= desktopHeight / g_MenuData.dwMenuHeight;

	dwCnt = 0;
	while (::lstrlen(g_MenuData.szName[dwCnt]) != 0) {
		if (GetApplicationFilename(g_MenuData.szName[dwCnt], szPath) == TRUE) {
			if (dwCnt % itemNum == 0 && dwCnt != 0)
				::AppendMenu(g_hListMenu, MF_OWNERDRAW | MF_MENUBARBREAK, ID_MENU_MIN + dwCnt, (LPCTSTR) dwCnt);
			else
				::AppendMenu(g_hListMenu, MF_OWNERDRAW | MF_POPUP, ID_MENU_MIN + dwCnt, (LPCTSTR) dwCnt);
			dwValidCnt++;
		}
		dwCnt++;
	}
	if (dwValidCnt == 0)
		::AppendMenu(g_hListMenu, MF_STRING | MF_GRAYED, ID_NOENTRY, (LPCTSTR) STR_NOENTRY);

	return TRUE;
}

/* ==========================================================================
	Function Name	: (BOOL) RegSaveLoginHostInformation()
	Outline			: ���W�X�g���ɐݒ����ۑ�����B
	Arguments		: JobInfo		*jobInfo	(In) �ݒ���\����
	Return Value	: ���� TRUE / ���s FALSE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL RegSaveLoginHostInformation(JobInfo *jobInfo)
{
	HKEY	hKey;
	char	szSubKey[MAX_PATH];
	char	szEncodePassword[MAX_PATH];

	::wsprintf(szSubKey, "%s\\%s", TTERM_KEY, jobInfo->szName);
	if ((hKey = RegCreate(HKEY_CURRENT_USER, szSubKey)) == INVALID_HANDLE_VALUE)
		return FALSE;

	RegSetStr(hKey, KEY_HOSTNAME, jobInfo->szHostName);
	RegSetDword(hKey, KEY_MODE, jobInfo->dwMode);

	RegSetDword(hKey, KEY_USERFLAG, (DWORD) jobInfo->bUsername);
	RegSetStr(hKey, KEY_USERNAME, jobInfo->szUsername);
	RegSetDword(hKey, KEY_PASSWDFLAG, (DWORD) jobInfo->bPassword);
	EncodePassword(jobInfo->szPassword, szEncodePassword);
	RegSetBinary(hKey, KEY_PASSWORD, szEncodePassword, ::lstrlen(szEncodePassword) + 1);

	RegSetStr(hKey, KEY_TERATERM, jobInfo->szTeraTerm);
	RegSetStr(hKey, KEY_INITFILE, jobInfo->szInitFile);
	RegSetStr(hKey, KEY_OPTION, jobInfo->szOption);
	RegSetStr(hKey, KEY_LOGIN_PROMPT, jobInfo->szLoginPrompt);
	RegSetStr(hKey, KEY_PASSWORD_PROMPT, jobInfo->szPasswdPrompt);

	RegSetStr(hKey, KEY_MACROFILE, jobInfo->szMacroFile);

	RegSetDword(hKey, KEY_TTSSH, (DWORD) jobInfo->bTtssh);
	RegSetDword(hKey, KEY_STARTUP, (DWORD) jobInfo->bStartup);

	RegSetStr(hKey, KEY_LOG, jobInfo->szLog);

	// SSH2
	RegSetStr(hKey, KEY_KEYFILE, jobInfo->PrivateKeyFile);

	RegClose(hKey);

	return TRUE;
}

/* ==========================================================================
	Function Name	: (BOOL) RegLoadLoginHostInformation()
	Outline			: ���W�X�g������ݒ�����擾����B
	Arguments		: char			*szName		(In) �ݒ���
					: JobInfo		*jobInfo	(In) �ݒ���\����
	Return Value	: ���� TRUE / ���s FALSE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL RegLoadLoginHostInformation(char *szName, JobInfo *job_Info)
{
	HKEY	hKey;
	char	szSubKey[MAX_PATH];
	char	szEncodePassword[MAX_PATH];
	DWORD	dwSize = MAX_PATH;
	JobInfo jobInfo;

	memset(&jobInfo, 0, sizeof(JobInfo));

	::wsprintf(szSubKey, "%s\\%s", TTERM_KEY, szName);
	if ((hKey = RegOpen(HKEY_CURRENT_USER, szSubKey)) == INVALID_HANDLE_VALUE)
		return FALSE;

	::lstrcpy(jobInfo.szName, szName);

	RegGetStr(hKey, KEY_HOSTNAME, jobInfo.szHostName, MAX_PATH);
	RegGetDword(hKey, KEY_MODE, &(jobInfo.dwMode));

	RegGetDword(hKey, KEY_USERFLAG, (DWORD *) &(jobInfo.bUsername));
	RegGetStr(hKey, KEY_USERNAME, jobInfo.szUsername, MAX_PATH);
	RegGetDword(hKey, KEY_PASSWDFLAG, (DWORD *) &(jobInfo.bPassword));
	RegGetBinary(hKey, KEY_PASSWORD, szEncodePassword, &dwSize);
	EncodePassword(szEncodePassword, jobInfo.szPassword);

	RegGetStr(hKey, KEY_TERATERM, jobInfo.szTeraTerm, MAX_PATH);
	RegGetStr(hKey, KEY_INITFILE, jobInfo.szInitFile, MAX_PATH);
	RegGetStr(hKey, KEY_OPTION, jobInfo.szOption, MAX_PATH);
	RegGetStr(hKey, KEY_LOGIN_PROMPT, jobInfo.szLoginPrompt, MAX_PATH);
	RegGetStr(hKey, KEY_PASSWORD_PROMPT, jobInfo.szPasswdPrompt, MAX_PATH);

	RegGetStr(hKey, KEY_MACROFILE, jobInfo.szMacroFile, MAX_PATH);

	RegGetDword(hKey, KEY_TTSSH, (LPDWORD) &(jobInfo.bTtssh));
	RegGetDword(hKey, KEY_STARTUP, (LPDWORD) &(jobInfo.bStartup));

	RegGetStr(hKey, KEY_LOG, jobInfo.szLog, MAX_PATH);

	// SSH2
	ZeroMemory(jobInfo.PrivateKeyFile, sizeof(jobInfo.PrivateKeyFile));
	RegGetStr(hKey, KEY_KEYFILE, jobInfo.PrivateKeyFile, MAX_PATH);

	RegClose(hKey);

	*job_Info = jobInfo;

	return TRUE;
}

/* ==========================================================================
	Function Name	: (BOOL) SaveEtcInformation()
	Outline			: �ڍאݒ�����O���[�o���ϐ��ɕۑ�����B
	Arguments		: HWND			hWnd		(In) �_�C�A���O�̃n���h��
	Return Value	: ���� TRUE / ���s FALSE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL SaveEtcInformation(HWND hWnd)
{
	::GetDlgItemText(hWnd, EDIT_TTMPATH, g_JobInfo.szTeraTerm, MAX_PATH);
	::GetDlgItemText(hWnd, EDIT_INITFILE, g_JobInfo.szInitFile, MAX_PATH);
	::GetDlgItemText(hWnd, EDIT_OPTION, g_JobInfo.szOption, MAX_PATH);
	::GetDlgItemText(hWnd, EDIT_PROMPT_USER, g_JobInfo.szLoginPrompt, MAX_PATH);
	::GetDlgItemText(hWnd, EDIT_PROMPT_PASS, g_JobInfo.szPasswdPrompt, MAX_PATH);

	::GetDlgItemText(hWnd, EDIT_LOG, g_JobInfo.szLog, MAX_PATH);

	return TRUE;
}

/* ==========================================================================
	Function Name	: (BOOL) SaveLoginHostInformation()
	Outline			: �ݒ����ۑ�����B
	Arguments		: HWND			hWnd		(In) �_�C�A���O�̃n���h��
	Return Value	: ���� TRUE / ���s FALSE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL SaveLoginHostInformation(HWND hWnd)
{
	long	index;
	char	szDefault[MAX_PATH] = DEFAULT_PATH;
	char	szTTermPath[MAX_PATH];
	char	szName[MAX_PATH];
	DWORD	dwErr;

	if (::GetDlgItemText(hWnd, EDIT_ENTRY, g_JobInfo.szName, MAX_PATH) == 0) {
//		::MessageBox(hWnd, "�o�^������͂��ĉ������B", "TeraTerm Menu", MB_ICONSTOP | MB_OK);
		::MessageBox(hWnd, "error: no registry name", "TeraTerm Menu", MB_ICONSTOP | MB_OK);
		return FALSE;
	}
	if (_tcschr(g_JobInfo.szName, '\\') != NULL) {
//		::MessageBox(hWnd, "�o�^����\"\\\"�͎g�p�ł��܂���B", "TeraTerm Menu", MB_ICONSTOP | MB_OK);
		::MessageBox(hWnd, "can't use \"\\\" in registry name", "TeraTerm Menu", MB_ICONSTOP | MB_OK);
		return FALSE;
	}

	if (::IsDlgButtonChecked(hWnd, RADIO_LOGIN) == 1)
		g_JobInfo.dwMode = MODE_AUTOLOGIN;
	if (::IsDlgButtonChecked(hWnd, RADIO_MACRO) == 1)
		g_JobInfo.dwMode = MODE_MACRO;
	if (::IsDlgButtonChecked(hWnd, RADIO_DIRECT) == 1)
		g_JobInfo.dwMode = MODE_DIRECT;

	if (::GetDlgItemText(hWnd, EDIT_HOST, g_JobInfo.szHostName, MAX_PATH) == 0 && g_JobInfo.dwMode == MODE_AUTOLOGIN) {
//		::MessageBox(hWnd, "�z�X�g������͂��ĉ������B", "TeraTerm Menu", MB_ICONSTOP | MB_OK);
		::MessageBox(hWnd, "error: no host name", "TeraTerm Menu", MB_ICONSTOP | MB_OK);
		return FALSE;
	}

	g_JobInfo.bUsername	= (BOOL) ::IsDlgButtonChecked(hWnd, CHECK_USER);
	::GetDlgItemText(hWnd, EDIT_USER, g_JobInfo.szUsername, MAX_PATH);

	g_JobInfo.bPassword	= (BOOL) ::IsDlgButtonChecked(hWnd, CHECK_PASSWORD);
	::GetDlgItemText(hWnd, EDIT_PASSWORD, g_JobInfo.szPassword, MAX_PATH);

	if (::GetDlgItemText(hWnd, EDIT_MACRO, g_JobInfo.szMacroFile, MAX_PATH) == 0 && g_JobInfo.dwMode == MODE_MACRO) {
//		::MessageBox(hWnd, "�}�N���t�@�C��������͂��ĉ������B", "TeraTerm Menu", MB_ICONSTOP | MB_OK);
		::MessageBox(hWnd, "error: no macro filename", "TeraTerm Menu", MB_ICONSTOP | MB_OK);
		return FALSE;
	}

	g_JobInfo.bStartup	= (BOOL) ::IsDlgButtonChecked(hWnd, CHECK_STARTUP);

	g_JobInfo.bTtssh	= (BOOL) ::IsDlgButtonChecked(hWnd, CHECK_TTSSH);
	if (g_JobInfo.bTtssh == TRUE && lstrstri(g_JobInfo.szTeraTerm, TTSSH) == NULL) {
		::GetProfileString("Tera Term Pro", "Path", szDefault, szTTermPath, MAX_PATH);
		::wsprintf(g_JobInfo.szTeraTerm, "%s\\%s", szTTermPath, TTSSH);
	} else if (::lstrlen(g_JobInfo.szTeraTerm) == 0) {
		::GetProfileString("Tera Term Pro", "Path", szDefault, szTTermPath, MAX_PATH);
		::wsprintf(g_JobInfo.szTeraTerm, "%s\\%s", szTTermPath, g_JobInfo.bTtssh ? TTSSH : TERATERM);
	}
	
	if (::GetFileAttributes(g_JobInfo.szTeraTerm) == 0xFFFFFFFF) {
		dwErr = ::GetLastError();
		if (dwErr == ERROR_FILE_NOT_FOUND || dwErr == ERROR_PATH_NOT_FOUND) {
			ErrorMessage(hWnd, "checking [%s] file was failure.\n", g_JobInfo.szTeraTerm);
//			ErrorMessage(hWnd, "�t�@�C��[%s]�̃`�F�b�N�Ɏ��s���܂����B\n", g_JobInfo.szTeraTerm);
			return FALSE;
		}
	}

	// �閧���t�@�C���̒ǉ� (2005.1.28 yutaka)
	if (::GetDlgItemText(hWnd, IDC_KEYFILE_PATH, g_JobInfo.PrivateKeyFile, MAX_PATH) == 0) {
		ZeroMemory(g_JobInfo.PrivateKeyFile, sizeof(g_JobInfo.PrivateKeyFile));
	}

	if (RegSaveLoginHostInformation(&g_JobInfo) == FALSE) {
		ErrorMessage(hWnd, "���W�X�g���ւ̕ۑ��Ɏ��s���܂����B\r\n");
		return FALSE;
	}

	InitListMenu(hWnd);
	InitListBox(hWnd);

	index = 0;
	while ((index = ::SendDlgItemMessage(hWnd, LIST_HOST, LB_SELECTSTRING, index, (LPARAM)(LPCTSTR) g_JobInfo.szName)) != LB_ERR) {
		::SendDlgItemMessage(hWnd, LIST_HOST, LB_GETTEXT, index, (LPARAM)(LPCTSTR) szName);
		if (::lstrcmpi(g_JobInfo.szName, szName) == 0)
			break;
	}

	return TRUE;
}

/* ==========================================================================
	Function Name	: (BOOL) LoadLoginHostInformation()
	Outline			: �ݒ�����擾����B
	Arguments		: HWND			hWnd		(In) �_�C�A���O�̃n���h��
	Return Value	: ���� TRUE / ���s FALSE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL LoadLoginHostInformation(HWND hWnd)
{
	long	index;
//	char	*pt;
	char	szName[MAX_PATH];

	index = ::SendDlgItemMessage(hWnd, LIST_HOST, LB_GETCURSEL, 0, 0);
	::SendDlgItemMessage(hWnd, LIST_HOST, LB_GETTEXT, (WPARAM) index, (LPARAM) (LPCTSTR) szName);

	if (RegLoadLoginHostInformation(szName, &g_JobInfo) == FALSE) {
		ErrorMessage(hWnd, "���W�X�g���̃I�[�v���Ɏ��s���܂����B\r\n");
		return FALSE;
	}

	switch (g_JobInfo.dwMode) {
	case MODE_AUTOLOGIN:
		::CheckRadioButton(hWnd, RADIO_LOGIN, RADIO_DIRECT, RADIO_LOGIN);
		EnableItem(hWnd, EDIT_HOST, TRUE);
		EnableItem(hWnd, CHECK_USER, TRUE);
		EnableItem(hWnd, EDIT_USER, g_JobInfo.bUsername);
		EnableItem(hWnd, CHECK_PASSWORD, TRUE);
		EnableItem(hWnd, EDIT_PASSWORD, g_JobInfo.bPassword);
		EnableItem(hWnd, EDIT_MACRO, FALSE);
		EnableItem(hWnd, BUTTON_MACRO, FALSE);
		break;
	case MODE_MACRO:
		::CheckRadioButton(hWnd, RADIO_LOGIN, RADIO_DIRECT, RADIO_MACRO);			
		EnableItem(hWnd, EDIT_HOST, FALSE);
		EnableItem(hWnd, CHECK_USER, FALSE);
		EnableItem(hWnd, EDIT_USER, FALSE);
		EnableItem(hWnd, CHECK_PASSWORD, FALSE);
		EnableItem(hWnd, EDIT_PASSWORD, FALSE);
		EnableItem(hWnd, EDIT_MACRO, TRUE);
		EnableItem(hWnd, BUTTON_MACRO, TRUE);
		break;
	case MODE_DIRECT:
		::CheckRadioButton(hWnd, RADIO_LOGIN, RADIO_DIRECT, RADIO_DIRECT);			
		EnableItem(hWnd, EDIT_HOST, FALSE);
		EnableItem(hWnd, CHECK_USER, FALSE);
		EnableItem(hWnd, EDIT_USER, FALSE);
		EnableItem(hWnd, CHECK_PASSWORD, FALSE);
		EnableItem(hWnd, EDIT_PASSWORD, FALSE);
		EnableItem(hWnd, EDIT_MACRO, FALSE);
		EnableItem(hWnd, BUTTON_MACRO, FALSE);
		break;
	}

	if (::lstrlen(g_JobInfo.szName) == 0)
		::lstrcpy(g_JobInfo.szName, g_JobInfo.szHostName);

	::SetDlgItemText(hWnd, EDIT_ENTRY, g_JobInfo.szName);
	::SetDlgItemText(hWnd, EDIT_HOST, g_JobInfo.szHostName);
	::SetDlgItemText(hWnd, EDIT_USER, g_JobInfo.szUsername);
	::SetDlgItemText(hWnd, EDIT_PASSWORD, g_JobInfo.szPassword);

	::SetDlgItemText(hWnd, EDIT_MACRO, g_JobInfo.szMacroFile);

	::CheckDlgButton(hWnd, CHECK_USER, g_JobInfo.bUsername);

	::CheckDlgButton(hWnd, CHECK_PASSWORD, g_JobInfo.bPassword);

	::CheckDlgButton(hWnd, CHECK_TTSSH, g_JobInfo.bTtssh);

	// �閧���t�@�C���̒ǉ� (2005.1.28 yutaka)
	::SetDlgItemText(hWnd, IDC_KEYFILE_PATH, g_JobInfo.PrivateKeyFile);
	if (g_JobInfo.bTtssh == TRUE) {
		EnableWindow(GetDlgItem(hWnd, IDC_KEYFILE_PATH), TRUE);
		EnableWindow(GetDlgItem(hWnd, IDC_KEYFILE_BUTTON), TRUE);

	} else {
		EnableWindow(GetDlgItem(hWnd, IDC_KEYFILE_PATH), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_KEYFILE_BUTTON), FALSE);

	}

	// ttssh.exe�͔p�~�����̂ŉ��L�`�F�b�N�͍폜����B(2004.12.3 yutaka)
#if 0
	if ((pt = lstrstri(g_JobInfo.szTeraTerm, TTSSH)) != NULL)
		if (::lstrcmpi(pt, TTSSH) == 0)
			::CheckDlgButton(hWnd, CHECK_TTSSH, TRUE);
#endif

	::CheckDlgButton(hWnd, CHECK_STARTUP, g_JobInfo.bStartup);

	return TRUE;
}

/* ==========================================================================
	Function Name	: (BOOL) DeleteLoginHostInformation()
	Outline			: �ݒ�����폜����B
	Arguments		: HWND			hWnd		(In) �_�C�A���O�̃n���h��
	Return Value	: ���� TRUE / ���s FALSE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL DeleteLoginHostInformation(HWND hWnd)
{
	long	index;
	char	szEntryName[MAX_PATH];
	char	szSubKey[MAX_PATH];

	if ((index = ::SendDlgItemMessage(hWnd, LIST_HOST, LB_GETCURSEL, 0, 0)) == LB_ERR) {
//		::MessageBox(hWnd, "�폜����o�^��I�����ĉ������B", "TeraTerm Menu", MB_ICONSTOP | MB_OK);
		::MessageBox(hWnd, "Select deleted registry name", "TeraTerm Menu", MB_ICONSTOP | MB_OK);
		return FALSE;
	}

	if (::SendDlgItemMessage(hWnd, LIST_HOST, LB_GETTEXT, (WPARAM) index, (LPARAM) (LPCTSTR) szEntryName) == LB_ERR) {
		::MessageBox(hWnd, "�폜����o�^�̎擾�Ɏ��s���܂����B", "TeraTerm Menu", MB_ICONSTOP | MB_OK);
		return FALSE;
	}

	::wsprintf(szSubKey, "%s\\%s", TTERM_KEY, szEntryName);
	if (::RegDeleteKey(HKEY_CURRENT_USER, szSubKey) != ERROR_SUCCESS) {
		ErrorMessage(hWnd, "���W�X�g���̍폜�Ɏ��s���܂����B\r\n");
		return FALSE;
	}

	InitListMenu(hWnd);
	InitListBox(hWnd);

	return TRUE;
}

/* ==========================================================================
	Function Name	: (BOOL) ManageWMCommand_Config()
	Outline			: �ݒ���_�C�A���O��WM_COMMAND����������B
	Arguments		: HWND			hWnd		(In) �_�C�A���O�̃n���h��
					: WPARAM		wParam		(In) 
	Return Value	: ���� TRUE / ������ FALSE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL ManageWMCommand_Config(HWND hWnd, WPARAM wParam)
{
	char	*pt;
	int ret = 0;

	// �閧���t�@�C���̃R���g���[�� (2005.1.28 yutaka)
	switch(wParam) {
	case CHECK_TTSSH | (BN_CLICKED << 16) :
		ret = SendMessage(GetDlgItem(hWnd, CHECK_TTSSH), BM_GETCHECK, 0, 0);
		if (ret & BST_CHECKED) {
			EnableWindow(GetDlgItem(hWnd, IDC_KEYFILE_PATH), TRUE);
			EnableWindow(GetDlgItem(hWnd, IDC_KEYFILE_BUTTON), TRUE);

		} else {
			EnableWindow(GetDlgItem(hWnd, IDC_KEYFILE_PATH), FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_KEYFILE_BUTTON), FALSE);

		}
		return TRUE;

	default:
		break;
	}

	switch(LOWORD(wParam)) {
	case IDOK:
	case IDCANCEL:
		g_hWndMenu = NULL;
		::EndDialog(hWnd, TRUE);
		return TRUE;
	case BUTTON_SET:
		SaveLoginHostInformation(hWnd);
		return TRUE;
	case BUTTON_DELETE:
		DeleteLoginHostInformation(hWnd);
		return TRUE;
	case BUTTON_ETC:
		::GetDlgItemText(hWnd, EDIT_ENTRY, g_JobInfo.szName, MAX_PATH);
		g_JobInfo.bTtssh	= ::IsDlgButtonChecked(hWnd, CHECK_TTSSH);
		if (::DialogBox(g_hI, (LPCTSTR) DIALOG_ETC, hWnd, DlgCallBack_Etc) == TRUE) {
			::CheckDlgButton(hWnd, CHECK_TTSSH, 0);
			if ((pt = lstrstri(g_JobInfo.szTeraTerm, TTSSH)) != NULL)
				if (::lstrcmpi(pt, TTSSH) == 0)
					::CheckDlgButton(hWnd, CHECK_TTSSH, 1);
		}
		return TRUE;
	case LIST_HOST:
		if (HIWORD(wParam) == LBN_SELCHANGE)
			LoadLoginHostInformation(hWnd);
		return TRUE;
	case CHECK_USER:
		if (IsDlgButtonChecked(hWnd, CHECK_USER) == 1)
			EnableItem(hWnd, EDIT_USER, TRUE);
		else {
			EnableItem(hWnd, EDIT_USER, FALSE);
			::CheckDlgButton(hWnd, CHECK_PASSWORD, 0);
			::PostMessage(hWnd, WM_COMMAND, (WPARAM) CHECK_PASSWORD, (LPARAM) 0);
		}
		return TRUE;
	case CHECK_PASSWORD:
		if (IsDlgButtonChecked(hWnd, CHECK_PASSWORD) == 1)
			EnableItem(hWnd, EDIT_PASSWORD, TRUE);
		else
			EnableItem(hWnd, EDIT_PASSWORD, FALSE);
		return TRUE;
	case CHECK_INI_FILE:
		if (IsDlgButtonChecked(hWnd, CHECK_INI_FILE) == 1)
			EnableItem(hWnd, COMBO_INI_FILE, TRUE);
		else
			EnableItem(hWnd, COMBO_INI_FILE, FALSE);
		return TRUE;
	case BUTTON_MACRO:
		::GetDlgItemText(hWnd, EDIT_MACRO, g_JobInfo.szMacroFile, MAX_PATH);
//		OpenFileDlg(hWnd, EDIT_MACRO, "�}�N���t�@�C�����w��", "�}�N���t�@�C��(*.ttl)\0*.ttl\0���ׂẴt�@�C��(*.*)\0*.*\0\0", g_JobInfo.szMacroFile);
		OpenFileDlg(hWnd, EDIT_MACRO, "specifying macro file", "macro file(*.ttl)\0*.ttl\0all files(*.*)\0*.*\0\0", g_JobInfo.szMacroFile);
		return TRUE;

	case IDC_KEYFILE_BUTTON:
		::GetDlgItemText(hWnd, IDC_KEYFILE_PATH, g_JobInfo.PrivateKeyFile, MAX_PATH);
		OpenFileDlg(hWnd, 
			IDC_KEYFILE_PATH, 
			"specifying private key file", 
			"identity(RSA1)\0identity\0id_rsa(SSH2)\0id_rsa\0id_dsa(SSH2)\0id_dsa\0all(*.*)\0*.*\0\0", 
			g_JobInfo.PrivateKeyFile
		);
		return TRUE;

	case RADIO_LOGIN:
		EnableItem(hWnd, EDIT_HOST, TRUE);
		EnableItem(hWnd, CHECK_USER, TRUE);
		if (IsDlgButtonChecked(hWnd, CHECK_USER) == 1)
			EnableItem(hWnd, EDIT_USER, TRUE);
		else {
			EnableItem(hWnd, EDIT_USER, FALSE);
		}
		EnableItem(hWnd, CHECK_PASSWORD, TRUE);
		if (IsDlgButtonChecked(hWnd, CHECK_PASSWORD) == 1)
			EnableItem(hWnd, EDIT_PASSWORD, TRUE);
		else
			EnableItem(hWnd, EDIT_PASSWORD, FALSE);
		EnableItem(hWnd, EDIT_MACRO, FALSE);
		EnableItem(hWnd, BUTTON_MACRO, FALSE);
		return TRUE;
	case RADIO_MACRO:
		EnableItem(hWnd, EDIT_HOST, FALSE);
		EnableItem(hWnd, CHECK_USER, FALSE);
		EnableItem(hWnd, EDIT_USER, FALSE);
		EnableItem(hWnd, CHECK_PASSWORD, FALSE);
		EnableItem(hWnd, EDIT_PASSWORD, FALSE);
		EnableItem(hWnd, EDIT_MACRO, TRUE);
		EnableItem(hWnd, BUTTON_MACRO, TRUE);
		return TRUE;
	case RADIO_DIRECT:
		EnableItem(hWnd, EDIT_HOST, FALSE);
		EnableItem(hWnd, CHECK_USER, FALSE);
		EnableItem(hWnd, EDIT_USER, FALSE);
		EnableItem(hWnd, CHECK_PASSWORD, FALSE);
		EnableItem(hWnd, EDIT_PASSWORD, FALSE);
		EnableItem(hWnd, EDIT_MACRO, FALSE);
		EnableItem(hWnd, BUTTON_MACRO, FALSE);
		return TRUE;
	}

	return FALSE;
}

/* ==========================================================================
	Function Name	: (BOOL) ManageWMCommand_Etc()
	Outline			: �ڍאݒ���_�C�A���O��WM_COMMAND����������B
	Arguments		: HWND			hWnd		(In) �_�C�A���O�̃n���h��
					: WPARAM		wParam		(In) 
	Return Value	: ���� TRUE / ������ FALSE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL ManageWMCommand_Etc(HWND hWnd, WPARAM wParam)
{
	char	szPath[MAX_PATH];

	switch(LOWORD(wParam)) {
	case IDOK:
		SaveEtcInformation(hWnd);
		::EndDialog(hWnd, TRUE);
		return TRUE;
	case IDCANCEL:
		::EndDialog(hWnd, FALSE);
		return TRUE;
	case BUTTON_DEFAULT:
		SetDefaultEtcDlg(hWnd);
		return TRUE;
	case BUTTON_TTMPATH:
		::GetDlgItemText(hWnd, EDIT_TTMPATH, szPath, MAX_PATH);
//		OpenFileDlg(hWnd, EDIT_TTMPATH, "TeraTerm���w��", "���s�t�@�C��(*.exe)\0*.exe\0���ׂẴt�@�C��(*.*)\0*.*\0\0", szPath);
		OpenFileDlg(hWnd, EDIT_TTMPATH, "specifying TeraTerm", "execute file(*.exe)\0*.exe\0all files(*.*)\0*.*\0\0", szPath);
		return TRUE;
	case BUTTON_INITFILE:
		::GetDlgItemText(hWnd, EDIT_INITFILE, szPath, MAX_PATH);
//		OpenFileDlg(hWnd, EDIT_INITFILE, "�ݒ�t�@�C�����w��", "�ݒ�t�@�C��(*.ini)\0*.ini\0���ׂẴt�@�C��(*.*)\0*.*\0\0", szPath);
		OpenFileDlg(hWnd, EDIT_INITFILE, "specifying config file", "config file(*.ini)\0*.ini\0all files(*.*)\0*.*\0\0", szPath);
		return TRUE;
	case BUTTON_LOG:
		::GetDlgItemText(hWnd, EDIT_LOG, szPath, MAX_PATH);
//		OpenFileDlg(hWnd, EDIT_LOG, "���O�t�@�C�����w��", "���O�t�@�C��(*.log)\0*.ini\0���ׂẴt�@�C��(*.*)\0*.*\0\0", szPath);
		OpenFileDlg(hWnd, EDIT_LOG, "specifying log file", "log file(*.log)\0*.ini\0all files(*.*)\0*.*\0\0", szPath);
		return TRUE;
	}

	return FALSE;
}


/* ==========================================================================
	Function Name	: (BOOL) ManageWMCommand_Version()
	Outline			: �u�o�[�W�������v�_�C�A���O��WM_COMMAND����������B
	Arguments		: HWND			hWnd		(In) �_�C�A���O�̃n���h��
					: WPARAM		wParam		(In) 
	Return Value	: ���� TRUE / ������ FALSE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL ManageWMCommand_Version(HWND hWnd, WPARAM wParam)
{
	switch(LOWORD(wParam)) {
	case IDOK:
		::EndDialog(hWnd, TRUE);
		return TRUE;
	case IDCANCEL:
		::EndDialog(hWnd, TRUE);
		return TRUE;
	}

	return FALSE;
}

/* ==========================================================================
	Function Name	: (BOOL) ManageWMCommand_Menu()
	Outline			: ���C���E�C���h�E��WM_COMMAND����������B
	Arguments		: HWND			hWnd		(In) �E�C���h�E�̃n���h��
					: WPARAM		wParam		(In) 
	Return Value	: ���� TRUE / ������ FALSE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL ManageWMCommand_Menu(HWND hWnd, WPARAM wParam)
{
	switch(LOWORD(wParam)) {
	case ID_TMENU_ADD:
		::DialogBox(g_hI, (LPCTSTR) DIALOG_CONFIG, 0, DlgCallBack_Config);
		return TRUE;
	case ID_TMENU_CLOSE:
		::DestroyWindow(hWnd);
		return	TRUE;
	case ID_VERSION:
		::DialogBox(g_hI, (LPCTSTR) DIALOG_VERSION, hWnd, DlgCallBack_Version);
		return TRUE;
	case ID_ICON:
		if (GetMenuState(g_hConfigMenu, ID_ICON, MF_BYCOMMAND & MF_CHECKED) != 0) {
			::ModifyMenu(g_hConfigMenu, ID_ICON, MF_BYCOMMAND, ID_ICON, STR_ICONMODE);
			g_MenuData.dwIconMode = MODE_SMALLICON;
		} else {
			::ModifyMenu(g_hConfigMenu, ID_ICON, MF_CHECKED | MF_BYCOMMAND, ID_ICON, STR_ICONMODE);
			g_MenuData.dwIconMode = MODE_LARGEICON;
		}
		RedrawMenu(hWnd);
		return	TRUE;
	case ID_LEFTPOPUP:
		if (GetMenuState(g_hConfigMenu, ID_LEFTPOPUP, MF_BYCOMMAND & MF_CHECKED) != 0) {
			::ModifyMenu(g_hConfigMenu, ID_LEFTPOPUP, MF_BYCOMMAND, ID_LEFTPOPUP, STR_LEFTBUTTONPOPUP);
			g_MenuData.bLeftButtonPopup = FALSE;
		} else {
			::ModifyMenu(g_hConfigMenu, ID_LEFTPOPUP, MF_CHECKED | MF_BYCOMMAND, ID_LEFTPOPUP, STR_LEFTBUTTONPOPUP);
			g_MenuData.bLeftButtonPopup = TRUE;
		}
		return	TRUE;
	case ID_HOTKEY:
		if (GetMenuState(g_hConfigMenu, ID_HOTKEY, MF_BYCOMMAND & MF_CHECKED) != 0) {
			::ModifyMenu(g_hConfigMenu, ID_HOTKEY, MF_BYCOMMAND, ID_HOTKEY, STR_HOTKEY);
			::UnregisterHotKey(g_hWnd, WM_MENUOPEN);
			g_MenuData.bHotkey = FALSE;
		} else {
			::ModifyMenu(g_hConfigMenu, ID_HOTKEY, MF_CHECKED | MF_BYCOMMAND, ID_HOTKEY, STR_HOTKEY);
			::RegisterHotKey(g_hWnd, WM_MENUOPEN, MOD_CONTROL | MOD_ALT, 'M');
			g_MenuData.bHotkey = TRUE;
		}
		return	TRUE;
	case ID_FONT:
		SetMenuFont(hWnd);
		break;
	case ID_NOENTRY:
		return	TRUE;
	default:
		ConnectHost(hWnd, LOWORD(wParam));
		return TRUE;
	}

	return FALSE;
}

/* ==========================================================================
	Function Name	: (BOOL CALLBACK) DlgCallBack_Config()
	Outline			: �ݒ�_�C�A���O�̃R�[���o�b�N�֐�
					: �iDialogProc�̃w���v�Q�Ɓj
	Arguments		: 
					: 
	Return Value	: 
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL CALLBACK DlgCallBack_Config(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TEXTMETRIC			textMetric;
	PDRAWITEMSTRUCT		lpdis;
	LPMEASUREITEMSTRUCT	lpmis;
	static COLORREF		crSelText;
	static COLORREF		crSelBkgnd;
	static COLORREF		crText;
	static COLORREF		crBkgnd;

	switch(uMsg) {
	case WM_INITDIALOG:
		if (g_hWndMenu == NULL)
			g_hWndMenu = hWnd;
		else {
			::SetForceForegroundWindow(g_hWndMenu);
			::EndDialog(hWnd, FALSE);
		}
		SetDlgPos(hWnd, POSITION_CENTER);
		::SetClassLong(hWnd, GCL_HICON, (LONG) g_hIcon);
		CreateTooltip();
		crText		= ::GetSysColor(COLOR_WINDOWTEXT);
		crBkgnd		= ::GetSysColor(COLOR_WINDOW);
		crSelText	= ::GetSysColor(COLOR_HIGHLIGHTTEXT);
		crSelBkgnd	= ::GetSysColor(COLOR_HIGHLIGHT);
		InitConfigDlg(hWnd);
		return TRUE;
	case WM_COMMAND:
		return ManageWMCommand_Config(hWnd, wParam);
	case WM_NOTIFY:
		return ManageWMNotify_Config(lParam);
	case WM_DESTROY:
		::UnhookWindowsHookEx(g_hHook);
		return TRUE;
	case WM_MEASUREITEM:
		lpmis = (LPMEASUREITEMSTRUCT) lParam;
		lpmis->itemHeight = LISTBOX_HEIGHT;
		return TRUE;
	case WM_DRAWITEM:
		lpdis = (LPDRAWITEMSTRUCT) lParam;
		if (lpdis->itemID == -1)
			return TRUE;
		if (lpdis->itemState & ODS_SELECTED) {
			::SetTextColor(lpdis->hDC, crSelText);
			::SetBkColor(lpdis->hDC, crSelBkgnd);
		} else {
			::SetTextColor(lpdis->hDC, crText);
			::SetBkColor(lpdis->hDC, crBkgnd);
		}
		::GetTextMetrics(lpdis->hDC, &textMetric);
		::ExtTextOut(lpdis->hDC,
					lpdis->rcItem.left + LISTBOX_WIDTH,
					lpdis->rcItem.top + (ICONSIZE_SMALL - textMetric.tmHeight) / 2,
					ETO_OPAQUE,
					&lpdis->rcItem,
					g_MenuData.szName[lpdis->itemData],
					::lstrlen(g_MenuData.szName[lpdis->itemData]),
					NULL);
		::DrawIconEx(lpdis->hDC,
					lpdis->rcItem.left + (LISTBOX_WIDTH - ICONSIZE_SMALL) / 2,
					lpdis->rcItem.top + (LISTBOX_HEIGHT - ICONSIZE_SMALL) / 2,
					g_MenuData.hSmallIcon[lpdis->itemData],
					ICONSIZE_SMALL,
					ICONSIZE_SMALL,
					NULL,
					NULL,
					DI_NORMAL);
		return TRUE;
	}

	return FALSE;
}

/* ==========================================================================
	Function Name	: (BOOL CALLBACK) DlgCallBack_Config()
	Outline			: �ڍאݒ�_�C�A���O�̃R�[���o�b�N�֐�
					: �iDialogProc�̃w���v�Q�Ɓj
	Arguments		: 
					: 
	Return Value	: 
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL CALLBACK DlgCallBack_Etc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg) {
	case WM_INITDIALOG:
		SetDlgPos(hWnd, POSITION_CENTER);
		InitEtcDlg(hWnd);
		return TRUE;
	case WM_COMMAND:
		return ManageWMCommand_Etc(hWnd, wParam);
	}

	return FALSE;
}

/* ==========================================================================
	Function Name	: (BOOL CALLBACK) DlgCallBack_Config()
	Outline			: �u�o�[�W�������v�_�C�A���O�̃R�[���o�b�N�֐�
					: �iDialogProc�̃w���v�Q�Ɓj
	Arguments		: 
					: 
	Return Value	: 
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL CALLBACK DlgCallBack_Version(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg) {
	case WM_INITDIALOG:
		SetDlgPos(hWnd, POSITION_CENTER);
		::SetClassLong(hWnd, GCL_HICON, (LONG) g_hIcon);
		InitVersionDlg(hWnd);
		return TRUE;
	case WM_COMMAND:
		return ManageWMCommand_Version(hWnd, wParam);
	}

	return FALSE;
}

/* ==========================================================================
	Function Name	: (LRESULT CALLBACK) WinProc()
	Outline			: ���C���E�C���h�E�̃R�[���o�b�N�֐�
					: �iWindowProc�̃w���v�Q�Ɓj
	Arguments		: 
					: 
	Return Value	: 
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
LRESULT CALLBACK WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC					hDC;
	HWND				hWndItem;
	BOOL				bRet;
	SIZE				size;
	DWORD				dwIconSize;
	DWORD				dwIconSpace;
	TEXTMETRIC			textMetric;
	LPDRAWITEMSTRUCT	lpdis;
	LPMEASUREITEMSTRUCT	lpmis;
	static UINT			WM_TASKBAR_RESTART;

	g_hWnd	= hWnd;

	switch(uMsg) {
	case WM_CREATE:
		::SetClassLong(hWnd, GCL_HICON, (LONG) g_hIcon);
		SetDlgPos(hWnd, POSITION_CENTER);
		::ShowWindow(hWnd, SW_HIDE);
		SetTaskTray(hWnd, NIM_ADD);
		WM_TASKBAR_RESTART = ::RegisterWindowMessage("TaskbarCreated");
		InitMenu();
		LoadConfig();
		InitListMenu(hWnd);
		ExecStartup(hWnd);
		return TRUE;
	case WM_COMMAND:
		bRet = ManageWMCommand_Menu(hWnd, wParam);
		return bRet;
	case WM_TMENU_NOTIFY:
		::PostMessage(hWnd, (UINT) lParam, 0, 0);
		return TRUE;
	case WM_DISPLAYCHANGE:
		InitListMenu(hWnd);
		return TRUE;
	case WM_ENDSESSION:
	case WM_DESTROY:
		SaveConfig();
		SetTaskTray(hWnd, NIM_DELETE);
		::UnregisterHotKey(hWnd, WM_MENUOPEN);
		::DestroyMenu(g_hListMenu);
		::DestroyMenu(g_hMenu);
		::PostQuitMessage(0);
		return TRUE;	
	case WM_HOTKEY:
		if (g_MenuData.bHotkey == TRUE)
			PopupListMenu(hWnd);
		return TRUE;
	case WM_LBUTTONDOWN:
	case WM_NCLBUTTONDOWN:
		if (g_MenuData.bLeftButtonPopup == TRUE)
			PopupListMenu(hWnd);
		else
			PopupMenu(hWnd);
		return TRUE;
	case WM_RBUTTONDOWN:
	case WM_NCRBUTTONDOWN:
		PopupMenu(hWnd);
		return TRUE;
	case WM_MEASUREITEM:
		lpmis		= (LPMEASUREITEMSTRUCT) lParam;
		hWndItem	= ::GetDlgItem((HWND) g_hListMenu, (UINT) wParam);
		hDC			= ::GetWindowDC(hWndItem);
		if (g_MenuData.hFont != NULL)
			::SelectObject(hDC, (HGDIOBJ) g_MenuData.hFont);
		::GetTextExtentPoint32(hDC, g_MenuData.szName[lpmis->itemData], ::lstrlen(g_MenuData.szName[lpmis->itemData]), &size);
		if (g_MenuData.dwIconMode == MODE_SMALLICON) {
			lpmis->itemWidth	= ICONSPACE_SMALL + size.cx;
			lpmis->itemHeight	= g_MenuData.dwMenuHeight;
		} else {
			lpmis->itemWidth	= ICONSPACE_LARGE + size.cx;
			lpmis->itemHeight	= g_MenuData.dwMenuHeight;
		}
		::ReleaseDC(hWndItem, hDC);
		return TRUE;
	case WM_DRAWITEM:
		lpdis = (LPDRAWITEMSTRUCT) lParam;
		if (lpdis->itemID == -1)
			return TRUE;
		if (g_MenuData.hFont != NULL)
			::SelectObject(lpdis->hDC, (HGDIOBJ) g_MenuData.hFont);
		if (lpdis->itemState & ODS_SELECTED) {
			::SetTextColor(lpdis->hDC, g_MenuData.crSelMenuTxt);
			::SetBkColor(lpdis->hDC, g_MenuData.crSelMenuBg);
		} else {
			::SetTextColor(lpdis->hDC, g_MenuData.crMenuTxt);
			::SetBkColor(lpdis->hDC, g_MenuData.crMenuBg);
		}
		if (g_MenuData.dwIconMode == MODE_LARGEICON) {
			dwIconSize	= ICONSIZE_LARGE;
			dwIconSpace	= ICONSPACE_LARGE;
		} else {
			dwIconSize	= ICONSIZE_SMALL;
			dwIconSpace	= ICONSPACE_SMALL;
		}
		::GetTextMetrics(lpdis->hDC, &textMetric);
		::ExtTextOut(lpdis->hDC,
					lpdis->rcItem.left + dwIconSpace,
					lpdis->rcItem.top + (g_MenuData.dwMenuHeight - textMetric.tmHeight) / 2,
					ETO_OPAQUE,
					&lpdis->rcItem,
					g_MenuData.szName[lpdis->itemData],
					::lstrlen(g_MenuData.szName[lpdis->itemData]),
					NULL);
		::DrawIconEx(lpdis->hDC,
					lpdis->rcItem.left + (dwIconSpace - dwIconSize) / 2,
					lpdis->rcItem.top + (g_MenuData.dwMenuHeight - dwIconSize) / 2,
					(g_MenuData.dwIconMode == MODE_LARGEICON) ? g_MenuData.hLargeIcon[lpdis->itemData] : g_MenuData.hSmallIcon[lpdis->itemData],
					dwIconSize,
					dwIconSize,
					NULL,
					NULL,
					DI_NORMAL);
		return TRUE;
	}

	if (WM_TASKBAR_RESTART != 0 && uMsg == WM_TASKBAR_RESTART)
		SetTaskTray(hWnd, NIM_ADD);

	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/* ==========================================================================
	Function Name	: (int WINAPI) WinMain()
	Outline			: ���C���֐�
	Arguments		: 
					: 
	Return Value	: 
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
int WINAPI WinMain(HINSTANCE hI, HINSTANCE, LPSTR nCmdLine, int nCmdShow)
{
	MSG			msg;
	HWND		hWnd;
	WNDCLASS	winClass;

	g_hI	= hI;
	g_hIcon	= ::LoadIcon(g_hI, (LPCSTR) TTERM_ICON);

	memset(&winClass, 0, sizeof(winClass));
	winClass.style			= (CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW | CS_DBLCLKS);
	winClass.lpfnWndProc	= WinProc;
	winClass.cbClsExtra		= 0;
	winClass.cbWndExtra		= 0;
	winClass.hInstance		= g_hI;
	winClass.hIcon			= NULL;
	winClass.hCursor		= NULL;
	winClass.hbrBackground	= NULL;
	winClass.lpszMenuName	= NULL;
	winClass.lpszClassName	= TTPMENU_CLASS;

	if (::FindWindow(TTPMENU_CLASS, NULL) == NULL) {
		if (::RegisterClass(&winClass) == 0) {
			ErrorMessage(NULL, "�E�C���h�E�N���X�̓o�^�Ɏ��s���܂����B\r\n");
			return FALSE;
		}
	}
	
	hWnd	= ::CreateWindowEx(0,
							TTPMENU_CLASS,
							"Main Window",
							WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
							CW_USEDEFAULT,
							CW_USEDEFAULT,
							CW_USEDEFAULT,
							CW_USEDEFAULT,
							(HWND) NULL,
							(HMENU) NULL,
							g_hI,
							NULL);
	if (hWnd == NULL)
		return FALSE;

	while (::GetMessage(&msg, NULL, 0, 0)) {
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}

	return TRUE;
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2005/01/25 14:06:06  yutakakn
 * SSH�������O�C���ɂ����āA���[�U�p�����[�^���w��ł���悤�ɂ����B
 * �܂��A�}�N���̎w����폜�����B
 * �f�t�H���g�I�v�V�����Ɋ����R�[�h(/KT, /KR)��ǉ������B
 *
 * Revision 1.4  2004/12/14 13:23:40  yutakakn
 * ttermpro.exe �̏����t�H���_�p�X���J�����g�ɕύX�B
 * 'use TTSSH' -> 'use SSH'�փ��l�[���B
 *
 * Revision 1.3  2004/12/03 13:35:41  yutakakn
 * SSH2�������O�C�����T�|�[�g����
 * �������A���݂�password�F�؂̂݁B
 *
 * Revision 1.2  2004/12/02 14:01:27  yutakakn
 * ���j���[������̉p�ꉻ
 *
 */