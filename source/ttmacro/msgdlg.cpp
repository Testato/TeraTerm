/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TTMACRO.EXE, message dialog box */

#include "stdafx.h"
#include "teraterm.h"
#ifdef TERATERM32
#include "ttm_res.h"
#else
#include "ttm_re16.h"
#endif
#include "ttmlib.h"

#include "msgdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CMsgDlg dialog

CMsgDlg::CMsgDlg(PCHAR Text, PCHAR Title, BOOL YesNo, int x, int y)
	: CDialog(CMsgDlg::IDD)
{
  //{{AFX_DATA_INIT(CMsgDlg)
  //}}AFX_DATA_INIT
  TextStr = Text;
  TitleStr = Title;
  YesNoFlag = YesNo;
  PosX = x;
  PosY = y;
}

BEGIN_MESSAGE_MAP(CMsgDlg, CDialog)
	//{{AFX_MSG_MAP(CMsgDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// CMsgDlg message handler

BOOL CMsgDlg::OnInitDialog()
{
  RECT R;
  HDC TmpDC;
  SIZE s;
  HWND HText, HOk, HNo;
  int WW, WH, CW, CH, TW, TH, BW, BH;

  CDialog::OnInitDialog();
  SetWindowText(TitleStr);
  SetDlgItemText(IDC_MSGTEXT,TextStr);

  HText = ::GetDlgItem(GetSafeHwnd(), IDC_MSGTEXT);
  
  TmpDC = ::GetDC(GetSafeHwnd());
  CalcTextExtent(TmpDC,TextStr,&s);
  ::ReleaseDC(GetSafeHwnd(),TmpDC);
  TW = s.cx + s.cx/10;
  TH = s.cy;

  HOk = ::GetDlgItem(GetSafeHwnd(), IDOK);
  HNo = ::GetDlgItem(GetSafeHwnd(), IDCANCEL);
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
  if (YesNoFlag && (TW < 7*BW/2))
    TW = 7*BW/2;
  WW = WW + TW - CW;
  WH = WH + 2*TH+3*BH/2 - CH;

  ::MoveWindow(HText,(TW-s.cx)/2,TH/2,TW,TH,TRUE);
  if (YesNoFlag)
  {
    ::SetWindowText(HOk,"&Yes");
    ::MoveWindow(HOk,(2*TW-5*BW)/4,2*TH,BW,BH,TRUE);
    ::SetWindowText(HNo,"&No");
    ::MoveWindow(HNo,(2*TW+BW)/4,2*TH,BW,BH,TRUE);
    ::ShowWindow(HNo,SW_SHOW);
  }
  else
    ::MoveWindow(HOk,(TW-BW)/2,2*TH,BW,BH,TRUE);

  if (PosX<=-100)
  {
    TmpDC = ::GetDC(GetSafeHwnd());
    PosX = (GetDeviceCaps(TmpDC,HORZRES)-WW) / 2;
    PosY = (GetDeviceCaps(TmpDC,VERTRES)-WH) / 2;
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
