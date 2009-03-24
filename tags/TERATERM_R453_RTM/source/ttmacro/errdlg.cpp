/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TTMACRO.EXE, error dialog box */

#include "stdafx.h"
#include "teraterm.h"
#include "ttlib.h"
#ifdef TERATERM32
#include "ttm_res.h"
#else
#include "ttm_re16.h"
#endif

#include "errdlg.h"
#include "ttmlib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CErrDlg dialog
CErrDlg::CErrDlg(PCHAR Msg, PCHAR Line, int x, int y)
	: CDialog(CErrDlg::IDD)
{
  //{{AFX_DATA_INIT(CErrDlg)
  //}}AFX_DATA_INIT
  MsgStr = Msg;
  LineStr = Line;
  PosX = x;
  PosY = y;
}

BEGIN_MESSAGE_MAP(CErrDlg, CDialog)
	//{{AFX_MSG_MAP(CErrDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// CErrDlg message handler

BOOL CErrDlg::OnInitDialog()
{
  RECT R;
  HDC TmpDC;
#ifndef NO_I18N
  char uimsg[MAX_UIMSG];
  LOGFONT logfont;
  HFONT font;
#endif

  CDialog::OnInitDialog();
#ifndef NO_I18N
  font = (HFONT)SendMessage(WM_GETFONT, 0, 0);
  GetObject(font, sizeof(LOGFONT), &logfont);
  if (get_lang_font("DLG_SYSTEM_FONT", m_hWnd, &logfont, &DlgFont, UILanguageFile)) {
    SendDlgItemMessage(IDC_ERRMSG, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
    SendDlgItemMessage(IDC_ERRLINE, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
    SendDlgItemMessage(IDOK, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
    SendDlgItemMessage(IDCANCEL, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
  }

  GetDlgItemText(IDOK, uimsg, sizeof(uimsg));
  get_lang_msg("BTN_STOP", uimsg, UILanguageFile);
  SetDlgItemText(IDOK, uimsg);
  GetDlgItemText(IDCANCEL, uimsg, sizeof(uimsg));
  get_lang_msg("BTN_CONTINUE", uimsg, UILanguageFile);
  SetDlgItemText(IDCANCEL, uimsg);
#endif

  SetDlgItemText(IDC_ERRMSG,MsgStr);
  SetDlgItemText(IDC_ERRLINE,LineStr);

  if (PosX<=-100)
  {
    GetWindowRect(&R);
    TmpDC = ::GetDC(GetSafeHwnd());
    PosX = (GetDeviceCaps(TmpDC,HORZRES)-R.right+R.left) / 2;
    PosY = (GetDeviceCaps(TmpDC,VERTRES)-R.bottom+R.top) / 2;
    ::ReleaseDC(GetSafeHwnd(),TmpDC);
  }
  SetWindowPos(&wndTop,PosX,PosY,0,0,SWP_NOSIZE);
#ifdef TERATERM32
  SetForegroundWindow();
#else
  SetActiveWindow();
#endif
  return TRUE;
}
