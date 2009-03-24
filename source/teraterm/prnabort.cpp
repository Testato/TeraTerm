/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TERATERM.EXE, print-abort dialog box */
#include "stdafx.h"
#include "teraterm.h"
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
BOOL CPrnAbortDlg::Create(CWnd* p_Parent, PBOOL AbortFlag)
{
  BOOL Ok;
  HWND HParent;

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
