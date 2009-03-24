/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TTMACRO.EXE, input dialog box */

#include "stdafx.h"
#include "teraterm.h"
#ifdef TERATERM32
#include "ttm_res.h"
#else
#include "ttm_re16.h"
#endif

#include "inpdlg.h"

#define MaxStrLen 256

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CInpDlg dialog
CInpDlg::CInpDlg(PCHAR Input, PCHAR Text, PCHAR Title,
		 BOOL Paswd, int x, int y)
	: CDialog(CInpDlg::IDD)
{
  //{{AFX_DATA_INIT(CInpDlg)
  //}}AFX_DATA_INIT
  InputStr = Input;
  TextStr = Text;
  TitleStr = Title;
  PaswdFlag = Paswd;
  PosX = x;
  PosY = y;
}

BEGIN_MESSAGE_MAP(CInpDlg, CDialog)
	//{{AFX_MSG_MAP(CInpDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// CInpDlg message handler

BOOL CInpDlg::OnInitDialog()
{
  RECT R;
  HDC TmpDC;

  CDialog::OnInitDialog();
  SetWindowText(TitleStr);
  SetDlgItemText(IDC_INPTEXT,TextStr);
  if (PaswdFlag)
    SendDlgItemMessage(IDC_INPEDIT,EM_SETPASSWORDCHAR,(UINT)'*',0);

  SendDlgItemMessage(IDC_INPEDIT, EM_LIMITTEXT, MaxStrLen, 0);

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

void CInpDlg::OnOK()
{
  GetDlgItemText(IDC_INPEDIT,InputStr,MaxStrLen-1);
  EndDialog(IDOK);
}
