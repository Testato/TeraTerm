/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TTMACRO.EXE, error dialog box */

#include "stdafx.h"
#include "teraterm.h"
#include "ttlib.h"
#include "ttm_res.h"

#include "tttypes.h"
#include "ttcommon.h"
#include "helpid.h"

#include "errdlg.h"
#include "ttmlib.h"
#include "ttmparse.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CErrDlg dialog
CErrDlg::CErrDlg(PCHAR Msg, PCHAR Line, int x, int y, int lineno, int start, int end)
	: CDialog(CErrDlg::IDD)
{
	//{{AFX_DATA_INIT(CErrDlg)
	//}}AFX_DATA_INIT
	MsgStr = Msg;
	LineStr = Line;
	PosX = x;
	PosY = y;
	LineNo = lineno;
	StartPos = start;
	EndPos = end;
}

BEGIN_MESSAGE_MAP(CErrDlg, CDialog)
	//{{AFX_MSG_MAP(CErrDlg)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_MACROERRHELP, &CErrDlg::OnBnClickedMacroerrhelp)
END_MESSAGE_MAP()

// CErrDlg message handler

BOOL CErrDlg::OnInitDialog()
{
	RECT R;
	HDC TmpDC;
	char uimsg[MAX_UIMSG], uimsg2[MAX_UIMSG];
	LOGFONT logfont;
	HFONT font;
	char buf[MaxLineLen*2], buf2[10];
	int i, len;

	CDialog::OnInitDialog();
	font = (HFONT)SendMessage(WM_GETFONT, 0, 0);
	GetObject(font, sizeof(LOGFONT), &logfont);
	if (get_lang_font("DLG_SYSTEM_FONT", m_hWnd, &logfont, &DlgFont, UILanguageFile)) {
		SendDlgItemMessage(IDC_ERRMSG, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
		SendDlgItemMessage(IDC_ERRLINE, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
		SendDlgItemMessage(IDOK, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
		SendDlgItemMessage(IDCANCEL, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
	}

	GetDlgItemText(IDOK, uimsg2, sizeof(uimsg2));
	get_lang_msg("BTN_STOP", uimsg, sizeof(uimsg), uimsg2, UILanguageFile);
	SetDlgItemText(IDOK, uimsg);
	GetDlgItemText(IDCANCEL, uimsg2, sizeof(uimsg2));
	get_lang_msg("BTN_CONTINUE", uimsg, sizeof(uimsg), uimsg2, UILanguageFile);
	SetDlgItemText(IDCANCEL, uimsg);
	GetDlgItemText(IDC_MACROERRHELP, uimsg2, sizeof(uimsg2));
	get_lang_msg("BTN_HELP", uimsg, sizeof(uimsg), uimsg2, UILanguageFile);
	SetDlgItemText(IDC_MACROERRHELP, uimsg);

	SetDlgItemText(IDC_ERRMSG,MsgStr);

	// �s�ԍ���擪�ɂ���B
	// TODO: �t�@�C�������������B
	// �G���[�ӏ��Ɉ������B
	_snprintf_s(buf, sizeof(buf), _TRUNCATE, "%d:", LineNo);
	SetDlgItemText(IDC_ERRLINE, buf);

	len = strlen(LineStr);
	buf[0] = 0;
	for (i = 0 ; i < len ; i++) {
		if (i == StartPos)
			strncat_s(buf, sizeof(buf), "<<<", _TRUNCATE);
		if (i == EndPos)
			strncat_s(buf, sizeof(buf), ">>>", _TRUNCATE);
		buf2[0] = LineStr[i];
		buf2[1] = 0;
		strncat_s(buf, sizeof(buf), buf2, _TRUNCATE);
	}
	if (EndPos == len)
		strncat_s(buf, sizeof(buf), ">>>", _TRUNCATE);
	SetDlgItemText(IDC_EDIT_ERRLINE, buf);

	if (PosX<=GetMonitorLeftmost(PosX, PosY)-100) {
		GetWindowRect(&R);
		TmpDC = ::GetDC(GetSafeHwnd());
		PosX = (GetDeviceCaps(TmpDC,HORZRES)-R.right+R.left) / 2;
		PosY = (GetDeviceCaps(TmpDC,VERTRES)-R.bottom+R.top) / 2;
		::ReleaseDC(GetSafeHwnd(),TmpDC);
	}
	SetWindowPos(&wndTop,PosX,PosY,0,0,SWP_NOSIZE);
	SetForegroundWindow();

	return TRUE;
}

void CErrDlg::OnBnClickedMacroerrhelp()
{
	OpenHelp(HH_HELP_CONTEXT,HlpMacroAppendixesError);
}
