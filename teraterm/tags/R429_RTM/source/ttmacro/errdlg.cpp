/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TTMACRO.EXE, error dialog box */

#include "stdafx.h"
#include "teraterm.h"
#ifdef TERATERM32
#include "ttm_res.h"
#else
#include "ttm_re16.h"
#endif

#include "errdlg.h"

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

  CDialog::OnInitDialog();
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
