/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TERATERM.EXE, file transfer dialog box */
#include "stdafx.h"
#include "teraterm.h"
#include "tttypes.h"
#include "ttftypes.h"
#include "ttlib.h"
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

#ifndef NO_I18N
BOOL CFileTransDlg::Create(PFileVar pfv, PComVar pcv, PTTSet pts)
#else
BOOL CFileTransDlg::Create(PFileVar pfv, PComVar pcv)
#endif
{
  BOOL Ok;
  WNDCLASS wc;

  fv = pfv;
  cv = pcv;
  cv->FilePause &= ~fv->OpId;
#ifndef NO_I18N
  ts = pts;
  LOGFONT logfont;
  HFONT font;
#endif

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

#ifndef NO_I18N
  font = (HFONT)SendMessage(WM_GETFONT, 0, 0);
  GetObject(font, sizeof(LOGFONT), &logfont);
  if (get_lang_font("DLG_SYSTEM_FONT", fv->HWin, &logfont, &DlgFont, ts->UILanguageFile)) {
	SendDlgItemMessage(IDC_TRANS_FILENAME, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
	SendDlgItemMessage(IDC_TRANSFNAME, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
	SendDlgItemMessage(IDC_FULLPATH_LABEL, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
	SendDlgItemMessage(IDC_EDIT_FULLPATH, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
	SendDlgItemMessage(IDC_TRANS_TRANS, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
	SendDlgItemMessage(IDC_TRANSBYTES, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
	SendDlgItemMessage(IDC_TRANSPAUSESTART, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
	SendDlgItemMessage(IDCANCEL, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
	SendDlgItemMessage(IDC_TRANSHELP, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
  }
#endif

  return Ok;
}

void CFileTransDlg::ChangeButton(BOOL PauseFlag)
{
  Pause = PauseFlag;
  if (Pause)
  {
#ifndef NO_I18N
    strcpy(ts->UIMsg, "&Start");
	get_lang_msg("DLG_FILETRANS_START", ts->UIMsg, ts->UILanguageFile);
    SetDlgItemText(IDC_TRANSPAUSESTART, ts->UIMsg);
#else
    SetDlgItemText(IDC_TRANSPAUSESTART, "&Start");
#endif
    cv->FilePause |= fv->OpId;
  }
  else {
#ifndef NO_I18N
    strcpy(ts->UIMsg, "Pau&se");
	get_lang_msg("DLG_FILETRANS_PAUSE", ts->UIMsg, ts->UILanguageFile);
    SetDlgItemText(IDC_TRANSPAUSESTART, ts->UIMsg);
#else
    SetDlgItemText(IDC_TRANSPAUSESTART, "Pau&se");
#endif
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
