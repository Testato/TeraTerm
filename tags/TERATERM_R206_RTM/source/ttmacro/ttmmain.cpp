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
#include "ttmdlg.h"
#include "ttl.h"
#include "ttmparse.h"
#include "ttmdde.h"

#include "ttmmain.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CCtrlWindow dialog
CCtrlWindow::CCtrlWindow()
	: CDialog()
{
  m_hIcon = AfxGetApp()->LoadIcon(IDI_TTMACRO);
}

BOOL CCtrlWindow::Create()
{
  if (! CDialog::Create(CCtrlWindow::IDD, NULL))
  {
    PostQuitMessage(0);
    return FALSE;
  }
  return TRUE;
}

// TTMACRO main engine
BOOL CCtrlWindow::OnIdle()
{
  int ResultCode;
  char Temp[2];

  if (TTLStatus==IdTTLEnd)
  {
    DestroyWindow();
    return FALSE;
  }

  SendSync(); // for sync mode

  if (OutLen>0)
  {
    DDESend();
    return TRUE;
  }
  else if (! Pause &&
    (TTLStatus==IdTTLRun))
  {
    Exec();
    return TRUE;
  }
  else if (TTLStatus==IdTTLWait)
  {
    ResultCode = Wait();
    if (ResultCode>0)
    {
      KillTimer(IdTimeOutTimer);
      TTLStatus = IdTTLRun;
      LockVar();
      SetResult(ResultCode);
      UnlockVar();
      return TRUE;
    }
    else if (ComReady==0)
      SetTimer(IdTimeOutTimer,0, NULL);
  }
  else if (TTLStatus==IdTTLWaitLn)
  {
    ResultCode = Wait();
    if (ResultCode>0)
    {
      LockVar();
      SetResult(ResultCode);
      UnlockVar();
      Temp[0] = 0x0a;
      Temp[1] = 0;
      if (CmpWait(ResultCode,Temp)==0)
      { // new-line is received
	KillTimer(IdTimeOutTimer);
	ClearWait();
	TTLStatus = IdTTLRun;
	LockVar();
	SetInputStr(GetRecvLnBuff());
	UnlockVar();
      }
      else { // wait new-line
	ClearWait();
	SetWait(1,Temp);
	TTLStatus = IdTTLWaitNL;
      }
      return TRUE;
    }
    else if (ComReady==0)
      SetTimer(IdTimeOutTimer,0, NULL);
  }
  else if (TTLStatus==IdTTLWaitNL)
  {
    ResultCode = Wait();
    if (ResultCode>0)
    {
      KillTimer(IdTimeOutTimer);
      TTLStatus = IdTTLRun;
      LockVar();
      SetInputStr(GetRecvLnBuff());
      UnlockVar();
      return TRUE;
    }
    else if (ComReady==0)
      SetTimer(IdTimeOutTimer,0, NULL);
  }
  else if (TTLStatus==IdTTLWait2)
  {
    if (Wait2())
    {
      KillTimer(IdTimeOutTimer);
      TTLStatus = IdTTLRun;
      LockVar();
      SetInputStr(Wait2Str);
      SetResult(1);
      UnlockVar();
      return TRUE;
    }
    else if (ComReady==0)
      SetTimer(IdTimeOutTimer,0, NULL);
  }

  return FALSE;
}

