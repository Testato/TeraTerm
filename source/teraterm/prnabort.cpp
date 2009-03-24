/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TERATERM.EXE, print-abort dialog box */
#include "stdafx.h"
#include "teraterm.h"
#include "tttypes.h"
#include "ttlib.h"
#ifdef TERATERM32
#include "tt_res.h"
#else
#include "tt_res16.h"
#endif
#include "prnabort.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CPrnAbortDlg dialog
BEGIN_MESSAGE_MAP(CPrnAbortDlg, CDialog)
	//{{AFX_MSG_MAP(CPrnAbortDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// CPrnAbortDlg message handler
#ifndef NO_I18N
BOOL CPrnAbortDlg::Create(CWnd* p_Parent, PBOOL AbortFlag, PTTSet pts)
#else
BOOL CPrnAbortDlg::Create(CWnd* p_Parent, PBOOL AbortFlag)
#endif
{
  BOOL Ok;
  HWND HParent;
#ifndef NO_I18N
  LOGFONT logfont;
  HFONT font;
#endif

  m_pParent = p_Parent;
  if (p_Parent!=NULL)
    HParent = p_Parent->GetSafeHwnd();
  else
    HParent = NULL;
  Abort = AbortFlag;
  Ok = (CDialog::Create(CPrnAbortDlg::IDD, m_pParent));
  if (Ok)
  {
    ::EnableWindow(HParent,FALSE);
    ::EnableWindow(GetSafeHwnd(),TRUE);
  }

#ifndef NO_I18N
  font = (HFONT)SendMessage(WM_GETFONT, 0, 0);
  GetObject(font, sizeof(LOGFONT), &logfont);
  if (get_lang_font("DLG_SYSTEM_FONT", GetSafeHwnd(), &logfont, &DlgFont, pts->UILanguageFile)) {
	SendDlgItemMessage(IDC_PRNABORT_PRINTING, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
	SendDlgItemMessage(IDCANCEL, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
  }
#endif

  return Ok;
}

void CPrnAbortDlg::OnCancel()
{
  *Abort = TRUE;
  DestroyWindow();
}

BOOL CPrnAbortDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
  *Abort = TRUE;
  DestroyWindow();
	
  return CDialog::OnCommand(wParam, lParam);
}

void CPrnAbortDlg::PostNcDestroy() 
{
  delete this;
}

BOOL CPrnAbortDlg::DestroyWindow() 
{
  HWND HParent;

  HParent = m_pParent->GetSafeHwnd();
  ::EnableWindow(HParent,TRUE);
  ::SetFocus(HParent);
  return CDialog::DestroyWindow();
}
