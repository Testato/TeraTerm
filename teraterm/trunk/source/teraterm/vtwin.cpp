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
	ON_COMMAND(ID_FILE_NEWCONNECTION, OnFileNewConnection)
	ON_COMMAND(ID_FILE_CYGWINCONNECTION, OnCygwinConnection)
	ON_COMMAND(ID_FILE_TERATERMMENU, OnTTMenuLaunch)
	ON_COMMAND(ID_FILE_LOG, OnFileLog)
	ON_COMMAND(ID_FILE_COMMENTTOLOG, OnCommentToLog)
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
	ON_COMMAND(ID_EDIT_EXTERNALSETUP, OnExternalSetup)
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


static void SetWindowStyle(TTTSet *ts)
{
	HCURSOR hc;
	char *Temp = ts->MouseCursorName;
	LPCTSTR name = NULL;
	LONG_PTR lp;
	int i;

	for (i = 0 ; MouseCursor[i].name ; i++) {
		if (stricmp(Temp, MouseCursor[i].name) == 0) {
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
	  // 2�߈ȍ~�̃v���Z�X�ɂ����Ă��A�f�B�X�N���� TERATERM.INI ��ǂށB(2004.11.4 yutaka)
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

  /* Initialize scroll buffer */
  InitBuffer();

  InitDisp();

  if (ts.HideTitle>0)
    Style = WS_VSCROLL | WS_HSCROLL |
			WS_BORDER | WS_THICKFRAME | WS_POPUP;
  else
    Style = WS_VSCROLL | WS_HSCROLL |
	    WS_BORDER | WS_THICKFRAME |
	    WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

  wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = AfxWndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = AfxGetInstanceHandle();
  wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_VT));
  wc.hCursor = LoadCursor(NULL,IDC_IBEAM);
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
  // ���P�[���̐ݒ�
  setlocale(LC_ALL, ts.Locale);

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
  if (LButton || MButton || RButton)
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
  if (Paste)
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
      BuffStartSelect(p.x,p.y,(LMR==IdLeftButton) & ShiftKey());
      TplClk = FALSE;
      /* for AutoScrolling */
      ::SetCapture(HVTWin);
      ::SetTimer(HVTWin, IdScrollTimer, 100, NULL);
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
  if ((ts.MenuFlag & MF_SHOWWINMENU) !=0)
  {
    WinMenu = CreatePopupMenu();
    ::InsertMenu(*Menu,ID_HELPMENU,
		 MF_STRING | MF_ENABLED |
		 MF_POPUP | MF_BYPOSITION,
		 (int)WinMenu, "&Window");
  }

  TTXModifyMenu(*Menu); /* TTPLUG */
}

