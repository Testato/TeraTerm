/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */
/* IPv6 modification is Copyright(C) 2000 Jun-ya Kato <kato@win6.jp> */

/* TERATERM.EXE, VT window */

#include "stdafx.h"
#include "teraterm.h"
#include "tttypes.h"

#include "ttcommon.h"
#include "ttwinman.h"
#include "ttsetup.h"
#include "keyboard.h"
#include "buffer.h"
#include "vtterm.h"
#include "vtdisp.h"
#include "ttdialog.h"
#include "ttime.h"
#include "commlib.h"
#include "clipboar.h"
#include "ttftypes.h"
#include "filesys.h"
#include "telnet.h"
#include "tektypes.h"
#include "tekwin.h"
#include "ttdde.h"
#include "ttlib.h"
#include "helpid.h"
#include "teraprn.h"
#ifdef INET6
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <winsock.h>
#endif /* INET6 */
#include "ttplug.h"  /* TTPLUG */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include <shlobj.h>
#include <io.h>
#include <errno.h>

#ifdef TERATERM32
#include "tt_res.h"
#else
#include "ttctl3d.h"
#include "tt_res16.h"
#endif
#include "vtwin.h"

#ifdef TERATERM32
#define VTClassName "VTWin32"
#else
#define VTClassName "VTWin"
#endif

/* mouse buttons */
#define IdLeftButton 0
#define IdMiddleButton 1
#define IdRightButton 2

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ウィンドウ最大化ボタンを有効にする (2005.1.15 yutaka)
#define WINDOW_MAXMIMUM_ENABLED 1

// WM_COPYDATAによるプロセス間通信の種別 (2005.1.22 yutaka)
#define IPC_BROADCAST_COMMAND 1

/* HTML help file */
#define HTML_HELP_EN "teraterm.chm"
#define HTML_HELP_JP "teratermj.chm"

#ifdef I18N
static HFONT DlgBroadcastFont;
static HFONT DlgCommentFont;
static HFONT DlgAdditionalFont;
static HFONT DlgGeneralFont;
static HFONT DlgVisualFont;
static HFONT DlgLogFont;
static HFONT DlgCygwinFont;
#endif

typedef struct {
	char *name;
	LPCTSTR id;
} mouse_cursor_t;

#define MOUSE_CURSOR_MAX (sizeof(MouseCursor)/sizeof(MouseCursor[0]) - 1)

mouse_cursor_t MouseCursor[] = {
	{"ARROW", IDC_ARROW},
	{"IBEAM", IDC_IBEAM},
	{"CROSS", IDC_CROSS},
	{"HAND", IDC_HAND},
	{NULL, NULL},
};

/////////////////////////////////////////////////////////////////////////////
// CVTWindow

BEGIN_MESSAGE_MAP(CVTWindow, CFrameWnd)
	//{{AFX_MSG_MAP(CVTWindow)
	ON_WM_ACTIVATE()
	ON_WM_CHAR()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_DROPFILES()
	ON_WM_GETMINMAXINFO()
	ON_WM_HSCROLL()
	ON_WM_INITMENUPOPUP()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_KILLFOCUS()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_MOUSEACTIVATE()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOVE()
	ON_WM_NCLBUTTONDBLCLK()
	ON_WM_NCRBUTTONDOWN()
	ON_WM_PAINT()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_SETFOCUS()
	ON_WM_SIZE()
	ON_WM_SYSCHAR()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_SYSCOMMAND()
	ON_WM_SYSKEYDOWN()
	ON_WM_SYSKEYUP()
	ON_WM_TIMER()
	ON_WM_VSCROLL()
	ON_MESSAGE(WM_IME_COMPOSITION,OnIMEComposition)
//<!--by AKASI
	ON_MESSAGE(WM_WINDOWPOSCHANGING,OnWindowPosChanging)
	ON_MESSAGE(WM_SETTINGCHANGE,OnSettingChange)
	ON_MESSAGE(WM_ENTERSIZEMOVE,OnEnterSizeMove)
	ON_MESSAGE(WM_EXITSIZEMOVE ,OnExitSizeMove)
//-->
	ON_MESSAGE(WM_USER_ACCELCOMMAND, OnAccelCommand)
	ON_MESSAGE(WM_USER_CHANGEMENU,OnChangeMenu)
	ON_MESSAGE(WM_USER_CHANGETBAR,OnChangeTBar)
	ON_MESSAGE(WM_USER_COMMNOTIFY,OnCommNotify)
	ON_MESSAGE(WM_USER_COMMOPEN,OnCommOpen)
	ON_MESSAGE(WM_USER_COMMSTART,OnCommStart)
	ON_MESSAGE(WM_USER_DDEEND,OnDdeEnd)
	ON_MESSAGE(WM_USER_DLGHELP2,OnDlgHelp)
	ON_MESSAGE(WM_USER_FTCANCEL,OnFileTransEnd)
	ON_MESSAGE(WM_USER_GETSERIALNO,OnGetSerialNo)
	ON_MESSAGE(WM_USER_KEYCODE,OnKeyCode)
	ON_MESSAGE(WM_USER_PROTOCANCEL,OnProtoEnd)
	ON_MESSAGE(WM_COPYDATA,OnReceiveIpcMessage)
	ON_COMMAND(ID_FILE_NEWCONNECTION, OnFileNewConnection)
	ON_COMMAND(ID_FILE_DUPLICATESESSION, OnDuplicateSession)
	ON_COMMAND(ID_FILE_CYGWINCONNECTION, OnCygwinConnection)
	ON_COMMAND(ID_FILE_TERATERMMENU, OnTTMenuLaunch)
	ON_COMMAND(ID_FILE_LOGMEIN, OnLogMeInLaunch)
	ON_COMMAND(ID_FILE_LOG, OnFileLog)
	ON_COMMAND(ID_FILE_COMMENTTOLOG, OnCommentToLog)
	ON_COMMAND(ID_FILE_VIEWLOG, OnViewLog)
	ON_COMMAND(ID_FILE_REPLAYLOG, OnReplayLog)
	ON_COMMAND(ID_FILE_SENDFILE, OnFileSend)
	ON_COMMAND(ID_FILE_KERMITRCV, OnFileKermitRcv)
	ON_COMMAND(ID_FILE_KERMITGET, OnFileKermitGet)
	ON_COMMAND(ID_FILE_KERMITSEND, OnFileKermitSend)
	ON_COMMAND(ID_FILE_KERMITFINISH, OnFileKermitFinish)
	ON_COMMAND(ID_FILE_XRCV, OnFileXRcv)
	ON_COMMAND(ID_FILE_XSEND, OnFileXSend)
	ON_COMMAND(ID_FILE_ZRCV, OnFileZRcv)
	ON_COMMAND(ID_FILE_ZSEND, OnFileZSend)
	ON_COMMAND(ID_FILE_BPRCV, OnFileBPRcv)
	ON_COMMAND(ID_FILE_BPSEND, OnFileBPSend)
	ON_COMMAND(ID_FILE_QVRCV, OnFileQVRcv)
	ON_COMMAND(ID_FILE_QVSEND, OnFileQVSend)
	ON_COMMAND(ID_FILE_CHANGEDIR, OnFileChangeDir)
	ON_COMMAND(ID_FILE_PRINT2, OnFilePrint)
	ON_COMMAND(ID_FILE_DISCONNECT, OnFileDisconnect)
	ON_COMMAND(ID_FILE_EXIT, OnFileExit)
	ON_COMMAND(ID_EDIT_COPY2, OnEditCopy)
	ON_COMMAND(ID_EDIT_COPYTABLE, OnEditCopyTable)
	ON_COMMAND(ID_EDIT_PASTE2, OnEditPaste)
	ON_COMMAND(ID_EDIT_PASTECR, OnEditPasteCR)
	ON_COMMAND(ID_EDIT_CLEARSCREEN, OnEditClearScreen)
	ON_COMMAND(ID_EDIT_CLEARBUFFER, OnEditClearBuffer)
	ON_COMMAND(ID_EDIT_SELECTALL, OnSelectAllBuffer)
	ON_COMMAND(ID_SETUP_ADDITIONALSETTINGS, OnExternalSetup)
	ON_COMMAND(ID_SETUP_TERMINAL, OnSetupTerminal)
	ON_COMMAND(ID_SETUP_WINDOW, OnSetupWindow)
	ON_COMMAND(ID_SETUP_FONT, OnSetupFont)
	ON_COMMAND(ID_SETUP_KEYBOARD, OnSetupKeyboard)
	ON_COMMAND(ID_SETUP_SERIALPORT, OnSetupSerialPort)
	ON_COMMAND(ID_SETUP_TCPIP, OnSetupTCPIP)
	ON_COMMAND(ID_SETUP_GENERAL, OnSetupGeneral)
	ON_COMMAND(ID_SETUP_SAVE, OnSetupSave)
	ON_COMMAND(ID_SETUP_RESTORE, OnSetupRestore)
	ON_COMMAND(ID_SETUP_LOADKEYMAP, OnSetupLoadKeyMap)
	ON_COMMAND(ID_CONTROL_RESETTERMINAL, OnControlResetTerminal)
	ON_COMMAND(ID_CONTROL_AREYOUTHERE, OnControlAreYouThere)
	ON_COMMAND(ID_CONTROL_SENDBREAK, OnControlSendBreak)
	ON_COMMAND(ID_CONTROL_RESETPORT, OnControlResetPort)
	ON_COMMAND(ID_CONTROL_BROADCASTCOMMAND, OnControlBroadcastCommand)
	ON_COMMAND(ID_CONTROL_OPENTEK, OnControlOpenTEK)
	ON_COMMAND(ID_CONTROL_CLOSETEK, OnControlCloseTEK)
	ON_COMMAND(ID_CONTROL_MACRO, OnControlMacro)
	ON_COMMAND(ID_WINDOW_WINDOW, OnWindowWindow)
	ON_COMMAND(ID_HELP_INDEX2, OnHelpIndex)
	ON_COMMAND(ID_HELP_USING2, OnHelpUsing)
	ON_COMMAND(ID_HELP_ABOUT, OnHelpAbout)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVTWindow constructor


static BOOL MySetLayeredWindowAttributes(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags)
{
	typedef BOOL (WINAPI *func)(HWND,COLORREF,BYTE,DWORD);
	static HMODULE g_hmodUser32 = NULL;
	static func g_pSetLayeredWindowAttributes = NULL;

	if (g_hmodUser32 == NULL) {
	    g_hmodUser32 = LoadLibrary("user32.dll");
		if (g_hmodUser32 == NULL)
			return FALSE;

		g_pSetLayeredWindowAttributes = (func)GetProcAddress(g_hmodUser32, "SetLayeredWindowAttributes");
	}
    
    if (g_pSetLayeredWindowAttributes == NULL)
		return FALSE;

    return g_pSetLayeredWindowAttributes(hwnd, crKey, 
            bAlpha, dwFlags);
}


// TeraTerm起動時とURL文字列mouse over時に呼ばれる (2005.4.2 yutaka)
extern "C" void SetMouseCursor(char *cursor)
{
	HCURSOR hc;
	LPCTSTR name = NULL;
	int i;

	for (i = 0 ; MouseCursor[i].name ; i++) {
		if (_stricmp(cursor, MouseCursor[i].name) == 0) {
			name = MouseCursor[i].id;
			break;
		}
	}
	if (name == NULL)
		return;


	hc = (HCURSOR)LoadImage(NULL,
			MAKEINTRESOURCE(name),
			IMAGE_CURSOR,
			0,
			0,
			LR_DEFAULTSIZE | LR_SHARED);

	if (hc != NULL) {
		SetClassLongPtr(HVTWin, GCLP_HCURSOR, (LONG_PTR)hc);
	}
}


static void SetWindowStyle(TTTSet *ts)
{
	LONG_PTR lp;

	SetMouseCursor(ts->MouseCursorName);

	// 2006/03/16 by 337: BGUseAlphaBlendAPIがOnならばLayered属性とする
	//if (ts->EtermLookfeel.BGUseAlphaBlendAPI) {
	// アルファ値が255の場合、画面のちらつきを抑えるため何もしないこととする。(2006.4.1 yutaka)
	if (ts->AlphaBlend < 255) {
		lp = GetWindowLongPtr(HVTWin, GWL_EXSTYLE);
		if (lp != 0) {
			SetWindowLongPtr(HVTWin, GWL_EXSTYLE, lp | WS_EX_LAYERED);
			MySetLayeredWindowAttributes(HVTWin, 0, ts->AlphaBlend, LWA_ALPHA);
		}
	}
}

CVTWindow::CVTWindow()
{
  WNDCLASS wc;
  RECT rect;
  DWORD Style;
#ifdef ALPHABLEND_TYPE2
  DWORD ExStyle;
#endif
  char Temp[MAXPATHLEN];
  int CmdShow;
  PKeyMap tempkm;
#ifndef TERATERM32
  int i;
#endif

#if 0
  #include <crtdbg.h>
  #define _CRTDBG_MAP_ALLOC
  _CrtSetBreakAlloc(52);
#endif

  TTXInit(&ts, &cv); /* TTPLUG */

  CommInit(&cv);

  MsgDlgHelp = RegisterWindowMessage(HELPMSGSTRING);

  if (StartTeraTerm(&ts))
  { /* first instance */
	  if (LoadTTSET())
	  {
		  /* read setup info from "teraterm.ini" */
		  (*ReadIniFile)(ts.SetupFName, &ts);
		  /* read keycode map from "keyboard.cnf" */
		  tempkm = (PKeyMap)malloc(sizeof(TKeyMap));
		  if (tempkm!=NULL) {
			  strcpy(Temp, ts.HomeDir);
			  AppendSlash(Temp);
			  strcat(Temp,"KEYBOARD.CNF");
			  (*ReadKeyboardCnf)(Temp,tempkm,TRUE);
		  }
		  FreeTTSET();
		  /* store default sets in TTCMN */
		  ChangeDefaultSet(&ts,tempkm);
		  if (tempkm!=NULL) free(tempkm);
	  }

  } else {
	  // 2つめ以降のプロセスにおいても、ディスクから TERATERM.INI を読む。(2004.11.4 yutaka)
	  if (LoadTTSET())
	  {
		  /* read setup info from "teraterm.ini" */
		  (*ReadIniFile)(ts.SetupFName, &ts);
		  /* read keycode map from "keyboard.cnf" */
		  tempkm = (PKeyMap)malloc(sizeof(TKeyMap));
		  if (tempkm!=NULL) {
			  strcpy(Temp, ts.HomeDir);
			  AppendSlash(Temp);
			  strcat(Temp,"KEYBOARD.CNF");
			  (*ReadKeyboardCnf)(Temp,tempkm,TRUE);
		  }
		  FreeTTSET();
		  /* store default sets in TTCMN */
		  if (tempkm!=NULL) free(tempkm);
	  }

  }

  /* Parse command line parameters*/
#ifdef TERATERM32
  strcpy(Temp,GetCommandLine());
#else
  strcpy(Temp,"teraterm ");
  i = (int)*(LPBYTE)MAKELP(GetCurrentPDB(),0x80);
  memcpy(&Temp[9],MAKELP(GetCurrentPDB(),0x81),i);
  Temp[9+i] = 0;
#endif
  if (LoadTTSET())
    (*ParseParam)(Temp, &ts, &(TopicName[0]));
  FreeTTSET();

  // duplicate sessionの指定があるなら、共有メモリからコピーする (2004.12.7 yutaka)
  if (ts.DuplicateSession == 1) {
	  CopyShmemToTTSet(&ts);
  }

  InitKeyboard();
  SetKeyMap();

  /* window status */
  AdjustSize = TRUE;
  Minimized = FALSE;
  LButton = FALSE;
  MButton = FALSE;
  RButton = FALSE;
  DblClk = FALSE;
  AfterDblClk = FALSE;
  TplClk = FALSE;
  Hold = FALSE;
  FirstPaint = TRUE;
  ScrollLock = FALSE;  // 初期値は無効 (2006.11.14 yutaka)

  /* Initialize scroll buffer */
  InitBuffer();

  InitDisp();

  if (ts.HideTitle>0)
  {
    Style = WS_VSCROLL | WS_HSCROLL |
			WS_BORDER | WS_THICKFRAME | WS_POPUP;

#ifdef ALPHABLEND_TYPE2
     if(BGNoFrame)
       Style &= ~(WS_BORDER | WS_THICKFRAME);
#endif
  }
  else
#ifdef WINDOW_MAXMIMUM_ENABLED
    Style = WS_VSCROLL | WS_HSCROLL |
	    WS_BORDER | WS_THICKFRAME |
	    WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
#else
    Style = WS_VSCROLL | WS_HSCROLL |
	    WS_BORDER | WS_THICKFRAME |
	    WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
#endif

  wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = AfxWndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = AfxGetInstanceHandle();
  wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_VT));
  //wc.hCursor = LoadCursor(NULL,IDC_IBEAM);
  wc.hCursor = NULL; // マウスカーソルは動的に変更する (2005.4.2 yutaka)
  wc.hbrBackground = NULL;
  wc.lpszMenuName = NULL;
  wc.lpszClassName = VTClassName;

  RegisterClass(&wc);
  LoadAccelTable(MAKEINTRESOURCE(IDR_ACC));

  if (ts.VTPos.x==CW_USEDEFAULT)
    rect = rectDefault;
  else {
    rect.left = ts.VTPos.x;
    rect.top = ts.VTPos.y;
    rect.right = rect.left + 100;
    rect.bottom = rect.top + 100;
  }
  Create(VTClassName, "Tera Term", Style, rect, NULL, NULL);

  /*--------- Init2 -----------------*/
  HVTWin = GetSafeHwnd();
  if (HVTWin == NULL) return;
  // register this window to the window list
  SerialNo = RegWin(HVTWin,NULL);

  logfile_lock_initialize();
  SetWindowStyle(&ts);
  // ロケールの設定
  // wctomb のため
  setlocale(LC_ALL, ts.Locale);

#ifdef ALPHABLEND_TYPE2
//<!--by AKASI
  if(BGNoFrame && ts.HideTitle > 0)
  {
    ExStyle  = GetWindowLong(HVTWin,GWL_EXSTYLE);
    ExStyle &= ~WS_EX_CLIENTEDGE;
    SetWindowLong(HVTWin,GWL_EXSTYLE,ExStyle);
  }
//-->
#endif

#ifdef TERATERM32
  // set the small icon
  ::PostMessage(HVTWin,WM_SETICON,0,
    (LPARAM)LoadImage(AfxGetInstanceHandle(),
      MAKEINTRESOURCE(IDI_VT),
      IMAGE_ICON,16,16,0));
#endif
  MainMenu = NULL;
  WinMenu = NULL;
  if ((ts.HideTitle==0) && (ts.PopupMenu==0))
  {
    InitMenu(&MainMenu);
    ::SetMenu(HVTWin,MainMenu);
  }

  /* Reset Terminal */
  ResetTerminal();

  if ((ts.PopupMenu>0) || (ts.HideTitle>0))
    ::PostMessage(HVTWin,WM_USER_CHANGEMENU,0,0);

  ChangeFont();

  ResetIME();

  BuffChangeWinSize(NumOfColumns,NumOfLines);

  ChangeTitle();
  /* Enable drag-drop */
  ::DragAcceptFiles(HVTWin,TRUE);

  if (ts.HideWindow>0)
  {
    if (strlen(TopicName)>0)
    {
      InitDDE();
      SendDDEReady();
    }
    FirstPaint = FALSE;
    Startup();
    return;
  }
#ifdef TERATERM32
  CmdShow = SW_SHOWDEFAULT;
#else
  CmdShow = AfxGetApp()->m_nCmdShow;
#endif
  if (ts.Minimize>0)
    CmdShow = SW_SHOWMINIMIZED;
  ShowWindow(CmdShow);
  ChangeCaret();
}

/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void CVTWindow::AssertValid() const
{
  CFrameWnd::AssertValid();
}

