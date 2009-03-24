/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TERATERM.EXE, file transfer dialog box */
#include "stdafx.h"
#include "teraterm.h"
#include "tttypes.h"
#include "ttftypes.h"
#ifdef TERATERM32
#include "tt_res.h"
#else
#include "ttctl3d.h"
#include "tt_res16.h"
#endif
#include "ftdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileTransDlg dialog

BEGIN_MESSAGE_MAP(CFileTransDlg, CDialog)
	//{{AFX_MSG_MAP(CFileTransDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CFileTransDlg::Create(PFileVar pfv, PComVar pcv)
{
  BOOL Ok;
  WNDCLASS wc;

  fv = pfv;
  cv = pcv;
  cv->FilePause &= ~fv->OpId;

  wc.style = CS_PARENTDC;
  wc.lpfnWndProc = AfxWndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = DLGWINDOWEXTRA;
  wc.hInstance = AfxGetInstanceHandle();
  wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_TTERM));
  wc.hCursor = LoadCursor(NULL,IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
  wc.lpszMenuName = NULL;
#ifdef TERATERM32
  wc.lpszClassName = "FTDlg32";
#else
  wc.lpszClassName = "FTDlg";
#endif
  RegisterClass(&wc);

  Pause = FALSE;
  if (fv->OpId == OpLog) // parent window is desktop
    Ok = CDialog::Create(CFileTransDlg::IDD, GetDesktopWindow());
  else // parent window is VT window
    Ok = CDialog::Create(CFileTransDlg::IDD, NULL);

  fv->HWin = GetSafeHwnd();
  return Ok;
}

void CFileTransDlg::ChangeButton(BOOL PauseFlag)
{
  Pause = PauseFlag;
  if (Pause)
  {
    SetDlgItemText(IDC_TRANSPAUSESTART, "&Start");
    cv->FilePause |= fv->OpId;
  }
  else {
    SetDlgItemText(IDC_TRANSPAUSESTART, "Pau&se");
    cv->FilePause &= ~fv->OpId;
  }
}

void CFileTransDlg::RefreshNum()
{
  char NumStr[13];

  sprintf(NumStr,"%u",fv->ByteCount);
  SetDlgItemText(IDC_TRANSBYTES, NumStr);
}

/////////////////////////////////////////////////////////////////////////////
// CFileTransDlg message handler

BOOL CFileTransDlg::OnInitDialog()
{
#ifndef TERATERM32
  SubClassDlg(GetSafeHwnd()); /* CTL3D */
#endif
  SetWindowText(fv->DlgCaption);
  SetDlgItemText(IDC_TRANSFNAME, &(fv->FullName[fv->DirLen]));

  // ログファイルはフルパス表示にする(2004.8.6 yutaka)
  SetDlgItemText(IDC_EDIT_FULLPATH, &(fv->FullName[0]));

#ifdef TERATERM32
  // set the small icon
  ::PostMessage(GetSafeHwnd(),WM_SETICON,0,
    (LPARAM)LoadImage(AfxGetInstanceHandle(),
      MAKEINTRESOURCE(IDI_TTERM),
      IMAGE_ICON,16,16,0));
#endif
  return 1;
}

void CFileTransDlg::OnCancel( )
{
  ::PostMessage(fv->HMainWin,WM_USER_FTCANCEL,fv->OpId,0);
}

BOOL CFileTransDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
  switch (LOWORD(wParam)) {
    case IDCANCEL:
      ::PostMessage(fv->HMainWin,WM_USER_FTCANCEL,fv->OpId,0);
      return TRUE;
    case IDC_TRANSPAUSESTART:
      ChangeButton(! Pause);
      return TRUE;
    case IDC_TRANSHELP:
      ::PostMessage(fv->HMainWin,WM_USER_DLGHELP2,0,0);
      return TRUE;
    default:
      return (CDialog::OnCommand(wParam,lParam));
  }
}

void CFileTransDlg::PostNcDestroy()
{
  delete this;
}

LRESULT CFileTransDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
  return DefDlgProc(GetSafeHwnd(),message,wParam,lParam);
}