void CVTWindow::InitMenuPopup(HMENU SubMenu)
{
	if ( SubMenu == FileMenu )
	{
		if ( Connecting )
			EnableMenuItem(FileMenu,ID_FILE_NEWCONNECTION,MF_BYCOMMAND | MF_GRAYED);
		else
			EnableMenuItem(FileMenu,ID_FILE_NEWCONNECTION,MF_BYCOMMAND | MF_ENABLED);

		if (LogVar!=NULL) { // ���O�̎惂�[�h�̏ꍇ
			EnableMenuItem(FileMenu,ID_FILE_LOG,MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(FileMenu,ID_FILE_COMMENTTOLOG, MF_BYCOMMAND | MF_ENABLED);
		} else {
			EnableMenuItem(FileMenu,ID_FILE_LOG,MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(FileMenu,ID_FILE_COMMENTTOLOG, MF_BYCOMMAND | MF_GRAYED);
		}

		if ( (! cv.Ready) || (SendVar!=NULL) || (FileVar!=NULL) ||
			(cv.PortType==IdFile) )
		{
			EnableMenuItem(FileMenu,ID_FILE_SENDFILE,MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(FileMenu,ID_TRANSFER,MF_BYPOSITION | MF_GRAYED); /* Transfer */
			EnableMenuItem(FileMenu,ID_FILE_CHANGEDIR,MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(FileMenu,ID_FILE_DISCONNECT,MF_BYCOMMAND | MF_GRAYED);
		}
		else {
			EnableMenuItem(FileMenu,ID_FILE_SENDFILE,MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(FileMenu,ID_TRANSFER,MF_BYPOSITION | MF_ENABLED); /* Transfer */
			EnableMenuItem(FileMenu,ID_FILE_CHANGEDIR,MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(FileMenu,ID_FILE_DISCONNECT,MF_BYCOMMAND | MF_ENABLED);
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
		SetWinMenu(WinMenu);
	}

	TTXModifyPopupMenu(SubMenu); /* TTPLUG */
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
      Result = HTCAPTION;
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
  if (cv.Ready && (cv.PortType==IdTCPIP) &&
      ((ts.PortFlag & PF_CONFIRMDISCONN) != 0) &&
      ! CloseTT &&
      (::MessageBox(HVTWin,"Disconnect?","Tera Term",
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

			// �f�B���N�g���̏ꍇ�̓t���p�X����\��t���� (2004.11.3 yutaka)
			attr = GetFileAttributes(SendVar->FullName);
			if (attr != -1 && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
				ptr = SendVar->FullName;
				// �p�X�̋�؂�� \ -> / ��
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

				// �p�X�ɋ󔒂�����΃G�X�P�[�v����
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

				// console�֑��M
				while (*ptr) {
					FSOut1(*ptr);
					if (ts.LocalEcho > 0) {
						FSEcho1(*ptr);
					}
					ptr++;
				}
				FreeFileVar(&SendVar); // �����Y�ꂸ��

			} else {
				SendVar->DirLen = 0;
				ts.TransBin = 0;
				FileSendStart();
			}
		}
		else
			FreeFileVar(&SendVar);
	}
	DragFinish(hDropInfo);
}

void CVTWindow::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
  lpMMI->ptMaxSize.x = 10000;
  lpMMI->ptMaxSize.y = 10000;
  lpMMI->ptMaxTrackSize.x = 10000;
  lpMMI->ptMaxTrackSize.y = 10000;
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
  BuffDblClk(DblClkX, DblClkY);
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
  if (! (LButton || MButton || RButton)) return;
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

// �}�E�X�z�C�[���̉�]
BOOL CVTWindow::OnMouseWheel(
   UINT nFlags,   // ���z�L�[
   short zDelta,  // ��]����
   CPoint pt      // �J�[�\���ʒu
)
{
	int line, i, backward;
	short delta;

	if (zDelta < 0) { // �}�C�i�X�Ȃ烆�[�U�Ɍ������ĉ�]
		backward = 1;
	} else {
		backward = 0;
	}
	delta = abs(zDelta);
	line = delta / WHEEL_DELTA; // ���C����
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
  ButtonUp(TRUE);
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
}

void CVTWindow::OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  char e = ESC;
  char Code;
  unsigned int i;

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
  DispVScroll(Func,nPos);
}

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
      ::InsertMenu(MainMenu,ID_HELPMENU,
	MF_STRING | MF_ENABLED |
	MF_POPUP | MF_BYPOSITION,
	(int)WinMenu, "&Window");
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
    AppendMenu(SysMenu, MF_STRING, ID_SHOWMENUBAR, "Show menu &bar");
  }
  return 0;
}

LONG CVTWindow::OnChangeTBar(UINT wParam, LONG lParam)
{
  BOOL TBar;
  DWORD Style;
  HMENU SysMenu;

  Style = GetWindowLong (HVTWin, GWL_STYLE);
  TBar = ((Style & WS_SYSMENU)!=0);
  if (TBar == (ts.HideTitle==0)) return 0;
  if (ts.HideTitle>0)
    Style = Style & ~(WS_SYSMENU | WS_CAPTION |
		      WS_MINIMIZEBOX) | WS_BORDER | WS_POPUP;
  else
    Style = Style & ~WS_POPUP | WS_SYSMENU | WS_CAPTION |
	    WS_MINIMIZEBOX;
  AdjustSize = TRUE;
  SetWindowLong(HVTWin, GWL_STYLE, Style);
  ::SetWindowPos(HVTWin, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |
		 SWP_NOZORDER | SWP_FRAMECHANGED);
  ::ShowWindow(HVTWin, SW_SHOW);

  if ((ts.HideTitle==0) && (MainMenu==NULL) &&
      ((ts.MenuFlag & MF_NOSHOWMENU) == 0))
  {
    SysMenu = ::GetSystemMenu(HVTWin,FALSE);
    AppendMenu(SysMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(SysMenu, MF_STRING, ID_SHOWMENUBAR, "Show menu &bar");
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
  CommStart(&cv,lParam);
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
  if ((ts.PortType!=IdSerial) && (ts.HostName[0]==0))
    OnFileNewConnection();
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
        /* �����Ƀv���g�R������������ */
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
		::MessageBox(NULL, "Can't find Cygwin directory.", "ERROR", MB_OK | MB_ICONWARNING);
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
		::MessageBox(NULL, "Can't execute Cygterm.", "ERROR", MB_OK | MB_ICONWARNING);
	}
}


//
// TeraTerm Menu�̋N��
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
		_snprintf(buf, sizeof(buf), "Can't execute TeraTerm Menu. (%d)", GetLastError());
		::MessageBox(NULL, buf, "ERROR", MB_OK | MB_ICONWARNING);
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

    switch (msg) {
        case WM_INITDIALOG:
			//SetDlgItemText(hDlgWnd, IDC_EDIT_COMMENT, "�T���v��");
			// �G�f�B�b�g�R���g���[���Ƀt�H�[�J�X�����Ă�
			SetFocus(GetDlgItem(hDlgWnd, IDC_EDIT_COMMENT));
			return FALSE;

        case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDOK:
					memset(buf, 0, sizeof(buf));
					ret = GetDlgItemText(hDlgWnd, IDC_EDIT_COMMENT, buf, sizeof(buf) - 1);
					if (ret > 0) { // �e�L�X�g�擾����
						//buf[sizeof(buf) - 1] = '\0';  // null-terminate
						CommentLogToFile(buf, ret);
					}

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

	// ���O�t�@�C���փR�����g��ǉ����� (2004.8.6 yutaka)
	ret = DialogBox(hInst, MAKEINTRESOURCE(IDD_COMMENT_DIALOG), HVTWin, (DLGPROC)OnCommentDlgProc);
	if (ret == 0 || ret == -1) {
		ret = GetLastError();
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
  if ((cv.PortType==IdTCPIP) &&
      ((ts.PortFlag & PF_CONFIRMDISCONN) != 0) &&
      (::MessageBox(HVTWin,"Disconnect?","Tera Term",
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
	// �o�b�t�@�̑S�I��
	POINT p = {0, 0};

	ButtonDown(p, IdLeftButton);
	BuffAllSelect();
    ButtonUp(FALSE);
	ChangeSelectRegion();
}


//
// cf. http://homepage2.nifty.com/DSS/VCPP/API/SHBrowseForFolder.htm
//
static void doSelectFolder(HWND hWnd, char *path, int pathlen)
{
	BROWSEINFO      bi;
	LPSTR           lpBuffer;
	LPITEMIDLIST    pidlRoot;      // �u���E�Y�̃��[�gPIDL
	LPITEMIDLIST    pidlBrowse;    // ���[�U�[���I������PIDL
	LPMALLOC        lpMalloc = NULL;

	HRESULT hr = SHGetMalloc(&lpMalloc);
	if (FAILED(hr)) 
		return;

	// �u���E�Y���󂯎��o�b�t�@�̈�̊m��
	if ((lpBuffer = (LPSTR) lpMalloc->Alloc(_MAX_PATH)) == NULL) {
		return;
	}
	// �_�C�A���O�\�����̃��[�g�t�H���_��PIDL���擾
	// ���ȉ��̓f�X�N�g�b�v�����[�g�Ƃ��Ă���B�f�X�N�g�b�v�����[�g�Ƃ���
	//   �ꍇ�́A�P�� bi.pidlRoot �ɂO��ݒ肷�邾���ł��悢�B���̑��̓�
	//   ��t�H���_�����[�g�Ƃ��鎖���ł���B�ڍׂ�SHGetSpecialFolderLoca
	//   tion�̃w���v���Q�Ƃ̎��B
	if (!SUCCEEDED(SHGetSpecialFolderLocation(  hWnd,
		CSIDL_DESKTOP,
		&pidlRoot))) { 
			lpMalloc->Free(lpBuffer);
			return;
	}

	// BROWSEINFO�\���̂̏����l�ݒ�
	// ��BROWSEINFO�\���̂̊e�����o�̏ڍא������w���v���Q��
	bi.hwndOwner = hWnd;
	bi.pidlRoot = pidlRoot;
	bi.pszDisplayName = lpBuffer;
	bi.lpszTitle = "select folder";
	bi.ulFlags = 0;
	bi.lpfn = 0;
	bi.lParam = 0;
	// �t�H���_�I���_�C�A���O�̕\�� 
	pidlBrowse = SHBrowseForFolder(&bi);
	if (pidlBrowse != NULL) {  
		// PIDL�`���̖߂�l�̃t�@�C���V�X�e���̃p�X�ɕϊ�
		if (SHGetPathFromIDList(pidlBrowse, lpBuffer)) {
			// �擾����
			strncpy(path, lpBuffer, pathlen);
		}
		// SHBrowseForFolder�̖߂�lPIDL�����
		lpMalloc->Free(pidlBrowse);
	}
	// �N���[���A�b�v����
	lpMalloc->Free(pidlRoot); 
	lpMalloc->Free(lpBuffer);
	lpMalloc->Release();
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


static LRESULT CALLBACK OnExtSetupDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static HDC label_hdc = NULL;
	HWND hWnd;
	int i;
	LRESULT lr;
	char buf[MAXPATHLEN];

    switch (msg) {
        case WM_INITDIALOG:
			// (1)Enable continued-line copy
			hWnd = GetDlgItem(hDlgWnd, IDC_LINECOPY);
			if (ts.EnableContinuedLineCopy == TRUE) {
				SendMessage(hWnd, BM_SETCHECK, BST_CHECKED, 0);
			} else {
				SendMessage(hWnd, BM_SETCHECK, BST_UNCHECKED, 0);
			}

			// (2)Mouse cursor type
			hWnd = GetDlgItem(hDlgWnd, IDC_MOUSE_CURSOR);
			for (i = 0 ; MouseCursor[i].name ; i++) {
				SendMessage(hWnd, LB_INSERTSTRING, i, (LPARAM)MouseCursor[i].name);
			}
			SendMessage(hWnd, LB_SELECTSTRING , 0, (LPARAM)ts.MouseCursorName);

			// (3)AlphaBlend 
			hWnd = GetDlgItem(hDlgWnd, IDC_ALPHA_BLEND);
			_snprintf(buf, sizeof(buf), "%d", ts.AlphaBlend);
			SendMessage(hWnd, WM_SETTEXT , 0, (LPARAM)buf);

			// (4)Cygwin install path
			hWnd = GetDlgItem(hDlgWnd, IDC_CYGWIN_PATH);
			SendMessage(hWnd, WM_SETTEXT , 0, (LPARAM)ts.CygwinDirectory);

			// (5)delimiter characters
			hWnd = GetDlgItem(hDlgWnd, IDC_DELIM_LIST);
			SendMessage(hWnd, WM_SETTEXT , 0, (LPARAM)ts.DelimList);

			// (6)ANSI color
			hWnd = GetDlgItem(hDlgWnd, IDC_ANSI_COLOR);
			for (i = 0 ; i < 16 ; i++) {
				_snprintf(buf, sizeof(buf), "%d", i);
				SendMessage(hWnd, LB_INSERTSTRING, i, (LPARAM)buf);
			}
			SetupRGBbox(hDlgWnd, 0);

			return FALSE;

        case WM_COMMAND:
            switch (wp) {
				case IDC_LINECOPY | (BN_CLICKED << 16):
					return TRUE;

				case IDC_SELECT_FILE | (BN_CLICKED << 16):
					// Cygwin install �f�B���N�g���̑I���_�C�A���O
					doSelectFolder(hDlgWnd, ts.CygwinDirectory, sizeof(ts.CygwinDirectory));
					// (4)Cygwin install path
					hWnd = GetDlgItem(hDlgWnd, IDC_CYGWIN_PATH);
					SendMessage(hWnd, WM_SETTEXT , 0, (LPARAM)ts.CygwinDirectory);
					return TRUE;

				case IDC_ANSI_COLOR | (LBN_SELCHANGE << 16):
					hWnd = GetDlgItem(hDlgWnd, IDC_ANSI_COLOR);
					lr = SendMessage(hWnd, LB_GETCURSEL, 0, 0);
					if (lr != -1) {
						SetupRGBbox(hDlgWnd, lr);
						SendMessage(hDlgWnd, WM_CTLCOLORSTATIC, (WPARAM)label_hdc, (LPARAM)hWnd);
					}
					return TRUE;

				case IDC_COLOR_RED | (EN_KILLFOCUS << 16):
				case IDC_COLOR_GREEN | (EN_KILLFOCUS << 16):
				case IDC_COLOR_BLUE | (EN_KILLFOCUS << 16):
					{
					BYTE r, g, b;

					hWnd = GetDlgItem(hDlgWnd, IDC_ANSI_COLOR);
					lr = SendMessage(hWnd, LB_GETCURSEL, 0, 0);

					hWnd = GetDlgItem(hDlgWnd, IDC_COLOR_RED);
					SendMessage(hWnd, WM_GETTEXT , sizeof(buf), (LPARAM)buf);
					r = atoi(buf);

					hWnd = GetDlgItem(hDlgWnd, IDC_COLOR_GREEN);
					SendMessage(hWnd, WM_GETTEXT , sizeof(buf), (LPARAM)buf);
					g = atoi(buf);

					hWnd = GetDlgItem(hDlgWnd, IDC_COLOR_BLUE);
					SendMessage(hWnd, WM_GETTEXT , sizeof(buf), (LPARAM)buf);
					b = atoi(buf);

					ts.ANSIColor[lr] = RGB(r, g, b);
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
					hWnd = GetDlgItem(hDlgWnd, IDC_MOUSE_CURSOR);
					lr = SendMessage(hWnd, LB_GETCURSEL, 0, 0);
					if (lr >= 0 && lr < MOUSE_CURSOR_MAX) {
						strcpy(ts.MouseCursorName, MouseCursor[lr].name);
					}

					// (3)
					hWnd = GetDlgItem(hDlgWnd, IDC_ALPHA_BLEND);
					SendMessage(hWnd, WM_GETTEXT , sizeof(buf), (LPARAM)buf);
					ts.AlphaBlend = atoi(buf);

					// (4)
					hWnd = GetDlgItem(hDlgWnd, IDC_CYGWIN_PATH);
					SendMessage(hWnd, WM_GETTEXT , sizeof(ts.CygwinDirectory), (LPARAM)ts.CygwinDirectory);

					// (5)
					hWnd = GetDlgItem(hDlgWnd, IDC_DELIM_LIST);
					SendMessage(hWnd, WM_GETTEXT , sizeof(ts.DelimList), (LPARAM)ts.DelimList);


                    EndDialog(hDlgWnd, IDOK);
                    break;

                case IDCANCEL:
                    EndDialog(hDlgWnd, IDCANCEL);
                    break;

                default:
                    return FALSE;
            }

        case WM_CLOSE:
		    EndDialog(hDlgWnd, 0);
			return TRUE;

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


        default:
            return FALSE;
    }
    return TRUE;
}

// �R���t�B�O���[�V�����_�C�A���O
void CVTWindow::OnExternalSetup()
{
	DWORD ret;

	// �ݒ�_�C�A���O (2004.9.5 yutaka)
	ret = DialogBox(hInst, MAKEINTRESOURCE(IDD_EXTERNAL_SETUP), HVTWin, (DLGPROC)OnExtSetupDlgProc);
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

  if (Ok) ChangeWin();
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

  strcpy(TmpSetupFN,ts.SetupFName);
  if (! LoadTTFILE()) return;
  HelpId = HlpSetupSave;
  Ok = (*GetSetupFname)(HVTWin,GSF_SAVE,&ts);
  FreeTTFILE();
  if (! Ok) return;

  if (LoadTTSET())
  {
    /* write current setup values to file */
    (*WriteIniFile)(ts.SetupFName,&ts);
    /* copy host list */
    (*CopyHostList)(TmpSetupFN,ts.SetupFName);
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
  OpenHelp(HVTWin,HELP_INDEX,0);	
}

void CVTWindow::OnHelpUsing()
{
  ::WinHelp(HVTWin, "", HELP_HELPONHELP, 0);	
}

void CVTWindow::OnHelpAbout()
{
  if (! LoadTTDLG()) return;
  (*AboutDialog)(HVTWin);
  FreeTTDLG();
}

/*
 * $Log: not supported by cvs2svn $
 */