BEGIN_MESSAGE_MAP(CCtrlWindow, CDialog)
	//{{AFX_MSG_MAP(CCtrlWindow)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_TIMER()
	ON_MESSAGE(WM_USER_DDECMNDEND,OnDdeCmndEnd)
	ON_MESSAGE(WM_USER_DDECOMREADY,OnDdeComReady)
	ON_MESSAGE(WM_USER_DDEREADY,OnDdeReady)
	ON_MESSAGE(WM_USER_DDEEND,OnDdeEnd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// CCtrlWindow message handler

BOOL CCtrlWindow::OnInitDialog()
{
  HDC TmpDC;
  int CRTWidth, CRTHeight;
  RECT Rect;
  char Temp[MAXPATHLEN];
  BOOL IOption, VOption;
  int CmdShow;

#ifndef TERATERM32
  SubClassDlg(GetSafeHwnd()); /* CTL3D */
#endif
  CDialog::OnInitDialog();

  Pause = FALSE;

  TmpDC = ::GetDC(GetSafeHwnd());
  CRTWidth = GetDeviceCaps(TmpDC,HORZRES);
  CRTHeight = GetDeviceCaps(TmpDC,VERTRES);
  GetWindowRect(&Rect);
  ::ReleaseDC(GetSafeHwnd(), TmpDC);
  ::SetWindowPos(GetSafeHwnd(),HWND_TOP,
    (CRTWidth-Rect.right+Rect.left) / 2,
    (CRTHeight-Rect.bottom+Rect.top) / 2,
    0,0,SWP_NOSIZE | SWP_NOZORDER);
#ifdef TERATERM32
  // set the small icon
  ::PostMessage(GetSafeHwnd(),WM_SETICON,0,
    (LPARAM)LoadImage(AfxGetInstanceHandle(),
    MAKEINTRESOURCE(IDI_TTMACRO),
    IMAGE_ICON,16,16,0));
#endif
  ParseParam(&IOption,&VOption);

  if (TopicName[0] != 0) InitDDE(GetSafeHwnd());

  if ((FileName[0]==0) &&
      (! GetFileName(GetSafeHwnd())))
  {
    EndDDE();
    PostQuitMessage(0);
    return TRUE;
  }

  if (! InitTTL(GetSafeHwnd()))
  {
    EndDDE();
    PostQuitMessage(0);
  }

  strcpy(Temp,"MACRO - ");
  strcat(Temp,ShortName);
  SetWindowText(Temp);

  // send the initialization signal to TT
  SendCmnd(CmdInit,0);

  if (VOption) return TRUE;
  if (IOption)
    CmdShow = SW_SHOWMINIMIZED;
  else
#ifdef TERATERM32
    CmdShow = SW_SHOWDEFAULT;
#else
    CmdShow = AfxGetApp()->m_nCmdShow;
#endif
  ShowWindow(CmdShow);
  return TRUE;
}

void CCtrlWindow::OnCancel( )
{
  DestroyWindow();
}

BOOL CCtrlWindow::OnCommand(WPARAM wParam, LPARAM lParam)
{
  switch (LOWORD(wParam)) {
    case IDC_CTRLPAUSESTART:
      if (Pause)
	SetDlgItemText(IDC_CTRLPAUSESTART, "Pau&se");
      else
	SetDlgItemText(IDC_CTRLPAUSESTART, "&Start");
      Pause = ! Pause;
      return TRUE;
    case IDC_CTRLEND:
      TTLStatus = IdTTLEnd;
      return TRUE;
    default:
      return (CDialog::OnCommand(wParam,lParam));
  }
}

void CCtrlWindow::OnClose()
{	
  EndTTL();
  EndDDE();
  CDialog::OnClose();
}

void CCtrlWindow::OnDestroy()
{
  EndTTL();
  EndDDE();
  CDialog::OnDestroy();
}	

// for icon drawing in Win NT 3.5
BOOL CCtrlWindow::OnEraseBkgnd(CDC* pDC)
{
  if (IsIconic())
    return TRUE;
  else
    return CDialog::OnEraseBkgnd(pDC);
}

// for icon drawing in Win NT 3.5
void CCtrlWindow::OnPaint()
{
  int OldMapMode;
  CPaintDC dc(this);

  OldMapMode = dc.GetMapMode();
  dc.SetMapMode(MM_TEXT);
  
  if (!IsIconic()) return;
  SendMessage(WM_ICONERASEBKGND,(UINT)(dc.m_hDC));
  dc.DrawIcon(0, 0, m_hIcon);
  dc.SetMapMode(OldMapMode);
}

// for icon drawing in Win NT 3.5
HCURSOR CCtrlWindow::OnQueryDragIcon()
{
  return m_hIcon;
}

void CCtrlWindow::OnSysColorChange()
{
#ifndef TERATERM32
  SysColorChange();
#else
  CDialog::OnSysColorChange();
#endif
}

void CCtrlWindow::OnTimer(UINT nIDEvent)
{
  BOOL TimeOut;

  KillTimer(nIDEvent);
  if (nIDEvent!=IdTimeOutTimer) return;
  if (TTLStatus==IdTTLRun) return;

  TimeOut = CheckTimeout();
  LockVar();

  if ((TTLStatus == IdTTLWait) ||
      (TTLStatus == IdTTLWaitLn) ||
      (TTLStatus == IdTTLWaitNL))
  {
    if ((! Linked) || (ComReady==0)|| TimeOut)
    {
      SetResult(0);
      if (TTLStatus==IdTTLWaitNL)
	SetInputStr(GetRecvLnBuff());
      TTLStatus = IdTTLRun;
    }
  }
  else if (TTLStatus == IdTTLWait2)
  {
    if ((! Linked) || (ComReady==0) || TimeOut)
    {
      if (Wait2Found)
      {
	SetInputStr(Wait2Str);
	SetResult(-1);
      }
      else {
	SetInputStr("");
	SetResult(0);
      }
      TTLStatus = IdTTLRun;
    }
  }
  else if (TTLStatus==IdTTLPause)
  {
    if (TimeOut)
      TTLStatus = IdTTLRun;
  }
  else if (TTLStatus==IdTTLSleep)
  {
    if ((TimeOut) &&
	TestWakeup(IdWakeupTimeout))
    {
      SetResult(IdWakeupTimeout);
      TTLStatus = IdTTLRun;
    }
  }
  else
    TTLStatus = IdTTLRun;

  UnlockVar();

  if (TimeOut || (TTLStatus==IdTTLRun))
    return;

  SetTimer(IdTimeOutTimer,1000, NULL);
}

void CCtrlWindow::PostNcDestroy()
{
  delete this;
  PostQuitMessage(0);
}

BOOL CCtrlWindow::PreTranslateMessage(MSG* pMsg)
{	
  if ((pMsg->message==WM_KEYDOWN) &&
      (pMsg->wParam==VK_ESCAPE)) // ignore ESC key
    return FALSE;
  return CDialog::PreTranslateMessage(pMsg);
}

LONG CCtrlWindow::OnDdeCmndEnd(UINT wParam, LONG lParam)
{
  if (TTLStatus == IdTTLWaitCmndResult)
  {
    LockVar();
    SetResult(wParam);
    UnlockVar();
  }

  if ((TTLStatus == IdTTLWaitCmndEnd) ||
      (TTLStatus == IdTTLWaitCmndResult))
    TTLStatus = IdTTLRun;
  return 0;
}

LONG CCtrlWindow::OnDdeComReady(UINT wParam, LONG lParam)
{
  ComReady = wParam;
  if ((TTLStatus == IdTTLWait) ||
      (TTLStatus == IdTTLWaitLn) ||
      (TTLStatus == IdTTLWaitNL) ||
      (TTLStatus == IdTTLWait2))
  {
    if (ComReady==0)
      SetTimer(IdTimeOutTimer,0, NULL);
  }
  else if (TTLStatus==IdTTLSleep)
  {
    LockVar();
    if (TestWakeup(IdWakeupInit))
    {
      if (ComReady!=0)
	SetResult(2);
      else
	SetResult(1);
      TTLStatus = IdTTLRun;
    }
    else if ((ComReady!=0) && TestWakeup(IdWakeupConnect))
    {
      SetResult(IdWakeupConnect);
      TTLStatus = IdTTLRun;
    }
    else if ((ComReady==0) && TestWakeup(IdWakeupDisconn))
    {
      SetResult(IdWakeupDisconn);
      TTLStatus = IdTTLRun;
    }
    UnlockVar();
  }
  return 0;
}

LONG CCtrlWindow::OnDdeReady(UINT wParam, LONG lParam)
{
  if (TTLStatus != IdTTLInitDDE) return 0;
  SetWakeup(IdWakeupInit);
  TTLStatus = IdTTLSleep;

  if (! InitDDE(GetSafeHwnd()))
  {
    LockVar();
    SetResult(0);
    UnlockVar();
    TTLStatus = IdTTLRun;
  }
  return 0;
}

LONG CCtrlWindow::OnDdeEnd(UINT wParam, LONG lParam)
{
  EndDDE();
  if ((TTLStatus == IdTTLWaitCmndEnd) ||
      (TTLStatus == IdTTLWaitCmndResult))
    TTLStatus = IdTTLRun;
  else if ((TTLStatus == IdTTLWait) ||
	   (TTLStatus == IdTTLWaitLn) ||
	   (TTLStatus == IdTTLWaitNL) ||
	   (TTLStatus == IdTTLWait2))
    SetTimer(IdTimeOutTimer,0, NULL);
  else if (TTLStatus==IdTTLSleep)
  {
    LockVar();
    if (TestWakeup(IdWakeupInit))
    {
      SetResult(0);
      TTLStatus = IdTTLRun;
    }
    else if (TestWakeup(IdWakeupUnlink))
    {
      SetResult(IdWakeupUnlink);
      TTLStatus = IdTTLRun;
    }
    UnlockVar();
  }
  return 0;
}
