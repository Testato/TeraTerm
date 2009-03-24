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
#include "ttmlib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef I18N
char UILanguageFile[MAX_PATH];
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
#ifdef I18N
  PMap pm;
  HANDLE HMap = NULL;
#endif

  // インストーラで実行を検出するために mutex を作成する (2006.8.12 maya)
  // 2重起動防止のためではないので、特に返り値は見ない
  HANDLE hMutex;
  hMutex = CreateMutex(NULL, TRUE, "TeraTermProMacroAppMutex");

#ifdef I18N
  HMap = CreateFileMapping(
    (HANDLE) 0xFFFFFFFF, NULL, PAGE_READONLY,
    0, sizeof(TMap), "ttset_memfilemap");
  if (HMap != NULL) {
    pm = (PMap)MapViewOfFile(
    HMap,FILE_MAP_READ,0,0,0);
    if (pm != NULL) {
      strncpy(UILanguageFile, pm->ts.UILanguageFile, sizeof(UILanguageFile)-1);
      UILanguageFile[sizeof(UILanguageFile)-1] = 0;
	}
  }
#endif

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
