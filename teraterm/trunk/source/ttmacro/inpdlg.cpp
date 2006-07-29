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

// msgdlg のように、メッセージが長い場合にはダイアログを拡げるようにした (2006.7.xx maya)
BOOL CInpDlg::OnInitDialog()
{
  RECT R;
  HDC TmpDC;
  SIZE s;
  HWND HText, HEdit, HOk;
  int WW, WH, CW, CH, TW, TH, BW, BH, EW, EH;

  CDialog::OnInitDialog();
  SetWindowText(TitleStr);
  SetDlgItemText(IDC_INPTEXT,TextStr);

  HText = ::GetDlgItem(GetSafeHwnd(), IDC_INPTEXT);

  TmpDC = ::GetDC(GetSafeHwnd());
  CalcTextExtent(TmpDC,TextStr,&s);
  ::ReleaseDC(GetSafeHwnd(),TmpDC);
  TW = s.cx + s.cx/10;
  TH = s.cy;

  HEdit = ::GetDlgItem(GetSafeHwnd(), IDC_INPEDIT);
  ::GetWindowRect(HEdit,&R);
  EW = R.right-R.left;
  EH = R.bottom-R.top;

  HOk = ::GetDlgItem(GetSafeHwnd(), IDOK);
  ::GetWindowRect(HOk,&R);
  BW = R.right-R.left;
  BH = R.bottom-R.top;

  GetWindowRect(&R);
  WW = R.right-R.left;
  WH = R.bottom-R.top;
  GetClientRect(&R);
  CW = R.right-R.left;
  CH = R.bottom-R.top;
  if (TW < CW)
    TW = CW;
  if (EW < s.cx)
    EW = s.cx;
  WW = WW + TW - CW;
  WH = WH + 2*TH+3*BH/2 - CH + EH/2+BH/2+12;

  ::MoveWindow(HText,(TW-s.cx)/2,TH/2,TW,TH,TRUE);
  ::MoveWindow(HEdit,(WW-EW)/2-4,2*TH+5,EW,EH,TRUE);
  ::MoveWindow(HOk,(TW-BW)/2,2*TH+EH/2+BH/2+12,BW,BH,TRUE);

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
  SetWindowPos(&wndTop,PosX,PosY,WW,WH,0);
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
