/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TTMACRO.EXE, status dialog box */

#include "stdafx.h"
#include "teraterm.h"
#ifdef TERATERM32
#include "ttm_res.h"
#else
#include "ttm_re16.h"
#endif
#include "ttmlib.h"

#include "statdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CStatDlg dialog

BEGIN_MESSAGE_MAP(CStatDlg, CDialog)
	//{{AFX_MSG_MAP(CStatDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CStatDlg::Create(PCHAR Text, PCHAR Title, int x, int y)
{
  RestoreNewLine(Text); // (2006.7.29 maya)
  TextStr = Text;
  TitleStr = Title;
  PosX = x;
  PosY = y;
  return CDialog::Create(CStatDlg::IDD, GetDesktopWindow());
}

void CStatDlg::Update(PCHAR Text, PCHAR Title, int x, int y)
{
  RECT R;
  HDC TmpDC;
  SIZE s;
  HWND HText;
  int WW, WH, CW, CH, TW, TH;

  if (Title!=NULL)
    SetWindowText(Title);

  GetWindowRect(&R);
  PosX = R.left;
  PosY = R.top;
  WW = R.right-R.left;
  WH = R.bottom-R.top;

  if (Text!=NULL)
  {
    SetDlgItemText(IDC_STATTEXT,Text);

    HText = ::GetDlgItem(GetSafeHwnd(), IDC_STATTEXT);

    TmpDC = ::GetDC(GetSafeHwnd());
    CalcTextExtent(TmpDC,Text,&s);
    ::ReleaseDC(GetSafeHwnd(),TmpDC);
    TW = s.cx + s.cx/10;
    TH = s.cy;

    GetClientRect(&R);
    CW = R.right-R.left;
    CH = R.bottom-R.top;
    if (TW < CW)
      TW = CW;
    WW = WW + TW - CW;
    WH = WH + 2*TH - CH;

    ::MoveWindow(HText,(TW-s.cx)/2,TH/2,TW,TH,TRUE);
  }

  if (x!=32767)
  {
    PosX = x;
    PosY = y;
  }
  if (PosX<=-100)
  {
    TmpDC = ::GetDC(GetSafeHwnd());
    PosX = (GetDeviceCaps(TmpDC,HORZRES)-WW) / 2;
    PosY = (GetDeviceCaps(TmpDC,VERTRES)-WH) / 2;
    ::ReleaseDC(GetSafeHwnd(),TmpDC);
  }
  SetWindowPos(&wndTop,PosX,PosY,WW,WH,SWP_NOZORDER);
}

// CStatDlg message handler

BOOL CStatDlg::OnInitDialog()
{
  CDialog::OnInitDialog();
  Update(TextStr,TitleStr,PosX,PosY);
#ifdef TERATERM32
  SetForegroundWindow();
#else
  SetActiveWindow();
#endif
  return TRUE;
}

void CStatDlg::OnCancel()
{
  DestroyWindow();
}

BOOL CStatDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
  switch (LOWORD(wParam)) {
    case IDCANCEL:
      if ((HWND)lParam!=NULL) // ignore ESC key
	DestroyWindow();
      return TRUE;
    default:
      return (CDialog::OnCommand(wParam,lParam));
  }
}

void CStatDlg::PostNcDestroy()
{
  delete this;
}