void CVTWindow::Dump(CDumpContext& dc) const
{
  CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////

int CVTWindow::Parse()
{
	// added ScrollLock (2006.11.14 yutaka)
  if (LButton || MButton || RButton || ScrollLock)
    return 0;
  return (VTParse()); // Parse received characters
}

void CVTWindow::ButtonUp(BOOL Paste)
{
	/* disable autoscrolling */
	::KillTimer(HVTWin,IdScrollTimer);
	ReleaseCapture();

	LButton = FALSE;
	MButton = FALSE;
	RButton = FALSE;
	DblClk = FALSE;
	TplClk = FALSE;
	CaretOn();

	BuffEndSelect();
	// added ConfirmPasteMouseRButton (2007.3.17 maya)
	if (Paste && !ts.ConfirmPasteMouseRButton)
		CBStartPaste(HVTWin,FALSE,0,NULL,0);
}

void CVTWindow::ButtonDown(POINT p, int LMR)
{
	HMENU PopupMenu, PopupBase;

	if ((LMR==IdLeftButton) && ControlKey() && (MainMenu==NULL)
		&& ((ts.MenuFlag & MF_NOPOPUP)==0))
	{
		/* TTPLUG BEGIN*/
		int i, numItems;
		char itemText[256];

		InitMenu(&PopupMenu);

		PopupBase = CreatePopupMenu();
		numItems = GetMenuItemCount(PopupMenu);

		for (i = 0; i < numItems; i++) {
			HMENU submenu = GetSubMenu(PopupMenu, i);

			if (submenu != NULL) {
				InitMenuPopup(submenu);
			}

			if (GetMenuString(PopupMenu, i, itemText, sizeof(itemText), MF_BYPOSITION) != 0) {
				int state = GetMenuState(PopupMenu, i, MF_BYPOSITION) &
					(MF_CHECKED | MF_DISABLED | MF_GRAYED | MF_HILITE | MF_MENUBARBREAK | MF_MENUBREAK | MF_SEPARATOR);

				AppendMenu(PopupBase,
					submenu != NULL ? LOBYTE(state) | MF_POPUP : state,
					submenu != NULL ? (UINT)submenu : GetMenuItemID(PopupMenu, i),
					itemText);
			}
		} /* TTPLUG END */

		//    InitMenu(&PopupMenu);
		//    InitMenuPopup(FileMenu);
		//    InitMenuPopup(EditMenu);
		//    InitMenuPopup(SetupMenu);
		//    InitMenuPopup(ControlMenu);
		//    if (WinMenu!=NULL)
		//      InitMenuPopup(WinMenu);
		//    PopupBase = CreatePopupMenu();
		//    AppendMenu(PopupBase, MF_STRING | MF_ENABLED | MF_POPUP,
		//	       (UINT)FileMenu, "&File");
		//    AppendMenu(PopupBase, MF_STRING | MF_ENABLED | MF_POPUP,
		//	       (UINT)EditMenu, "&Edit");
		//    AppendMenu(PopupBase, MF_STRING | MF_ENABLED | MF_POPUP,
		//	       (UINT)SetupMenu, "&Setup");
		//    AppendMenu(PopupBase, MF_STRING | MF_ENABLED | MF_POPUP,
		//	       (UINT)ControlMenu, "C&ontrol");
		//    if (WinMenu!=NULL)
		//      AppendMenu(PopupBase, MF_STRING | MF_ENABLED | MF_POPUP,
		//		 (UINT)WinMenu, "&Window");
		//    AppendMenu(PopupBase, MF_STRING | MF_ENABLED | MF_POPUP,
		//	       (UINT)HelpMenu, "&Help");
		::ClientToScreen(HVTWin, &p);
		TrackPopupMenu(PopupBase,TPM_LEFTALIGN | TPM_LEFTBUTTON,
			p.x,p.y,0,HVTWin,NULL);
		if (WinMenu!=NULL)
		{
			DestroyMenu(WinMenu);
			WinMenu = NULL;
		}
		DestroyMenu(PopupBase);
		DestroyMenu(PopupMenu);
		PopupMenu = 0;
		return;
	}

	// added ConfirmPasteMouseRButton (2007.3.17 maya)
	if ((LMR == IdRightButton) &&
		!ts.DisablePasteMouseRButton &&
		ts.ConfirmPasteMouseRButton &&
		cv.Ready &&
		(SendVar==NULL) && (FileVar==NULL) &&
		(cv.PortType!=IdFile) &&
		(IsClipboardFormatAvailable(CF_TEXT) ||
		 IsClipboardFormatAvailable(CF_OEMTEXT)))
	{
		int i, numItems;
		char itemText[256];

		InitPasteMenu(&PopupMenu);
		PopupBase = CreatePopupMenu();
		numItems = GetMenuItemCount(PopupMenu);

		for (i = 0; i < numItems; i++) {
			if (GetMenuString(PopupMenu, i, itemText, sizeof(itemText), MF_BYPOSITION) != 0) {
				int state = GetMenuState(PopupMenu, i, MF_BYPOSITION) &
					(MF_CHECKED | MF_DISABLED | MF_GRAYED | MF_HILITE | MF_MENUBARBREAK | MF_MENUBREAK | MF_SEPARATOR);

				AppendMenu(PopupBase,
					state,
					GetMenuItemID(PopupMenu, i),
					itemText);
			}
		}

		::ClientToScreen(HVTWin, &p);
		TrackPopupMenu(PopupBase,TPM_LEFTALIGN | TPM_LEFTBUTTON,
			p.x,p.y,0,HVTWin,NULL);
		if (WinMenu!=NULL)
		{
			DestroyMenu(WinMenu);
			WinMenu = NULL;
		}
		DestroyMenu(PopupBase);
		DestroyMenu(PopupMenu);
		PopupMenu = 0;
		return;
	}

	if (AfterDblClk && (LMR==IdLeftButton) &&
		(abs(p.x-DblClkX)<=GetSystemMetrics(SM_CXDOUBLECLK)) &&
		(abs(p.y-DblClkY)<=GetSystemMetrics(SM_CYDOUBLECLK)))
	{  /* triple click */
		::KillTimer(HVTWin, IdDblClkTimer);
		AfterDblClk = FALSE;
		BuffTplClk(p.y);
		LButton = TRUE;
		TplClk = TRUE;
		/* for AutoScrolling */
		::SetCapture(HVTWin);
		::SetTimer(HVTWin, IdScrollTimer, 100, NULL);
	}
	else {
		if (! (LButton || MButton || RButton))
		{
			BOOL box = FALSE;

			// select several pages of output from TeraTerm window (2005.5.15 yutaka)
			if (LMR == IdLeftButton && ShiftKey()) {
				BuffSeveralPagesSelect(p.x, p.y);

			} else {
				// Select rectangular block with Alt Key. Delete Shift key.(2005.5.15 yutaka)
				if (LMR == IdLeftButton && AltKey()) {
					box = TRUE;
				}

				BuffStartSelect(p.x,p.y, box);
				TplClk = FALSE;
				/* for AutoScrolling */
				::SetCapture(HVTWin);
				::SetTimer(HVTWin, IdScrollTimer, 100, NULL);
			}
		}

		switch (LMR) {
		case IdRightButton: RButton = TRUE; break;
		case IdMiddleButton: MButton = TRUE; break;
		case IdLeftButton: LButton = TRUE; break;
		}
	}
}

void CVTWindow::InitMenu(HMENU *Menu)
{
  *Menu = LoadMenu(AfxGetInstanceHandle(),
    MAKEINTRESOURCE(IDR_MENU));
  FileMenu = GetSubMenu(*Menu,ID_FILE);
  TransMenu = GetSubMenu(FileMenu,ID_TRANSFER);
  EditMenu = GetSubMenu(*Menu,ID_EDIT);
  SetupMenu = GetSubMenu(*Menu,ID_SETUP);
  ControlMenu = GetSubMenu(*Menu,ID_CONTROL);
  HelpMenu = GetSubMenu(*Menu,ID_HELPMENU);
#ifdef I18N
  GetMenuString(*Menu, ID_FILE, ts.UIMsg, sizeof(ts.UIMsg), MF_BYPOSITION);
  get_lang_msg("MENU_FILE", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(*Menu, ID_FILE, MF_BYPOSITION, ID_FILE, ts.UIMsg);
  GetMenuString(FileMenu, ID_FILE_NEWCONNECTION, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_FILE_NEW", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(FileMenu, ID_FILE_NEWCONNECTION, MF_BYCOMMAND, ID_FILE_NEWCONNECTION, ts.UIMsg);
  GetMenuString(FileMenu, ID_FILE_DUPLICATESESSION, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_FILE_DUPLICATE", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(FileMenu, ID_FILE_DUPLICATESESSION, MF_BYCOMMAND, ID_FILE_DUPLICATESESSION, ts.UIMsg);
  GetMenuString(FileMenu, ID_FILE_CYGWINCONNECTION, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_FILE_GYGWIN", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(FileMenu, ID_FILE_CYGWINCONNECTION, MF_BYCOMMAND, ID_FILE_CYGWINCONNECTION, ts.UIMsg);
  GetMenuString(FileMenu, ID_FILE_LOG, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_FILE_LOG", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(FileMenu, ID_FILE_LOG, MF_BYCOMMAND, ID_FILE_LOG, ts.UIMsg);
  GetMenuString(FileMenu, ID_FILE_COMMENTTOLOG, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_FILE_COMMENTLOG", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(FileMenu, ID_FILE_COMMENTTOLOG, MF_BYCOMMAND, ID_FILE_COMMENTTOLOG, ts.UIMsg);
  GetMenuString(FileMenu, ID_FILE_VIEWLOG, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_FILE_VIEWLOG", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(FileMenu, ID_FILE_VIEWLOG, MF_BYCOMMAND, ID_FILE_VIEWLOG, ts.UIMsg);
  GetMenuString(FileMenu, ID_FILE_SENDFILE, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_FILE_SENDFILE", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(FileMenu, ID_FILE_SENDFILE, MF_BYCOMMAND, ID_FILE_SENDFILE, ts.UIMsg);
  GetMenuString(FileMenu, ID_FILE_REPLAYLOG, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_FILE_REPLAYLOG", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(FileMenu, ID_FILE_REPLAYLOG, MF_BYCOMMAND, ID_FILE_REPLAYLOG, ts.UIMsg);
  GetMenuString(FileMenu, ID_FILE_CHANGEDIR, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_FILE_CHANGEDIR", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(FileMenu, ID_FILE_CHANGEDIR, MF_BYCOMMAND, ID_FILE_CHANGEDIR, ts.UIMsg);
  GetMenuString(FileMenu, ID_FILE_LOGMEIN, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_FILE_LOGMETT", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(FileMenu, ID_FILE_LOGMEIN, MF_BYCOMMAND, ID_FILE_LOGMEIN, ts.UIMsg);
  GetMenuString(FileMenu, ID_FILE_PRINT2, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_FILE_PRINT", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(FileMenu, ID_FILE_PRINT2, MF_BYCOMMAND, ID_FILE_PRINT2, ts.UIMsg);
  GetMenuString(FileMenu, ID_FILE_DISCONNECT, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_FILE_DISCONNECT", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(FileMenu, ID_FILE_DISCONNECT, MF_BYCOMMAND, ID_FILE_DISCONNECT, ts.UIMsg);
  GetMenuString(FileMenu, ID_FILE_EXIT, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_FILE_EXIT", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(FileMenu, ID_FILE_EXIT, MF_BYCOMMAND, ID_FILE_EXIT, ts.UIMsg);

  GetMenuString(FileMenu, 8, ts.UIMsg, sizeof(ts.UIMsg), MF_BYPOSITION);
  get_lang_msg("MENU_TRANS", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(FileMenu, 8, MF_BYPOSITION, 8, ts.UIMsg);

  GetMenuString(FileMenu, ID_FILE_KERMITRCV, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_TRANS_KERMIT_RCV", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(FileMenu, ID_FILE_KERMITRCV, MF_BYCOMMAND, ID_FILE_KERMITRCV, ts.UIMsg);
  GetMenuString(FileMenu, ID_FILE_KERMITGET, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_TRANS_KERMIT_GET", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(FileMenu, ID_FILE_KERMITGET, MF_BYCOMMAND, ID_FILE_KERMITGET, ts.UIMsg);
  GetMenuString(FileMenu, ID_FILE_KERMITSEND, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_TRANS_KERMIT_SEND", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(FileMenu, ID_FILE_KERMITSEND, MF_BYCOMMAND, ID_FILE_KERMITSEND, ts.UIMsg);
  GetMenuString(FileMenu, ID_FILE_KERMITFINISH, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_TRANS_KERMIT_FINISH", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(FileMenu, ID_FILE_KERMITFINISH, MF_BYCOMMAND, ID_FILE_KERMITFINISH, ts.UIMsg);

  GetMenuString(FileMenu, ID_FILE_XRCV, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_TRANS_X_RCV", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(FileMenu, ID_FILE_XRCV, MF_BYCOMMAND, ID_FILE_XRCV, ts.UIMsg);
  GetMenuString(FileMenu, ID_FILE_XSEND, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_TRANS_X_SEND", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(FileMenu, ID_FILE_XSEND, MF_BYCOMMAND, ID_FILE_XSEND, ts.UIMsg);

  GetMenuString(FileMenu, ID_FILE_ZRCV, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_TRANS_Z_RCV", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(FileMenu, ID_FILE_ZRCV, MF_BYCOMMAND, ID_FILE_ZRCV, ts.UIMsg);
  GetMenuString(FileMenu, ID_FILE_ZSEND, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_TRANS_Z_SEND", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(FileMenu, ID_FILE_ZSEND, MF_BYCOMMAND, ID_FILE_ZSEND, ts.UIMsg);

  GetMenuString(FileMenu, ID_FILE_BPRCV, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_TRANS_BP_RCV", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(FileMenu, ID_FILE_BPRCV, MF_BYCOMMAND, ID_FILE_BPRCV, ts.UIMsg);
  GetMenuString(FileMenu, ID_FILE_BPSEND, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_TRANS_BP_SEND", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(FileMenu, ID_FILE_BPSEND, MF_BYCOMMAND, ID_FILE_BPSEND, ts.UIMsg);

  GetMenuString(FileMenu, ID_FILE_QVRCV, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_TRANS_QV_RCV", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(FileMenu, ID_FILE_QVRCV, MF_BYCOMMAND, ID_FILE_QVRCV, ts.UIMsg);
  GetMenuString(FileMenu, ID_FILE_QVSEND, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_TRANS_QV_SEND", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(FileMenu, ID_FILE_QVSEND, MF_BYCOMMAND, ID_FILE_QVSEND, ts.UIMsg);

  GetMenuString(*Menu, ID_EDIT, ts.UIMsg, sizeof(ts.UIMsg), MF_BYPOSITION);
  get_lang_msg("MENU_EDIT", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(*Menu, ID_EDIT, MF_BYPOSITION, ID_EDIT, ts.UIMsg);
  GetMenuString(EditMenu, ID_EDIT_COPY2, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_EDIT_COPY", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(EditMenu, ID_EDIT_COPY2, MF_BYCOMMAND, ID_EDIT_COPY2, ts.UIMsg);
  GetMenuString(EditMenu, ID_EDIT_COPYTABLE, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_EDIT_COPYTABLE", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(EditMenu, ID_EDIT_COPYTABLE, MF_BYCOMMAND, ID_EDIT_COPYTABLE, ts.UIMsg);
  GetMenuString(EditMenu, ID_EDIT_PASTE2, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_EDIT_PASTE", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(EditMenu, ID_EDIT_PASTE2, MF_BYCOMMAND, ID_EDIT_PASTE2, ts.UIMsg);
  GetMenuString(EditMenu, ID_EDIT_PASTECR, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_EDIT_PASTECR", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(EditMenu, ID_EDIT_PASTECR, MF_BYCOMMAND, ID_EDIT_PASTECR, ts.UIMsg);
  GetMenuString(EditMenu, ID_EDIT_CLEARSCREEN, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_EDIT_CLSCREEN", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(EditMenu, ID_EDIT_CLEARSCREEN, MF_BYCOMMAND, ID_EDIT_CLEARSCREEN, ts.UIMsg);
  GetMenuString(EditMenu, ID_EDIT_CLEARBUFFER, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_EDIT_CLBUFFER", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(EditMenu, ID_EDIT_CLEARBUFFER, MF_BYCOMMAND, ID_EDIT_CLEARBUFFER, ts.UIMsg);
  GetMenuString(EditMenu, ID_EDIT_SELECTALL, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_EDIT_SELECTALL", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(EditMenu, ID_EDIT_SELECTALL, MF_BYCOMMAND, ID_EDIT_SELECTALL, ts.UIMsg);

  GetMenuString(*Menu, ID_SETUP, ts.UIMsg, sizeof(ts.UIMsg), MF_BYPOSITION);
  get_lang_msg("MENU_SETUP", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(*Menu, ID_SETUP, MF_BYPOSITION, ID_SETUP, ts.UIMsg);
  GetMenuString(SetupMenu, ID_SETUP_TERMINAL, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_SETUP_TERMINAL", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(SetupMenu, ID_SETUP_TERMINAL, MF_BYCOMMAND, ID_SETUP_TERMINAL, ts.UIMsg);
  GetMenuString(SetupMenu, ID_SETUP_WINDOW, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_SETUP_WINDOW", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(SetupMenu, ID_SETUP_WINDOW, MF_BYCOMMAND, ID_SETUP_WINDOW, ts.UIMsg);
  GetMenuString(SetupMenu, ID_SETUP_FONT, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_SETUP_FONT", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(SetupMenu, ID_SETUP_FONT, MF_BYCOMMAND, ID_SETUP_FONT, ts.UIMsg);
  GetMenuString(SetupMenu, ID_SETUP_KEYBOARD, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_SETUP_KEYBOARD", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(SetupMenu, ID_SETUP_KEYBOARD, MF_BYCOMMAND, ID_SETUP_KEYBOARD, ts.UIMsg);
  GetMenuString(SetupMenu, ID_SETUP_SERIALPORT, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_SETUP_SERIALPORT", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(SetupMenu, ID_SETUP_SERIALPORT, MF_BYCOMMAND, ID_SETUP_SERIALPORT, ts.UIMsg);
  GetMenuString(SetupMenu, ID_SETUP_TCPIP, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_SETUP_TCPIP", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(SetupMenu, ID_SETUP_TCPIP, MF_BYCOMMAND, ID_SETUP_TCPIP, ts.UIMsg);
  GetMenuString(SetupMenu, ID_SETUP_GENERAL, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_SETUP_GENERAL", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(SetupMenu, ID_SETUP_GENERAL, MF_BYCOMMAND, ID_SETUP_GENERAL, ts.UIMsg);
  GetMenuString(SetupMenu, ID_SETUP_ADDITIONALSETTINGS, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_SETUP_ADDITION", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(SetupMenu, ID_SETUP_ADDITIONALSETTINGS, MF_BYCOMMAND, ID_SETUP_ADDITIONALSETTINGS, ts.UIMsg);
  GetMenuString(SetupMenu, ID_SETUP_SAVE, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_SETUP_SAVE", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(SetupMenu, ID_SETUP_SAVE, MF_BYCOMMAND, ID_SETUP_SAVE, ts.UIMsg);
  GetMenuString(SetupMenu, ID_SETUP_RESTORE, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_SETUP_RESTORE", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(SetupMenu, ID_SETUP_RESTORE, MF_BYCOMMAND, ID_SETUP_RESTORE, ts.UIMsg);
  GetMenuString(SetupMenu, ID_SETUP_LOADKEYMAP, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_SETUP_LOADKEYMAP", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(SetupMenu, ID_SETUP_LOADKEYMAP, MF_BYCOMMAND, ID_SETUP_LOADKEYMAP, ts.UIMsg);

  GetMenuString(*Menu, ID_CONTROL, ts.UIMsg, sizeof(ts.UIMsg), MF_BYPOSITION);
  get_lang_msg("MENU_CONTROL", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(*Menu, ID_CONTROL, MF_BYPOSITION, ID_CONTROL, ts.UIMsg);
  GetMenuString(ControlMenu, ID_CONTROL_RESETTERMINAL, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_CONTROL_RESET", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(ControlMenu, ID_CONTROL_RESETTERMINAL, MF_BYCOMMAND, ID_CONTROL_RESETTERMINAL, ts.UIMsg);
  GetMenuString(ControlMenu, ID_CONTROL_AREYOUTHERE, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_CONTROL_AREYOUTHERE", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(ControlMenu, ID_CONTROL_AREYOUTHERE, MF_BYCOMMAND, ID_CONTROL_AREYOUTHERE, ts.UIMsg);
  GetMenuString(ControlMenu, ID_CONTROL_SENDBREAK, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_CONTROL_SENDBREAK", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(ControlMenu, ID_CONTROL_SENDBREAK, MF_BYCOMMAND, ID_CONTROL_SENDBREAK, ts.UIMsg);
  GetMenuString(ControlMenu, ID_CONTROL_RESETPORT, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_CONTROL_RESETPORT", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(ControlMenu, ID_CONTROL_RESETPORT, MF_BYCOMMAND, ID_CONTROL_RESETPORT, ts.UIMsg);
  GetMenuString(ControlMenu, ID_CONTROL_BROADCASTCOMMAND, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_CONTROL_BROADCAST", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(ControlMenu, ID_CONTROL_BROADCASTCOMMAND, MF_BYCOMMAND, ID_CONTROL_BROADCASTCOMMAND, ts.UIMsg);
  GetMenuString(ControlMenu, ID_CONTROL_OPENTEK, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_CONTROL_OPENTEK", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(ControlMenu, ID_CONTROL_OPENTEK, MF_BYCOMMAND, ID_CONTROL_OPENTEK, ts.UIMsg);
  GetMenuString(ControlMenu, ID_CONTROL_CLOSETEK, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_CONTROL_CLOSETEK", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(ControlMenu, ID_CONTROL_CLOSETEK, MF_BYCOMMAND, ID_CONTROL_CLOSETEK, ts.UIMsg);
  GetMenuString(ControlMenu, ID_CONTROL_MACRO, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_CONTROL_MACRO", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(ControlMenu, ID_CONTROL_MACRO, MF_BYCOMMAND, ID_CONTROL_MACRO, ts.UIMsg);

  GetMenuString(*Menu, ID_HELPMENU, ts.UIMsg, sizeof(ts.UIMsg), MF_BYPOSITION);
  get_lang_msg("MENU_HELP", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(*Menu, ID_HELPMENU, MF_BYPOSITION, ID_HELPMENU, ts.UIMsg);
  GetMenuString(HelpMenu, ID_HELP_INDEX2, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_HELP_INDEX", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(HelpMenu, ID_HELP_INDEX2, MF_BYCOMMAND, ID_HELP_INDEX2, ts.UIMsg);
  GetMenuString(HelpMenu, ID_HELP_USING2, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_HELP_USING", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(HelpMenu, ID_HELP_USING2, MF_BYCOMMAND, ID_HELP_USING2, ts.UIMsg);
  GetMenuString(HelpMenu, ID_HELP_ABOUT, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_HELP_ABOUT", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(HelpMenu, ID_HELP_ABOUT, MF_BYCOMMAND, ID_HELP_ABOUT, ts.UIMsg);
#endif
  if ((ts.MenuFlag & MF_SHOWWINMENU) !=0)
  {
    WinMenu = CreatePopupMenu();
#ifdef I18N
    strcpy(ts.UIMsg, "&Window");
    get_lang_msg("MENU_WINDOW", ts.UIMsg, ts.UILanguageFile);
    ::InsertMenu(*Menu,ID_HELPMENU,
		 MF_STRING | MF_ENABLED |
		 MF_POPUP | MF_BYPOSITION,
		 (int)WinMenu, ts.UIMsg);
#else
    ::InsertMenu(*Menu,ID_HELPMENU,
		 MF_STRING | MF_ENABLED |
		 MF_POPUP | MF_BYPOSITION,
		 (int)WinMenu, "&Window");
#endif
  }

  TTXModifyMenu(*Menu); /* TTPLUG */
}

void CVTWindow::InitMenuPopup(HMENU SubMenu)
{
	if ( SubMenu == FileMenu )
	{
		if ( Connecting ) {
			EnableMenuItem(FileMenu,ID_FILE_NEWCONNECTION,MF_BYCOMMAND | MF_GRAYED);
		} else {
			EnableMenuItem(FileMenu,ID_FILE_NEWCONNECTION,MF_BYCOMMAND | MF_ENABLED);
		}

		if ( (! cv.Ready) || (SendVar!=NULL) || (FileVar!=NULL) ||
			(cv.PortType==IdFile) )
		{
			EnableMenuItem(FileMenu,ID_FILE_SENDFILE,MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(FileMenu,ID_TRANSFER,MF_BYPOSITION | MF_GRAYED); /* Transfer */
			EnableMenuItem(FileMenu,ID_FILE_CHANGEDIR,MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(FileMenu,ID_FILE_DISCONNECT,MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(FileMenu,ID_FILE_DUPLICATESESSION,MF_BYCOMMAND | MF_GRAYED);
		}
		else {
			EnableMenuItem(FileMenu,ID_FILE_SENDFILE,MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(FileMenu,ID_TRANSFER,MF_BYPOSITION | MF_ENABLED); /* Transfer */
			EnableMenuItem(FileMenu,ID_FILE_CHANGEDIR,MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(FileMenu,ID_FILE_DISCONNECT,MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(FileMenu,ID_FILE_DUPLICATESESSION,MF_BYCOMMAND | MF_ENABLED);
		}

		// 新規メニューを追加 (2004.12.5 yutaka)
		EnableMenuItem(FileMenu,ID_FILE_CYGWINCONNECTION,MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(FileMenu,ID_FILE_TERATERMMENU,MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(FileMenu,ID_FILE_LOGMEIN,MF_BYCOMMAND | MF_ENABLED);

		// XXX: この位置にしないと、logがグレイにならない。 (2005.2.1 yutaka)
		if (LogVar!=NULL) { // ログ採取モードの場合
			EnableMenuItem(FileMenu,ID_FILE_LOG,MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(FileMenu,ID_FILE_COMMENTTOLOG, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(FileMenu,ID_FILE_VIEWLOG, MF_BYCOMMAND | MF_ENABLED);
		} else {
			EnableMenuItem(FileMenu,ID_FILE_LOG,MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(FileMenu,ID_FILE_COMMENTTOLOG, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(FileMenu,ID_FILE_VIEWLOG, MF_BYCOMMAND | MF_GRAYED);
		}

	}
	else if ( SubMenu == TransMenu )
	{
		if ((cv.PortType==IdSerial) &&
			((ts.DataBit==IdDataBit7) || (ts.Flow==IdFlowX)))
		{
			EnableMenuItem(TransMenu,1,MF_BYPOSITION | MF_GRAYED);  /* XMODEM */
			EnableMenuItem(TransMenu,4,MF_BYPOSITION | MF_GRAYED);  /* Quick-VAN */
		}
		else {
			EnableMenuItem(TransMenu,1,MF_BYPOSITION | MF_ENABLED); /* XMODEM */
			EnableMenuItem(TransMenu,4,MF_BYPOSITION | MF_ENABLED); /* Quick-VAN */
		}
		if ((cv.PortType==IdSerial) &&
			(ts.DataBit==IdDataBit7))
		{
			EnableMenuItem(TransMenu,2,MF_BYPOSITION | MF_GRAYED); /* ZMODEM */
			EnableMenuItem(TransMenu,3,MF_BYPOSITION | MF_GRAYED); /* B-Plus */
		}
		else {
			EnableMenuItem(TransMenu,2,MF_BYPOSITION | MF_ENABLED); /* ZMODEM */
			EnableMenuItem(TransMenu,3,MF_BYPOSITION | MF_ENABLED); /* B-Plus */
		}
	}
	else if (SubMenu == EditMenu)
	{
		if (Selected) {
			EnableMenuItem(EditMenu,ID_EDIT_COPY2,MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(EditMenu,ID_EDIT_COPYTABLE,MF_BYCOMMAND | MF_ENABLED);
		}
		else {
			EnableMenuItem(EditMenu,ID_EDIT_COPY2,MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(EditMenu,ID_EDIT_COPYTABLE,MF_BYCOMMAND | MF_GRAYED);
		}
		if (cv.Ready &&
			(SendVar==NULL) && (FileVar==NULL) &&
			(cv.PortType!=IdFile) &&
			(IsClipboardFormatAvailable(CF_TEXT) ||
			IsClipboardFormatAvailable(CF_OEMTEXT)))
		{
			EnableMenuItem(EditMenu,ID_EDIT_PASTE2,MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(EditMenu,ID_EDIT_PASTECR,MF_BYCOMMAND | MF_ENABLED);
		}
		else {
			EnableMenuItem(EditMenu,ID_EDIT_PASTE2,MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(EditMenu,ID_EDIT_PASTECR,MF_BYCOMMAND | MF_GRAYED);
		}
	}
	else if (SubMenu == SetupMenu)
		if (cv.Ready &&
			((cv.PortType==IdTCPIP) || (cv.PortType==IdFile)) ||
			(SendVar!=NULL) || (FileVar!=NULL) || Connecting)
			EnableMenuItem(SetupMenu,ID_SETUP_SERIALPORT,MF_BYCOMMAND | MF_GRAYED);
		else
			EnableMenuItem(SetupMenu,ID_SETUP_SERIALPORT,MF_BYCOMMAND | MF_ENABLED);

	else if (SubMenu == ControlMenu)
	{
		if (cv.Ready &&
			(SendVar==NULL) && (FileVar==NULL))
		{
			EnableMenuItem(ControlMenu,ID_CONTROL_SENDBREAK,MF_BYCOMMAND | MF_ENABLED);
			if (cv.PortType==IdSerial)
				EnableMenuItem(ControlMenu,ID_CONTROL_RESETPORT,MF_BYCOMMAND | MF_ENABLED);
			else
				EnableMenuItem(ControlMenu,ID_CONTROL_RESETPORT,MF_BYCOMMAND | MF_GRAYED);
		}
		else {
			EnableMenuItem(ControlMenu,ID_CONTROL_SENDBREAK,MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(ControlMenu,ID_CONTROL_RESETPORT,MF_BYCOMMAND | MF_GRAYED);
		}

		if (cv.Ready && cv.TelFlag && (FileVar==NULL))
			EnableMenuItem(ControlMenu,ID_CONTROL_AREYOUTHERE,MF_BYCOMMAND | MF_ENABLED);
		else
			EnableMenuItem(ControlMenu,ID_CONTROL_AREYOUTHERE,MF_BYCOMMAND | MF_GRAYED);

		if (HTEKWin==0) EnableMenuItem(ControlMenu,ID_CONTROL_CLOSETEK,MF_BYCOMMAND | MF_GRAYED);
		else EnableMenuItem(ControlMenu,ID_CONTROL_CLOSETEK,MF_BYCOMMAND | MF_ENABLED);

		if ((ConvH!=0) || (FileVar!=NULL))
			EnableMenuItem(ControlMenu,ID_CONTROL_MACRO,MF_BYCOMMAND | MF_GRAYED);
		else
			EnableMenuItem(ControlMenu,ID_CONTROL_MACRO,MF_BYCOMMAND | MF_ENABLED);

	}
	else if (SubMenu == WinMenu)
	{
#ifdef I18N
		SetWinMenu(WinMenu, ts.UIMsg, ts.UILanguageFile);
#else
		SetWinMenu(WinMenu);
#endif
	}

	TTXModifyPopupMenu(SubMenu); /* TTPLUG */
}

// added ConfirmPasteMouseRButton (2007.3.17 maya)
void CVTWindow::InitPasteMenu(HMENU *Menu)
{
  *Menu = LoadMenu(AfxGetInstanceHandle(),
    MAKEINTRESOURCE(IDR_PASTEMENU));

#ifdef I18N
  GetMenuString(*Menu, ID_EDIT_PASTE2, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_EDIT_PASTE", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(*Menu, ID_EDIT_PASTE2, MF_BYCOMMAND, ID_EDIT_PASTE2, ts.UIMsg);
  GetMenuString(*Menu, ID_EDIT_PASTECR, ts.UIMsg, sizeof(ts.UIMsg), MF_BYCOMMAND);
  get_lang_msg("MENU_EDIT_PASTECR", ts.UIMsg, ts.UILanguageFile);
  ModifyMenu(*Menu, ID_EDIT_PASTECR, MF_BYCOMMAND, ID_EDIT_PASTECR, ts.UIMsg);
#endif
}

void CVTWindow::ResetSetup()
{
  ChangeFont();
  BuffChangeWinSize(WinWidth,WinHeight);
  ChangeCaret();

  if (cv.Ready)
  {
    ts.PortType = cv.PortType;
    if (cv.PortType==IdSerial)
    { /* if serial port, change port parameters */
      ts.ComPort = cv.ComPort;
      CommResetSerial(&ts,&cv);
    }
  }

  /* setup terminal */
  SetupTerm();

  /* setup window */
  ChangeWin();

  /* Language & IME */
  ResetIME();

  /* change TEK window */
  if (pTEKWin != NULL)
    ((CTEKWindow *)pTEKWin)->RestoreSetup();
}

void CVTWindow::RestoreSetup()
{
  char TempDir[MAXPATHLEN];
  char TempName[MAXPATHLEN];

  if ( strlen(ts.SetupFName)==0 ) return;
  ExtractFileName(ts.SetupFName,TempName);
  ExtractDirName(ts.SetupFName,TempDir);
  if (TempDir[0]==0)
	  strcpy(TempDir,ts.HomeDir);
  FitFileName(TempName,".INI");

  strcpy(ts.SetupFName,TempDir);
  AppendSlash(ts.SetupFName);
  strcat(ts.SetupFName,TempName);

  if (LoadTTSET())
    (*ReadIniFile)(ts.SetupFName,&ts);
  FreeTTSET();

#ifdef ALPHABLEND_TYPE2
  BGInitialize();
  BGSetupPrimary(TRUE);
  // 2006/03/17 by 337 : Alpha値も即時変更
  // Layered窓になっていない場合は効果が無い
  if (ts.EtermLookfeel.BGUseAlphaBlendAPI) {
	MySetLayeredWindowAttributes(HVTWin, 0, ts.AlphaBlend, LWA_ALPHA);
  }
#endif

  ChangeDefaultSet(&ts,NULL);

  ResetSetup();
}

/* called by the [Setup] Terminal command */
void CVTWindow::SetupTerm()
{
  if (ts.Language==IdJapanese)
    ResetCharSet();
  cv.CRSend = ts.CRSend;

  // for russian mode
  cv.RussHost = ts.RussHost;
  cv.RussClient = ts.RussClient;

  if (cv.Ready && cv.TelFlag && (ts.TelEcho>0))
    TelChangeEcho();

  if ((ts.TerminalWidth!=NumOfColumns) ||
      (ts.TerminalHeight!=NumOfLines-StatusLine))
  {
    LockBuffer();
    HideStatusLine();
    ChangeTerminalSize(ts.TerminalWidth,
		       ts.TerminalHeight);
    UnlockBuffer();
  }
  else if ((ts.TermIsWin>0) &&
	   ((ts.TerminalWidth!=WinWidth) ||
	    (ts.TerminalHeight!=WinHeight-StatusLine)))
    BuffChangeWinSize(ts.TerminalWidth,ts.TerminalHeight+StatusLine);
}

void CVTWindow::Startup()
{
  /* auto log */
  if ((ts.LogFN[0]!=0) && NewFileVar(&LogVar))
  {
    LogVar->DirLen = 0;
    strcpy(LogVar->FullName,ts.LogFN);
    LogStart();
  }

  if ((TopicName[0]==0) && (ts.MacroFN[0]!=0))
  { // start the macro specified in the command line or setup file
    RunMacro(ts.MacroFN,TRUE);
    ts.MacroFN[0] = 0;
  }
  else {// start connection
    if (TopicName[0]!=0)
      cv.NoMsg=1; /* suppress error messages */
    ::PostMessage(HVTWin,WM_USER_COMMSTART,0,0);
  }
}

void CVTWindow::OpenTEK()
{
  ActiveWin = IdTEK;
  if (HTEKWin==NULL)
  {
    pTEKWin = new CTEKWindow();
  }
  else {
    ::ShowWindow(HTEKWin,SW_SHOWNORMAL);
    ::SetFocus(HTEKWin);
  }
}

/////////////////////////////////////////////////////////////////////////////
// CVTWindow message handler

LRESULT CVTWindow::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
  LRESULT Result;

  if (message == MsgDlgHelp)
  {
    OnDlgHelp(wParam,lParam);
    return 0;
  }
  else if ((ts.HideTitle>0) &&
	   (message == WM_NCHITTEST))
  {
    Result = CFrameWnd::DefWindowProc(message,wParam,lParam);
    if ((Result==HTCLIENT) && AltKey())
#ifdef ALPHABLEND_TYPE2
    	if(ShiftKey())
    		Result = HTBOTTOMRIGHT;
	    else
	    	Result = HTCAPTION;
#else
		Result = HTCAPTION;
#endif
    return Result;
  }

  return (CFrameWnd::DefWindowProc(message,wParam,lParam));
}

BOOL CVTWindow::OnCommand(WPARAM wParam, LPARAM lParam)
{
#ifdef TERATERM32
  WORD wID = LOWORD(wParam);
  WORD wNotifyCode = HIWORD(wParam);
#else
  WORD wID = wParam;
  WORD wNotifyCode = HIWORD(lParam);
#endif	
  
  if (wNotifyCode==1)
  {
    switch (wID) {
      case ID_ACC_SENDBREAK:
	// added DisableAcceleratorSendBreak (2007.3.17 maya)
	if (!ts.DisableAcceleratorSendBreak)
	  OnControlSendBreak();
	return TRUE;
      case ID_ACC_PASTECR:
	OnEditPasteCR();
	return TRUE;
      case ID_ACC_AREYOUTHERE:
	OnControlAreYouThere();
	return TRUE;
      case ID_ACC_PASTE:
	OnEditPaste();
	return TRUE;
    }
    if (ActiveWin==IdVT)
    {
      switch (wID) {
	case ID_ACC_NEWCONNECTION:
	  OnFileNewConnection();
	  return TRUE;
	case ID_ACC_COPY:
	  OnEditCopy();
	  return TRUE;
	case ID_ACC_PRINT:
	  OnFilePrint();
	  return TRUE;
	case ID_ACC_EXIT:
	  OnFileExit();
	  return TRUE;
      }
    }
    else { // transfer accelerator message to TEK win
      switch (wID) {
	case ID_ACC_COPY:
	  ::PostMessage(HTEKWin,WM_COMMAND,ID_TEKEDIT_COPY,0);
	  return TRUE;
	case ID_ACC_PRINT:
	  ::PostMessage(HTEKWin,WM_COMMAND,ID_TEKFILE_PRINT,0);
	  return TRUE;
	case ID_ACC_EXIT:
	  ::PostMessage(HTEKWin,WM_COMMAND,ID_TEKFILE_EXIT,0);
	  return TRUE;
      }
    }
  }

  if ((wID>=ID_WINDOW_1) && (wID<ID_WINDOW_1+9))
  {
    SelectWin(wID-ID_WINDOW_1);
    return TRUE;
  }
  else
    if (TTXProcessCommand(HVTWin, wID)) return TRUE; else /* TTPLUG */
    return CFrameWnd::OnCommand(wParam, lParam);
}

void CVTWindow::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
  DispSetActive(nState!=WA_INACTIVE);
}

void CVTWindow::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  unsigned int i;
  char Code;

  if (!KeybEnabled || (TalkStatus!=IdTalkKeyb)) return;

  if ((ts.MetaKey>0) && AltKey())
  {
    ::PostMessage(HVTWin,WM_SYSCHAR,nChar,MAKELONG(nRepCnt,nFlags));
    return;
  }
  Code = nChar;

  if ((ts.Language==IdRussian) &&
      ((BYTE)Code>=128))
    Code =
      (char)RussConv(ts.RussKeyb,ts.RussClient,(BYTE)Code);

  for (i=1 ; i<=nRepCnt ; i++)
  {
    CommTextOut(&cv,&Code,1);
    if (ts.LocalEcho>0)
      CommTextEcho(&cv,&Code,1);
  }
}

void CVTWindow::OnClose()
{
  if ((HTEKWin!=NULL) && ! ::IsWindowEnabled(HTEKWin))
  {
    MessageBeep(0);
    return;
  }
#ifdef I18N
  strcpy(ts.UIMsg, "Disconnect?");
  get_lang_msg("MSG_DISCONNECT_CONF", ts.UIMsg, ts.UILanguageFile);
#endif
  if (cv.Ready && (cv.PortType==IdTCPIP) &&
      ((ts.PortFlag & PF_CONFIRMDISCONN) != 0) &&
      ! CloseTT &&
#ifdef I18N
      (::MessageBox(HVTWin,ts.UIMsg,"Tera Term",
#else
      (::MessageBox(HVTWin,"Disconnect?","Tera Term",
#endif
	MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON2)==IDCANCEL))
    return;

  FileTransEnd(0);
  ProtoEnd();

  DestroyWindow();
}

void CVTWindow::OnDestroy()
{
  // remove this window from the window list
  UnregWin(HVTWin);

  EndKeyboard();

  /* Disable drag-drop */
  ::DragAcceptFiles(HVTWin,FALSE);

  EndDDE();

  if (cv.TelFlag) EndTelnet();
  CommClose(&cv);

  OpenHelp(HVTWin,HELP_QUIT,0);

  FreeIME();
  FreeTTSET();
  do { }
    while (FreeTTDLG());

  do {} while (FreeTTFILE());

  if (HTEKWin != NULL)
    ::DestroyWindow(HTEKWin);

  EndDisp();

  FreeBuffer();

  CFrameWnd::OnDestroy();
  TTXEnd(); /* TTPLUG */
}

void CVTWindow::OnDropFiles(HDROP hDropInfo)
{
#ifdef TERATERM32
	::SetForegroundWindow(HVTWin);
#else
	::SetActiveWindow(HVTWin);
#endif
	if (cv.Ready && (SendVar==NULL) && NewFileVar(&SendVar))
	{
		if (DragQueryFile(hDropInfo,0,SendVar->FullName,
			sizeof(SendVar->FullName))>0)
		{
			DWORD attr;
			char *ptr, *q;
			char tmpbuf[_MAX_PATH * 2];

			// ディレクトリの場合はフルパス名を貼り付ける (2004.11.3 yutaka)
			attr = GetFileAttributes(SendVar->FullName);
			if (attr != -1 && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
				ptr = SendVar->FullName;
				// パスの区切りを \ -> / へ
				setlocale(LC_ALL, ts.Locale);
				while (*ptr) {
					if (isleadbyte(*ptr)) { // multi-byte
						ptr += 2;
						continue;
					}
					if (*ptr == '\\')
						*ptr = '/';
					ptr++;
				}

				// パスに空白があればエスケープする
				q = tmpbuf;
				ptr = SendVar->FullName;
				while (*ptr) {
					if (*ptr == ' ')
						*q++ = '\\';
					*q++ = *ptr;
					ptr++;
				}
				*q = '\0'; // null-terminate

				ptr = tmpbuf;

				// consoleへ送信
				while (*ptr) {
					FSOut1(*ptr);
					if (ts.LocalEcho > 0) {
						FSEcho1(*ptr);
					}
					ptr++;
				}
				FreeFileVar(&SendVar); // 解放を忘れずに

			} else {
				// いきなりファイルの内容を送り込む前に、ユーザに問い合わせを行う。(2006.1.21 yutaka)
#ifdef I18N
				char uimsg[MAX_UIMSG];
				strcpy(uimsg, "Tera Term: File Drag and Drop");
				get_lang_msg("MSG_DANDD_CONF_TITLE", uimsg, ts.UILanguageFile);
				strcpy(ts.UIMsg, "Are you sure that you want to send the file content?");
				get_lang_msg("MSG_DANDD_CONF", ts.UIMsg, ts.UILanguageFile);
#endif
				if (MessageBox(
#ifdef I18N
					ts.UIMsg, uimsg, MB_YESNO | MB_DEFBUTTON2) == IDYES) {
#else
					"Are you sure that you want to send the file content?", 
					"Tera Term: File Drag and Drop", MB_YESNO | MB_DEFBUTTON2) == IDYES) {
#endif
						SendVar->DirLen = 0;
						ts.TransBin = 0;
						FileSendStart();

					} else {
						FreeFileVar(&SendVar);

					}
			}
		}
		else
			FreeFileVar(&SendVar);
	}
	DragFinish(hDropInfo);
}

void CVTWindow::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
#ifndef WINDOW_MAXMIMUM_ENABLED
	lpMMI->ptMaxSize.x = 10000;
	lpMMI->ptMaxSize.y = 10000;
	lpMMI->ptMaxTrackSize.x = 10000;
	lpMMI->ptMaxTrackSize.y = 10000;
#endif
}

void CVTWindow::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
  int Func;

  switch (nSBCode) {
    case SB_BOTTOM: Func = SCROLL_BOTTOM; break;
    case SB_ENDSCROLL: return;
    case SB_LINEDOWN: Func = SCROLL_LINEDOWN; break;
    case SB_LINEUP: Func = SCROLL_LINEUP; break;
    case SB_PAGEDOWN: Func = SCROLL_PAGEDOWN; break;
    case SB_PAGEUP: Func = SCROLL_PAGEUP; break;
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK: Func = SCROLL_POS; break;
    case SB_TOP: Func = SCROLL_TOP; break;
    default:
      return;
  }
  DispHScroll(Func,nPos);
}

void CVTWindow::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
  InitMenuPopup(pPopupMenu->m_hMenu);
}

void CVTWindow::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  BYTE KeyState[256];
  MSG M;

  if (KeyDown(HVTWin,nChar,nRepCnt,nFlags & 0x1ff)) return;

  if ((ts.MetaKey>0) && ((nFlags & 0x2000) != 0))
  { /* for Ctrl+Alt+Key combination */
    GetKeyboardState((PBYTE)KeyState);
    KeyState[VK_MENU] = 0;
    SetKeyboardState((PBYTE)KeyState);
    M.hwnd = HVTWin;
    M.message = WM_KEYDOWN;
    M.wParam = nChar;
    M.lParam = MAKELONG(nRepCnt,nFlags & 0xdfff);
    TranslateMessage(&M);

  } else {
	// ScrollLockキーが点灯している場合は、マウスをクリックしっぱなし状態であると
	// 見なす。すなわち、パージング処理が一時停止する。
    // 当該キーを消灯させると、処理が再開される。(2006.11.14 yutaka)
    GetKeyboardState((PBYTE)KeyState);
	if (KeyState[VK_SCROLL] == 0x81) { // on : scroll locked
		ScrollLock = TRUE;

	} else if (KeyState[VK_SCROLL] == 0x80) { // off : scroll unlocked
		ScrollLock = FALSE;

	} else {
		// do nothing

	}

  }

}

void CVTWindow::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  KeyUp(nChar);
}

void CVTWindow::OnKillFocus(CWnd* pNewWnd)
{
  DispDestroyCaret();
  CFrameWnd::OnKillFocus(pNewWnd);
}

void CVTWindow::OnLButtonDblClk(UINT nFlags, CPoint point)
{
  if (LButton || MButton || RButton) return;

  DblClkX = point.x;
  DblClkY = point.y;

  if (BuffDblClk(DblClkX, DblClkY)) // ブラウザ呼び出しの場合は何もしない。 (2005.4.3 yutaka)
	  return;

  LButton = TRUE;
  DblClk = TRUE;
  AfterDblClk = TRUE;
  ::SetTimer(HVTWin, IdDblClkTimer, GetDoubleClickTime(), NULL);

  /* for AutoScrolling */
  ::SetCapture(HVTWin);
  ::SetTimer(HVTWin, IdScrollTimer, 100, NULL);
}

void CVTWindow::OnLButtonDown(UINT nFlags, CPoint point)
{
  POINT p;

  p.x = point.x;
  p.y = point.y;
  ButtonDown(p,IdLeftButton);
}

void CVTWindow::OnLButtonUp(UINT nFlags, CPoint point)
{
  if (! LButton) return;
  ButtonUp(FALSE);
}

void CVTWindow::OnMButtonDown(UINT nFlags, CPoint point)
{
  POINT p;

  p.x = point.x;
  p.y = point.y;
  ButtonDown(p,IdMiddleButton);
}

void CVTWindow::OnMButtonUp(UINT nFlags, CPoint point)
{
  if (! MButton) return;
  ButtonUp(TRUE);
}

int CVTWindow::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
  if ((ts.SelOnActive==0) &&
      (nHitTest==HTCLIENT))   //disable mouse event for text selection
    return MA_ACTIVATEANDEAT; //     when window is activated
  else
    return MA_ACTIVATE;
}

void CVTWindow::OnMouseMove(UINT nFlags, CPoint point)
{
  int i;

  if (! (LButton || MButton || RButton)) {
	  // マウスカーソル直下にURL文字列があるかを走査する (2005.4.2 yutaka)
	  BuffChangeSelect(point.x, point.y,0);
	  return;
  }

  if (DblClk)
    i = 2;
  else if (TplClk)
    i = 3;
  else
    i = 1;
  BuffChangeSelect(point.x, point.y,i);
}

void CVTWindow::OnMove(int x, int y)
{
  DispSetWinPos();
}

// マウスホイールの回転
BOOL CVTWindow::OnMouseWheel(
   UINT nFlags,   // 仮想キー
   short zDelta,  // 回転距離
   CPoint pt      // カーソル位置
)
{
	int line, i, backward;
	short delta;

	if (zDelta < 0) { // マイナスならユーザに向かって回転
		backward = 1;
	} else {
		backward = 0;
	}
	delta = abs(zDelta);
	line = delta / WHEEL_DELTA; // ライン数
	for (i = 0 ; i < line ; i++) {
		if (backward == 1) {
			OnVScroll(SB_LINEDOWN, 0, NULL);
		} else {
			OnVScroll(SB_LINEUP, 0, NULL);
		}
	}

	return (TRUE);
}


void CVTWindow::OnNcLButtonDblClk(UINT nHitTest, CPoint point)
{
  if (! Minimized && (nHitTest == HTCAPTION))
    DispRestoreWinSize();
  else
    CFrameWnd::OnNcLButtonDblClk(nHitTest,point);
}

void CVTWindow::OnNcRButtonDown(UINT nHitTest, CPoint point)
{
  if ((nHitTest==HTCAPTION) &&
      (ts.HideTitle>0) &&
      AltKey())
    ::CloseWindow(HVTWin); /* iconize */
}

void CVTWindow::OnPaint()
{
  PAINTSTRUCT ps;
  CDC *cdc;
  HDC PaintDC;
  int Xs, Ys, Xe, Ye;

#ifdef ALPHABLEND_TYPE2
//<!--by AKASI
  BGSetupPrimary(FALSE);
//-->
#endif

  cdc = BeginPaint(&ps);
  PaintDC = cdc->GetSafeHdc();

  PaintWindow(PaintDC,ps.rcPaint,ps.fErase, &Xs,&Ys,&Xe,&Ye);
  LockBuffer();
  BuffUpdateRect(Xs,Ys,Xe,Ye);
  UnlockBuffer();
  DispEndPaint();

  EndPaint(&ps);

  if (FirstPaint)
  {
    if (strlen(TopicName)>0)
    {
      InitDDE();
      SendDDEReady();
    }
    FirstPaint = FALSE;
    Startup();
  }
}

void CVTWindow::OnRButtonDown(UINT nFlags, CPoint point)
{
  POINT p;

  p.x = point.x;
  p.y = point.y;
  ButtonDown(p,IdRightButton);
}

void CVTWindow::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (! RButton) return;

	// 右ボタン押下でのペーストを禁止する (2005.3.16 yutaka)
	if (ts.DisablePasteMouseRButton) {
		ButtonUp(FALSE);
	} else {
		ButtonUp(TRUE);
	}
}

void CVTWindow::OnSetFocus(CWnd* pOldWnd)
{
  ChangeCaret();
  CFrameWnd::OnSetFocus(pOldWnd);
}

void CVTWindow::OnSize(UINT nType, int cx, int cy)
{
	RECT R;
	int w, h;

	Minimized = (nType==SIZE_MINIMIZED);

	if (FirstPaint && Minimized)
	{
		if (strlen(TopicName)>0)
		{
			InitDDE();
			SendDDEReady();
		}
		FirstPaint = FALSE;
		Startup();
		return;
	}
	if (Minimized || DontChangeSize) return;

	::GetWindowRect(HVTWin,&R);
	w = R.right - R.left;
	h = R.bottom - R.top;
	if (AdjustSize)
		ResizeWindow(R.left,R.top,w,h,cx,cy);
	else {
		w = cx / FontWidth;
		h = cy / FontHeight;
		HideStatusLine();
		BuffChangeWinSize(w,h);
	}

#ifdef WINDOW_MAXMIMUM_ENABLED
	if (nType == SIZE_MAXIMIZED) {
		AdjustSize = 0;
	}
#endif
}

void CVTWindow::OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	char e = ESC;
	char Code;
	unsigned int i;

	// ALT + xを押下すると WM_SYSCHAR が飛んでくる。
	// ALT + Enterでウィンドウの最大化 (2005.4.24 yutaka)
	if (AltKey() && nChar == 13) {
		if (IsZoomed()) { // window is maximum
			ShowWindow(SW_RESTORE);
		} else {
			ShowWindow(SW_MAXIMIZE);
		}
	}

	if (ts.MetaKey>0)
	{
		if (!KeybEnabled || (TalkStatus!=IdTalkKeyb)) return;
		Code = nChar;
		for (i=1 ; i<=nRepCnt ; i++)
		{
			CommTextOut(&cv,&e,1);
			CommTextOut(&cv,&Code,1);
			if (ts.LocalEcho>0)
			{
				CommTextEcho(&cv,&e,1);
				CommTextEcho(&cv,&Code,1);
			}
		}
		return;
	}
	CFrameWnd::OnSysChar(nChar, nRepCnt, nFlags);
}

void CVTWindow::OnSysColorChange()
{
#ifndef TERATERM32
  SysColorChange();
#else
  CFrameWnd::OnSysColorChange();
#endif
}

void CVTWindow::OnSysCommand(UINT nID, LPARAM lParam)
{
  if (nID==ID_SHOWMENUBAR)
  {
    ts.PopupMenu = 0;
    SwitchMenu();
  }
  else if (((nID & 0xfff0)==SC_CLOSE) && (cv.PortType==IdTCPIP) &&
	   cv.Open && ! cv.Ready && (cv.ComPort>0))
	// now getting host address (see CommOpen() in commlib.c)
    ::PostMessage(HVTWin,WM_SYSCOMMAND,nID,lParam);
  else CFrameWnd::OnSysCommand(nID,lParam);
}

void CVTWindow::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  if ((nChar==VK_F10) ||
      (ts.MetaKey>0) &&
      ((MainMenu==NULL) || (nChar!=VK_MENU)) &&
      ((nFlags & 0x2000) != 0))
    KeyDown(HVTWin,nChar,nRepCnt,nFlags & 0x1ff);
    // OnKeyDown(nChar,nRepCnt,nFlags);
  else
    CFrameWnd::OnSysKeyDown(nChar,nRepCnt,nFlags);
}

void CVTWindow::OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  if (nChar==VK_F10)
    OnKeyUp(nChar,nRepCnt,nFlags);
  else
    CFrameWnd::OnSysKeyUp(nChar,nRepCnt,nFlags);
}

void CVTWindow::OnTimer(UINT nIDEvent)
{
  POINT Point;
  WORD PortType;
  UINT T;

  if (nIDEvent==IdCaretTimer)
  {
    if (ts.NonblinkingCursor!=0)
    {
      T = GetCaretBlinkTime();
      SetCaretBlinkTime(T);
    }
    else
      ::KillTimer(HVTWin,IdCaretTimer);
    return;
  }
  else if (nIDEvent==IdScrollTimer)
  {
    GetCursorPos(&Point);
    ::ScreenToClient(HVTWin,&Point);
    DispAutoScroll(Point);
    if ((Point.x < 0) || (Point.x >= ScreenWidth) ||
	(Point.y < 0) || (Point.y >= ScreenHeight))
      ::PostMessage(HVTWin,WM_MOUSEMOVE,MK_LBUTTON,MAKELONG(Point.x,Point.y));
    return;
  } 
  else if (nIDEvent == IdCancelConnectTimer)
  {
	  // まだ接続が完了していなければ、ソケットを強制クローズ。
	  // CloseSocket()を呼びたいが、ここからは呼べないので、直接Win32APIをコールする。
	  if (!cv.Ready) {
	    closesocket(cv.s);
	    //::PostMessage(HVTWin, WM_USER_COMMNOTIFY, 0, FD_CLOSE);
	  }
  }

  ::KillTimer(HVTWin, nIDEvent);

  switch (nIDEvent) {
    case IdDelayTimer: cv.CanSend = TRUE; break;
    case IdProtoTimer: ProtoDlgTimeOut();
    case IdDblClkTimer: AfterDblClk = FALSE; break;
    case IdComEndTimer:
      if (! CommCanClose(&cv))
      { // wait if received data remains
	SetTimer(IdComEndTimer,1,NULL);
	break;
      }
      cv.Ready = FALSE;
      if (cv.TelFlag) EndTelnet();
	  PortType = cv.PortType;
      CommClose(&cv);
      SetDdeComReady(0);
      if ((PortType==IdTCPIP) &&
	  ((ts.PortFlag & PF_BEEPONCONNECT) != 0))
	MessageBeep(0);
      if ((PortType==IdTCPIP) &&
	  (ts.AutoWinClose>0) &&
	  ::IsWindowEnabled(HVTWin) &&
	  ((HTEKWin==NULL) || ::IsWindowEnabled(HTEKWin)) )
	OnClose();
      else
	ChangeTitle();
      break;
    case IdPrnStartTimer: PrnFileStart(); break;
    case IdPrnProcTimer: PrnFileDirectProc(); break;
  }
}

void CVTWindow::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	int Func;
	SCROLLINFO si;

	switch (nSBCode) {
	case SB_BOTTOM: Func = SCROLL_BOTTOM; break;
	case SB_ENDSCROLL: return;
	case SB_LINEDOWN: Func = SCROLL_LINEDOWN; break;
	case SB_LINEUP: Func = SCROLL_LINEUP; break;
	case SB_PAGEDOWN: Func = SCROLL_PAGEDOWN; break;
	case SB_PAGEUP: Func = SCROLL_PAGEUP; break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK: Func = SCROLL_POS; break;
	case SB_TOP: Func = SCROLL_TOP; break;
	default:
		return;
	}

	// スクロールレンジを 16bit から 32bit へ拡張した (2005.10.4 yutaka)
	ZeroMemory(&si, sizeof(SCROLLINFO));
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_TRACKPOS;
	if (::GetScrollInfo(HVTWin, SB_VERT, &si)) { // success
		nPos = si.nTrackPos;
	}

	DispVScroll(Func,nPos);
}

//<!--by AKASI
LONG CVTWindow::OnWindowPosChanging(UINT wParam, LONG lParam)
{
#ifdef ALPHABLEND_TYPE2
  if(BGEnable && BGNoCopyBits)
    ((WINDOWPOS*)lParam)->flags |= SWP_NOCOPYBITS;
#endif

  return CFrameWnd::DefWindowProc(WM_WINDOWPOSCHANGING,wParam,lParam);
}

LONG CVTWindow::OnSettingChange(UINT wParam, LONG lParam)
{
#ifdef ALPHABLEND_TYPE2
  BGOnSettingChange();
#endif
  return CFrameWnd::DefWindowProc(WM_SETTINGCHANGE,wParam,lParam);
}

LONG CVTWindow::OnEnterSizeMove(UINT wParam, LONG lParam)
{
#ifdef ALPHABLEND_TYPE2
  BGOnEnterSizeMove();
#endif
  return CFrameWnd::DefWindowProc(WM_ENTERSIZEMOVE,wParam,lParam);
}

LONG CVTWindow::OnExitSizeMove(UINT wParam, LONG lParam)
{
#ifdef ALPHABLEND_TYPE2
  BGOnExitSizeMove();
#endif
  return CFrameWnd::DefWindowProc(WM_EXITSIZEMOVE,wParam,lParam);
}
//-->

LONG CVTWindow::OnIMEComposition(UINT wParam, LONG lParam)
{
#ifdef TERATERM32
	HGLOBAL hstr;
	//LPSTR lpstr;
	wchar_t *lpstr;
	int Len;
	char *mbstr;
	int mlen;

	if (CanUseIME())
		hstr = GetConvString(wParam, lParam);
	else
		hstr = NULL;

	if (hstr!=NULL)
	{
		//lpstr = (LPSTR)GlobalLock(hstr);
		lpstr = (wchar_t *)GlobalLock(hstr);
		if (lpstr!=NULL)
		{
			mlen = wcstombs(NULL, lpstr, 0);
			mbstr = (char *)malloc(sizeof(char) * (mlen + 1));
			if (mbstr == NULL)
				goto skip;
			Len = wcstombs(mbstr, lpstr, mlen + 1);

			// add this string into text buffer of application
			Len = strlen(mbstr);
			if (Len==1)
			{
				switch (mbstr[0]) {
				case 0x20:
					if (ControlKey()) mbstr[0] = 0; /* Ctrl-Space */
					break;
				case 0x5c: // Ctrl-\ support for NEC-PC98
					if (ControlKey()) mbstr[0] = 0x1c;
					break;
				}
			}
			if (ts.LocalEcho>0)
				CommTextEcho(&cv,mbstr,Len);
			CommTextOut(&cv,mbstr,Len);

			free(mbstr);
			GlobalUnlock(hstr);
		}
skip:
		GlobalFree(hstr);
		return 0;
	}
#endif
	return CFrameWnd::DefWindowProc
		(WM_IME_COMPOSITION,wParam,lParam);
}

LONG CVTWindow::OnAccelCommand(UINT wParam, LONG lParam)
{
  switch (wParam) {
    case IdHold:
      if (TalkStatus==IdTalkKeyb)
      {
	Hold = ! Hold;
	CommLock(&ts,&cv,Hold);
      }
      break;
    case IdPrint:
      OnFilePrint();
      break;
    case IdBreak:
      OnControlSendBreak();
      break;
    case IdCmdEditCopy:
      OnEditCopy();
      break;
    case IdCmdEditPaste:
      OnEditPaste();
      break;
    case IdCmdEditPasteCR:
      OnEditPasteCR();
      break;
    case IdCmdEditCLS:
      OnEditClearScreen();
      break;
    case IdCmdEditCLB:
      OnEditClearBuffer();
      break;
    case IdCmdCtrlOpenTEK:
      OnControlOpenTEK();
      break;
    case IdCmdCtrlCloseTEK:
      OnControlCloseTEK();
      break;
    case IdCmdLineUp:
      OnVScroll(SB_LINEUP,0,NULL);
      break;
    case IdCmdLineDown:
      OnVScroll(SB_LINEDOWN,0,NULL);
      break;
    case IdCmdPageUp:
      OnVScroll(SB_PAGEUP,0,NULL);
      break;
    case IdCmdPageDown:
      OnVScroll(SB_PAGEDOWN,0,NULL);
      break;
    case IdCmdBuffTop:
      OnVScroll(SB_TOP,0,NULL);
      break;
    case IdCmdBuffBottom:
      OnVScroll(SB_BOTTOM,0,NULL);
      break;
    case IdCmdNextWin:
      SelectNextWin(HVTWin,1);
      break;
    case IdCmdPrevWin:
      SelectNextWin(HVTWin,-1);
      break;
    case IdCmdLocalEcho:
      if (ts.LocalEcho==0)
	ts.LocalEcho = 1;
      else
	ts.LocalEcho = 0;
      if (cv.Ready && cv.TelFlag && (ts.TelEcho>0))
	TelChangeEcho();
      break;
    case IdCmdDisconnect: // called by TTMACRO
      OnFileDisconnect();
      break;
    case IdCmdLoadKeyMap: // called by TTMACRO
      SetKeyMap();
      break;
    case IdCmdRestoreSetup: // called by TTMACRO
      RestoreSetup();
      break;
  }
  return 0;
}

LONG CVTWindow::OnChangeMenu(UINT wParam, LONG lParam)
{
  HMENU SysMenu;
  BOOL Show, B1, B2;

  Show = (ts.PopupMenu==0) && (ts.HideTitle==0);
  if (Show != (MainMenu!=NULL))
  {
    if (! Show)
    {
      if (WinMenu!=NULL)
	DestroyMenu(WinMenu);
      WinMenu = NULL;
      DestroyMenu(MainMenu);
      MainMenu = NULL;
    }
    else
      InitMenu(&MainMenu);

    AdjustSize = TRUE;
    ::SetMenu(HVTWin, MainMenu);
    ::DrawMenuBar(HVTWin);
  }

  B1 = ((ts.MenuFlag & MF_SHOWWINMENU)!=0);
  B2 = (WinMenu!=NULL);
  if ((MainMenu!=NULL) &&
	  (B1 != B2))
  {
    if (WinMenu==NULL)
    {
      WinMenu = CreatePopupMenu();
#ifdef I18N
      strcpy(ts.UIMsg, "&Window");
      get_lang_msg("MENU_WINDOW", ts.UIMsg, ts.UILanguageFile);
      ::InsertMenu(MainMenu,ID_HELPMENU,
	MF_STRING | MF_ENABLED |
	MF_POPUP | MF_BYPOSITION,
	(int)WinMenu, ts.UIMsg);
#else
      ::InsertMenu(MainMenu,ID_HELPMENU,
	MF_STRING | MF_ENABLED |
	MF_POPUP | MF_BYPOSITION,
	(int)WinMenu, "&Window");
#endif
    }
    else {
      RemoveMenu(MainMenu,ID_HELPMENU,MF_BYPOSITION);
      DestroyMenu(WinMenu);
      WinMenu = NULL;
    }
    ::DrawMenuBar(HVTWin);
  }

  ::GetSystemMenu(HVTWin,TRUE);
  if ((! Show) && ((ts.MenuFlag & MF_NOSHOWMENU)==0))
  {
    SysMenu = ::GetSystemMenu(HVTWin,FALSE);
    AppendMenu(SysMenu, MF_SEPARATOR, 0, NULL);
#ifdef I18N
    strcpy(ts.UIMsg, "Show menu &bar");
    get_lang_msg("MENU_SHOW_MENUBAR", ts.UIMsg, ts.UILanguageFile);
    AppendMenu(SysMenu, MF_STRING, ID_SHOWMENUBAR, ts.UIMsg);
#else
    AppendMenu(SysMenu, MF_STRING, ID_SHOWMENUBAR, "Show menu &bar");
#endif
  }
  return 0;
}

LONG CVTWindow::OnChangeTBar(UINT wParam, LONG lParam)
{
  BOOL TBar;
  DWORD Style,ExStyle;
  HMENU SysMenu;

  Style = GetWindowLong (HVTWin, GWL_STYLE);
   ExStyle = GetWindowLong (HVTWin, GWL_EXSTYLE);
  TBar = ((Style & WS_SYSMENU)!=0);
  if (TBar == (ts.HideTitle==0)) return 0;

#ifndef WINDOW_MAXMIMUM_ENABLED
  if (ts.HideTitle>0)
    Style = Style & ~(WS_SYSMENU | WS_CAPTION |
		      WS_MINIMIZEBOX) | WS_BORDER | WS_POPUP;
  else
    Style = Style & ~WS_POPUP | WS_SYSMENU | WS_CAPTION |
	    WS_MINIMIZEBOX;
#else
  if (ts.HideTitle>0)
  {
    Style = Style & ~(WS_SYSMENU | WS_CAPTION |
	WS_MINIMIZEBOX | WS_MAXIMIZEBOX) | WS_BORDER | WS_POPUP;

#ifdef ALPHABLEND_TYPE2
     if(BGNoFrame)
     {
       Style   &= ~(WS_THICKFRAME | WS_BORDER);
       ExStyle &= ~WS_EX_CLIENTEDGE;
     }else{
       ExStyle |=  WS_EX_CLIENTEDGE;
     }
#endif
  }
  else {
    Style = Style & ~WS_POPUP | WS_SYSMENU | WS_CAPTION |
	    WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_BORDER;

       ExStyle |=  WS_EX_CLIENTEDGE;
  }
#endif

  AdjustSize = TRUE;
  SetWindowLong(HVTWin, GWL_STYLE, Style);
#ifdef ALPHABLEND_TYPE2
  SetWindowLong(HVTWin, GWL_EXSTYLE, ExStyle);
#endif
  ::SetWindowPos(HVTWin, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |
		 SWP_NOZORDER | SWP_FRAMECHANGED);
  ::ShowWindow(HVTWin, SW_SHOW);

  if ((ts.HideTitle==0) && (MainMenu==NULL) &&
      ((ts.MenuFlag & MF_NOSHOWMENU) == 0))
  {
    SysMenu = ::GetSystemMenu(HVTWin,FALSE);
    AppendMenu(SysMenu, MF_SEPARATOR, 0, NULL);
#ifdef I18N
    strcpy(ts.UIMsg, "Show menu &bar");
    get_lang_msg("MENU_SHOW_MENUBAR", ts.UIMsg, ts.UILanguageFile);
    AppendMenu(SysMenu, MF_STRING, ID_SHOWMENUBAR, ts.UIMsg);
#else
    AppendMenu(SysMenu, MF_STRING, ID_SHOWMENUBAR, "Show menu &bar");
#endif
  }
  return 0;
}

LONG CVTWindow::OnCommNotify(UINT wParam, LONG lParam)
{
  switch (LOWORD(lParam)) {
    case FD_READ:  // TCP/IP
#ifndef TERATERM32
    case CN_EVENT: // Win 3.1 serial
#endif
      CommProcRRQ(&cv);
      break;
    case FD_CLOSE:
      Connecting = FALSE;
      TCPIPClosed = TRUE;
      // disable transmition
      cv.OutBuffCount = 0;
      SetTimer(IdComEndTimer,1,NULL);
      break;
  }
  return 0;
}

LONG CVTWindow::OnCommOpen(UINT wParam, LONG lParam)
{
#ifdef I18N
  CommStart(&cv,lParam,&ts);
#else
  CommStart(&cv,lParam);
#endif
#ifdef INET6
  if (ts.PortType == IdTCPIP && cv.RetryWithOtherProtocol == TRUE)
    Connecting = TRUE;
  else
    Connecting = FALSE;
#else
  Connecting = FALSE;
#endif /* INET6 */
  ChangeTitle();
  if (! cv.Ready) return 0;

  /* ログ採取が有効で開始していなければ開始する (2006.9.18 maya) */
  if ((ts.LogFN[0]!=0) && (LogVar==NULL) && NewFileVar(&LogVar))
  {
    LogVar->DirLen = 0;
    strcpy(LogVar->FullName,ts.LogFN);
    LogStart();
  }

  if ((ts.PortType==IdTCPIP) &&
      ((ts.PortFlag & PF_BEEPONCONNECT) != 0))
    MessageBeep(0);

  if (cv.PortType==IdTCPIP)
  {
    InitTelnet();

    if ((cv.TelFlag) && (ts.TCPPort==ts.TelPort))
    { // Start telnet option negotiation from this side
      //   if telnet flag is set and port#==default telnet port# (23)
      TelEnableMyOpt(TERMTYPE);

      TelEnableHisOpt(SGA);

      TelEnableMyOpt(SGA);

      if (ts.TelEcho>0)
	TelChangeEcho();
      else
	TelEnableHisOpt(ECHO);

      TelEnableMyOpt(NAWS);
      if (ts.TelBin>0)
      {
	TelEnableMyOpt(BINARY);
	TelEnableHisOpt(BINARY);
      }
    }
    else {
      if (ts.TCPCRSend>0)
      {
	ts.CRSend = ts.TCPCRSend;
	cv.CRSend = ts.TCPCRSend;
      }
      if (ts.TCPLocalEcho>0)
	ts.LocalEcho = ts.TCPLocalEcho;
    }
  }

  if (DDELog || FileLog)
  {
    if (! CreateLogBuf())
    {
      if (DDELog) EndDDE();
      if (FileLog) FileTransEnd(OpLog);
    }
  }

  if (BinLog)
  {
    if (! CreateBinBuf())
      FileTransEnd(OpLog);
  }

  SetDdeComReady(1);

  return 0;
}

LONG CVTWindow::OnCommStart(UINT wParam, LONG lParam)
{
  // 自動接続が無効のときも接続ダイアログを出すようにした (2006.9.15 maya)
  if (((ts.PortType!=IdSerial) && (ts.HostName[0]==0)) ||
	  ((ts.PortType==IdSerial) && (ts.ComAutoConnect == FALSE))) {
    OnFileNewConnection();
  }
  else {
    Connecting = TRUE;
    ChangeTitle();
    CommOpen(HVTWin,&ts,&cv);
  }
  return 0;
}

LONG CVTWindow::OnDdeEnd(UINT wParam, LONG lParam)
{
  EndDDE();
  if (CloseTT) OnClose();
  return 0;
}

LONG CVTWindow::OnDlgHelp(UINT wParam, LONG lParam)
{
  OpenHelp(HVTWin,HELP_CONTEXT,HelpId);
  return 0;
}

LONG CVTWindow::OnFileTransEnd(UINT wParam, LONG lParam)
{
  FileTransEnd(wParam);
  return 0;
}

LONG CVTWindow::OnGetSerialNo(UINT wParam, LONG lParam)
{
  return (LONG)SerialNo;
}

LONG CVTWindow::OnKeyCode(UINT wParam, LONG lParam)
{
  KeyCodeSend(wParam,(WORD)lParam);
  return 0;
}

LONG CVTWindow::OnProtoEnd(UINT wParam, LONG lParam)
{
  ProtoDlgCancel();
  return 0;
}

void CVTWindow::OnFileNewConnection()
{
//  char Command[MAXPATHLEN], Command2[MAXPATHLEN];
	char Command[MAXPATHLEN + HostNameMaxLength], Command2[MAXPATHLEN + HostNameMaxLength]; // yutaka
  TGetHNRec GetHNRec; /* record for dialog box */

  if (Connecting) return;

  HelpId = HlpFileNewConnection;
  GetHNRec.SetupFN = ts.SetupFName;
  GetHNRec.PortType = ts.PortType;
  GetHNRec.Telnet = ts.Telnet;
  GetHNRec.TelPort = ts.TelPort;
  GetHNRec.TCPPort = ts.TCPPort;
#ifdef INET6
  GetHNRec.ProtocolFamily = ts.ProtocolFamily;
#endif /* INET6 */
  GetHNRec.ComPort = ts.ComPort;
  GetHNRec.MaxComPort = ts.MaxComPort;

#ifdef TERATERM32
  strcpy(Command,"ttermpro ");
#else
  strcpy(Command,"teraterm ");
#endif
  GetHNRec.HostName = &Command[9];

  if (! LoadTTDLG()) return;
  if ((*GetHostName)(HVTWin,&GetHNRec))
  {
    if ((GetHNRec.PortType==IdTCPIP) &&
        (ts.HistoryList>0) &&
        LoadTTSET())
    {
      (*AddHostToList)(ts.SetupFName,GetHNRec.HostName);
      FreeTTSET();
    }

    if (! cv.Ready)
    {
      ts.PortType = GetHNRec.PortType;
      ts.Telnet = GetHNRec.Telnet;
      ts.TCPPort = GetHNRec.TCPPort;
#ifdef INET6
      ts.ProtocolFamily = GetHNRec.ProtocolFamily;
#endif /* INET6 */
      ts.ComPort = GetHNRec.ComPort;

      if ((GetHNRec.PortType==IdTCPIP) &&
	  LoadTTSET())
      {
	(*ParseParam)(Command, &ts, NULL);
	FreeTTSET();
      }
      SetKeyMap();
      if (ts.MacroFN[0]!=0)
      {
	RunMacro(ts.MacroFN,TRUE);
	ts.MacroFN[0] = 0;
      }
      else {
	Connecting = TRUE;
	ChangeTitle();
	CommOpen(HVTWin,&ts,&cv);
      }
      ResetSetup();
    }
    else {
      if (GetHNRec.PortType==IdSerial)
      {
	Command[8] = 0;
	strcat(Command," /C=");
	uint2str(GetHNRec.ComPort,&Command[strlen(Command)],2);
      }
      else {
	strcpy(Command2, &Command[9]);
	Command[9] = 0;
	if (GetHNRec.Telnet==0)
	  strcat(Command," /T=0");
	else
	  strcat(Command," /T=1");
	if (GetHNRec.TCPPort<65535)
	{
	  strcat(Command," /P=");
	  uint2str(GetHNRec.TCPPort,&Command[strlen(Command)],5);
	}
#ifdef INET6
        /********************************/
        /* ここにプロトコル処理を入れる */
        /********************************/
        if (GetHNRec.ProtocolFamily == AF_INET) {
          strcat(Command," /4");
        } else if (GetHNRec.ProtocolFamily == AF_INET6) {
          strcat(Command," /6");
        }
#endif /* INET6 */
	strcat(Command," ");
	strncat(Command, Command2, sizeof(Command)-1-strlen(Command));
      }
      TTXSetCommandLine(Command, sizeof(Command), &GetHNRec); /* TTPLUG */
      WinExec(Command,SW_SHOW);
    }
  }
  else {/* canceled */
    if (! cv.Ready)
      SetDdeComReady(0);
  }

  FreeTTDLG();
}


// すでに開いているセッションの複製を作る
// (2004.12.6 yutaka)
void CVTWindow::OnDuplicateSession()
{
	char Command[MAX_PATH] = "notepad.exe";
	char *exec = "ttermpro";
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	
	// 現在の設定内容を共有メモリへコピーしておく
	CopyTTSetToShmem(&ts);

	if (ts.TCPPort == 23) { // telnet
		_snprintf(Command, sizeof(Command), "%s %s:%d /DUPLICATE /nossh", 
			exec, ts.HostName, ts.TCPPort);

	} else if (ts.TCPPort == 22) { // SSH
		// ここの処理は TTSSH 側にやらせるべき (2004.12.7 yutaka)
		// TTSSH側でのオプション生成を追加。(2005.4.8 yutaka)
		_snprintf(Command, sizeof(Command), "%s %s:%d /DUPLICATE", 
			exec, ts.HostName, ts.TCPPort);

		TTXSetCommandLine(Command, sizeof(Command), NULL); /* TTPLUG */

	} else {
		// 接続先が localhost ならCygwin接続の複製を行う。
		// (2005.10.15 yutaka)
		if (strcmp(ts.HostName, "127.0.0.1") == 0 || 
			strcmp(ts.HostName, "localhost") == 0) {
			OnCygwinConnection();
		}

		return;

	}

	memset(&si, 0, sizeof(si));
	GetStartupInfo(&si);
	memset(&pi, 0, sizeof(pi));

	if (CreateProcess(
			NULL, 
			Command, 
			NULL, NULL, FALSE, 0,
			NULL, NULL,
			&si, &pi) == 0) {
		char buf[80];
#ifdef I18N
		char uimsg[MAX_UIMSG];
		strcpy(uimsg, "ERROR");
		get_lang_msg("MSG_ERROR", uimsg, ts.UILanguageFile);
		strcpy(ts.UIMsg, "Can't execute TeraTerm. (%d)");
		get_lang_msg("MSG_USE_IME_ERROR", ts.UIMsg, ts.UILanguageFile);
		_snprintf(buf, sizeof(buf), ts.UIMsg, GetLastError());
		::MessageBox(NULL, buf, uimsg, MB_OK | MB_ICONWARNING);
#else
		_snprintf(buf, sizeof(buf), "Can't execute TeraTerm. (%d)", GetLastError());
		::MessageBox(NULL, buf, "ERROR", MB_OK | MB_ICONWARNING);
#endif
	}

}


//
// Connect to local cygwin
//
void CVTWindow::OnCygwinConnection()
{
	char file[MAX_PATH], buf[1024];
	char c, *envptr;
	char *exename = "cygterm.exe";
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
#ifdef I18N
	char uimsg[MAX_UIMSG];
#endif

	envptr = getenv("PATH");
	if (strstr(envptr, "cygwin\\bin") != NULL) {
		goto found_path;
	}

	_snprintf(file, MAX_PATH, "%s\\bin", ts.CygwinDirectory);
	if (GetFileAttributes(file) == -1) { // open error
		for (c = 'C' ; c <= 'Z' ; c++) {
			file[0] = c;
			if (GetFileAttributes(file) != -1) { // open success
				goto found_dll;
			}
		}
#ifdef I18N
		strcpy(uimsg, "ERROR");
		get_lang_msg("MSG_ERROR", uimsg, ts.UILanguageFile);
		strcpy(ts.UIMsg, "Can't find Cygwin directory.");
		get_lang_msg("MSG_FIND_CYGTERM_DIR_ERROR", ts.UIMsg, ts.UILanguageFile);
		::MessageBox(NULL, ts.UIMsg, uimsg, MB_OK | MB_ICONWARNING);
#else
		::MessageBox(NULL, "Can't find Cygwin directory.", "ERROR", MB_OK | MB_ICONWARNING);
#endif
		return;
	}
found_dll:;
	if (envptr != NULL) {
		_snprintf(buf, sizeof(buf), "PATH=%s;%s", file, envptr);
	} else {
		_snprintf(buf, sizeof(buf), "PATH=%s", file);
	}
	_putenv(buf);

found_path:;
	memset(&si, 0, sizeof(si));
	GetStartupInfo(&si);
	memset(&pi, 0, sizeof(pi));

	if (CreateProcess(
			NULL, 
			exename, 
			NULL, NULL, FALSE, 0,
			NULL, NULL,
			&si, &pi) == 0) {
#ifdef I18N
		strcpy(uimsg, "ERROR");
		get_lang_msg("MSG_ERROR", uimsg, ts.UILanguageFile);
		strcpy(ts.UIMsg, "Can't execute Cygterm.");
		get_lang_msg("MSG_EXEC_CYGTERM_ERROR", ts.UIMsg, ts.UILanguageFile);
		::MessageBox(NULL, ts.UIMsg, uimsg, MB_OK | MB_ICONWARNING);
#else
		::MessageBox(NULL, "Can't execute Cygterm.", "ERROR", MB_OK | MB_ICONWARNING);
#endif
	}
}


//
// TeraTerm Menuの起動
//
void CVTWindow::OnTTMenuLaunch()
{
	char *exename = "ttpmenu.exe";
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	memset(&si, 0, sizeof(si));
	GetStartupInfo(&si);
	memset(&pi, 0, sizeof(pi));

	if (CreateProcess(
			NULL, 
			exename, 
			NULL, NULL, FALSE, 0,
			NULL, NULL,
			&si, &pi) == 0) {
		char buf[80];
#ifdef I18N
		char uimsg[MAX_UIMSG];
		strcpy(uimsg, "ERROR");
		get_lang_msg("MSG_ERROR", uimsg, ts.UILanguageFile);
		strcpy(ts.UIMsg, "Can't execute TeraTerm Menu. (%d)");
		get_lang_msg("MSG_EXEC_TTMENU_ERROR", ts.UIMsg, ts.UILanguageFile);
		_snprintf(buf, sizeof(buf), ts.UIMsg, GetLastError());
		::MessageBox(NULL, buf, uimsg, MB_OK | MB_ICONWARNING);
#else
		_snprintf(buf, sizeof(buf), "Can't execute TeraTerm Menu. (%d)", GetLastError());
		::MessageBox(NULL, buf, "ERROR", MB_OK | MB_ICONWARNING);
#endif
	}
}


//
// LogMeInの起動
//
// URL: http://www.neocom.ca/freeware/LogMeIn/
//
void CVTWindow::OnLogMeInLaunch()
{
	// LogMeIn.exe -> LogMeTT.exe へリネーム (2005.2.21 yutaka)
	char *exename = "LogMeTT.exe";
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	memset(&si, 0, sizeof(si));
	GetStartupInfo(&si);
	memset(&pi, 0, sizeof(pi));

	if (CreateProcess(
			NULL, 
			exename, 
			NULL, NULL, FALSE, 0,
			NULL, NULL,
			&si, &pi) == 0) {
		char buf[80];
#ifdef I18N
		char uimsg[MAX_UIMSG];
		strcpy(uimsg, "ERROR");
		get_lang_msg("MSG_ERROR", uimsg, ts.UILanguageFile);
		strcpy(ts.UIMsg, "Can't execute LogMeTT. (%d)");
		get_lang_msg("MSG_EXEC_LOGMETT_ERROR", ts.UIMsg, ts.UILanguageFile);
		_snprintf(buf, sizeof(buf), ts.UIMsg, GetLastError());
		::MessageBox(NULL, buf, uimsg, MB_OK | MB_ICONWARNING);
#else
		_snprintf(buf, sizeof(buf), "Can't execute LogMeTT. (%d)", GetLastError());
		::MessageBox(NULL, buf, "ERROR", MB_OK | MB_ICONWARNING);
#endif
	}
}


void CVTWindow::OnFileLog()
{
  HelpId = HlpFileLog;
  LogStart();
}


static LRESULT CALLBACK OnCommentDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	char buf[256];
	UINT ret;
#ifdef I18N
	LOGFONT logfont;
	HFONT font;
#endif

    switch (msg) {
        case WM_INITDIALOG:
			//SetDlgItemText(hDlgWnd, IDC_EDIT_COMMENT, "サンプル");
			// エディットコントロールにフォーカスをあてる
			SetFocus(GetDlgItem(hDlgWnd, IDC_EDIT_COMMENT));

#ifdef I18N
			font = (HFONT)SendMessage(hDlgWnd, WM_GETFONT, 0, 0);
			GetObject(font, sizeof(LOGFONT), &logfont);
			if (get_lang_font("DLG_SYSTEM_FONT", hDlgWnd, &logfont, &DlgCommentFont, ts.UILanguageFile)) {
				SendDlgItemMessage(hDlgWnd, IDC_EDIT_COMMENT, WM_SETFONT, (WPARAM)DlgCommentFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDOK, WM_SETFONT, (WPARAM)DlgCommentFont, MAKELPARAM(TRUE,0));
			}
			else {
				DlgCommentFont = NULL;
			}

			GetWindowText(hDlgWnd, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("DLG_COMMENT_TITLE", ts.UIMsg, ts.UILanguageFile);
			SetWindowText(hDlgWnd, ts.UIMsg);
			GetDlgItemText(hDlgWnd, IDOK, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("BTN_OK", ts.UIMsg, ts.UILanguageFile);
			SetDlgItemText(hDlgWnd, IDOK, ts.UIMsg);
#endif

			return FALSE;

        case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDOK:
					memset(buf, 0, sizeof(buf));
					ret = GetDlgItemText(hDlgWnd, IDC_EDIT_COMMENT, buf, sizeof(buf) - 1);
					if (ret > 0) { // テキスト取得成功
						//buf[sizeof(buf) - 1] = '\0';  // null-terminate
						CommentLogToFile(buf, ret);
					}
#ifdef I18N
      if (DlgCommentFont != NULL) {
        DeleteObject(DlgCommentFont);
      }
#endif
                    EndDialog(hDlgWnd, IDOK);
                    break;
                default:
                    return FALSE;
            }
        case WM_CLOSE:
		    EndDialog(hDlgWnd, 0);
			return TRUE;

        default:
            return FALSE;
    }
    return TRUE;
}

void CVTWindow::OnCommentToLog()
{
	DWORD ret;

	// ログファイルへコメントを追加する (2004.8.6 yutaka)
	ret = DialogBox(hInst, MAKEINTRESOURCE(IDD_COMMENT_DIALOG), HVTWin, (DLGPROC)OnCommentDlgProc);
	if (ret == 0 || ret == -1) {
		ret = GetLastError();
	}

}


// ログの閲覧 (2005.1.29 yutaka)
void CVTWindow::OnViewLog()
{
	char command[MAX_PATH];
	char *file;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	if (LogVar == NULL || !LogVar->FileOpen) {
		return;
	}

	file = LogVar->FullName;

	memset(&si, 0, sizeof(si));
	GetStartupInfo(&si);
	memset(&pi, 0, sizeof(pi));

	_snprintf(command, sizeof(command), "%s %s", ts.ViewlogEditor, file);

	if (CreateProcess(
			NULL, 
			command, 
			NULL, NULL, FALSE, 0,
			NULL, NULL,
			&si, &pi) == 0) {
		char buf[80];
#ifdef I18N
		char uimsg[MAX_UIMSG];
		strcpy(uimsg, "ERROR");
		get_lang_msg("MSG_ERROR", uimsg, ts.UILanguageFile);
		strcpy(ts.UIMsg, "Can't view logging file. (%d)");
		get_lang_msg("MSG_VIEW_LOGFILE_ERROR", ts.UIMsg, ts.UILanguageFile);
		_snprintf(buf, sizeof(buf), ts.UIMsg, GetLastError());
		::MessageBox(NULL, buf, uimsg, MB_OK | MB_ICONWARNING);
#else
		_snprintf(buf, sizeof(buf), "Can't view logging file. (%d)", GetLastError());
		::MessageBox(NULL, buf, "ERROR", MB_OK | MB_ICONWARNING);
#endif
	}
}


// ログの再生 (2006.12.13 yutaka)
void CVTWindow::OnReplayLog()
{
    OPENFILENAME ofn;
	OSVERSIONINFO osvi;
    char szFile[MAX_PATH];
	char Command[MAX_PATH] = "notepad.exe";
	char *exec = "ttermpro";
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
#ifdef I18N
    char uimsg[MAX_UIMSG];
#endif

	// バイナリモードで採取したログファイルを選択する
	memset(&ofn, 0, sizeof(OPENFILENAME));
	memset(szFile, 0, sizeof(szFile));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT &&
		osvi.dwMajorVersion >= 5) {
		ofn.lStructSize = sizeof(OPENFILENAME);
	}
	else {
		ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	}
    ofn.hwndOwner = HVTWin;
#ifdef I18N
	strncpy(ts.UIMsg, "all(*.*)\\0*.*\\0\\0", sizeof(ts.UIMsg));
	get_lang_msg("FILEDLG_OPEN_LOGFILE_FILTER", ts.UIMsg, ts.UILanguageFile);
    ofn.lpstrFilter = ts.UIMsg;
#else
    ofn.lpstrFilter = "all(*.*)\0*.*\0\0";
#endif
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "log";
#ifdef I18N
    strcpy(uimsg, "Select replay log file with binary mode");
    get_lang_msg("FILEDLG_OPEN_LOGFILE_TITLE", uimsg, ts.UILanguageFile);
	ofn.lpstrTitle = uimsg;
#else
    ofn.lpstrTitle = "Select replay log file with binary mode";
#endif
    if(GetOpenFileName(&ofn) == 0) 
        return;


	// "/R"オプション付きでTeraTermを起動する（ログが再生される）
	_snprintf(Command, sizeof(Command), "%s /R=\"%s\"", 
		exec, szFile);

	memset(&si, 0, sizeof(si));
	GetStartupInfo(&si);
	memset(&pi, 0, sizeof(pi));

	if (CreateProcess(
			NULL, 
			Command, 
			NULL, NULL, FALSE, 0,
			NULL, NULL,
			&si, &pi) == 0) {
		char buf[80];
#ifdef I18N
		strcpy(uimsg, "ERROR");
		get_lang_msg("MSG_ERROR", uimsg, ts.UILanguageFile);
		strcpy(ts.UIMsg, "Can't execute TeraTerm. (%d)");
		get_lang_msg("MSG_EXEC_TT_ERROR", ts.UIMsg, ts.UILanguageFile);
		_snprintf(buf, sizeof(buf), ts.UIMsg, GetLastError());
		::MessageBox(NULL, buf, uimsg, MB_OK | MB_ICONWARNING);
#else
		_snprintf(buf, sizeof(buf), "Can't execute TeraTerm. (%d)", GetLastError());
		::MessageBox(NULL, buf, "ERROR", MB_OK | MB_ICONWARNING);
#endif
	}
}


void CVTWindow::OnFileSend()
{
  HelpId = HlpFileSend;
  FileSendStart();
}

void CVTWindow::OnFileKermitRcv()
{
  KermitStart(IdKmtReceive);	
}

void CVTWindow::OnFileKermitGet()
{
  HelpId = HlpFileKmtGet;
  KermitStart(IdKmtGet);
}

void CVTWindow::OnFileKermitSend()
{
  HelpId = HlpFileKmtSend;
  KermitStart(IdKmtSend);
}

void CVTWindow::OnFileKermitFinish()
{
  KermitStart(IdKmtFinish);
}

void CVTWindow::OnFileXRcv()
{
  HelpId = HlpFileXmodemRecv;
  XMODEMStart(IdXReceive);
}

void CVTWindow::OnFileXSend()
{
  HelpId = HlpFileXmodemSend;
  XMODEMStart(IdXSend);
}

void CVTWindow::OnFileZRcv()
{
  ZMODEMStart(IdZReceive);	
}

void CVTWindow::OnFileZSend()
{
  HelpId = HlpFileZmodemSend;
  ZMODEMStart(IdZSend);
}

void CVTWindow::OnFileBPRcv()
{
  BPStart(IdBPReceive); 
}

void CVTWindow::OnFileBPSend()
{
  HelpId = HlpFileBPlusSend;
  BPStart(IdBPSend);
}

void CVTWindow::OnFileQVRcv()
{
  QVStart(IdQVReceive); 
}

void CVTWindow::OnFileQVSend()
{
  HelpId = HlpFileQVSend;
  QVStart(IdQVSend);
}

void CVTWindow::OnFileChangeDir()
{
  HelpId = HlpFileChangeDir;
  if (! LoadTTDLG()) return;
  (*ChangeDirectory)(HVTWin,ts.FileDir);
  FreeTTDLG();
}

void CVTWindow::OnFilePrint()
{
  HelpId = HlpFilePrint;
  BuffPrint(FALSE);
}

void CVTWindow::OnFileDisconnect()
{
  if (! cv.Ready) return;
#ifdef I18N
  strcpy(ts.UIMsg, "Disconnect?");
  get_lang_msg("MSG_DISCONNECT_CONF", ts.UIMsg, ts.UILanguageFile);
#endif
  if ((cv.PortType==IdTCPIP) &&
      ((ts.PortFlag & PF_CONFIRMDISCONN) != 0) &&
#ifdef I18N
      (::MessageBox(HVTWin,ts.UIMsg,"Tera Term",
#else
      (::MessageBox(HVTWin,"Disconnect?","Tera Term",
#endif
	   MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON2)==IDCANCEL))
    return;
  ::PostMessage(HVTWin, WM_USER_COMMNOTIFY, 0, FD_CLOSE);
}

void CVTWindow::OnFileExit()
{
  OnClose();	
}

void CVTWindow::OnEditCopy()
{
  // copy selected text to clipboard
  BuffCBCopy(FALSE);
}

void CVTWindow::OnEditCopyTable()
{
  // copy selected text to clipboard in Excel format
  BuffCBCopy(TRUE);
}

void CVTWindow::OnEditPaste()
{
  CBStartPaste(HVTWin,FALSE,0,NULL,0);	
}

void CVTWindow::OnEditPasteCR()
{
  CBStartPaste(HVTWin,TRUE,0,NULL,0);	
}

void CVTWindow::OnEditClearScreen()
{
  LockBuffer();
  BuffClearScreen();
  if ((StatusLine>0) && (CursorY==NumOfLines-1))
    MoveCursor(0,CursorY);
  else
    MoveCursor(0,0);
  BuffUpdateScroll();
  BuffSetCaretWidth();
  UnlockBuffer();
}

void CVTWindow::OnEditClearBuffer()
{
  LockBuffer();
  ClearBuffer();
  UnlockBuffer();
}

void CVTWindow::OnSelectAllBuffer()
{
	// バッファの全選択
	POINT p = {0, 0};

	ButtonDown(p, IdLeftButton);
	BuffAllSelect();
    ButtonUp(FALSE);
	ChangeSelectRegion();
}



// Additional settingsで使うタブコントロールの親ハンドル
static HWND gTabControlParent;

//
// cf. http://homepage2.nifty.com/DSS/VCPP/API/SHBrowseForFolder.htm
//
static void doSelectFolder(HWND hWnd, char *path, int pathlen)
{
	BROWSEINFO      bi;
	LPSTR           lpBuffer;
	LPITEMIDLIST    pidlRoot;      // ブラウズのルートPIDL
	LPITEMIDLIST    pidlBrowse;    // ユーザーが選択したPIDL
	LPMALLOC        lpMalloc = NULL;

	HRESULT hr = SHGetMalloc(&lpMalloc);
	if (FAILED(hr)) 
		return;

	// ブラウズ情報受け取りバッファ領域の確保
	if ((lpBuffer = (LPSTR) lpMalloc->Alloc(_MAX_PATH)) == NULL) {
		return;
	}
	// ダイアログ表示時のルートフォルダのPIDLを取得
	// ※以下はデスクトップをルートとしている。デスクトップをルートとする
	//   場合は、単に bi.pidlRoot に０を設定するだけでもよい。その他の特
	//   殊フォルダをルートとする事もできる。詳細はSHGetSpecialFolderLoca
	//   tionのヘルプを参照の事。
	if (!SUCCEEDED(SHGetSpecialFolderLocation(  hWnd,
		CSIDL_DESKTOP,
		&pidlRoot))) { 
			lpMalloc->Free(lpBuffer);
			return;
	}

	// BROWSEINFO構造体の初期値設定
	// ※BROWSEINFO構造体の各メンバの詳細説明もヘルプを参照
	bi.hwndOwner = hWnd;
	bi.pidlRoot = pidlRoot;
	bi.pszDisplayName = lpBuffer;
#ifdef I18N
	strcpy(ts.UIMsg, "select folder");
	get_lang_msg("DIRDLG_CYGTERM_DIR_TITLE", ts.UIMsg, ts.UILanguageFile);
	bi.lpszTitle = ts.UIMsg;
#else
	bi.lpszTitle = "select folder";
#endif
	bi.ulFlags = 0;
	bi.lpfn = 0;
	bi.lParam = 0;
	// フォルダ選択ダイアログの表示 
	pidlBrowse = SHBrowseForFolder(&bi);
	if (pidlBrowse != NULL) {  
		// PIDL形式の戻り値のファイルシステムのパスに変換
		if (SHGetPathFromIDList(pidlBrowse, lpBuffer)) {
			// 取得成功
			strncpy(path, lpBuffer, pathlen);
		}
		// SHBrowseForFolderの戻り値PIDLを解放
		lpMalloc->Free(pidlBrowse);
	}
	// クリーンアップ処理
	lpMalloc->Free(pidlRoot); 
	lpMalloc->Free(lpBuffer);
	lpMalloc->Release();
}


static void split_buffer(char *buffer, int delimiter, char **head, char **body)
{
	char *p = buffer;

	*head = *body = NULL;

	while (*p) {
		if (isspace(*p)) {
			*p = '\0'; 
			*head = buffer;
		}
		if (*p == delimiter) {
			p++;
			break;
		}
		p++;
	}

	// skip space
	while (*p && isspace(*p)) 
		p++;

	*body = p;
}


// Cygwin tab
static LRESULT CALLBACK OnTabSheetCygwinProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	HWND hWnd;
	char *cfgfile = "cygterm.cfg"; // CygTerm configuration file
	FILE *fp;
	typedef struct cygterm {
		char term[128];
		char term_type[80];
		char port_start[80];
		char port_range[80];
		char shell[80];
		char env1[128];
		char env2[128];
	} cygterm_t;
	cygterm_t settings;
	char buf[256], *head, *body;
#ifdef I18N
	LOGFONT logfont;
	HFONT font;
#endif

    switch (msg) {
        case WM_INITDIALOG:

#ifdef I18N
			font = (HFONT)SendMessage(hDlgWnd, WM_GETFONT, 0, 0);
			GetObject(font, sizeof(LOGFONT), &logfont);
			if (get_lang_font("DLG_TAHOMA_FONT", hDlgWnd, &logfont, &DlgCygwinFont, ts.UILanguageFile)) {
				SendDlgItemMessage(hDlgWnd, IDC_CYGWIN_PATH_LABEL, WM_SETFONT, (WPARAM)DlgCygwinFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_CYGWIN_PATH, WM_SETFONT, (WPARAM)DlgCygwinFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_SELECT_FILE, WM_SETFONT, (WPARAM)DlgCygwinFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_CYGWIN, WM_SETFONT, (WPARAM)DlgCygwinFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_TERM_LABEL, WM_SETFONT, (WPARAM)DlgCygwinFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_TERM_EDIT, WM_SETFONT, (WPARAM)DlgCygwinFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_TERMTYPE_LABEL, WM_SETFONT, (WPARAM)DlgCygwinFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_TERM_TYPE, WM_SETFONT, (WPARAM)DlgCygwinFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_PORTSTART_LABEL, WM_SETFONT, (WPARAM)DlgCygwinFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_PORT_START, WM_SETFONT, (WPARAM)DlgCygwinFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_PORTRANGE_LABEL, WM_SETFONT, (WPARAM)DlgCygwinFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_PORT_RANGE, WM_SETFONT, (WPARAM)DlgCygwinFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_SHELL_LABEL, WM_SETFONT, (WPARAM)DlgCygwinFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_SHELL, WM_SETFONT, (WPARAM)DlgCygwinFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_ENV1_LABEL, WM_SETFONT, (WPARAM)DlgCygwinFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_ENV1, WM_SETFONT, (WPARAM)DlgCygwinFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_ENV2_LABEL, WM_SETFONT, (WPARAM)DlgCygwinFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_ENV2, WM_SETFONT, (WPARAM)DlgCygwinFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDOK, WM_SETFONT, (WPARAM)DlgCygwinFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDCANCEL, WM_SETFONT, (WPARAM)DlgCygwinFont, MAKELPARAM(TRUE,0));
			}
			else {
				DlgCygwinFont = NULL;
			}

			GetDlgItemText(hDlgWnd, IDC_CYGWIN_PATH_LABEL, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("DLG_TAB_CYGWIN_PATH", ts.UIMsg, ts.UILanguageFile);
			SetDlgItemText(hDlgWnd, IDC_CYGWIN_PATH_LABEL, ts.UIMsg);
			GetDlgItemText(hDlgWnd, IDOK, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("BTN_OK", ts.UIMsg, ts.UILanguageFile);
			SetDlgItemText(hDlgWnd, IDOK, ts.UIMsg);
			GetDlgItemText(hDlgWnd, IDCANCEL, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("BTN_CANCEL", ts.UIMsg, ts.UILanguageFile);
			SetDlgItemText(hDlgWnd, IDCANCEL, ts.UIMsg);
#endif

			// try to read CygTerm config file
			memset(&settings, 0, sizeof(settings));
			_snprintf(settings.term, sizeof(settings.term), ".\\ttermpro.exe %%s %%d /KR=SJIS /KT=SJIS /nossh");
			_snprintf(settings.term_type, sizeof(settings.term_type), "vt100");
			_snprintf(settings.port_start, sizeof(settings.port_start), "20000");
			_snprintf(settings.port_range, sizeof(settings.port_range), "40");
			_snprintf(settings.shell, sizeof(settings.shell), "/bin/tcsh");
			_snprintf(settings.env1, sizeof(settings.env1), "MAKE_MODE=unix");
			_snprintf(settings.env2, sizeof(settings.env2), "HOME=/home/yutaka");

			fp = fopen(cfgfile, "r");
			if (fp != NULL) { 
				while (fgets(buf, sizeof(buf), fp) != NULL) {
					int len = strlen(buf);

					if (buf[len - 1] == '\n')
						buf[len - 1] = '\0';

					split_buffer(buf, '=', &head, &body);
					if (head == NULL || body == NULL)
						continue;

					if (strcmp(head, "TERM") == 0) {
						_snprintf(settings.term, sizeof(settings.term), "%s", body);

					} else if (strcmp(head, "TERM_TYPE") == 0) {
						_snprintf(settings.term_type, sizeof(settings.term_type), "%s", body);

					} else if (strcmp(head, "PORT_START") == 0) {
						_snprintf(settings.port_start, sizeof(settings.port_start), "%s", body);

					} else if (strcmp(head, "PORT_RANGE") == 0) {
						_snprintf(settings.port_range, sizeof(settings.port_range), "%s", body);

					} else if (strcmp(head, "SHELL") == 0) {
						_snprintf(settings.shell, sizeof(settings.shell), "%s", body);

					} else if (strcmp(head, "ENV_1") == 0) {
						_snprintf(settings.env1, sizeof(settings.env1), "%s", body);

					} else if (strcmp(head, "ENV_2") == 0) {
						_snprintf(settings.env2, sizeof(settings.env2), "%s", body);

					} else {
						// TODO: error check

					}
				}
				fclose(fp);
			} 
			SendMessage(GetDlgItem(hDlgWnd, IDC_TERM_EDIT), WM_SETTEXT , 0, (LPARAM)settings.term);
			SendMessage(GetDlgItem(hDlgWnd, IDC_TERM_TYPE), WM_SETTEXT , 0, (LPARAM)settings.term_type);
			SendMessage(GetDlgItem(hDlgWnd, IDC_PORT_START), WM_SETTEXT , 0, (LPARAM)settings.port_start);
			SendMessage(GetDlgItem(hDlgWnd, IDC_PORT_RANGE), WM_SETTEXT , 0, (LPARAM)settings.port_range);
			SendMessage(GetDlgItem(hDlgWnd, IDC_SHELL), WM_SETTEXT , 0, (LPARAM)settings.shell);
			SendMessage(GetDlgItem(hDlgWnd, IDC_ENV1), WM_SETTEXT , 0, (LPARAM)settings.env1);
			SendMessage(GetDlgItem(hDlgWnd, IDC_ENV2), WM_SETTEXT , 0, (LPARAM)settings.env2);


			// (4)Cygwin install path
			hWnd = GetDlgItem(hDlgWnd, IDC_CYGWIN_PATH);
			SendMessage(hWnd, WM_SETTEXT , 0, (LPARAM)ts.CygwinDirectory);

			// ダイアログにフォーカスを当てる 
			SetFocus(GetDlgItem(hDlgWnd, IDC_CYGWIN_PATH));

			return FALSE;

        case WM_COMMAND:
			switch (wp) {
				case IDC_SELECT_FILE | (BN_CLICKED << 16):
					// Cygwin install ディレクトリの選択ダイアログ
					doSelectFolder(hDlgWnd, ts.CygwinDirectory, sizeof(ts.CygwinDirectory));
					// (4)Cygwin install path
					hWnd = GetDlgItem(hDlgWnd, IDC_CYGWIN_PATH);
					SendMessage(hWnd, WM_SETTEXT , 0, (LPARAM)ts.CygwinDirectory);
					return TRUE;
			}

			switch (LOWORD(wp)) {
                case IDOK:
					// writing to CygTerm config file
					SendMessage(GetDlgItem(hDlgWnd, IDC_TERM_EDIT), WM_GETTEXT , sizeof(settings.term), (LPARAM)settings.term);
					SendMessage(GetDlgItem(hDlgWnd, IDC_TERM_TYPE), WM_GETTEXT , sizeof(settings.term_type), (LPARAM)settings.term_type);
					SendMessage(GetDlgItem(hDlgWnd, IDC_PORT_START), WM_GETTEXT , sizeof(settings.port_start), (LPARAM)settings.port_start);
					SendMessage(GetDlgItem(hDlgWnd, IDC_PORT_RANGE), WM_GETTEXT , sizeof(settings.port_range), (LPARAM)settings.port_range);
					SendMessage(GetDlgItem(hDlgWnd, IDC_SHELL), WM_GETTEXT , sizeof(settings.shell), (LPARAM)settings.shell);
					SendMessage(GetDlgItem(hDlgWnd, IDC_ENV1), WM_GETTEXT , sizeof(settings.env1), (LPARAM)settings.env1);
					SendMessage(GetDlgItem(hDlgWnd, IDC_ENV2), WM_GETTEXT , sizeof(settings.env2), (LPARAM)settings.env2);

					fp = fopen(cfgfile, "w");
					if (fp == NULL) { 
#ifdef I18N
						char uimsg[MAX_UIMSG];
						strcpy(uimsg, "ERROR");
						get_lang_msg("MSG_ERROR", uimsg, ts.UILanguageFile);
						strcpy(ts.UIMsg, "Can't write CygTerm configuration file (%d).");
						get_lang_msg("MSG_CYGTERM_CONF_WRITEFILE_ERROR", ts.UIMsg, ts.UILanguageFile);
						_snprintf(buf, sizeof(buf), ts.UIMsg, GetLastError());
						MessageBox(hDlgWnd, buf, uimsg, MB_ICONEXCLAMATION);
#else
						_snprintf(buf, sizeof(buf), "Can't write CygTerm configuration file (%d).", GetLastError());
						MessageBox(hDlgWnd, buf, "ERROR", MB_ICONEXCLAMATION);
#endif
					} else {
						fputs("# CygTerm setting\n", fp);
						fputs("\n", fp);
						fprintf(fp, "TERM = %s\n", settings.term);
						fprintf(fp, "TERM_TYPE = %s\n", settings.term_type);
						fprintf(fp, "PORT_START = %s\n", settings.port_start);
						fprintf(fp, "PORT_RANGE = %s\n", settings.port_range);
						fprintf(fp, "SHELL = %s\n", settings.shell);
						fprintf(fp, "ENV_1 = %s\n", settings.env1);
						fprintf(fp, "ENV_2 = %s\n", settings.env2);
						fclose(fp);
					}

					// (4)
					hWnd = GetDlgItem(hDlgWnd, IDC_CYGWIN_PATH);
					SendMessage(hWnd, WM_GETTEXT , sizeof(ts.CygwinDirectory), (LPARAM)ts.CygwinDirectory);

                    EndDialog(hDlgWnd, IDOK);
					SendMessage(gTabControlParent, WM_CLOSE, 0, 0);
                    break;

                case IDCANCEL:
                    EndDialog(hDlgWnd, IDCANCEL);
					SendMessage(gTabControlParent, WM_CLOSE, 0, 0);
                    break;

                default:
                    return FALSE;
            }

        case WM_CLOSE:
		    EndDialog(hDlgWnd, 0);
#ifdef I18N
			if (DlgCygwinFont != NULL) {
				DeleteObject(DlgCygwinFont);
			}
#endif
			return TRUE;

        default:
            return FALSE;
    }
    return TRUE;
}

// log tab
static LRESULT CALLBACK OnTabSheetLogProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	HWND hWnd;
#ifdef I18N
	char uimsg[MAX_UIMSG];
	LOGFONT logfont;
	HFONT font;
#endif

    switch (msg) {
        case WM_INITDIALOG:

#ifdef I18N
			font = (HFONT)SendMessage(hDlgWnd, WM_GETFONT, 0, 0);
			GetObject(font, sizeof(LOGFONT), &logfont);
			if (get_lang_font("DLG_TAHOMA_FONT", hDlgWnd, &logfont, &DlgLogFont, ts.UILanguageFile)) {
				SendDlgItemMessage(hDlgWnd, IDC_VIEWLOG_LABEL, WM_SETFONT, (WPARAM)DlgLogFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_VIEWLOG_EDITOR, WM_SETFONT, (WPARAM)DlgLogFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_VIEWLOG_PATH, WM_SETFONT, (WPARAM)DlgLogFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_DEFAULTNAME_LABEL, WM_SETFONT, (WPARAM)DlgLogFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_DEFAULTNAME_EDITOR, WM_SETFONT, (WPARAM)DlgLogFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDOK, WM_SETFONT, (WPARAM)DlgLogFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDCANCEL, WM_SETFONT, (WPARAM)DlgLogFont, MAKELPARAM(TRUE,0));
			}
			else {
				DlgVisualFont = NULL;
			}

			GetDlgItemText(hDlgWnd, IDC_VIEWLOG_LABEL, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("DLG_TAB_LOG_EDITOR", ts.UIMsg, ts.UILanguageFile);
			SetDlgItemText(hDlgWnd, IDC_VIEWLOG_LABEL, ts.UIMsg);
			GetDlgItemText(hDlgWnd, IDC_DEFAULTNAME_LABEL, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("DLG_TAB_LOG_FILENAME", ts.UIMsg, ts.UILanguageFile);
			SetDlgItemText(hDlgWnd, IDC_DEFAULTNAME_LABEL, ts.UIMsg);
			GetDlgItemText(hDlgWnd, IDOK, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("BTN_OK", ts.UIMsg, ts.UILanguageFile);
			SetDlgItemText(hDlgWnd, IDOK, ts.UIMsg);
			GetDlgItemText(hDlgWnd, IDCANCEL, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("BTN_CANCEL", ts.UIMsg, ts.UILanguageFile);
			SetDlgItemText(hDlgWnd, IDCANCEL, ts.UIMsg);
#endif

			// (7)Viewlog Editor path (2005.1.29 yutaka)
			hWnd = GetDlgItem(hDlgWnd, IDC_VIEWLOG_EDITOR);
			SendMessage(hWnd, WM_SETTEXT , 0, (LPARAM)ts.ViewlogEditor);

			// Log Default File Name (2006.8.28 maya)
			hWnd = GetDlgItem(hDlgWnd, IDC_DEFAULTNAME_EDITOR);
			SendMessage(hWnd, WM_SETTEXT , 0, (LPARAM)ts.LogDefaultName);

			// ダイアログにフォーカスを当てる 
			SetFocus(GetDlgItem(hDlgWnd, IDC_VIEWLOG_EDITOR));

			return FALSE;

        case WM_COMMAND:
			switch (wp) {
				case IDC_VIEWLOG_PATH | (BN_CLICKED << 16):
					{
					OPENFILENAME ofn;
					OSVERSIONINFO osvi;

					ZeroMemory(&ofn, sizeof(ofn));
					osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
					GetVersionEx(&osvi);
					if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT &&
						osvi.dwMajorVersion >= 5) {
						ofn.lStructSize = sizeof(OPENFILENAME);
					}
					else {
						ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
					}
					ofn.hwndOwner = hDlgWnd;
#ifdef I18N
					strncpy(ts.UIMsg, "exe(*.exe)\\0*.exe\\0all(*.*)\\0*.*\\0\\0", sizeof(ts.UIMsg));
					get_lang_msg("FILEDLG_SELECT_LOGVIEW_APP_FILTER", ts.UIMsg, ts.UILanguageFile);
					ofn.lpstrFilter = ts.UIMsg;
#else
					ofn.lpstrFilter = "exe(*.exe)\0*.exe\0all(*.*)\0*.*\0\0";
#endif
					ofn.lpstrFile = ts.ViewlogEditor;
					ofn.nMaxFile = sizeof(ts.ViewlogEditor);
#ifdef I18N
					strcpy(uimsg, "Choose a executing file with launching logging file");
					get_lang_msg("FILEDLG_SELECT_LOGVIEW_APP_TITLE", uimsg, ts.UILanguageFile);
					ofn.lpstrTitle = uimsg;
#else
					ofn.lpstrTitle = "Choose a executing file with launching logging file";
#endif
					ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_FORCESHOWHIDDEN | OFN_HIDEREADONLY;
					if (GetOpenFileName(&ofn) != 0) {
						hWnd = GetDlgItem(hDlgWnd, IDC_VIEWLOG_EDITOR);
						SendMessage(hWnd, WM_SETTEXT , 0, (LPARAM)ts.ViewlogEditor);
					}
					}
					return TRUE;

			}

			switch (LOWORD(wp)) {
                case IDOK:
#ifdef I18N
					char buf[80], buf2[80];
#else
					char buf[80], buf2[80], msg[80];
#endif
					time_t time_local;
					struct tm *tm_local;

					// (6)
					hWnd = GetDlgItem(hDlgWnd, IDC_VIEWLOG_EDITOR);
					SendMessage(hWnd, WM_GETTEXT , sizeof(ts.ViewlogEditor), (LPARAM)ts.ViewlogEditor);

					// Log Default File Name (2006.8.28 maya)
					hWnd = GetDlgItem(hDlgWnd, IDC_DEFAULTNAME_EDITOR);
					SendMessage(hWnd, WM_GETTEXT , sizeof(buf), (LPARAM)buf);
					if (isInvalidStrftimeChar(buf)) {
#ifdef I18N
						strcpy(uimsg, "ERROR");
						get_lang_msg("MSG_ERROR", uimsg, ts.UILanguageFile);
						strcpy(ts.UIMsg, "Invalid character is included in log file name.");
						get_lang_msg("MSG_LOGFILE_INVALID_CHAR_ERROR", ts.UIMsg, ts.UILanguageFile);
						MessageBox(hDlgWnd, ts.UIMsg, uimsg, MB_ICONEXCLAMATION);
#else
						strncpy(msg, "Invalid character is included in log file name.", sizeof(msg));
						MessageBox(hDlgWnd, msg, "ERROR", MB_ICONEXCLAMATION);
#endif
						return FALSE;
					}
					// 現在時刻を取得
					time(&time_local);
					tm_local = localtime(&time_local);
					// 時刻文字列に変換
					if (strftime(buf2, sizeof(buf2), buf, tm_local) == 0) {
#ifdef I18N
						strcpy(uimsg, "ERROR");
						get_lang_msg("MSG_ERROR", uimsg, ts.UILanguageFile);
						strcpy(ts.UIMsg, "The log file name is too long.");
						get_lang_msg("MSG_LOGFILE_TOOLONG_ERROR", ts.UIMsg, ts.UILanguageFile);
						MessageBox(hDlgWnd, ts.UIMsg, uimsg, MB_ICONEXCLAMATION);
#else
						strncpy(msg, "The log file name is too long.", sizeof(msg));
						MessageBox(hDlgWnd, msg, "ERROR", MB_ICONEXCLAMATION);
#endif
						return FALSE;
					}
					if (isInvalidFileNameChar(buf2)) {
#ifdef I18N
						strcpy(uimsg, "ERROR");
						get_lang_msg("MSG_ERROR", uimsg, ts.UILanguageFile);
						strcpy(ts.UIMsg, "Invalid character is included in log file name.");
						get_lang_msg("MSG_LOGFILE_INVALID_CHAR_ERROR", ts.UIMsg, ts.UILanguageFile);
						MessageBox(hDlgWnd, ts.UIMsg, uimsg, MB_ICONEXCLAMATION);
#else
						strncpy(msg, "Invalid character is included in log file name.", sizeof(msg));
						MessageBox(hDlgWnd, msg, "ERROR", MB_ICONEXCLAMATION);
#endif
						return FALSE;
					}
					strncpy(ts.LogDefaultName, buf, sizeof(ts.LogDefaultName));

                    EndDialog(hDlgWnd, IDOK);
					SendMessage(gTabControlParent, WM_CLOSE, 0, 0);
                    break;

                case IDCANCEL:
                    EndDialog(hDlgWnd, IDCANCEL);
					SendMessage(gTabControlParent, WM_CLOSE, 0, 0);
                    break;

                default:
                    return FALSE;
            }

        case WM_CLOSE:
		    EndDialog(hDlgWnd, 0);
#ifdef I18N
			if (DlgLogFont != NULL) {
				DeleteObject(DlgLogFont);
			}
#endif
			return TRUE;

        default:
            return FALSE;
    }
    return TRUE;
}

static void SetupRGBbox(HWND hDlgWnd, int index)
{
	HWND hWnd;
	BYTE c;
	char buf[10];

	hWnd = GetDlgItem(hDlgWnd, IDC_COLOR_RED);
	c = GetRValue(ts.ANSIColor[index]);
	_snprintf(buf, sizeof(buf), "%d", c);
	SendMessage(hWnd, WM_SETTEXT , 0, (LPARAM)buf);

	hWnd = GetDlgItem(hDlgWnd, IDC_COLOR_GREEN);
	c = GetGValue(ts.ANSIColor[index]);
	_snprintf(buf, sizeof(buf), "%d", c);
	SendMessage(hWnd, WM_SETTEXT , 0, (LPARAM)buf);

	hWnd = GetDlgItem(hDlgWnd, IDC_COLOR_BLUE);
	c = GetBValue(ts.ANSIColor[index]);
	_snprintf(buf, sizeof(buf), "%d", c);
	SendMessage(hWnd, WM_SETTEXT , 0, (LPARAM)buf);
}

// visual tab
static LRESULT CALLBACK OnTabSheetVisualProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	HWND hWnd;
	int i;
	char buf[MAXPATHLEN];
	LRESULT ret;
	static HDC label_hdc = NULL;
#ifdef I18N
	LOGFONT logfont;
	HFONT font;
#endif

    switch (msg) {
        case WM_INITDIALOG:

#ifdef I18N
			font = (HFONT)SendMessage(hDlgWnd, WM_GETFONT, 0, 0);
			GetObject(font, sizeof(LOGFONT), &logfont);
			if (get_lang_font("DLG_TAHOMA_FONT", hDlgWnd, &logfont, &DlgVisualFont, ts.UILanguageFile)) {
				SendDlgItemMessage(hDlgWnd, IDC_ALPHABLEND, WM_SETFONT, (WPARAM)DlgVisualFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_ALPHA_BLEND, WM_SETFONT, (WPARAM)DlgVisualFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_ETERM_LOOKFEEL, WM_SETFONT, (WPARAM)DlgVisualFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_MOUSE, WM_SETFONT, (WPARAM)DlgVisualFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_MOUSE_CURSOR, WM_SETFONT, (WPARAM)DlgVisualFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_ANSICOLOR, WM_SETFONT, (WPARAM)DlgVisualFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_ANSI_COLOR, WM_SETFONT, (WPARAM)DlgVisualFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_RED, WM_SETFONT, (WPARAM)DlgVisualFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_COLOR_RED, WM_SETFONT, (WPARAM)DlgVisualFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_GREEN, WM_SETFONT, (WPARAM)DlgVisualFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_COLOR_GREEN, WM_SETFONT, (WPARAM)DlgVisualFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_BLUE, WM_SETFONT, (WPARAM)DlgVisualFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_COLOR_BLUE, WM_SETFONT, (WPARAM)DlgVisualFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_SAMPLE_COLOR, WM_SETFONT, (WPARAM)DlgVisualFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDOK, WM_SETFONT, (WPARAM)DlgVisualFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDCANCEL, WM_SETFONT, (WPARAM)DlgVisualFont, MAKELPARAM(TRUE,0));
			}
			else {
				DlgVisualFont = NULL;
			}

			GetDlgItemText(hDlgWnd, IDC_ALPHABLEND, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("DLG_TAB_VISUAL_ALHPA", ts.UIMsg, ts.UILanguageFile);
			SetDlgItemText(hDlgWnd, IDC_ALPHABLEND, ts.UIMsg);
			GetDlgItemText(hDlgWnd, IDC_ETERM_LOOKFEEL, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("DLG_TAB_VISUAL_ETERM", ts.UIMsg, ts.UILanguageFile);
			SetDlgItemText(hDlgWnd, IDC_ETERM_LOOKFEEL, ts.UIMsg);
			GetDlgItemText(hDlgWnd, IDC_MOUSE, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("DLG_TAB_VISUAL_MOUSE", ts.UIMsg, ts.UILanguageFile);
			SetDlgItemText(hDlgWnd, IDC_MOUSE, ts.UIMsg);
			GetDlgItemText(hDlgWnd, IDC_RED, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("DLG_TAB_VISUAL_RED", ts.UIMsg, ts.UILanguageFile);
			SetDlgItemText(hDlgWnd, IDC_RED, ts.UIMsg);
			GetDlgItemText(hDlgWnd, IDC_GREEN, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("DLG_TAB_VISUAL_GREEN", ts.UIMsg, ts.UILanguageFile);
			SetDlgItemText(hDlgWnd, IDC_GREEN, ts.UIMsg);
			GetDlgItemText(hDlgWnd, IDC_BLUE, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("DLG_TAB_VISUAL_BLUE", ts.UIMsg, ts.UILanguageFile);
			SetDlgItemText(hDlgWnd, IDC_BLUE, ts.UIMsg);
			GetDlgItemText(hDlgWnd, IDOK, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("BTN_OK", ts.UIMsg, ts.UILanguageFile);
			SetDlgItemText(hDlgWnd, IDOK, ts.UIMsg);
			GetDlgItemText(hDlgWnd, IDCANCEL, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("BTN_CANCEL", ts.UIMsg, ts.UILanguageFile);
			SetDlgItemText(hDlgWnd, IDCANCEL, ts.UIMsg);
#endif

			// (1)AlphaBlend 
			hWnd = GetDlgItem(hDlgWnd, IDC_ALPHA_BLEND);
			_snprintf(buf, sizeof(buf), "%d", ts.AlphaBlend);
			SendMessage(hWnd, WM_SETTEXT , 0, (LPARAM)buf);

			// (2)[BG] BGEnable 
			hWnd = GetDlgItem(hDlgWnd, IDC_ETERM_LOOKFEEL);
			if (ts.EtermLookfeel.BGEnable) {
				SendMessage(hWnd, BM_SETCHECK, BST_CHECKED, 0);
			} else {
				SendMessage(hWnd, BM_SETCHECK, BST_UNCHECKED, 0);
			}

			// (3)Mouse cursor type
			hWnd = GetDlgItem(hDlgWnd, IDC_MOUSE_CURSOR);
			for (i = 0 ; MouseCursor[i].name ; i++) {
				SendMessage(hWnd, LB_INSERTSTRING, i, (LPARAM)MouseCursor[i].name);
			}
			SendMessage(hWnd, LB_SELECTSTRING , 0, (LPARAM)ts.MouseCursorName);

			// (4)ANSI color
			hWnd = GetDlgItem(hDlgWnd, IDC_ANSI_COLOR);
			for (i = 0 ; i < 16 ; i++) {
				_snprintf(buf, sizeof(buf), "%d", i);
				SendMessage(hWnd, LB_INSERTSTRING, i, (LPARAM)buf);
			}
			SetupRGBbox(hDlgWnd, 0);

			// ダイアログにフォーカスを当てる 
			SetFocus(GetDlgItem(hDlgWnd, IDC_ALPHA_BLEND));

			return FALSE;

        case WM_COMMAND:
			switch (wp) {
				case IDC_ANSI_COLOR | (LBN_SELCHANGE << 16):
					hWnd = GetDlgItem(hDlgWnd, IDC_ANSI_COLOR);
					ret = SendMessage(hWnd, LB_GETCURSEL, 0, 0);
					if (ret != -1) {
						SetupRGBbox(hDlgWnd, ret);
						SendMessage(hDlgWnd, WM_CTLCOLORSTATIC, (WPARAM)label_hdc, (LPARAM)hWnd);
					}
					return TRUE;

				case IDC_COLOR_RED | (EN_KILLFOCUS << 16):
				case IDC_COLOR_GREEN | (EN_KILLFOCUS << 16):
				case IDC_COLOR_BLUE | (EN_KILLFOCUS << 16):
					{
					BYTE r, g, b;

					hWnd = GetDlgItem(hDlgWnd, IDC_ANSI_COLOR);
					ret = SendMessage(hWnd, LB_GETCURSEL, 0, 0);
					if (ret < 0 && ret > sizeof(ts.ANSIColor)-1) {
						return TRUE;
					}

					hWnd = GetDlgItem(hDlgWnd, IDC_COLOR_RED);
					SendMessage(hWnd, WM_GETTEXT , sizeof(buf), (LPARAM)buf);
					r = atoi(buf);

					hWnd = GetDlgItem(hDlgWnd, IDC_COLOR_GREEN);
					SendMessage(hWnd, WM_GETTEXT , sizeof(buf), (LPARAM)buf);
					g = atoi(buf);

					hWnd = GetDlgItem(hDlgWnd, IDC_COLOR_BLUE);
					SendMessage(hWnd, WM_GETTEXT , sizeof(buf), (LPARAM)buf);
					b = atoi(buf);

					ts.ANSIColor[ret] = RGB(r, g, b);
					}

					return TRUE;
			}

			switch (LOWORD(wp)) {
                case IDOK:
					// (1)
					hWnd = GetDlgItem(hDlgWnd, IDC_ALPHA_BLEND);
					SendMessage(hWnd, WM_GETTEXT , sizeof(buf), (LPARAM)buf);
					ts.AlphaBlend = atoi(buf);

					// (2)
					// グローバル変数 BGEnable を直接書き換えると、プログラムが落ちることが
					// あるのでコピーを修正するのみとする。(2005.4.24 yutaka)
					hWnd = GetDlgItem(hDlgWnd, IDC_ETERM_LOOKFEEL);
					ret = SendMessage(hWnd, BM_GETCHECK , 0, 0);
					if (ret & BST_CHECKED) {
						ts.EtermLookfeel.BGEnable = 1;
					} else {
						ts.EtermLookfeel.BGEnable = 0;
					}

					// (3)
					hWnd = GetDlgItem(hDlgWnd, IDC_MOUSE_CURSOR);
					ret = SendMessage(hWnd, LB_GETCURSEL, 0, 0);
					if (ret >= 0 && ret < MOUSE_CURSOR_MAX) {
						strcpy(ts.MouseCursorName, MouseCursor[ret].name);
					}

                    EndDialog(hDlgWnd, IDOK);
					SendMessage(gTabControlParent, WM_CLOSE, 0, 0);
					// 2006/03/11 by 337 : Alpha値も即時変更
					// Layered窓になっていない場合は効果が無い
					if (ts.EtermLookfeel.BGUseAlphaBlendAPI) {
						// 起動時に半透明レイヤにしていない場合でも、即座に半透明となるようにする。(2006.4.1 yutaka)
						//MySetLayeredWindowAttributes(HVTWin, 0, (ts.AlphaBlend > 255) ? 255: ts.AlphaBlend, LWA_ALPHA);
						SetWindowStyle(&ts);
					}
					break;

                case IDCANCEL:
                    EndDialog(hDlgWnd, IDCANCEL);
					SendMessage(gTabControlParent, WM_CLOSE, 0, 0);
                    break;

                default:
                    return FALSE;
            }

        case WM_CLOSE:
		    EndDialog(hDlgWnd, 0);
#ifdef I18N
			if (DlgVisualFont != NULL) {
				DeleteObject(DlgVisualFont);
			}
#endif
			return TRUE;

#if 0
		case WM_CTLCOLORSTATIC :
			{
				HDC		hDC = (HDC)wp;
				HWND		hWnd = (HWND)lp;
				LOGBRUSH	lb;

				if (label_hdc == NULL) {
					label_hdc = hDC;
				}

				if ( hWnd == GetDlgItem(hDlgWnd, IDC_SAMPLE_COLOR) ) {
					lr = SendMessage(hWnd, LB_GETCURSEL, 0, 0);
					if (lr != -1) {
						SetTextColor( hDC, ts.ANSIColor[lr]);
					}
					SetDlgItemText( hDlgWnd, IDC_SAMPLE_COLOR, "SAMPLE TEXT" ) ;

					lb.lbStyle = BS_SOLID;
					lb.lbColor = RGB(0,0,0);

					//hBrush = CreateBrushIndirect(&lb);
					//return (BOOL)hBrush;
					return (BOOL)(HBRUSH)GetStockObject(NULL_BRUSH) ;

				}
			}
			break ;
#endif

        default:
            return FALSE;
    }
    return TRUE;
}

// general tab
static LRESULT CALLBACK OnTabSheetGeneralProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	HWND hWnd, hWnd2;
#ifdef I18N
	LOGFONT logfont;
	HFONT font;
#endif

    switch (msg) {
        case WM_INITDIALOG:

#ifdef I18N
			font = (HFONT)SendMessage(hDlgWnd, WM_GETFONT, 0, 0);
			GetObject(font, sizeof(LOGFONT), &logfont);
			if (get_lang_font("DLG_TAHOMA_FONT", hDlgWnd, &logfont, &DlgGeneralFont, ts.UILanguageFile)) {
				SendDlgItemMessage(hDlgWnd, IDC_LINECOPY, WM_SETFONT, (WPARAM)DlgGeneralFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_DISABLE_PASTE_RBUTTON, WM_SETFONT, (WPARAM)DlgGeneralFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_CONFIRM_PASTE_RBUTTON, WM_SETFONT, (WPARAM)DlgGeneralFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_DISABLE_SENDBREAK, WM_SETFONT, (WPARAM)DlgGeneralFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_CLICKABLE_URL, WM_SETFONT, (WPARAM)DlgGeneralFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_DELIMITER, WM_SETFONT, (WPARAM)DlgGeneralFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_DELIM_LIST, WM_SETFONT, (WPARAM)DlgGeneralFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_ACCEPT_BROADCAST, WM_SETFONT, (WPARAM)DlgGeneralFont, MAKELPARAM(TRUE,0)); // 337: 2007/03/20
				SendDlgItemMessage(hDlgWnd, IDOK, WM_SETFONT, (WPARAM)DlgGeneralFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDCANCEL, WM_SETFONT, (WPARAM)DlgGeneralFont, MAKELPARAM(TRUE,0));
			}
			else {
				DlgGeneralFont = NULL;
			}

			GetDlgItemText(hDlgWnd, IDC_LINECOPY, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("DLG_TAB_GENERAL_CONTINUE", ts.UIMsg, ts.UILanguageFile);
			SetDlgItemText(hDlgWnd, IDC_LINECOPY, ts.UIMsg);
			GetDlgItemText(hDlgWnd, IDC_DISABLE_PASTE_RBUTTON, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("DLG_TAB_GENERAL_MOUSEPASTE", ts.UIMsg, ts.UILanguageFile);
			SetDlgItemText(hDlgWnd, IDC_DISABLE_PASTE_RBUTTON, ts.UIMsg);
			GetDlgItemText(hDlgWnd, IDC_CONFIRM_PASTE_RBUTTON, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("DLG_TAB_GENERAL_CONFIRMPASTE", ts.UIMsg, ts.UILanguageFile);
			SetDlgItemText(hDlgWnd, IDC_CONFIRM_PASTE_RBUTTON, ts.UIMsg);
			GetDlgItemText(hDlgWnd, IDC_DISABLE_SENDBREAK, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("DLG_TAB_GENERAL_DISABLESENDBREAK", ts.UIMsg, ts.UILanguageFile);
			SetDlgItemText(hDlgWnd, IDC_DISABLE_SENDBREAK, ts.UIMsg);
			GetDlgItemText(hDlgWnd, IDC_CLICKABLE_URL, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("DLG_TAB_GENERAL_CLICKURL", ts.UIMsg, ts.UILanguageFile);
			SetDlgItemText(hDlgWnd, IDC_CLICKABLE_URL, ts.UIMsg);
			GetDlgItemText(hDlgWnd, IDC_DELIMITER, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("DLG_TAB_GENERAL_DEMILITER", ts.UIMsg, ts.UILanguageFile);
			SetDlgItemText(hDlgWnd, IDC_DELIMITER, ts.UIMsg);
			GetDlgItemText(hDlgWnd, IDOK, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("BTN_OK", ts.UIMsg, ts.UILanguageFile);
			SetDlgItemText(hDlgWnd, IDOK, ts.UIMsg);
			GetDlgItemText(hDlgWnd, IDCANCEL, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("BTN_CANCEL", ts.UIMsg, ts.UILanguageFile);
			SetDlgItemText(hDlgWnd, IDCANCEL, ts.UIMsg);
#endif

			// (1)Enable continued-line copy
			hWnd = GetDlgItem(hDlgWnd, IDC_LINECOPY);
			if (ts.EnableContinuedLineCopy == TRUE) {
				SendMessage(hWnd, BM_SETCHECK, BST_CHECKED, 0);
			} else {
				SendMessage(hWnd, BM_SETCHECK, BST_UNCHECKED, 0);
			}

			// (2)DisablePasteMouseRButton
			hWnd  = GetDlgItem(hDlgWnd, IDC_DISABLE_PASTE_RBUTTON);
			hWnd2 = GetDlgItem(hDlgWnd, IDC_CONFIRM_PASTE_RBUTTON);
			if (ts.DisablePasteMouseRButton == TRUE) {
				SendMessage(hWnd, BM_SETCHECK, BST_CHECKED, 0);
				EnableWindow(hWnd2, FALSE);
			} else {
				SendMessage(hWnd, BM_SETCHECK, BST_UNCHECKED, 0);
				EnableWindow(hWnd2, TRUE);
			}

			// (3)ConfirmPasteMouseRButton
			if (ts.ConfirmPasteMouseRButton == TRUE) {
				SendMessage(hWnd2, BM_SETCHECK, BST_CHECKED, 0);
			} else {
				SendMessage(hWnd2, BM_SETCHECK, BST_UNCHECKED, 0);
			}

			// (4)DisableAcceleratorSendBreak
			hWnd = GetDlgItem(hDlgWnd, IDC_DISABLE_SENDBREAK);
			if (ts.DisableAcceleratorSendBreak == TRUE) {
				SendMessage(hWnd, BM_SETCHECK, BST_CHECKED, 0);
			} else {
				SendMessage(hWnd, BM_SETCHECK, BST_UNCHECKED, 0);
			}

			// (5)EnableClickableUrl
			hWnd = GetDlgItem(hDlgWnd, IDC_CLICKABLE_URL);
			if (ts.EnableClickableUrl == TRUE) {
				SendMessage(hWnd, BM_SETCHECK, BST_CHECKED, 0);
			} else {
				SendMessage(hWnd, BM_SETCHECK, BST_UNCHECKED, 0);
			}

			// (6)delimiter characters
			hWnd = GetDlgItem(hDlgWnd, IDC_DELIM_LIST);
			SendMessage(hWnd, WM_SETTEXT , 0, (LPARAM)ts.DelimList);

			// (7)AcceptBroadcast 337: 2007/03/20
			hWnd = GetDlgItem(hDlgWnd, IDC_ACCEPT_BROADCAST);
			if (ts.AcceptBroadcast == TRUE) {
				SendMessage(hWnd, BM_SETCHECK, BST_CHECKED, 0);
			} else {
				SendMessage(hWnd, BM_SETCHECK, BST_UNCHECKED, 0);
			}

			// ダイアログにフォーカスを当てる (2004.12.7 yutaka)
			SetFocus(GetDlgItem(hDlgWnd, IDC_LINECOPY));

			return FALSE;

        case WM_COMMAND:
            switch (wp) {
				case IDC_LINECOPY | (BN_CLICKED << 16):
					return TRUE;

				case IDC_DISABLE_PASTE_RBUTTON | (BN_CLICKED << 16):
					hWnd  = GetDlgItem(hDlgWnd, IDC_DISABLE_PASTE_RBUTTON);
					hWnd2 = GetDlgItem(hDlgWnd, IDC_CONFIRM_PASTE_RBUTTON);
					if (SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED) {
						EnableWindow(hWnd2, FALSE);
					} else {
						EnableWindow(hWnd2, TRUE);
					}
					return TRUE;
			}

			switch (LOWORD(wp)) {
                case IDOK:
					// (1)
					hWnd = GetDlgItem(hDlgWnd, IDC_LINECOPY);
					if (SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED) {
						ts.EnableContinuedLineCopy = TRUE;
					} else {
						ts.EnableContinuedLineCopy = FALSE;
					}

					// (2)
					hWnd = GetDlgItem(hDlgWnd, IDC_DISABLE_PASTE_RBUTTON);
					if (SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED) {
						ts.DisablePasteMouseRButton = TRUE;
					} else {
						ts.DisablePasteMouseRButton = FALSE;
					}

					// (3)
					hWnd = GetDlgItem(hDlgWnd, IDC_CONFIRM_PASTE_RBUTTON);
					if (SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED) {
						ts.ConfirmPasteMouseRButton = TRUE;
					} else {
						ts.ConfirmPasteMouseRButton = FALSE;
					}

					// (4)
					hWnd = GetDlgItem(hDlgWnd, IDC_DISABLE_SENDBREAK);
					if (SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED) {
						ts.DisableAcceleratorSendBreak = TRUE;
					} else {
						ts.DisableAcceleratorSendBreak = FALSE;
					}

					// (5)
					hWnd = GetDlgItem(hDlgWnd, IDC_CLICKABLE_URL);
					if (SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED) {
						ts.EnableClickableUrl = TRUE;
					} else {
						ts.EnableClickableUrl = FALSE;
					}

					// (6)
					hWnd = GetDlgItem(hDlgWnd, IDC_DELIM_LIST);
					SendMessage(hWnd, WM_GETTEXT , sizeof(ts.DelimList), (LPARAM)ts.DelimList);

					// (7) 337: 2007/03/20  
					hWnd = GetDlgItem(hDlgWnd, IDC_ACCEPT_BROADCAST);
					if (SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED) {
						ts.AcceptBroadcast = TRUE;
					} else {
						ts.AcceptBroadcast = FALSE;
					}

					EndDialog(hDlgWnd, IDOK);
					SendMessage(gTabControlParent, WM_CLOSE, 0, 0);
                    break;

                case IDCANCEL:
					EndDialog(hDlgWnd, IDCANCEL);
					SendMessage(gTabControlParent, WM_CLOSE, 0, 0);
                    break;

                default:
                    return FALSE;
            }

        case WM_CLOSE:
		    EndDialog(hDlgWnd, 0);
#ifdef I18N
			if (DlgGeneralFont != NULL) {
				DeleteObject(DlgGeneralFont);
			}
#endif
			return TRUE;

        default:
            return FALSE;
    }
    return TRUE;
}

// tab control: main
//
// タブシートのプロパティで、Style=子、Border=なし、Control=trueにする必要がある。
// cf. http://home.a03.itscom.net/tsuzu/programing/tips28.htm
static LRESULT CALLBACK OnAdditionalSetupDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	TCITEM tc;
	RECT rect;
	LPPOINT pt = (LPPOINT)&rect;
	NMHDR *nm = (NMHDR *)lp;
	// dialog handle
#define MAX_TABSHEET 4
	static HWND hTabCtrl; // parent
	static HWND hTabSheet[MAX_TABSHEET]; //0:general 1:visual 2:log 3:Cygwin
	int i;
#ifdef I18N
	LOGFONT logfont;
	HFONT font;
#endif

	switch (msg) {
        case WM_INITDIALOG:
#ifdef I18N
		font = (HFONT)SendMessage(hDlgWnd, WM_GETFONT, 0, 0);
		GetObject(font, sizeof(LOGFONT), &logfont);
		if (get_lang_font("DLG_TAHOMA_FONT", hDlgWnd, &logfont, &DlgAdditionalFont, ts.UILanguageFile)) {
			SendDlgItemMessage(hDlgWnd, IDC_SETUP_TAB, WM_SETFONT, (WPARAM)DlgAdditionalFont, MAKELPARAM(TRUE,0));
		}
#endif
			gTabControlParent = hDlgWnd;

			// コモンコントロールの初期化
			InitCommonControls();
#ifdef I18N
			GetWindowText(hDlgWnd, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("DLG_TABSHEET_TITLE", ts.UIMsg, ts.UILanguageFile);
			SetWindowText(hDlgWnd, ts.UIMsg);
#endif

			// シート枠の作成
			hTabCtrl = GetDlgItem(hDlgWnd, IDC_SETUP_TAB);
			ZeroMemory(&tc, sizeof(tc));
			tc.mask = TCIF_TEXT;
#ifdef I18N
			strcpy(ts.UIMsg, "General");
			get_lang_msg("DLG_TABSHEET_TITLE_GENERAL", ts.UIMsg, ts.UILanguageFile);
			tc.pszText = ts.UIMsg;
#else
			tc.pszText = "General";
#endif
			TabCtrl_InsertItem(hTabCtrl, 0, &tc);

			ZeroMemory(&tc, sizeof(tc));
			tc.mask = TCIF_TEXT;
#ifdef I18N
			strcpy(ts.UIMsg, "Visual");
			get_lang_msg("DLG_TABSHEET_TITLE_VISUAL", ts.UIMsg, ts.UILanguageFile);
			tc.pszText = ts.UIMsg;
#else
			tc.pszText = "Visual";
#endif
			TabCtrl_InsertItem(hTabCtrl, 1, &tc);

			ZeroMemory(&tc, sizeof(tc));
			tc.mask = TCIF_TEXT;
#ifdef I18N
			strcpy(ts.UIMsg, "Log");
			get_lang_msg("DLG_TABSHEET_TITLE_LOG", ts.UIMsg, ts.UILanguageFile);
			tc.pszText = ts.UIMsg;
#else
			tc.pszText = "Log";
#endif
			TabCtrl_InsertItem(hTabCtrl, 2, &tc);

			ZeroMemory(&tc, sizeof(tc));
			tc.mask = TCIF_TEXT;
#ifdef I18N
			strcpy(ts.UIMsg, "Cygwin");
			get_lang_msg("DLG_TABSHEET_TITLE_CYGWIN", ts.UIMsg, ts.UILanguageFile);
			tc.pszText = ts.UIMsg;
#else
			tc.pszText = "Cygwin";
#endif
			TabCtrl_InsertItem(hTabCtrl, 3, &tc);

			// シートに載せる子ダイアログの作成
			hTabSheet[0] = CreateDialog(
							hInst, 
							MAKEINTRESOURCE(IDD_TABSHEET_GENERAL), 
							hDlgWnd,
							(DLGPROC)OnTabSheetGeneralProc
							);

			hTabSheet[1] = CreateDialog(
							hInst, 
							MAKEINTRESOURCE(IDD_TABSHEET_VISUAL), 
							hDlgWnd,
							(DLGPROC)OnTabSheetVisualProc
							);

			hTabSheet[2] = CreateDialog(
							hInst, 
							MAKEINTRESOURCE(IDD_TABSHEET_LOG), 
							hDlgWnd,
							(DLGPROC)OnTabSheetLogProc
							);

			hTabSheet[3] = CreateDialog(
							hInst, 
							MAKEINTRESOURCE(IDD_TABSHEET_CYGWIN), 
							hDlgWnd,
							(DLGPROC)OnTabSheetCygwinProc
							);

			// タブコントロールの矩形座標を取得
			// 親ウィンドウがhDlgなので座標変換が必要(MapWindowPoints)
			GetClientRect(hTabCtrl, &rect);
			TabCtrl_AdjustRect(hTabCtrl, FALSE, &rect);
			MapWindowPoints(hTabCtrl, hDlgWnd, pt, 2);

			// 生成した子ダイアログをタブシートの上に貼り付ける
			// 実際は子ダイアログの表示位置をシート上に移動しているだけ
			for (i = 0 ; i < MAX_TABSHEET ; i++) {
				MoveWindow(hTabSheet[i], 
					rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, FALSE);
			}
		
			ShowWindow(hTabSheet[0], SW_SHOW);

			return FALSE;

		case WM_NOTIFY:
        	// タブコントロールのシート切り替え通知なら
        	switch (nm->code) {
        	case TCN_SELCHANGE:
            	if (nm->hwndFrom == hTabCtrl) {
					int n;
                	// 現在表示されているのシートの番号を判別
					n = TabCtrl_GetCurSel(hTabCtrl);
					for (i = 0 ; i < MAX_TABSHEET ; i++) {
                    	ShowWindow(hTabSheet[i], SW_HIDE);
					}
                    ShowWindow(hTabSheet[n], SW_SHOW);
            	}
            	break;
        	}
			return TRUE;

		case WM_CLOSE:
			for (i = 0 ; i < MAX_TABSHEET ; i++) {
				EndDialog(hTabSheet[i], FALSE);
			}
			EndDialog(hDlgWnd, FALSE);
#ifdef I18N
			if (DlgAdditionalFont != NULL) {
				DeleteObject(DlgAdditionalFont);
			}
#endif
			return TRUE;

        default:
            return FALSE;
	}

    return TRUE;
}

// Additional settings dialog
//
// (2004.9.5 yutaka) new added
// (2005.2.22 yutaka) changed to Tab Control
void CVTWindow::OnExternalSetup()
{
	DWORD ret;

	// 設定ダイアログ (2004.9.5 yutaka)
	ret = DialogBox(hInst, MAKEINTRESOURCE(IDD_ADDITIONAL_SETUPTAB), HVTWin, (DLGPROC)OnAdditionalSetupDlgProc);
	if (ret == 0 || ret == -1) {
		ret = GetLastError();
	}
}

void CVTWindow::OnSetupTerminal()
{
  BOOL Ok;

  if (ts.Language==IdRussian)
    HelpId = HlpSetupTerminalRuss;
  else
    HelpId = HlpSetupTerminal;
  if (! LoadTTDLG()) return;
  Ok = (*SetupTerminal)(HVTWin, &ts);
  FreeTTDLG();
  if (Ok) SetupTerm();
}

void CVTWindow::OnSetupWindow()
{
  BOOL Ok;

  HelpId = HlpSetupWindow;
  ts.VTFlag = 1;
  ts.SampleFont = VTFont[0];

  if (! LoadTTDLG()) return;
  Ok = (*SetupWin)(HVTWin, &ts);
  FreeTTDLG();

  if (Ok) {
	  ChangeWin();

	  // Eterm lookfeelの画面情報も更新することで、リアルタイムでの背景色変更が
	  // 可能となる。(2006.2.24 yutaka)
#ifdef ALPHABLEND_TYPE2
  	  BGInitialize();
	  BGSetupPrimary(TRUE);
	  ResetSetup();
#endif
  }

}

void CVTWindow::OnSetupFont()
{
  if (ts.Language==IdRussian)
    HelpId = HlpSetupFontRuss;
  else
    HelpId = HlpSetupFont;
  DispSetupFontDlg();
}

void CVTWindow::OnSetupKeyboard()
{
  BOOL Ok;

  if (ts.Language==IdRussian)
    HelpId = HlpSetupKeyboardRuss;
  else
    HelpId = HlpSetupKeyboard;
  if (! LoadTTDLG()) return;
  Ok = (*SetupKeyboard)(HVTWin, &ts);
  FreeTTDLG();

  if (Ok &&
      (ts.Language==IdJapanese))
    ResetIME();
}

void CVTWindow::OnSetupSerialPort()
{
  BOOL Ok;
  HelpId = HlpSetupSerialPort;
  if (! LoadTTDLG()) return;
  Ok = (*SetupSerialPort)(HVTWin, &ts);
  FreeTTDLG();

  if (Ok)
  {
    if (cv.Open)
    {
      if (ts.ComPort != cv.ComPort)
      {
	CommClose(&cv);
	CommOpen(HVTWin,&ts,&cv);
      }
      else
	CommResetSerial(&ts,&cv);
    }
    else
      CommOpen(HVTWin,&ts,&cv);
  }
}

void CVTWindow::OnSetupTCPIP()
{
  HelpId = HlpSetupTCPIP;
  if (! LoadTTDLG()) return;
  (*SetupTCPIP)(HVTWin, &ts);
  FreeTTDLG();
}

void CVTWindow::OnSetupGeneral()
{
  HelpId = HlpSetupGeneral;
  if (! LoadTTDLG()) return;
  if ((*SetupGeneral)(HVTWin,&ts))
  {
    ResetCharSet();
    ResetIME();
  }
  FreeTTDLG();
}

void CVTWindow::OnSetupSave()
{
	BOOL Ok;
	char TmpSetupFN[MAXPATHLEN];
	int ret;

	strcpy(TmpSetupFN,ts.SetupFName);
	if (! LoadTTFILE()) return;
	HelpId = HlpSetupSave;
	Ok = (*GetSetupFname)(HVTWin,GSF_SAVE,&ts);
	FreeTTFILE();
	if (! Ok) return;

	// 書き込みできるかの判別を追加 (2005.11.3 yutaka)
	if ((ret = _access(ts.SetupFName, 0x02)) != 0) {
		if (errno != ENOENT) {  // ファイルがすでに存在する場合のみエラーとする (2005.12.13 yutaka)
#ifdef I18N
			char uimsg[MAX_UIMSG];
			strcpy(uimsg, "Tera Term: ERROR");
			get_lang_msg("MSG_TT_ERROR", uimsg, ts.UILanguageFile);
			strcpy(ts.UIMsg, "Teraterm.ini file doesn't have the writable permission.");
			MessageBox(ts.UIMsg, uimsg, MB_OK|MB_ICONEXCLAMATION);
#else
			MessageBox("Teraterm.ini file doesn't have the writable permission.", 
				"Tera Term: ERROR", MB_OK|MB_ICONEXCLAMATION);
#endif
			return;
		}
	}

	if (LoadTTSET())
	{
		/* write current setup values to file */
		(*WriteIniFile)(ts.SetupFName,&ts);
		/* copy host list */
		(*CopyHostList)(TmpSetupFN,ts.SetupFName);
		/* copy broadcast command history list */
		(*CopySerialList)(TmpSetupFN,ts.SetupFName,"BroadcastCommands","Command");
		FreeTTSET();
	}

	ChangeDefaultSet(&ts,NULL);
}

void CVTWindow::OnSetupRestore()
{
  BOOL Ok;

  HelpId = HlpSetupRestore;
  if (! LoadTTFILE()) return;
  Ok = (*GetSetupFname)(HVTWin,GSF_RESTORE,&ts);
  FreeTTFILE();
  if (Ok) RestoreSetup();
}

void CVTWindow::OnSetupLoadKeyMap()
{
  BOOL Ok;

  HelpId = HlpSetupLoadKeyMap;
  if (! LoadTTFILE()) return;
  Ok = (*GetSetupFname)(HVTWin,GSF_LOADKEY,&ts);
  FreeTTFILE();
  if (! Ok) return;

  // load key map
  SetKeyMap();
}

void CVTWindow::OnControlResetTerminal()
{
  LockBuffer();
  HideStatusLine();
  DispScrollHomePos();
  ResetTerminal();
  UnlockBuffer();

  LButton = FALSE;
  MButton = FALSE;
  RButton = FALSE;

  Hold = FALSE;
  CommLock(&ts,&cv,FALSE);

  KeybEnabled  = TRUE;
}

void CVTWindow::OnControlAreYouThere()
{
  if (cv.Ready && (cv.PortType==IdTCPIP))
    TelSendAYT();
}

void CVTWindow::OnControlSendBreak()
{
  if (cv.Ready)
    switch (cv.PortType) {
      case IdTCPIP:
	TelSendBreak();
	break;
      case IdSerial:
	CommSendBreak(&cv);
	break;
    }
}

void CVTWindow::OnControlResetPort()
{
  CommResetSerial(&ts,&cv);
}

void ApplyBoradCastCommandHisotry(HWND Dialog)
{
	char EntName[10];
	char TempHost[HostNameMaxLength+1];
	int i = 1;

	SendDlgItemMessage(Dialog, IDC_COMMAND_EDIT, CB_RESETCONTENT, 0, 0);
	strcpy(EntName,"Command");
	do {
		uint2str(i,&EntName[7],2);
		GetPrivateProfileString("BroadcastCommands",EntName,"",
								TempHost,sizeof(TempHost),ts.SetupFName);
		if ( strlen(TempHost) > 0 )
			SendDlgItemMessage(Dialog, IDC_COMMAND_EDIT, CB_ADDSTRING,
							   0, (LPARAM)TempHost);
			i++;
		} while ((i <= 99) && (strlen(TempHost)>0));

	SendDlgItemMessage(Dialog, IDC_COMMAND_EDIT, EM_LIMITTEXT,
					   HostNameMaxLength-1, 0);

	SendDlgItemMessage(Dialog, IDC_COMMAND_EDIT, CB_SETCURSEL,0,0);
}

//
// すべてのターミナルへ同一コマンドを送信するモードレスダイアログの表示
// (2005.1.22 yutaka)
//
static LRESULT CALLBACK BroadcastCommandDlgProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	char buf[256 + 3];
	UINT ret;
	LRESULT checked;
	LRESULT history;
#ifdef I18N
	LOGFONT logfont;
	HFONT font;
#endif

    switch (msg) {
        case WM_INITDIALOG:
			// ラジオボタンのデフォルトは CR にする。
			SendMessage(GetDlgItem(hWnd, IDC_RADIO_CR), BM_SETCHECK, BST_CHECKED, 0);
			// デフォルトでチェックボックスを checked 状態にする。
			SendMessage(GetDlgItem(hWnd, IDC_ENTERKEY_CHECK), BM_SETCHECK, BST_CHECKED, 0);
			// history を反映する (2007.3.3 maya)
			if (ts.BroadcastCommandHistory) {
				SendMessage(GetDlgItem(hWnd, IDC_HISTORY_CHECK), BM_SETCHECK, BST_CHECKED, 0);
			}
			ApplyBoradCastCommandHisotry(hWnd);

			// エディットコントロールにフォーカスをあてる
			SetFocus(GetDlgItem(hWnd, IDC_COMMAND_EDIT));

#ifdef I18N
			font = (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0);
			GetObject(font, sizeof(LOGFONT), &logfont);
			if (get_lang_font("DLG_SYSTEM_FONT", hWnd, &logfont, &DlgBroadcastFont, ts.UILanguageFile)) {
				SendDlgItemMessage(hWnd, IDC_COMMAND_EDIT, WM_SETFONT, (WPARAM)DlgBroadcastFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hWnd, IDC_HISTORY_CHECK, WM_SETFONT, (WPARAM)DlgBroadcastFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hWnd, IDC_RADIO_CRLF, WM_SETFONT, (WPARAM)DlgBroadcastFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hWnd, IDC_RADIO_CR, WM_SETFONT, (WPARAM)DlgBroadcastFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hWnd, IDC_RADIO_LF, WM_SETFONT, (WPARAM)DlgBroadcastFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hWnd, IDC_ENTERKEY_CHECK, WM_SETFONT, (WPARAM)DlgBroadcastFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hWnd, IDC_PARENT_ONLY, WM_SETFONT, (WPARAM)DlgBroadcastFont, MAKELPARAM(TRUE,0));	// 337: 2007/03/20
				SendDlgItemMessage(hWnd, IDOK, WM_SETFONT, (WPARAM)DlgBroadcastFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hWnd, IDCANCEL, WM_SETFONT, (WPARAM)DlgBroadcastFont, MAKELPARAM(TRUE,0));
			}
			else {
				DlgBroadcastFont = NULL;
			}
			GetWindowText(hWnd, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("DLG_BROADCAST_TITLE", ts.UIMsg, ts.UILanguageFile);
			SetWindowText(hWnd, ts.UIMsg);
			GetDlgItemText(hWnd, IDC_HISTORY_CHECK, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("DLG_BROADCAST_HISTORY", ts.UIMsg, ts.UILanguageFile);
			SetDlgItemText(hWnd, IDC_HISTORY_CHECK, ts.UIMsg);
			GetDlgItemText(hWnd, IDC_ENTERKEY_CHECK, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("DLG_BROADCAST_ENTER", ts.UIMsg, ts.UILanguageFile);
			SetDlgItemText(hWnd, IDC_ENTERKEY_CHECK, ts.UIMsg);
			GetDlgItemText(hWnd, IDOK, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("DLG_BROADCAST_SUBMIT", ts.UIMsg, ts.UILanguageFile);
			SetDlgItemText(hWnd, IDOK, ts.UIMsg);
			GetDlgItemText(hWnd, IDCANCEL, ts.UIMsg, sizeof(ts.UIMsg));
			get_lang_msg("BTN_CLOSE", ts.UIMsg, ts.UILanguageFile);
			SetDlgItemText(hWnd, IDCANCEL, ts.UIMsg);
#endif

			return FALSE;

        case WM_COMMAND:
			switch (wp) {
			case IDC_ENTERKEY_CHECK | (BN_CLICKED << 16):
				// チェックの有無により、ラジオボタンの有効・無効を決める。
				checked = SendMessage(GetDlgItem(hWnd, IDC_ENTERKEY_CHECK), BM_GETCHECK, 0, 0);
				if (checked & BST_CHECKED) { // 改行コードあり
					EnableWindow(GetDlgItem(hWnd, IDC_RADIO_CRLF), TRUE);
					EnableWindow(GetDlgItem(hWnd, IDC_RADIO_CR), TRUE);
					EnableWindow(GetDlgItem(hWnd, IDC_RADIO_LF), TRUE);

				} else {
					EnableWindow(GetDlgItem(hWnd, IDC_RADIO_CRLF), FALSE);
					EnableWindow(GetDlgItem(hWnd, IDC_RADIO_CR), FALSE);
					EnableWindow(GetDlgItem(hWnd, IDC_RADIO_LF), FALSE);
				}
				return TRUE;
			}

            switch (LOWORD(wp)) {
                case IDOK:
					{
					int i;
					HWND hd;
					COPYDATASTRUCT cds;

					memset(buf, 0, sizeof(buf));
					ret = GetDlgItemText(hWnd, IDC_COMMAND_EDIT, buf, 256 - 1);
					if (ret == 0) { // error
						memset(buf, 0, sizeof(buf));
					}

					// ブロードキャストコマンドの履歴を保存 (2007.3.3 maya)
					history = SendMessage(GetDlgItem(hWnd, IDC_HISTORY_CHECK), BM_GETCHECK, 0, 0);
					if (history) {
						if (LoadTTSET()) {
							(*AddValueToList)(ts.SetupFName, buf, "BroadcastCommands", "Command");
							FreeTTSET();
						}
						ApplyBoradCastCommandHisotry(hWnd);
						ts.BroadcastCommandHistory = TRUE;
					}
					else {
						ts.BroadcastCommandHistory = FALSE;
					}
					checked = SendMessage(GetDlgItem(hWnd, IDC_ENTERKEY_CHECK), BM_GETCHECK, 0, 0);
					if (checked & BST_CHECKED) { // 改行コードあり
						if (SendMessage(GetDlgItem(hWnd, IDC_RADIO_CRLF), BM_GETCHECK, 0, 0) & BST_CHECKED) {
							strcat(buf, "\r\n");

						} else if (SendMessage(GetDlgItem(hWnd, IDC_RADIO_CR), BM_GETCHECK, 0, 0) & BST_CHECKED) {
							strcat(buf, "\r");

						} else if (SendMessage(GetDlgItem(hWnd, IDC_RADIO_LF), BM_GETCHECK, 0, 0) & BST_CHECKED) {
							strcat(buf, "\n");

						} else {
							strcat(buf, "\r");

						}
					}

					// 337: 2007/03/20 チェックされていたら親ウィンドウにのみ送信
					checked = SendMessage(GetDlgItem(hWnd, IDC_PARENT_ONLY), BM_GETCHECK, 0, 0);

					// すべてのTeraTermにメッセージとデータを送る
					for (i = 0 ; i < 50 ; i++) { // 50 = MAXNWIN(@ ttcmn.c)
						if (checked) {
							hd = GetParent(hWnd);
							i = 50;		// 337: 強引かつ直値 :P
						} else {
							hd = GetNthWin(i);
						}
						if (hd == NULL)
							break;

						ZeroMemory(&cds, sizeof(cds));
						cds.dwData = IPC_BROADCAST_COMMAND;
						cds.cbData = strlen(buf);
						cds.lpData = buf;

						// WM_COPYDATAを使って、プロセス間通信を行う。
						SendMessage(hd, WM_COPYDATA, (WPARAM)HVTWin, (LPARAM)&cds);

						// 送信先TeraTermウィンドウに適当なメッセージを送る。
						// これをしないと、送り込んだデータが反映されない模様。
						// (2006.2.7 yutaka)
						PostMessage(hd, WM_SETFOCUS, NULL, 0);
					}

					}

                    //EndDialog(hDlgWnd, IDOK);
                    return TRUE;

                case IDCANCEL:
				    EndDialog(hWnd, 0);
					//DestroyWindow(hWnd);

					return TRUE;

                default:
                    return FALSE;
            }
			break;

        case WM_CLOSE:
			//DestroyWindow(hWnd);
		    EndDialog(hWnd, 0);
#ifdef I18N
					if (DlgBroadcastFont != NULL) {
						DeleteObject(DlgBroadcastFont);
					}
#endif
			return TRUE;

        default:
            return FALSE;
    }
    return TRUE;

}

void CVTWindow::OnControlBroadcastCommand(void)
{
	// TODO: モードレスダイアログのハンドルは、親プロセスが DestroyWindow() APIで破棄する
	// 必要があるが、ここはOS任せとする。
	static HWND hDlgWnd = NULL;

	if (hDlgWnd != NULL)
		goto activate;

	hDlgWnd = CreateDialog(
				hInst, 
				MAKEINTRESOURCE(IDD_BROADCAST_DIALOG), 
				HVTWin, 
				(DLGPROC)BroadcastCommandDlgProc
				);

	if (hDlgWnd == NULL)
		return;

activate:;
	::ShowWindow(hDlgWnd, SW_SHOW);

}

// WM_COPYDATAの受信
LONG CVTWindow::OnReceiveIpcMessage(UINT wParam, LONG lParam)
{
	int len;
	COPYDATASTRUCT *cds;
	char *buf;

	if (!cv.Ready)
		return 0;

	if (!ts.AcceptBroadcast)	// 337: 2007/03/20
		return 0;

	cds = (COPYDATASTRUCT *)lParam;
	len = cds->cbData;
	buf = (char *)cds->lpData;
	if (cds->dwData == IPC_BROADCAST_COMMAND) {
		// 端末へ文字列を送り込む
		// DDE通信に使う関数に変更。(2006.2.7 yutaka)
		CBStartPaste(HVTWin, FALSE, 300/*CBBufSize*/, buf, len);
	}

	return 0;
}



void CVTWindow::OnControlOpenTEK()
{
  OpenTEK();
}

void CVTWindow::OnControlCloseTEK()
{
  if ((HTEKWin==NULL) ||
      ! ::IsWindowEnabled(HTEKWin))
    MessageBeep(0);
  else
    ::DestroyWindow(HTEKWin);
}

void CVTWindow::OnControlMacro()
{
  RunMacro(NULL,FALSE);
}

void CVTWindow::OnWindowWindow()
{
  BOOL Close;

  HelpId = HlpWindowWindow;
  if (! LoadTTDLG()) return;
  (*WindowWindow)(HVTWin,&Close);
  FreeTTDLG();
  if (Close) OnClose();
}

void CVTWindow::OnHelpIndex()
{
	// 英語版HTML helpの表示 (2006.3.11 yutaka)
#if 0
	OpenHelp(HVTWin,HELP_INDEX,0);	
#else
	OpenHtmlHelp(HVTWin, HTML_HELP_EN);
#endif
}

void CVTWindow::OnHelpUsing()
{
	// 日本語版HTML helpの表示 (2006.3.11 yutaka)
#if 0
	::WinHelp(HVTWin, "", HELP_HELPONHELP, 0);	
#else
	OpenHtmlHelp(HVTWin, HTML_HELP_JP);
#endif
}

void CVTWindow::OnHelpAbout()
{
  if (! LoadTTDLG()) return;
  (*AboutDialog)(HVTWin);
  FreeTTDLG();
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.56  2007/03/23 05:01:32  yutakapon
 * 337氏パッチを取り込んだ。
 * 1.Broadcastダイアログからの送信対象を親Windowのみにする、
 * および
 * 2.Broadcastダイアログからの送信を無視可能にする
 *
 * Revision 1.55  2007/03/18 13:29:05  maya
 * Additional setting に ConfirmPasteMouseRButton と DisableAcceleratorSendBreak 用のコントロールを追加した。
 *
 * Revision 1.54  2007/03/17 14:43:10  maya
 * Additional settings のマウスカーソルとANSIカラーの設定を Visual タブに移動した。
 *
 * Revision 1.53  2007/03/17 13:14:54  maya
 * Send break のアクセラレータキーを無効にできるようにした。
 *
 * Revision 1.52  2007/03/17 07:39:00  maya
 * 右クリックによる貼り付けをする前に、ユーザに問い合わせできるようにした。
 *
 * Revision 1.51  2007/03/08 13:30:33  maya
 * SetThreadLocale で TTProxy の言語を変えるのをやめた。
 * lng ファイルから LCID を読み込む。
 *
 * Revision 1.50  2007/03/03 03:51:20  maya
 * Broadcast Command の履歴を保存できるようにした。
 *
 * Revision 1.49  2007/02/19 17:04:30  maya
 * NT 系で GetOpenFileName が開かないバグを修正した。
 *
 * Revision 1.48  2007/02/04 13:45:34  maya
 * Windows98/NT4.0 では OPENFILENAME 構造体のサイズが違うために GetOpenFileName が開かないバグを修正した。
 *
 * Revision 1.47  2007/01/31 13:15:26  maya
 * 言語ファイルがないときに \0 が正しく認識されないバグを修正した。
 *
 * Revision 1.46  2007/01/21 16:18:35  maya
 * 表示メッセージの読み込み対応
 *
 * Revision 1.45  2007/01/11 12:27:16  yutakapon
 * ConnectingTimeout機構を追加した
 *
 * Revision 1.44  2007/01/04 15:11:44  maya
 * 表示メッセージの読み込み対応
 *
 * Revision 1.43  2006/12/23 02:50:17  maya
 * htmlヘルプをプログラムから呼び出すための準備をした。
 *
 * Revision 1.42  2006/12/13 12:31:25  yutakapon
 * ファイル名にゴミが入るのを修正。
 *
 * Revision 1.41  2006/12/12 15:43:48  yutakapon
 * Fileメニュー配下に、ログ再生を行う"Replay Log.."を追加した。
 *
 * Revision 1.40  2006/12/12 14:42:45  maya
 * TTProxy のダイアログの言語のため、SetThreadLocale するようにした。
 *
 * Revision 1.39  2006/11/23 02:19:12  maya
 * 表示メッセージを言語ファイルから読み込みむコードの作成を開始した。
 *
 * Revision 1.38  2006/11/13 15:59:49  yutakapon
 * ScrollLockキーをサポートした
 *
 * Revision 1.37  2006/09/18 02:23:19  maya
 * 最初のウインドウで、接続ダイアログの /L= パラメータが使用されないバグを修正した。
 *
 * Revision 1.36  2006/09/14 17:01:09  maya
 * ComAutoConnect セクションを削除した。
 * /M コマンドラインパラメータが指定されている場合、TeraTerm 起動時に自動的にシリアルポートへ接続しないようにした。
 *
 * Revision 1.35  2006/09/02 08:14:04  maya
 * シリアルポート接続が保存されている場合、自動的に接続するかどうかを設定するオプションを追加した。
 *
 * Revision 1.34  2006/08/28 12:27:16  maya
 * デフォルトのログファイル名を指定できるようにした。
 *   エディットコントロールを "Additional settings" ダイアログに追加した。
 *   teraterm.ini ファイルに LogDefaultName エントリを追加した。
 *   ファイル名に strftime のフォーマットを使えるようにした。
 *
 * Revision 1.33  2006/03/31 16:33:45  yutakakn
 * 半透明化を行わない場合に画面がちらつかないようにした。
 *
 * Revision 1.32  2006/03/17 14:26:57  yutakakn
 * (none)
 *
 * Revision 1.31  2006/03/16 15:35:00  yutakakn
 * (none)
 *
 * Revision 1.30  2006/03/12 14:27:41  yutakakn
 *   ・Additional settingsダイアログにおけるウィンドウの半透明変更を即座に反映させるようにした（teraterm.ini の AlphaBlend=256 の場合のみ）。
 *   ・文字の背景色をスクリーンの背景色と一致させるパッチのバグを修正した。パッチ作成に感謝します＞337氏
 *
 * Revision 1.29  2006/03/10 15:44:29  yutakakn
 * ヘルプファイルを .hlp から .chm へ変更した
 *
 * Revision 1.28  2006/02/24 13:20:47  yutakakn
 * Window setupでのカラー変更がリアルタイムに行われるようにした。
 *
 * Revision 1.27  2006/02/18 08:40:07  yutakakn
 *   ・コンパイラを Visual Studio 2005 Standard Edition に切り替えた。
 *   ・stricmp()を_stricmp()へ置換した
 *   ・strnicmp()を_strnicmp()へ置換した
 *   ・strdup()を_strdup()へ置換した
 *   ・chdir()を_chdir()へ置換した
 *   ・getcwd()を_getcwd()へ置換した
 *   ・strupr()を_strupr()へ置換した
 *   ・time_tの64ビット化にともなう警告メッセージを抑止した
 *   ・TeraTerm Menuがビルドエラーとなる現象に対処した
 *   ・Oniguruma 4.0.1へ差し替えた
 *
 * Revision 1.26  2006/02/07 14:42:12  yutakakn
 * Broadcast Command使用時にウィンドウがちらつく動作を改善した。
 *
 * Revision 1.25  2006/01/20 16:35:54  yutakakn
 * ファイル送信前にダイアログを追加
 *
 * Revision 1.24  2005/12/12 15:40:25  yutakakn
 * 設定内容がteraterm.ini以外のファイル名で保存できないバグを修正した。
 *
 * Revision 1.23  2005/11/03 13:34:27  yutakakn
 *   ・teraterm.iniを保存するときに書き込みできるかどうかの判別を追加した。
 *   ・TCP/IP setupダイアログの"Term type"を常に有効とするようにした。
 *
 * Revision 1.22  2005/10/15 10:29:33  yutakakn
 * Cygwin接続の複製ができるようにした
 *
 * Revision 1.21  2005/10/03 16:57:21  yutakakn
 * スクロールレンジを 16bit から 32bit へ拡張した
 *
 * Revision 1.20  2005/05/15 11:49:32  yutakakn
 * ブロック選択のキーバインドを Shift+MouseDrag から Alt+MouseDrag へ変更した。
 * 左クリックで開始位置の記録、Shift+左クリックで終了位置を取得し、ページをまたぐ選択
 * をできるようにした。
 *
 * Revision 1.19  2005/05/07 13:28:16  yutakakn
 * CygTermの設定を Addtional settings 上で行えるようにした。
 *
 * Revision 1.18  2005/04/24 11:16:31  yutakakn
 * Eterm lookfeelの初期値が ttset から反映していなかったバグを修正。
 *
 * Revision 1.17  2005/04/24 11:03:42  yutakakn
 * Eterm lookfeel alphablendの設定内容を teraterm.ini へ保存するようにした。
 * また、Additional settingsダイアログから on/off できるようにした。
 *
 * Revision 1.16  2005/04/24 05:37:05  yutakakn
 * ALT + Enterキー（トグル）でウィンドウの最大化を行えるようにした。
 *
 * Revision 1.15  2005/04/08 14:53:28  yutakakn
 * "Duplicate session"においてSSH自動ログインを行うようにした。
 *
 * Revision 1.14  2005/04/03 13:42:07  yutakakn
 * URL文字列をダブルクリックするとブラウザが起動するしかけを追加（石崎氏パッチがベース）。
 *
 * Revision 1.13  2005/03/16 14:10:39  yutakakn
 * マウス右ボタン押下でのペーストを制御する設定項目を追加。
 * teraterm.iniに DisablePasteMouseRButton エントリを追加。
 *
 * Revision 1.12  2005/02/22 11:46:46  yutakakn
 * Additional settingsをtab control化した
 *
 * Revision 1.11  2005/02/21 14:58:00  yutakakn
 * LogMeIn -> LogMeTT へリネームにより、起動実行ファイル名も変更した。
 *
 * Revision 1.10  2005/02/03 14:36:16  yutakakn
 * AKASI氏によるEterm風透過ウィンドウ機能を追加。
 * VTColorの初期値は、teraterm.iniのANSI Colorを優先させた。
 *
 * Revision 1.9  2005/02/02 12:54:39  yutakakn
 * ログ採取中に File -> log がグレイ表示にならない問題への対処。
 *
 * Revision 1.8  2005/01/29 16:13:42  yutakakn
 * "Additional settings"の"Viewlog Editor"で、OKボタン押下時にテキストボックスから
 * コピーしていなかったバグを修正。
 *
 * Revision 1.7  2005/01/29 05:27:35  yutakakn
 * "View Log"メニューの追加。
 * "Additional settings"にView Log Editorボックスを追加。
 * teraterm.iniに"ViewlogEditor"エントリを追加。
 *
 * Revision 1.6  2005/01/22 06:44:34  yutakakn
 * すべてのTeraTermへ同一コマンドを送信することができる 'Broadcast command' を
 * Control menu配下に追加した。
 * 'Additional settings'のフォントを tahoma(8) へ変更した。
 *
 * Revision 1.5  2005/01/15 13:29:29  yutakakn
 * TeraTermウィンドウの最大化ボタンを有効にした。
 * ただし、タイトルバーをダブルクリックしても最大化はしない。
 * また、TEKには未対応。
 *
 * Revision 1.4  2004/12/07 14:27:21  yutakakn
 * Additional settingsダイアログにフォーカスを当てるようにした。
 * また、tab orderの調整。
 *
 * Revision 1.3  2004/12/07 13:39:54  yutakakn
 * External SetupをSetupメニュー配下へ移動。
 * LogMeInの起動メニューを追加。
 * Duplication sessionメニューを追加。
 *
 * Revision 1.2  2004/12/03 15:52:55  yutakakn
 * FileメニューにTeraTerm Menuの起動エントリを追加。
 * また、アクセラレータキー(Alt+M)も追加した。
 *
 */
