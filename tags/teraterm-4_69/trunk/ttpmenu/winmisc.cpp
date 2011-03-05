#define		STRICT
static char *WinMisc_id = 
	"@(#)Copyright (C) NTT-IT 1998-2002  -- winmisc.cpp --  Ver1.00b2";
/* ==========================================================================
	Project Name		: Universal Library
	Outline				: WinMisc Function
	Create				: 1998-02-20(Wed)
	Update				: 2002-09-25(Wed)
	Copyright			: S.Hayakawa				NTT-IT
	Reference			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
#include	"winmisc.h"


/* ==========================================================================
	Function Name	: (void) SetDlgPos()
	Outline			: �_�C�A���O�̈ʒu���ړ�����
	Arguments		: HWND		hWnd	(in)	�_�C�A���O�̃E�C���h�E�n���h��
					: int		pos		(in)	�ړ��ꏊ�������l
	Return Value	: �Ȃ�
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
void SetDlgPos(HWND hWnd, int pos)
{
	int		x;
	int		y;
	RECT	rect;

	::GetWindowRect(hWnd, &rect);

	int	cx		= ::GetSystemMetrics(SM_CXFULLSCREEN);
	int	cy		= ::GetSystemMetrics(SM_CYFULLSCREEN);
	int	xsize	= rect.right - rect.left;
	int	ysize	= rect.bottom - rect.top;

	switch (pos) {
	case POSITION_LEFTTOP:
		x = 0;
		y = 0;
		break;
	case POSITION_LEFTBOTTOM:
		x = 0;
		y = cy - ysize;
		break;
	case POSITION_RIGHTTOP:
		x = cx - xsize;
		y = 0;
		break;
	case POSITION_RIGHTBOTTOM:
		x = cx - xsize;
		y = cy - ysize;
		break;
	case POSITION_CENTER:
		x = (cx - xsize) / 2;
		y = (cy - ysize) / 2;
		break;
	case POSITION_OUTSIDE:
		x = cx;
		y = cy;
		break;
	}

	::MoveWindow(hWnd, x, y, xsize, ysize, TRUE);
}

/* ==========================================================================
	Function Name	: (BOOL) EnableItem()
	Outline			: �_�C�A���O�A�C�e����L���^�����ɂ���
	Arguments		: HWND		hWnd		(in)	�_�C�A���O�̃n���h��
					: int		idControl	(in)	�_�C�A���O�A�C�e���� ID
					: BOOL		flag
	Return Value	: ���� TRUE
					: ���s FALSE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL EnableItem(HWND hWnd, int idControl, BOOL flag)
{
	HWND	hWndItem;

	if ((hWndItem = ::GetDlgItem(hWnd, idControl)) == NULL)
		return FALSE;

	return ::EnableWindow(hWndItem, flag);
}

/* ==========================================================================
	Function Name	: (void) EncodePassword()
	Outline			: �p�X���[�h���G���R�[�h(?)����B
	Arguments		: char		cPassword		(in)	�ϊ����镶����
					: int		cEncodePassword	(out)	�ϊ����ꂽ������
	Return Value	: �Ȃ�
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
void EncodePassword(TCHAR *cPassword, TCHAR *cEncodePassword)
{
	DWORD	dwCnt;
	DWORD	dwPasswordLength = ::lstrlen(cPassword);

	for (dwCnt = 0; dwCnt < dwPasswordLength; dwCnt++)
		cEncodePassword[dwPasswordLength - 1 - dwCnt] = cPassword[dwCnt] ^ 0xff;

	cEncodePassword[dwPasswordLength] = '\0';
}

/* ==========================================================================
	Function Name	: (BOOL) SaveFileDlg()
	Outline			: �u���O���w�肵�ĕۑ��v�_�C�A���O���J���A�w�肳�ꂽ�t�@�C��
					: �p�X���w�肳�ꂽ�A�C�e���ɑ���
	Arguments		: HWND		hWnd		(in)	�e�E�C���h�E�̃n���h��
					: UINT		editCtl		(in)	�A�C�e���� ID
					: char		*title		(in)	�E�C���h�E�^�C�g��
					: char		*filter		(in)	�\������t�@�C���̃t�B���^
					: char		*defaultDir	(in)	�f�t�H���g�̃p�X
	Return Value	: ���� TRUE
					: ���s FALSE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL SaveFileDlg(HWND hWnd, UINT editCtl, TCHAR *title, TCHAR *filter, TCHAR *defaultDir)
{
	TCHAR			*szDirName;
	TCHAR			szFile[MAX_PATH] = _T("");
	TCHAR			szPath[MAX_PATH];
	OPENFILENAME	ofn;

	szDirName	= (TCHAR *) malloc(MAX_PATH);

	if (editCtl == 0xffffffff) {
		::GetDlgItemText(hWnd, editCtl, szDirName, MAX_PATH);
		::lstrcpy(szDirName, defaultDir);
	}

	if (*szDirName == '"')
		szDirName++;
	if (szDirName[::lstrlen(szDirName) - 1] == '"')
		szDirName[::lstrlen(szDirName) - 1] = '\0';

	TCHAR *ptr = _tcsrchr(szDirName, '\\');
	if (ptr == NULL) {
		::lstrcpy(szFile, szDirName);
		if (defaultDir != NULL && *szDirName == 0)
			::lstrcpy(szDirName, defaultDir);
	} else {
		*ptr = 0;
		::lstrcpy(szFile, ptr + 1);
	}

	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize		= sizeof(OPENFILENAME);
	ofn.hwndOwner		= hWnd;
	ofn.lpstrFilter		= filter;
	ofn.nFilterIndex	= 1;
	ofn.lpstrFile		= szFile;
	ofn.nMaxFile		= sizeof(szFile);
	ofn.lpstrTitle		= title;
	ofn.lpstrInitialDir	= szDirName;
	ofn.Flags			= OFN_HIDEREADONLY;

	if (::GetSaveFileName(&ofn) == FALSE) {
		free(szDirName);
		return	FALSE;
	}

	::lstrcpy(szPath, ofn.lpstrFile);

	::SetDlgItemText(hWnd, editCtl, szPath);
	::lstrcpy(defaultDir, szPath);

	free(szDirName);

	return	TRUE;
}

/* ==========================================================================
	Function Name	: (BOOL) OpenFileDlg()
	Outline			: �u�t�@�C�����J���v�_�C�A���O���J���A�w�肳�ꂽ�t�@�C��
					: �p�X���w�肳�ꂽ�A�C�e���ɑ���
	Arguments		: HWND		hWnd		(in)	�e�E�C���h�E�̃n���h��
					: UINT		editCtl		(in)	�A�C�e���� ID
					: char		*title		(in)	�E�C���h�E�^�C�g��
					: char		*filter		(in)	�\������t�@�C���̃t�B���^
					: char		*defaultDir	(in)	�f�t�H���g�̃p�X
	Return Value	: ���� TRUE
					: ���s FALSE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL OpenFileDlg(HWND hWnd, UINT editCtl, TCHAR *title, TCHAR *filter, TCHAR *defaultDir)
{
	TCHAR			*szDirName;
	TCHAR			szFile[MAX_PATH] = _T("");
	TCHAR			szPath[MAX_PATH];
	OPENFILENAME	ofn;

	szDirName	= (TCHAR *) malloc(MAX_PATH);

	if (editCtl != 0xffffffff) {
		::GetDlgItemText(hWnd, editCtl, szDirName, MAX_PATH);
		::lstrcpy(szDirName, defaultDir);
	}

	if (*szDirName == '"')
		szDirName++;
	if (szDirName[::lstrlen(szDirName) - 1] == '"')
		szDirName[::lstrlen(szDirName) - 1] = '\0';

	TCHAR *ptr = _tcsrchr(szDirName, '\\');
	if (ptr == NULL) {
		::lstrcpy(szFile, szDirName);
		if (defaultDir != NULL && *szDirName == 0)
			::lstrcpy(szDirName, defaultDir);
	} else {
		*ptr = 0;
		::lstrcpy(szFile, ptr + 1);
	}

	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize		= sizeof(OPENFILENAME);
	ofn.hwndOwner		= hWnd;
	ofn.lpstrFilter		= filter;
	ofn.nFilterIndex	= 1;
	ofn.lpstrFile		= szFile;
	ofn.nMaxFile		= sizeof(szFile);
	ofn.lpstrTitle		= title;
	ofn.lpstrInitialDir	= szDirName;
	ofn.Flags			= OFN_HIDEREADONLY | OFN_NODEREFERENCELINKS;

	if (::GetOpenFileName(&ofn) == FALSE) {
		free(szDirName);
		return	FALSE;
	}

	::lstrcpy(szPath, ofn.lpstrFile);

	::SetDlgItemText(hWnd, editCtl, szPath);
	::lstrcpy(defaultDir, szPath);

	free(szDirName);

	return	TRUE;
}

/* ==========================================================================
	Function Name	: (int CALLBACK) BrowseCallbackProc()
	Outline			: BrowseForFolder()�̃R�[���o�b�N�֐��B
	Arguments		: HWND		hWnd		(in)	�e�E�C���h�E�̃n���h��
					: UINT		uMsg		(in)	�E�C���h�E�^�C�g��
					: LPARAM	lParam		(in)	LPARAM
					: LPARAM	lpData		(in)	BROWSEINFO �� lParam
	Return Value	: 
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
int CALLBACK BrowseCallbackProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    switch (uMsg) {
    case BFFM_INITIALIZED:
		::SendMessage(hWnd, BFFM_SETSELECTION, TRUE, lpData);
		break;
    case BFFM_SELCHANGED:
		break;
    }

    return 0;
}

/* ==========================================================================
	Function Name	: (BOOL) GetModulePath()
	Outline			: ���̊֐������s�������W���[���̃p�X���擾����B
	Arguments		: char		szPath		(out)	���W���[���̃p�X
					: DWORD		dwMaxPath	(in)	szPath�̑傫��
	Return Value	: ���� TRUE
					: ���s FALSE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL GetModulePath(TCHAR *szPath, DWORD dwMaxPath)
{
	TCHAR	*pt;

	if (::GetModuleFileName(NULL, szPath, dwMaxPath) == FALSE)
		return FALSE;

	pt	= _tcsrchr(szPath, '\\');
	*pt	= '\0';

	return TRUE;
}

/* ==========================================================================
	Function Name	: (UINT) GetResourceType()
	Outline			: �w�肳�ꂽ�p�X�̃h���C�u�����擾����
	Arguments		: LPCTSTR	lpszPath	(in)	�h���C�u����m�肽���p�X
	Return Value	: ���\�[�X�^�C�v
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
UINT GetResourceType(LPCTSTR lpszPath)
{
	UINT	ret;
	TCHAR	szCurrentPath[MAX_PATH];

	if (::GetCurrentDirectory(MAX_PATH, szCurrentPath) == 0)
		return 0;

	if (::SetCurrentDirectory(lpszPath) == FALSE)
		return 0;

	ret = ::GetDriveType(NULL);

	::SetCurrentDirectory(szCurrentPath);

	return ret;
}

/* ==========================================================================
	Function Name	: (char *) pathTok()
	Outline			: �p�X�p�� strtok
	Arguments		: char		*str		(in)	�g�[�N����͂���p�X
					: char		*separator	(in)	�g�[�N��������
	Return Value	: ���� �؂�o����������
					: ���s NULL
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
TCHAR *PathTok(TCHAR *str, TCHAR *separator)
{
	static TCHAR	*sv_str;

	if (str != NULL)
		sv_str = str;
	else if (sv_str != NULL)
		str = sv_str;
	else
		return	NULL;

	while (*str != '\0' && _tcschr(separator, *str) != NULL)
		str++;

 	if (*str == '\"') {
		for (sv_str = ++str; *sv_str != '\0' && *sv_str != '\"'; sv_str++) 
			;
		*sv_str++ = '\0';
	} else {
		for (sv_str=str ; *sv_str != '\0'; sv_str++) {
			if (_tcschr(separator, *sv_str) != NULL) {
				*sv_str++ = '\0';
				break;
			}
		}
	}
	if (sv_str != str)
		return	str;
	else
		return	sv_str = NULL;
}

TCHAR *lstrstri(TCHAR *s1, TCHAR *s2)
{
	DWORD	dwLen1= ::lstrlen(s1);
	DWORD	dwLen2= ::lstrlen(s2);

	for (DWORD dwCnt = 0; dwCnt <= dwLen1; dwCnt++) {
		// VS2005�Ńr���h�G���[�ƂȂ邽�� dwCnt2 �錾��ǉ� (2006.2.18 yutaka)
		DWORD dwCnt2;
		for (dwCnt2 = 0; dwCnt2 <= dwLen2; dwCnt2++)
			if (tolower(s1[dwCnt + dwCnt2]) != tolower(s2[dwCnt2]))
				break;
		if (dwCnt2 > dwLen2)
			return s1 + dwCnt;
	}

	return NULL;
}

BOOL SetForceForegroundWindow(HWND hWnd)
{
#ifndef SPI_GETFOREGROUNDLOCKTIMEOUT
#define SPI_GETFOREGROUNDLOCKTIMEOUT 0x2000
#define SPI_SETFOREGROUNDLOCKTIMEOUT 0x2001
#endif
	DWORD	foreId, targId, svTmOut;

	foreId = ::GetWindowThreadProcessId(::GetForegroundWindow(), NULL);
	targId = ::GetWindowThreadProcessId(hWnd, NULL);
	::AttachThreadInput(targId, foreId, TRUE);
	::SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, (void *)&svTmOut, 0);
	::SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, 0, 0);
	BOOL	ret = ::SetForegroundWindow(hWnd);
	::SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (void *)svTmOut, 0);
	::AttachThreadInput(targId, foreId, FALSE);

	return	ret;
}

void UTIL_get_lang_msg(PCHAR key, PCHAR buf, int buf_len, PCHAR def, PCHAR iniFile)
{
	GetI18nStr("TTMenu", key, buf, buf_len, def, iniFile);
}

int UTIL_get_lang_font(PCHAR key, HWND dlg, PLOGFONT logfont, HFONT *font, PCHAR iniFile)
{
	if (GetI18nLogfont("TTMenu", key, logfont,
					   GetDeviceCaps(GetDC(dlg),LOGPIXELSY),
					   iniFile) == FALSE) {
		return FALSE;
	}

	if ((*font = CreateFontIndirect(logfont)) == NULL) {
		return FALSE;
	}

	return TRUE;
}
