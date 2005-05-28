/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TERATERM.EXE, file-transfer-protocol dialog box */
#include "stdafx.h"
#include "teraterm.h"
#ifdef TERATERM32
#include "tt_res.h"
#else
#include "tt_res16.h"
#endif
#include "tttypes.h"
#include "ttftypes.h"
#include "protodlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProtoDlg dialog

BEGIN_MESSAGE_MAP(CProtoDlg, CDialog)
	//{{AFX_MSG_MAP(CProtoDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CProtoDlg::Create(PFileVar pfv)
{
  BOOL Ok;

  fv = pfv;

  Ok = CDialog::Create(CProtoDlg::IDD, NULL);
  fv->HWin = GetSafeHwnd();
  return Ok;
}

/////////////////////////////////////////////////////////////////////////////
// CProtoDlg message handler

void CProtoDlg::OnCancel( )
{
  ::PostMessage(fv->HMainWin,WM_USER_PROTOCANCEL,0,0);
}

BOOL CProtoDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
  switch (LOWORD(wParam)) {
    case IDCANCEL:
      ::PostMessage(fv->HMainWin,WM_USER_PROTOCANCEL,0,0);
      return TRUE;
    default:
      return (CDialog::OnCommand(wParam,lParam));
  }
}

void CProtoDlg::PostNcDestroy()
{
  delete this;
}
