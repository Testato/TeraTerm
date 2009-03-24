/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TTMACRO.EXE, main */

#include "stdafx.h"
#include "teraterm.h"
#ifdef TERATERM32
  #include "ttm_res.h"
#else
  #include "ttm_re16.h"
  #include "ttctl3d.h"
#endif
#include "ttmmain.h"
#include "ttl.h"

#include "ttmacro.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CCtrlApp, CWinApp)
	//{{AFX_MSG_MAP(CCtrlApp)
	//}}AFX_MSG
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CCtrlApp::CCtrlApp()
{
}

/////////////////////////////////////////////////////////////////////////////

CCtrlApp theApp;

/////////////////////////////////////////////////////////////////////////////

BOOL CCtrlApp::InitInstance()
{
  Busy = TRUE;
#ifndef TERATERM32
  LoadCtl3d(m_hInstance);
#endif
  m_pMainWnd = new CCtrlWindow();
  PCtrlWindow(m_pMainWnd)->Create();
  Busy = FALSE;  
  return TRUE;
}

int CCtrlApp::ExitInstance()
{
#ifndef TERATERM32
  FreeCtl3d();
#endif
  m_pMainWnd = NULL;
  return ExitCode;
}

// TTMACRO main engine
BOOL CCtrlApp::OnIdle(LONG lCount)
{
  BOOL Continue;

  // Avoid multi entry
  if (Busy) return FALSE;
  Busy = TRUE;
  if (m_pMainWnd != NULL)
    Continue = PCtrlWindow(m_pMainWnd)->OnIdle();
  else
    Continue = FALSE;
  Busy = FALSE;
  return Continue;
}
