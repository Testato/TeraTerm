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
#ifndef NO_INET6
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <winsock.h>
#endif /* NO_INET6 */
#include "ttplug.h"  /* TTPLUG */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include <shlobj.h>
#include <io.h>
#include <errno.h>
#include <imagehlp.h>

#include <windowsx.h>
#include <imm.h>

#include "tt_res.h"
#include "vtwin.h"
#include "addsetting.h"
#include "winjump.h"

#define VTClassName "VTWin32"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// �E�B���h�E�ő剻�{�^����L���ɂ��� (2005.1.15 yutaka)
#define WINDOW_MAXMIMUM_ENABLED 1

// WM_COPYDATA�ɂ��v���Z�X�ԒʐM�̎�� (2005.1.22 yutaka)
#define IPC_BROADCAST_COMMAND 1      // �S�[���֑��M
#define IPC_MULTICAST_COMMAND 2      // �C�ӂ̒[���Q�֑��M

#define BROADCAST_LOGFILE "broadcast.log"

static HFONT DlgBroadcastFont;
static HFONT DlgCommentFont;

static BOOL TCPLocalEchoUsed = FALSE;
static BOOL TCPCRSendUsed = FALSE;

static BOOL IgnoreRelease = FALSE;

// �{�̂� addsetting.cpp
extern mouse_cursor_t MouseCursor[];

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
	ON_WM_SIZING()
	ON_WM_SYSCHAR()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_SYSCOMMAND()
	ON_WM_SYSKEYDOWN()
	ON_WM_SYSKEYUP()
	ON_WM_TIMER()
	ON_WM_VSCROLL()
	ON_MESSAGE(WM_IME_COMPOSITION,OnIMEComposition)
	ON_MESSAGE(WM_INPUTLANGCHANGE,OnIMEInputChange)
	ON_MESSAGE(WM_IME_NOTIFY,OnIMENotify)
	ON_MESSAGE(WM_IME_REQUEST,OnIMERequest)
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
	ON_MESSAGE(WM_USER_CHANGETITLE,OnChangeTitle)
	ON_MESSAGE(WM_COPYDATA,OnReceiveIpcMessage)
	ON_MESSAGE(WM_USER_NONCONFIRM_CLOSE, OnNonConfirmClose)
	ON_COMMAND(ID_FILE_NEWCONNECTION, OnFileNewConnection)
	ON_COMMAND(ID_FILE_DUPLICATESESSION, OnDuplicateSession)
	ON_COMMAND(ID_FILE_CYGWINCONNECTION, OnCygwinConnection)
	ON_COMMAND(ID_FILE_TERATERMMENU, OnTTMenuLaunch)
	ON_COMMAND(ID_FILE_LOGMEIN, OnLogMeInLaunch)
	ON_COMMAND(ID_FILE_LOG, OnFileLog)
	ON_COMMAND(ID_FILE_COMMENTTOLOG, OnCommentToLog)
	ON_COMMAND(ID_FILE_VIEWLOG, OnViewLog)
	ON_COMMAND(ID_FILE_SHOWLOGDIALOG, OnShowLogDialog)
	ON_COMMAND(ID_FILE_REPLAYLOG, OnReplayLog)
	ON_COMMAND(ID_FILE_SENDFILE, OnFileSend)
	ON_COMMAND(ID_FILE_KERMITRCV, OnFileKermitRcv)
	ON_COMMAND(ID_FILE_KERMITGET, OnFileKermitGet)
	ON_COMMAND(ID_FILE_KERMITSEND, OnFileKermitSend)
	ON_COMMAND(ID_FILE_KERMITFINISH, OnFileKermitFinish)
	ON_COMMAND(ID_FILE_XRCV, OnFileXRcv)
	ON_COMMAND(ID_FILE_XSEND, OnFileXSend)
	ON_COMMAND(ID_FILE_YRCV, OnFileYRcv)
	ON_COMMAND(ID_FILE_YSEND, OnFileYSend)
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
	ON_COMMAND(ID_FILE_EXITALL, OnAllClose)
	ON_COMMAND(ID_EDIT_COPY2, OnEditCopy)
	ON_COMMAND(ID_EDIT_COPYTABLE, OnEditCopyTable)
	ON_COMMAND(ID_EDIT_PASTE2, OnEditPaste)
	ON_COMMAND(ID_EDIT_PASTECR, OnEditPasteCR)
	ON_COMMAND(ID_EDIT_CLEARSCREEN, OnEditClearScreen)
	ON_COMMAND(ID_EDIT_CLEARBUFFER, OnEditClearBuffer)
	ON_COMMAND(ID_EDIT_CANCELSELECT, OnEditCancelSelection)
	ON_COMMAND(ID_EDIT_SELECTALL, OnEditSelectAllBuffer)
	ON_COMMAND(ID_EDIT_SELECTSCREEN, OnEditSelectScreenBuffer)
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
	ON_COMMAND(ID_CONTROL_RESETREMOTETITLE, OnControlResetRemoteTitle)
	ON_COMMAND(ID_CONTROL_AREYOUTHERE, OnControlAreYouThere)
	ON_COMMAND(ID_CONTROL_SENDBREAK, OnControlSendBreak)
	ON_COMMAND(ID_CONTROL_RESETPORT, OnControlResetPort)
	ON_COMMAND(ID_CONTROL_BROADCASTCOMMAND, OnControlBroadcastCommand)
	ON_COMMAND(ID_CONTROL_OPENTEK, OnControlOpenTEK)
	ON_COMMAND(ID_CONTROL_CLOSETEK, OnControlCloseTEK)
	ON_COMMAND(ID_CONTROL_MACRO, OnControlMacro)
	ON_COMMAND(ID_CONTROL_SHOW_MACRO, OnShowMacroWindow)
	ON_COMMAND(ID_WINDOW_WINDOW, OnWindowWindow)
	ON_COMMAND(ID_WINDOW_MINIMIZEALL, OnWindowMinimizeAll)
	ON_COMMAND(ID_WINDOW_CASCADEALL, OnWindowCascade)
	ON_COMMAND(ID_WINDOW_STACKED, OnWindowStacked)
	ON_COMMAND(ID_WINDOW_SIDEBYSIDE, OnWindowSidebySide)
	ON_COMMAND(ID_WINDOW_RESTOREALL, OnWindowRestoreAll)
	ON_COMMAND(ID_WINDOW_UNDO, OnWindowUndo)
	ON_COMMAND(ID_HELP_INDEX2, OnHelpIndex)
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
	char user32_dll[MAX_PATH];

	GetSystemDirectory(user32_dll, sizeof(user32_dll));
	strncat_s(user32_dll, sizeof(user32_dll), "\\user32.dll", _TRUNCATE);
	if (g_hmodUser32 == NULL) {
		g_hmodUser32 = LoadLibrary(user32_dll);
		if (g_hmodUser32 == NULL) {
			return FALSE;
		}

		g_pSetLayeredWindowAttributes =
			(func)GetProcAddress(g_hmodUser32, "SetLayeredWindowAttributes");
	}

	if (g_pSetLayeredWindowAttributes == NULL) {
		return FALSE;
	}

	return g_pSetLayeredWindowAttributes(hwnd, crKey, 
	                                     bAlpha, dwFlags);
}


// Tera Term�N������URL������mouse over���ɌĂ΂�� (2005.4.2 yutaka)
void SetMouseCursor(char *cursor)
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
	if (name == NULL) {
		return;
	}

	hc = (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(name), IMAGE_CURSOR,
	                        0, 0, LR_DEFAULTSIZE | LR_SHARED);

	if (hc != NULL) {
		SetClassLongPtr(HVTWin, GCLP_HCURSOR, (LONG_PTR)hc);
	}
}


void SetWindowStyle(TTTSet *ts)
{
	LONG_PTR lp;

	SetMouseCursor(ts->MouseCursorName);

	// 2006/03/16 by 337: BGUseAlphaBlendAPI��On�Ȃ��Layered�����Ƃ���
	//if (ts->EtermLookfeel.BGUseAlphaBlendAPI) {
	// �A���t�@�l��255�̏ꍇ�A��ʂ̂������}���邽�߉������Ȃ����ƂƂ���B(2006.4.1 yutaka)
	// �Ăяo�����ŁA�l���ύX���ꂽ�Ƃ��̂ݐݒ�𔽉f����B(2007.10.19 maya)
	if (ts->AlphaBlend < 255) {
		lp = GetWindowLongPtr(HVTWin, GWL_EXSTYLE);
		if (lp != 0) {
			SetWindowLongPtr(HVTWin, GWL_EXSTYLE, lp | WS_EX_LAYERED);
			MySetLayeredWindowAttributes(HVTWin, 0, ts->AlphaBlend, LWA_ALPHA);
		}
	}
	// �A���t�@�l�� 255 �̏ꍇ�A�������������폜���čĕ`�悷��B(2007.10.22 maya)
	else {
		lp = GetWindowLongPtr(HVTWin, GWL_EXSTYLE);
		if (lp != 0) {
			SetWindowLongPtr(HVTWin, GWL_EXSTYLE, lp & ~WS_EX_LAYERED);
			RedrawWindow(HVTWin, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME);
		}
	}
}


//
// ��O�n���h���̃t�b�N�i�X�^�b�N�g���[�X�̃_���v�j
//
// cf. http://svn.collab.net/repos/svn/trunk/subversion/libsvn_subr/win32_crashrpt.c
// (2007.9.30 yutaka)
//
// ��O�R�[�h�𕶎���֕ϊ�����
static char *GetExceptionString(int exception)
{
#define EXCEPTION(x) case EXCEPTION_##x: return (#x);
	static char buf[16];

	switch (exception)
	{
		EXCEPTION(ACCESS_VIOLATION)
		EXCEPTION(DATATYPE_MISALIGNMENT)
		EXCEPTION(BREAKPOINT)
		EXCEPTION(SINGLE_STEP)
		EXCEPTION(ARRAY_BOUNDS_EXCEEDED)
		EXCEPTION(FLT_DENORMAL_OPERAND)
		EXCEPTION(FLT_DIVIDE_BY_ZERO)
		EXCEPTION(FLT_INEXACT_RESULT)
		EXCEPTION(FLT_INVALID_OPERATION)
		EXCEPTION(FLT_OVERFLOW)
		EXCEPTION(FLT_STACK_CHECK)
		EXCEPTION(FLT_UNDERFLOW)
		EXCEPTION(INT_DIVIDE_BY_ZERO)
		EXCEPTION(INT_OVERFLOW)
		EXCEPTION(PRIV_INSTRUCTION)
		EXCEPTION(IN_PAGE_ERROR)
		EXCEPTION(ILLEGAL_INSTRUCTION)
		EXCEPTION(NONCONTINUABLE_EXCEPTION)
		EXCEPTION(STACK_OVERFLOW)
		EXCEPTION(INVALID_DISPOSITION)
		EXCEPTION(GUARD_PAGE)
		EXCEPTION(INVALID_HANDLE)

	default:
		_snprintf_s(buf, sizeof(buf), _TRUNCATE, "0x%x", exception);
		return buf;
		//return "UNKNOWN_ERROR";
	}
#undef EXCEPTION
}

/* ��O�������Ɋ֐��̌Ăяo��������\������A��O�t�B���^�֐� */
static LONG CALLBACK ApplicationFaultHandler(EXCEPTION_POINTERS *ExInfo)
{
	HGLOBAL gptr;
	STACKFRAME sf;
	BOOL bResult;
	PIMAGEHLP_SYMBOL pSym;
	DWORD Disp;
	HANDLE hProcess = GetCurrentProcess();
	HANDLE hThread = GetCurrentThread();
	IMAGEHLP_MODULE	ih_module;
	IMAGEHLP_LINE	ih_line;
	int frame;
	char msg[3072], buf[256];
	HMODULE h, h2;
	char imagehlp_dll[MAX_PATH];

	// Windows98/Me/NT4�ł͓����Ȃ����߃X�L�b�v����B(2007.10.9 yutaka)
	GetSystemDirectory(imagehlp_dll, sizeof(imagehlp_dll));
	strncat_s(imagehlp_dll, sizeof(imagehlp_dll), "\\imagehlp.dll", _TRUNCATE);
	h2 = LoadLibrary(imagehlp_dll);
	if (((h = GetModuleHandle(imagehlp_dll)) == NULL) ||
		(GetProcAddress(h, "SymGetLineFromAddr") == NULL)) {
			FreeLibrary(h2);
			goto error;
	}
	FreeLibrary(h2);

	/* �V���{�����i�[�p�o�b�t�@�̏����� */
	gptr = GlobalAlloc(GMEM_FIXED, 10000);
	if (gptr == NULL) {
		goto error;
	}
	pSym = (PIMAGEHLP_SYMBOL)GlobalLock(gptr);
	ZeroMemory(pSym, sizeof(IMAGEHLP_SYMBOL));
	pSym->SizeOfStruct = 10000;
	pSym->MaxNameLength = 10000 - sizeof(IMAGEHLP_SYMBOL);

	/* �X�^�b�N�t���[���̏����� */
	ZeroMemory(&sf, sizeof(sf));
	sf.AddrPC.Offset = ExInfo->ContextRecord->Eip;
	sf.AddrStack.Offset = ExInfo->ContextRecord->Esp;
	sf.AddrFrame.Offset = ExInfo->ContextRecord->Ebp;
	sf.AddrPC.Mode = AddrModeFlat;
	sf.AddrStack.Mode = AddrModeFlat;
	sf.AddrFrame.Mode = AddrModeFlat;

	/* �V���{���n���h���̏����� */
	SymInitialize(hProcess, NULL, TRUE);
	
	// ���W�X�^�_���v
	msg[0] = '\0';
	_snprintf_s(buf, sizeof(buf), _TRUNCATE, "eax=%08X ebx=%08X ecx=%08X edx=%08X esi=%08X edi=%08X\r\n"
		   "ebp=%08X esp=%08X eip=%08X efl=%08X\r\n"
		   "cs=%04X ss=%04X ds=%04X es=%04X fs=%04X gs=%04X\r\n",
		   ExInfo->ContextRecord->Eax, 
		   ExInfo->ContextRecord->Ebx, 
		   ExInfo->ContextRecord->Ecx, 
		   ExInfo->ContextRecord->Edx, 
		   ExInfo->ContextRecord->Esi, 
		   ExInfo->ContextRecord->Edi, 
		   ExInfo->ContextRecord->Ebp, 
		   ExInfo->ContextRecord->Esp, 
		   ExInfo->ContextRecord->Eip,
		   ExInfo->ContextRecord->EFlags,
		   ExInfo->ContextRecord->SegCs,
		   ExInfo->ContextRecord->SegSs,
		   ExInfo->ContextRecord->SegDs,
		   ExInfo->ContextRecord->SegEs,
		   ExInfo->ContextRecord->SegFs,
		   ExInfo->ContextRecord->SegGs
	);
	strncat_s(msg, sizeof(msg), buf, _TRUNCATE);

	if (ExInfo->ExceptionRecord != NULL) {
		_snprintf_s(buf, sizeof(buf), _TRUNCATE, "Exception: %s\r\n", GetExceptionString(ExInfo->ExceptionRecord->ExceptionCode));
		strncat_s(msg, sizeof(msg), buf, _TRUNCATE);
	}

	/* �X�^�b�N�t���[�������ɕ\�����Ă��� */
	frame = 0;
	for (;;) {
		/* ���̃X�^�b�N�t���[���̎擾 */
		bResult = StackWalk(
			IMAGE_FILE_MACHINE_I386,
			hProcess,
			hThread,
			&sf,
			NULL, 
			NULL,
			SymFunctionTableAccess,
			SymGetModuleBase,
			NULL);

		/* ���s�Ȃ�΁A���[�v�𔲂��� */
		if (!bResult || sf.AddrFrame.Offset == 0) 
			break;
		
		frame++;

		/* �v���O�����J�E���^�i���z�A�h���X�j����֐����ƃI�t�Z�b�g���擾 */
		bResult = SymGetSymFromAddr(hProcess, sf.AddrPC.Offset, &Disp, pSym);
		
		/* �擾���ʂ�\�� */
		_snprintf_s(buf, sizeof(buf), _TRUNCATE, "#%d  0x%08x in ", frame, sf.AddrPC.Offset);
		strncat_s(msg, sizeof(msg), buf, _TRUNCATE);
		if (bResult) {
			_snprintf_s(buf, sizeof(buf), _TRUNCATE, "%s() + 0x%x ", pSym->Name, Disp);
			strncat_s(msg, sizeof(msg), buf, _TRUNCATE);
		} else {
			_snprintf_s(buf, sizeof(buf), _TRUNCATE, " --- ");
			strncat_s(msg, sizeof(msg), buf, _TRUNCATE);
		}
		
		// ���s�t�@�C�����̎擾
		ZeroMemory( &(ih_module), sizeof(ih_module) );
		ih_module.SizeOfStruct = sizeof(ih_module);
		bResult = SymGetModuleInfo( hProcess, sf.AddrPC.Offset, &(ih_module) );
		strncat_s(msg, sizeof(msg), "at ", _TRUNCATE);
		if (bResult) {
			_snprintf_s(buf, sizeof(buf), _TRUNCATE, "%s ", ih_module.ImageName );
			strncat_s(msg, sizeof(msg), buf, _TRUNCATE);
		} else {
			_snprintf_s(buf, sizeof(buf), _TRUNCATE, "%s ", "<Unknown Module>" );
			strncat_s(msg, sizeof(msg), buf, _TRUNCATE);
		}
		
		// �t�@�C�����ƍs�ԍ��̎擾
		ZeroMemory( &(ih_line), sizeof(ih_line) );
		ih_line.SizeOfStruct = sizeof(ih_line);
		bResult = SymGetLineFromAddr( hProcess, sf.AddrPC.Offset, &Disp, &ih_line );
		if (bResult)
		{
			_snprintf_s(buf, sizeof(buf), _TRUNCATE, "%s:%lu", ih_line.FileName, ih_line.LineNumber );
			strncat_s(msg, sizeof(msg), buf, _TRUNCATE);
		}
		
		strncat_s(msg, sizeof(msg), "\n", _TRUNCATE);
	}

	/* �㏈�� */
	SymCleanup(hProcess);
	GlobalUnlock(pSym);
	GlobalFree(pSym);

	MessageBox(NULL, msg, "Tera Term: Application fault", MB_OK | MB_ICONEXCLAMATION);

error:
//	return (EXCEPTION_EXECUTE_HANDLER);  /* ���̂܂܃v���Z�X���I�������� */
	return (EXCEPTION_CONTINUE_SEARCH);  /* ���������m�A�v���P�[�V�����G���[�n�|�b�v�A�b�v���b�Z�[�W�{�b�N�X���Ăяo�� */
}


CVTWindow::CVTWindow()
{
	WNDCLASS wc;
	RECT rect;
	DWORD Style;
#ifdef ALPHABLEND_TYPE2
	DWORD ExStyle;
#endif
	char Temp[MAX_PATH];
	char *Param;
	int CmdShow;
	PKeyMap tempkm;
	int fuLoad = LR_DEFAULTCOLOR;

#ifdef _DEBUG
  ::_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// ��O�n���h���̃t�b�N (2007.9.30 yutaka)
	SetUnhandledExceptionFilter(ApplicationFaultHandler);

	TTXInit(&ts, &cv); /* TTPLUG */

	CommInit(&cv);

	MsgDlgHelp = RegisterWindowMessage(HELPMSGSTRING);

	if (StartTeraTerm(&ts)) {
		/* first instance */
		if (LoadTTSET()) {
			/* read setup info from "teraterm.ini" */
			(*ReadIniFile)(ts.SetupFName, &ts);
			/* read keycode map from "keyboard.cnf" */
			tempkm = (PKeyMap)malloc(sizeof(TKeyMap));
			if (tempkm!=NULL) {
				strncpy_s(Temp, sizeof(Temp), ts.HomeDir, _TRUNCATE);
				AppendSlash(Temp,sizeof(Temp));
				strncat_s(Temp,sizeof(Temp),"KEYBOARD.CNF",_TRUNCATE);
				(*ReadKeyboardCnf)(Temp,tempkm,TRUE);
			}
			FreeTTSET();
			/* store default sets in TTCMN */
#if 0
			ChangeDefaultSet(&ts,tempkm);
#else
			ChangeDefaultSet(NULL,tempkm);
#endif
			if (tempkm!=NULL) free(tempkm);
		}

	} else {
		// 2�߈ȍ~�̃v���Z�X�ɂ����Ă��A�f�B�X�N���� TERATERM.INI ��ǂށB(2004.11.4 yutaka)
		if (LoadTTSET()) {
			/* read setup info from "teraterm.ini" */
			(*ReadIniFile)(ts.SetupFName, &ts);
			/* read keycode map from "keyboard.cnf" */
			tempkm = (PKeyMap)malloc(sizeof(TKeyMap));
			if (tempkm!=NULL) {
				strncpy_s(Temp, sizeof(Temp), ts.HomeDir, _TRUNCATE);
				AppendSlash(Temp,sizeof(Temp));
				strncat_s(Temp,sizeof(Temp),"KEYBOARD.CNF",_TRUNCATE);
				(*ReadKeyboardCnf)(Temp,tempkm,TRUE);
			}
			FreeTTSET();
			/* store default sets in TTCMN */
			if (tempkm!=NULL) {
				free(tempkm);
			}
		}

	}

	/* Parse command line parameters*/
	// 256�o�C�g�ȏ�̃R�}���h���C���p�����[�^�w�肪����ƁABOF(Buffer Over Flow)��
	// ������o�O���C���B(2007.6.12 maya)
	Param = GetCommandLine();
	if (LoadTTSET()) {
		(*ParseParam)(Param, &ts, &(TopicName[0]));
	}
	FreeTTSET();

	// duplicate session�̎w�肪����Ȃ�A���L����������R�s�[���� (2004.12.7 yutaka)
	if (ts.DuplicateSession == 1) {
		CopyShmemToTTSet(&ts);
	}

	InitKeyboard();
	SetKeyMap();

	// �R�}���h���C���ł��ݒ�t�@�C���ł��ύX���Ȃ��̂ł����ŏ����� (2008.1.25 maya)
	cv.isSSH = 0;
	cv.TitleRemote[0] = '\0';

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
	ScrollLock = FALSE;  // �����l�͖��� (2006.11.14 yutaka)

	/* Initialize scroll buffer */
	InitBuffer();

	InitDisp();

	if (ts.HideTitle>0) {
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
	wc.hIcon = NULL;
	//wc.hCursor = LoadCursor(NULL,IDC_IBEAM);
	wc.hCursor = NULL; // �}�E�X�J�[�\���͓��I�ɕύX���� (2005.4.2 yutaka)
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = VTClassName;

	RegisterClass(&wc);
	LoadAccelTable(MAKEINTRESOURCE(IDR_ACC));

	if (ts.VTPos.x==CW_USEDEFAULT) {
		rect = rectDefault;
	}
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
	// wctomb �̂���
	setlocale(LC_ALL, ts.Locale);

#ifdef ALPHABLEND_TYPE2
//<!--by AKASI
	if(BGNoFrame && ts.HideTitle > 0) {
		ExStyle  = GetWindowLong(HVTWin,GWL_EXSTYLE);
		ExStyle &= ~WS_EX_CLIENTEDGE;
		SetWindowLong(HVTWin,GWL_EXSTYLE,ExStyle);
	}
//-->
#endif

	if (is_NT4()) {
		fuLoad = LR_VGACOLOR;
	}
	::PostMessage(HVTWin,WM_SETICON,ICON_SMALL,
	              (LPARAM)LoadImage(AfxGetInstanceHandle(),
	                                MAKEINTRESOURCE((ts.VTIcon!=IdIconDefault)?ts.VTIcon:IDI_VT),
	                                IMAGE_ICON,16,16,fuLoad));
	// Vista �� Aero �ɂ����� Alt+Tab �؂�ւ��ŕ\�������A�C�R����
	// 16x16 �A�C�R���̊g��ɂȂ��Ă��܂��̂ŁA�傫���A�C�R����
	// �Z�b�g���� (2008.9.3 maya)
	::PostMessage(HVTWin,WM_SETICON,ICON_BIG,
	              (LPARAM)LoadImage(AfxGetInstanceHandle(),
	                                MAKEINTRESOURCE((ts.VTIcon!=IdIconDefault)?ts.VTIcon:IDI_VT),
	                                IMAGE_ICON, 0, 0, fuLoad));

	MainMenu = NULL;
	WinMenu = NULL;
	if ((ts.HideTitle==0) && (ts.PopupMenu==0)) {
		InitMenu(&MainMenu);
		::SetMenu(HVTWin,MainMenu);
	}

	/* Reset Terminal */
	ResetTerminal();

	if ((ts.PopupMenu>0) || (ts.HideTitle>0)) {
		::PostMessage(HVTWin,WM_USER_CHANGEMENU,0,0);
	}

	ChangeFont();

	ResetIME();

	BuffChangeWinSize(NumOfColumns,NumOfLines);

	ChangeTitle();
	/* Enable drag-drop */
	::DragAcceptFiles(HVTWin,TRUE);

	if (ts.HideWindow>0) {
		if (strlen(TopicName)>0) {
			InitDDE();
			SendDDEReady();
		}
		FirstPaint = FALSE;
		Startup();
		return;
	}
	CmdShow = SW_SHOWDEFAULT;
	if (ts.Minimize>0) {
		CmdShow = SW_SHOWMINIMIZED;
	}
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
	BOOL disableBuffEndSelect = false;
	BOOL pasteRButton = RButton && Paste;
	BOOL pasteMButton = MButton && Paste;

	/* disable autoscrolling */
	::KillTimer(HVTWin,IdScrollTimer);
	ReleaseCapture();

	if (ts.SelectOnlyByLButton &&
	    (MButton || RButton)) {
		disableBuffEndSelect = true;
	}

	LButton = FALSE;
	MButton = FALSE;
	RButton = FALSE;
	DblClk = FALSE;
	TplClk = FALSE;
	CaretOn();

	// SelectOnlyByLButton �� on �� ���E�E�N���b�N�����Ƃ���
	// �o�b�t�@���I����Ԃ�������A�I����e���N���b�v�{�[�h��
	// �R�s�[����Ă��܂������C�� (2007.12.6 maya)
	if (!disableBuffEndSelect) {
		BuffEndSelect();
	}

	// added ConfirmPasteMouseRButton (2007.3.17 maya)
	if (pasteRButton && !ts.ConfirmPasteMouseRButton) {
		if (CBStartPasteConfirmChange(HVTWin, FALSE)) {
			CBStartPaste(HVTWin, FALSE, BracketedPasteMode(), 0, NULL, 0);
			/* �ŉ��s�ł��������X�N���[������ݒ�̏ꍇ
			   �y�[�X�g�����ŃX�N���[�������� */
			if (ts.AutoScrollOnlyInBottomLine != 0 && WinOrgY != 0) {
				DispVScroll(SCROLL_BOTTOM, 0);
			}
		}
	}
	else if (pasteMButton) {
		if (CBStartPasteConfirmChange(HVTWin, FALSE)) {
			CBStartPaste(HVTWin, FALSE, BracketedPasteMode(), 0, NULL, 0);
			/* �ŉ��s�ł��������X�N���[������ݒ�̏ꍇ
			   �y�[�X�g�����ŃX�N���[�������� */
			if (ts.AutoScrollOnlyInBottomLine != 0 && WinOrgY != 0) {
				DispVScroll(SCROLL_BOTTOM, 0);
			}
		}
	}
}

void CVTWindow::ButtonDown(POINT p, int LMR)
{
	HMENU PopupMenu, PopupBase;
	BOOL mousereport;

	if ((LMR==IdLeftButton) && ControlKey() && (MainMenu==NULL) &&
	    ((ts.MenuFlag & MF_NOPOPUP)==0)) {
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
				            (MF_CHECKED | MF_DISABLED | MF_GRAYED | MF_HILITE |
				             MF_MENUBARBREAK | MF_MENUBREAK | MF_SEPARATOR);

				AppendMenu(PopupBase,
				           submenu != NULL ? LOBYTE(state) | MF_POPUP : state,
				           submenu != NULL ? (UINT)submenu : GetMenuItemID(PopupMenu, i),
				           itemText);
			}
		}

		::ClientToScreen(HVTWin, &p);
		TrackPopupMenu(PopupBase,TPM_LEFTALIGN | TPM_LEFTBUTTON,
		               p.x,p.y,0,HVTWin,NULL);
		if (WinMenu!=NULL) {
			DestroyMenu(WinMenu);
			WinMenu = NULL;
		}
		DestroyMenu(PopupBase);
		DestroyMenu(PopupMenu);
		PopupMenu = 0;
		return;
	}

	if (mousereport = MouseReport(IdMouseEventBtnDown, LMR, p.x, p.y)) {
		return;
	}

	// added ConfirmPasteMouseRButton (2007.3.17 maya)
	if ((LMR == IdRightButton) &&
		!ts.DisablePasteMouseRButton &&
		ts.ConfirmPasteMouseRButton &&
		cv.Ready &&
		!mousereport &&
		(SendVar==NULL) && (FileVar==NULL) &&
		(cv.PortType!=IdFile) &&
		(IsClipboardFormatAvailable(CF_TEXT) ||
		 IsClipboardFormatAvailable(CF_OEMTEXT))) {

		int i, numItems;
		char itemText[256];

		InitPasteMenu(&PopupMenu);
		PopupBase = CreatePopupMenu();
		numItems = GetMenuItemCount(PopupMenu);

		for (i = 0; i < numItems; i++) {
			if (GetMenuString(PopupMenu, i, itemText, sizeof(itemText), MF_BYPOSITION) != 0) {
				int state = GetMenuState(PopupMenu, i, MF_BYPOSITION) &
				            (MF_CHECKED | MF_DISABLED | MF_GRAYED | MF_HILITE |
				             MF_MENUBARBREAK | MF_MENUBREAK | MF_SEPARATOR);

				AppendMenu(PopupBase, state,
				           GetMenuItemID(PopupMenu, i), itemText);
			}
		}

		::ClientToScreen(HVTWin, &p);
		TrackPopupMenu(PopupBase,TPM_LEFTALIGN | TPM_LEFTBUTTON,
		               p.x,p.y,0,HVTWin,NULL);
		if (WinMenu!=NULL) {
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
	    (abs(p.y-DblClkY)<=GetSystemMetrics(SM_CYDOUBLECLK))) {
		/* triple click */
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
		if (! (LButton || MButton || RButton)) {
			BOOL box = FALSE;

			// select several pages of output from Tera Term window (2005.5.15 yutaka)
			if (LMR == IdLeftButton && ShiftKey()) {
				BuffSeveralPagesSelect(p.x, p.y);

			} else {
				// Select rectangular block with Alt Key. Delete Shift key.(2005.5.15 yutaka)
				if (LMR == IdLeftButton && AltKey()) {
					box = TRUE;
				}

				// Starting the selection only by a left button.(2007.11.20 maya)
				if (!ts.SelectOnlyByLButton ||
				    (ts.SelectOnlyByLButton && LMR == IdLeftButton) ) {
					BuffStartSelect(p.x,p.y, box);
					TplClk = FALSE;

					/* for AutoScrolling */
					::SetCapture(HVTWin);
					::SetTimer(HVTWin, IdScrollTimer, 100, NULL);
				}
			}
		}

		switch (LMR) {
			case IdRightButton:
				RButton = TRUE;
				break;
			case IdMiddleButton:
				MButton = TRUE;
				break;
			case IdLeftButton:
				LButton = TRUE;
				break;
		}
	}
}

// LogMeIn.exe -> LogMeTT.exe �փ��l�[�� (2005.2.21 yutaka)
static char LogMeTTMenuString[] = "Log&MeTT";
static char LogMeTT[MAX_PATH];

static BOOL isLogMeTTExist()
{
	const char *LogMeTTexename = "LogMeTT.exe";
	LONG result;
	HKEY key;
	int inregist = 0;
	DWORD dwSize;
	DWORD dwType;
	DWORD dwDisposition;
	char *path;

	/* LogMeTT 2.9.6����̓��W�X�g���ɃC���X�g�[���p�X���܂܂��B*/
	result = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\LogMeTT", 0, NULL,
				REG_OPTION_NON_VOLATILE, KEY_READ, NULL, &key, &dwDisposition);
	if (result == ERROR_SUCCESS) {
		result = RegQueryValueEx(key, "InstallPath", NULL, &dwType, NULL, &dwSize);
		if (result == ERROR_SUCCESS) {
			path = (char *)malloc(dwSize);
			if (path != NULL) {
				result = RegQueryValueEx(key, "InstallPath", NULL, &dwType, (LPBYTE)path, &dwSize);
				if (result == ERROR_SUCCESS) {
					inregist = 1;
					strncpy_s(LogMeTT, sizeof(LogMeTT), path, _TRUNCATE);
				}
				free(path);
			}
		}
		RegCloseKey(key);
	}

	if (inregist == 0) {
		strncpy_s(LogMeTT, sizeof(LogMeTT), ts.HomeDir, _TRUNCATE);
		AppendSlash(LogMeTT, sizeof(LogMeTT));
		strncat_s(LogMeTT, sizeof(LogMeTT), LogMeTTexename, _TRUNCATE);
	}

	if (_access(LogMeTT, 0) == -1) {
		return FALSE;
	}
	return TRUE;
}

void CVTWindow::InitMenu(HMENU *Menu)
{
	*Menu = LoadMenu(AfxGetInstanceHandle(),
	                 MAKEINTRESOURCE(IDR_MENU));
	char uimsg[MAX_UIMSG];
	int ret;

	FileMenu = GetSubMenu(*Menu,ID_FILE);
	TransMenu = GetSubMenu(FileMenu,ID_TRANSFER);
	EditMenu = GetSubMenu(*Menu,ID_EDIT);
	SetupMenu = GetSubMenu(*Menu,ID_SETUP);
	ControlMenu = GetSubMenu(*Menu,ID_CONTROL);
	HelpMenu = GetSubMenu(*Menu,ID_HELPMENU);

	/* LogMeTT �̑��݂��m�F���ă��j���[��ǉ����� */
	if (isLogMeTTExist()) {
		::InsertMenu(FileMenu, ID_FILE_PRINT2, MF_STRING | MF_ENABLED | MF_BYCOMMAND,
		             ID_FILE_LOGMEIN, LogMeTTMenuString);
		::InsertMenu(FileMenu, ID_FILE_PRINT2, MF_SEPARATOR, NULL, NULL);
	}

	GetMenuString(*Menu, ID_FILE, uimsg, sizeof(uimsg), MF_BYPOSITION);
	get_lang_msg("MENU_FILE", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(*Menu, ID_FILE, MF_BYPOSITION, ID_FILE, ts.UIMsg);
	GetMenuString(FileMenu, ID_FILE_NEWCONNECTION, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_FILE_NEW", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(FileMenu, ID_FILE_NEWCONNECTION, MF_BYCOMMAND, ID_FILE_NEWCONNECTION, ts.UIMsg);
	GetMenuString(FileMenu, ID_FILE_DUPLICATESESSION, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_FILE_DUPLICATE", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(FileMenu, ID_FILE_DUPLICATESESSION, MF_BYCOMMAND, ID_FILE_DUPLICATESESSION, ts.UIMsg);
	GetMenuString(FileMenu, ID_FILE_CYGWINCONNECTION, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_FILE_GYGWIN", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(FileMenu, ID_FILE_CYGWINCONNECTION, MF_BYCOMMAND, ID_FILE_CYGWINCONNECTION, ts.UIMsg);
	GetMenuString(FileMenu, ID_FILE_LOG, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_FILE_LOG", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(FileMenu, ID_FILE_LOG, MF_BYCOMMAND, ID_FILE_LOG, ts.UIMsg);
	GetMenuString(FileMenu, ID_FILE_COMMENTTOLOG, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_FILE_COMMENTLOG", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(FileMenu, ID_FILE_COMMENTTOLOG, MF_BYCOMMAND, ID_FILE_COMMENTTOLOG, ts.UIMsg);
	GetMenuString(FileMenu, ID_FILE_VIEWLOG, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_FILE_VIEWLOG", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(FileMenu, ID_FILE_VIEWLOG, MF_BYCOMMAND, ID_FILE_VIEWLOG, ts.UIMsg);
	GetMenuString(FileMenu, ID_FILE_SHOWLOGDIALOG, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_FILE_SHOWLOGDIALOG", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(FileMenu, ID_FILE_SHOWLOGDIALOG, MF_BYCOMMAND, ID_FILE_SHOWLOGDIALOG, ts.UIMsg);
	GetMenuString(FileMenu, ID_FILE_SENDFILE, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_FILE_SENDFILE", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(FileMenu, ID_FILE_SENDFILE, MF_BYCOMMAND, ID_FILE_SENDFILE, ts.UIMsg);
	GetMenuString(FileMenu, ID_FILE_REPLAYLOG, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_FILE_REPLAYLOG", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(FileMenu, ID_FILE_REPLAYLOG, MF_BYCOMMAND, ID_FILE_REPLAYLOG, ts.UIMsg);
	GetMenuString(FileMenu, ID_FILE_CHANGEDIR, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_FILE_CHANGEDIR", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(FileMenu, ID_FILE_CHANGEDIR, MF_BYCOMMAND, ID_FILE_CHANGEDIR, ts.UIMsg);
	ret = GetMenuString(FileMenu, ID_FILE_LOGMEIN, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	if (ret != 0) {
		get_lang_msg("MENU_FILE_LOGMETT", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
		ModifyMenu(FileMenu, ID_FILE_LOGMEIN, MF_BYCOMMAND, ID_FILE_LOGMEIN, ts.UIMsg);
	}
	GetMenuString(FileMenu, ID_FILE_PRINT2, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_FILE_PRINT", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(FileMenu, ID_FILE_PRINT2, MF_BYCOMMAND, ID_FILE_PRINT2, ts.UIMsg);
	GetMenuString(FileMenu, ID_FILE_DISCONNECT, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_FILE_DISCONNECT", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(FileMenu, ID_FILE_DISCONNECT, MF_BYCOMMAND, ID_FILE_DISCONNECT, ts.UIMsg);
	GetMenuString(FileMenu, ID_FILE_EXIT, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_FILE_EXIT", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(FileMenu, ID_FILE_EXIT, MF_BYCOMMAND, ID_FILE_EXIT, ts.UIMsg);
	GetMenuString(FileMenu, ID_FILE_EXITALL, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_FILE_EXITALL", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(FileMenu, ID_FILE_EXITALL, MF_BYCOMMAND, ID_FILE_EXITALL, ts.UIMsg);

	GetMenuString(FileMenu, 9, uimsg, sizeof(uimsg), MF_BYPOSITION);
	get_lang_msg("MENU_TRANS", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(FileMenu, 9, MF_BYPOSITION, 9, ts.UIMsg);

	GetMenuString(FileMenu, ID_FILE_KERMITRCV, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_TRANS_KERMIT_RCV", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(FileMenu, ID_FILE_KERMITRCV, MF_BYCOMMAND, ID_FILE_KERMITRCV, ts.UIMsg);
	GetMenuString(FileMenu, ID_FILE_KERMITGET, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_TRANS_KERMIT_GET", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(FileMenu, ID_FILE_KERMITGET, MF_BYCOMMAND, ID_FILE_KERMITGET, ts.UIMsg);
	GetMenuString(FileMenu, ID_FILE_KERMITSEND, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_TRANS_KERMIT_SEND", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(FileMenu, ID_FILE_KERMITSEND, MF_BYCOMMAND, ID_FILE_KERMITSEND, ts.UIMsg);
	GetMenuString(FileMenu, ID_FILE_KERMITFINISH, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_TRANS_KERMIT_FINISH", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(FileMenu, ID_FILE_KERMITFINISH, MF_BYCOMMAND, ID_FILE_KERMITFINISH, ts.UIMsg);

	GetMenuString(FileMenu, ID_FILE_XRCV, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_TRANS_X_RCV", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(FileMenu, ID_FILE_XRCV, MF_BYCOMMAND, ID_FILE_XRCV, ts.UIMsg);
	GetMenuString(FileMenu, ID_FILE_XSEND, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_TRANS_X_SEND", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(FileMenu, ID_FILE_XSEND, MF_BYCOMMAND, ID_FILE_XSEND, ts.UIMsg);

	GetMenuString(FileMenu, ID_FILE_YRCV, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_TRANS_Y_RCV", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(FileMenu, ID_FILE_YRCV, MF_BYCOMMAND, ID_FILE_YRCV, ts.UIMsg);
	GetMenuString(FileMenu, ID_FILE_YSEND, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_TRANS_Y_SEND", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(FileMenu, ID_FILE_YSEND, MF_BYCOMMAND, ID_FILE_YSEND, ts.UIMsg);

	GetMenuString(FileMenu, ID_FILE_ZRCV, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_TRANS_Z_RCV", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(FileMenu, ID_FILE_ZRCV, MF_BYCOMMAND, ID_FILE_ZRCV, ts.UIMsg);
	GetMenuString(FileMenu, ID_FILE_ZSEND, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_TRANS_Z_SEND", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(FileMenu, ID_FILE_ZSEND, MF_BYCOMMAND, ID_FILE_ZSEND, ts.UIMsg);

	GetMenuString(FileMenu, ID_FILE_BPRCV, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_TRANS_BP_RCV", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(FileMenu, ID_FILE_BPRCV, MF_BYCOMMAND, ID_FILE_BPRCV, ts.UIMsg);
	GetMenuString(FileMenu, ID_FILE_BPSEND, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_TRANS_BP_SEND", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(FileMenu, ID_FILE_BPSEND, MF_BYCOMMAND, ID_FILE_BPSEND, ts.UIMsg);

	GetMenuString(FileMenu, ID_FILE_QVRCV, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_TRANS_QV_RCV", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(FileMenu, ID_FILE_QVRCV, MF_BYCOMMAND, ID_FILE_QVRCV, ts.UIMsg);
	GetMenuString(FileMenu, ID_FILE_QVSEND, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_TRANS_QV_SEND", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(FileMenu, ID_FILE_QVSEND, MF_BYCOMMAND, ID_FILE_QVSEND, ts.UIMsg);

	GetMenuString(*Menu, ID_EDIT, uimsg, sizeof(uimsg), MF_BYPOSITION);
	get_lang_msg("MENU_EDIT", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(*Menu, ID_EDIT, MF_BYPOSITION, ID_EDIT, ts.UIMsg);
	GetMenuString(EditMenu, ID_EDIT_COPY2, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_EDIT_COPY", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(EditMenu, ID_EDIT_COPY2, MF_BYCOMMAND, ID_EDIT_COPY2, ts.UIMsg);
	GetMenuString(EditMenu, ID_EDIT_COPYTABLE, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_EDIT_COPYTABLE", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(EditMenu, ID_EDIT_COPYTABLE, MF_BYCOMMAND, ID_EDIT_COPYTABLE, ts.UIMsg);
	GetMenuString(EditMenu, ID_EDIT_PASTE2, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_EDIT_PASTE", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(EditMenu, ID_EDIT_PASTE2, MF_BYCOMMAND, ID_EDIT_PASTE2, ts.UIMsg);
	GetMenuString(EditMenu, ID_EDIT_PASTECR, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_EDIT_PASTECR", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(EditMenu, ID_EDIT_PASTECR, MF_BYCOMMAND, ID_EDIT_PASTECR, ts.UIMsg);
	GetMenuString(EditMenu, ID_EDIT_CLEARSCREEN, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_EDIT_CLSCREEN", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(EditMenu, ID_EDIT_CLEARSCREEN, MF_BYCOMMAND, ID_EDIT_CLEARSCREEN, ts.UIMsg);
	GetMenuString(EditMenu, ID_EDIT_CLEARBUFFER, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_EDIT_CLBUFFER", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(EditMenu, ID_EDIT_CLEARBUFFER, MF_BYCOMMAND, ID_EDIT_CLEARBUFFER, ts.UIMsg);
	GetMenuString(EditMenu, ID_EDIT_CANCELSELECT, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_EDIT_CANCELSELECT", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(EditMenu, ID_EDIT_CANCELSELECT, MF_BYCOMMAND, ID_EDIT_CANCELSELECT, ts.UIMsg);
	GetMenuString(EditMenu, ID_EDIT_SELECTSCREEN, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_EDIT_SELECTSCREEN", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(EditMenu, ID_EDIT_SELECTSCREEN, MF_BYCOMMAND, ID_EDIT_SELECTSCREEN, ts.UIMsg);
	GetMenuString(EditMenu, ID_EDIT_SELECTALL, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_EDIT_SELECTALL", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(EditMenu, ID_EDIT_SELECTALL, MF_BYCOMMAND, ID_EDIT_SELECTALL, ts.UIMsg);

	GetMenuString(*Menu, ID_SETUP, uimsg, sizeof(uimsg), MF_BYPOSITION);
	get_lang_msg("MENU_SETUP", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(*Menu, ID_SETUP, MF_BYPOSITION, ID_SETUP, ts.UIMsg);
	GetMenuString(SetupMenu, ID_SETUP_TERMINAL, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_SETUP_TERMINAL", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(SetupMenu, ID_SETUP_TERMINAL, MF_BYCOMMAND, ID_SETUP_TERMINAL, ts.UIMsg);
	GetMenuString(SetupMenu, ID_SETUP_WINDOW, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_SETUP_WINDOW", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(SetupMenu, ID_SETUP_WINDOW, MF_BYCOMMAND, ID_SETUP_WINDOW, ts.UIMsg);
	GetMenuString(SetupMenu, ID_SETUP_FONT, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_SETUP_FONT", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(SetupMenu, ID_SETUP_FONT, MF_BYCOMMAND, ID_SETUP_FONT, ts.UIMsg);
	GetMenuString(SetupMenu, ID_SETUP_KEYBOARD, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_SETUP_KEYBOARD", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(SetupMenu, ID_SETUP_KEYBOARD, MF_BYCOMMAND, ID_SETUP_KEYBOARD, ts.UIMsg);
	GetMenuString(SetupMenu, ID_SETUP_SERIALPORT, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_SETUP_SERIALPORT", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(SetupMenu, ID_SETUP_SERIALPORT, MF_BYCOMMAND, ID_SETUP_SERIALPORT, ts.UIMsg);
	GetMenuString(SetupMenu, ID_SETUP_TCPIP, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_SETUP_TCPIP", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(SetupMenu, ID_SETUP_TCPIP, MF_BYCOMMAND, ID_SETUP_TCPIP, ts.UIMsg);
	GetMenuString(SetupMenu, ID_SETUP_GENERAL, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_SETUP_GENERAL", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(SetupMenu, ID_SETUP_GENERAL, MF_BYCOMMAND, ID_SETUP_GENERAL, ts.UIMsg);
	GetMenuString(SetupMenu, ID_SETUP_ADDITIONALSETTINGS, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_SETUP_ADDITION", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(SetupMenu, ID_SETUP_ADDITIONALSETTINGS, MF_BYCOMMAND, ID_SETUP_ADDITIONALSETTINGS, ts.UIMsg);
	GetMenuString(SetupMenu, ID_SETUP_SAVE, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_SETUP_SAVE", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(SetupMenu, ID_SETUP_SAVE, MF_BYCOMMAND, ID_SETUP_SAVE, ts.UIMsg);
	GetMenuString(SetupMenu, ID_SETUP_RESTORE, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_SETUP_RESTORE", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(SetupMenu, ID_SETUP_RESTORE, MF_BYCOMMAND, ID_SETUP_RESTORE, ts.UIMsg);
	GetMenuString(SetupMenu, ID_SETUP_LOADKEYMAP, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_SETUP_LOADKEYMAP", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(SetupMenu, ID_SETUP_LOADKEYMAP, MF_BYCOMMAND, ID_SETUP_LOADKEYMAP, ts.UIMsg);

	GetMenuString(*Menu, ID_CONTROL, uimsg, sizeof(uimsg), MF_BYPOSITION);
	get_lang_msg("MENU_CONTROL", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(*Menu, ID_CONTROL, MF_BYPOSITION, ID_CONTROL, ts.UIMsg);
	GetMenuString(ControlMenu, ID_CONTROL_RESETTERMINAL, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_CONTROL_RESET", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(ControlMenu, ID_CONTROL_RESETTERMINAL, MF_BYCOMMAND, ID_CONTROL_RESETTERMINAL, ts.UIMsg);
	GetMenuString(ControlMenu, ID_CONTROL_RESETREMOTETITLE, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_CONTROL_RESETTITLE", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(ControlMenu, ID_CONTROL_RESETREMOTETITLE, MF_BYCOMMAND, ID_CONTROL_RESETREMOTETITLE, ts.UIMsg);
	GetMenuString(ControlMenu, ID_CONTROL_AREYOUTHERE, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_CONTROL_AREYOUTHERE", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(ControlMenu, ID_CONTROL_AREYOUTHERE, MF_BYCOMMAND, ID_CONTROL_AREYOUTHERE, ts.UIMsg);
	GetMenuString(ControlMenu, ID_CONTROL_SENDBREAK, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_CONTROL_SENDBREAK", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(ControlMenu, ID_CONTROL_SENDBREAK, MF_BYCOMMAND, ID_CONTROL_SENDBREAK, ts.UIMsg);
	GetMenuString(ControlMenu, ID_CONTROL_RESETPORT, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_CONTROL_RESETPORT", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(ControlMenu, ID_CONTROL_RESETPORT, MF_BYCOMMAND, ID_CONTROL_RESETPORT, ts.UIMsg);
	GetMenuString(ControlMenu, ID_CONTROL_BROADCASTCOMMAND, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_CONTROL_BROADCAST", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(ControlMenu, ID_CONTROL_BROADCASTCOMMAND, MF_BYCOMMAND, ID_CONTROL_BROADCASTCOMMAND, ts.UIMsg);
	GetMenuString(ControlMenu, ID_CONTROL_OPENTEK, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_CONTROL_OPENTEK", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(ControlMenu, ID_CONTROL_OPENTEK, MF_BYCOMMAND, ID_CONTROL_OPENTEK, ts.UIMsg);
	GetMenuString(ControlMenu, ID_CONTROL_CLOSETEK, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_CONTROL_CLOSETEK", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(ControlMenu, ID_CONTROL_CLOSETEK, MF_BYCOMMAND, ID_CONTROL_CLOSETEK, ts.UIMsg);

	GetMenuString(ControlMenu, ID_CONTROL_MACRO, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_CONTROL_MACRO", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(ControlMenu, ID_CONTROL_MACRO, MF_BYCOMMAND, ID_CONTROL_MACRO, ts.UIMsg);
	GetMenuString(ControlMenu, ID_CONTROL_SHOW_MACRO, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_CONTROL_SHOW_MACRO", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(ControlMenu, ID_CONTROL_SHOW_MACRO, MF_BYCOMMAND, ID_CONTROL_SHOW_MACRO, ts.UIMsg);

	GetMenuString(*Menu, ID_HELPMENU, uimsg, sizeof(uimsg), MF_BYPOSITION);
	get_lang_msg("MENU_HELP", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(*Menu, ID_HELPMENU, MF_BYPOSITION, ID_HELPMENU, ts.UIMsg);
	GetMenuString(HelpMenu, ID_HELP_INDEX2, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_HELP_INDEX", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(HelpMenu, ID_HELP_INDEX2, MF_BYCOMMAND, ID_HELP_INDEX2, ts.UIMsg);
	GetMenuString(HelpMenu, ID_HELP_ABOUT, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_HELP_ABOUT", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(HelpMenu, ID_HELP_ABOUT, MF_BYCOMMAND, ID_HELP_ABOUT, ts.UIMsg);

	if ((ts.MenuFlag & MF_SHOWWINMENU) !=0) {
		WinMenu = CreatePopupMenu();
		get_lang_msg("MENU_WINDOW", ts.UIMsg, sizeof(ts.UIMsg),
		             "&Window", ts.UILanguageFile);
		::InsertMenu(*Menu,ID_HELPMENU,
		             MF_STRING | MF_ENABLED | MF_POPUP | MF_BYPOSITION,
		             (int)WinMenu, ts.UIMsg);
	}

	TTXModifyMenu(*Menu); /* TTPLUG */
}

void CVTWindow::InitMenuPopup(HMENU SubMenu)
{
	if ( SubMenu == FileMenu )
	{
		if (ts.DisableMenuNewConnection) {
			if ( Connecting || cv.Open ) {
				EnableMenuItem(FileMenu,ID_FILE_NEWCONNECTION,MF_BYCOMMAND | MF_GRAYED);
			}
			else {
				EnableMenuItem(FileMenu,ID_FILE_NEWCONNECTION,MF_BYCOMMAND | MF_ENABLED);
			}
		}
		else {
			if ( Connecting ) {
				EnableMenuItem(FileMenu,ID_FILE_NEWCONNECTION,MF_BYCOMMAND | MF_GRAYED);
			}
			else {
				EnableMenuItem(FileMenu,ID_FILE_NEWCONNECTION,MF_BYCOMMAND | MF_ENABLED);
			}
		}

		if ( (! cv.Ready) || (SendVar!=NULL) ||
		     (FileVar!=NULL) || (cv.PortType==IdFile) ) {
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
			if (ts.DisableMenuDuplicateSession) {
				EnableMenuItem(FileMenu,ID_FILE_DUPLICATESESSION,MF_BYCOMMAND | MF_GRAYED);
			}
			else {
				EnableMenuItem(FileMenu,ID_FILE_DUPLICATESESSION,MF_BYCOMMAND | MF_ENABLED);
			}
		}

		// �V�K���j���[��ǉ� (2004.12.5 yutaka)
		EnableMenuItem(FileMenu,ID_FILE_CYGWINCONNECTION,MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(FileMenu,ID_FILE_TERATERMMENU,MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(FileMenu,ID_FILE_LOGMEIN,MF_BYCOMMAND | MF_ENABLED);

		// XXX: ���̈ʒu�ɂ��Ȃ��ƁAlog���O���C�ɂȂ�Ȃ��B (2005.2.1 yutaka)
		if (LogVar!=NULL) { // ���O�̎惂�[�h�̏ꍇ
			EnableMenuItem(FileMenu,ID_FILE_LOG,MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(FileMenu,ID_FILE_COMMENTTOLOG, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(FileMenu,ID_FILE_VIEWLOG, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(FileMenu,ID_FILE_SHOWLOGDIALOG, MF_BYCOMMAND | MF_ENABLED);
		} else {
			EnableMenuItem(FileMenu,ID_FILE_LOG,MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(FileMenu,ID_FILE_COMMENTTOLOG, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(FileMenu,ID_FILE_VIEWLOG, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(FileMenu,ID_FILE_SHOWLOGDIALOG, MF_BYCOMMAND | MF_GRAYED);
		}

	}
	else if ( SubMenu == TransMenu )
	{
		if ((cv.PortType==IdSerial) &&
		    ((ts.DataBit==IdDataBit7) || (ts.Flow==IdFlowX))) {
			EnableMenuItem(TransMenu,1,MF_BYPOSITION | MF_GRAYED);  /* XMODEM */
			EnableMenuItem(TransMenu,4,MF_BYPOSITION | MF_GRAYED);  /* Quick-VAN */
		}
		else {
			EnableMenuItem(TransMenu,1,MF_BYPOSITION | MF_ENABLED); /* XMODEM */
			EnableMenuItem(TransMenu,4,MF_BYPOSITION | MF_ENABLED); /* Quick-VAN */
		}
		if ((cv.PortType==IdSerial) &&
		    (ts.DataBit==IdDataBit7)) {
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
		    IsClipboardFormatAvailable(CF_OEMTEXT))) {
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
			(SendVar!=NULL) || (FileVar!=NULL) || Connecting) {
			EnableMenuItem(SetupMenu,ID_SETUP_SERIALPORT,MF_BYCOMMAND | MF_GRAYED);
		}
		else {
			EnableMenuItem(SetupMenu,ID_SETUP_SERIALPORT,MF_BYCOMMAND | MF_ENABLED);
		}

	else if (SubMenu == ControlMenu)
	{
		if (cv.Ready &&
		    (SendVar==NULL) && (FileVar==NULL)) {
			if (ts.DisableMenuSendBreak) {
				EnableMenuItem(ControlMenu,ID_CONTROL_SENDBREAK,MF_BYCOMMAND | MF_GRAYED);
			}
			else {
				EnableMenuItem(ControlMenu,ID_CONTROL_SENDBREAK,MF_BYCOMMAND | MF_ENABLED);
			}
			if (cv.PortType==IdSerial) {
				EnableMenuItem(ControlMenu,ID_CONTROL_RESETPORT,MF_BYCOMMAND | MF_ENABLED);
			}
			else {
				EnableMenuItem(ControlMenu,ID_CONTROL_RESETPORT,MF_BYCOMMAND | MF_GRAYED);
			}
		}
		else {
			EnableMenuItem(ControlMenu,ID_CONTROL_SENDBREAK,MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(ControlMenu,ID_CONTROL_RESETPORT,MF_BYCOMMAND | MF_GRAYED);
		}

		if (cv.Ready && cv.TelFlag && (FileVar==NULL)) {
			EnableMenuItem(ControlMenu,ID_CONTROL_AREYOUTHERE,MF_BYCOMMAND | MF_ENABLED);
		}
		else {
			EnableMenuItem(ControlMenu,ID_CONTROL_AREYOUTHERE,MF_BYCOMMAND | MF_GRAYED);
		}

		if (HTEKWin==0) {
			EnableMenuItem(ControlMenu,ID_CONTROL_CLOSETEK,MF_BYCOMMAND | MF_GRAYED);
		}
		else {
			EnableMenuItem(ControlMenu,ID_CONTROL_CLOSETEK,MF_BYCOMMAND | MF_ENABLED);
		}

		if ((ConvH!=0) || (FileVar!=NULL)) {
			EnableMenuItem(ControlMenu,ID_CONTROL_MACRO,MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(ControlMenu,ID_CONTROL_SHOW_MACRO,MF_BYCOMMAND | MF_ENABLED);
		}
		else {
			EnableMenuItem(ControlMenu,ID_CONTROL_MACRO,MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(ControlMenu,ID_CONTROL_SHOW_MACRO,MF_BYCOMMAND | MF_GRAYED);
		}

	}
	else if (SubMenu == WinMenu)
	{
		SetWinMenu(WinMenu, ts.UIMsg, sizeof(ts.UIMsg), ts.UILanguageFile, 1);
	}

	TTXModifyPopupMenu(SubMenu); /* TTPLUG */
}

// added ConfirmPasteMouseRButton (2007.3.17 maya)
void CVTWindow::InitPasteMenu(HMENU *Menu)
{
	char uimsg[MAX_UIMSG];

	*Menu = LoadMenu(AfxGetInstanceHandle(),
	                 MAKEINTRESOURCE(IDR_PASTEMENU));

	GetMenuString(*Menu, ID_EDIT_PASTE2, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_EDIT_PASTE", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(*Menu, ID_EDIT_PASTE2, MF_BYCOMMAND, ID_EDIT_PASTE2, ts.UIMsg);
	GetMenuString(*Menu, ID_EDIT_PASTECR, uimsg, sizeof(uimsg), MF_BYCOMMAND);
	get_lang_msg("MENU_EDIT_PASTECR", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
	ModifyMenu(*Menu, ID_EDIT_PASTECR, MF_BYCOMMAND, ID_EDIT_PASTECR, ts.UIMsg);
}

void CVTWindow::ResetSetup()
{
	ChangeFont();
	BuffChangeWinSize(WinWidth,WinHeight);
	ChangeCaret();

	if (cv.Ready) {
		ts.PortType = cv.PortType;
		if (cv.PortType==IdSerial) {
			/* if serial port, change port parameters */
			ts.ComPort = cv.ComPort;
			CommResetSerial(&ts, &cv, TRUE);
		}
	}

	/* setup terminal */
	SetupTerm();

	/* background and ANSI color */
#ifdef ALPHABLEND_TYPE2
	BGInitialize();
	BGSetupPrimary(TRUE);
	// 2006/03/17 by 337 : Alpha�l�������ύX
	// Layered���ɂȂ��Ă��Ȃ��ꍇ�͌��ʂ�����
	if (ts.EtermLookfeel.BGUseAlphaBlendAPI) {
		MySetLayeredWindowAttributes(HVTWin, 0, ts.AlphaBlend, LWA_ALPHA);
	}
#else
	DispApplyANSIColor();
#endif
	DispSetNearestColors(IdBack, IdFore+8, NULL);

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
	char TempName[MAX_PATH];

	if ( strlen(ts.SetupFName)==0 ) {
		return;
	}

	ExtractFileName(ts.SetupFName,TempName,sizeof(TempName));
	ExtractDirName(ts.SetupFName,TempDir);
	if (TempDir[0]==0)
		strncpy_s(TempDir, sizeof(TempDir),ts.HomeDir, _TRUNCATE);
	FitFileName(TempName,sizeof(TempName),".INI");

	strncpy_s(ts.SetupFName, sizeof(ts.SetupFName),TempDir, _TRUNCATE);
	AppendSlash(ts.SetupFName,sizeof(ts.SetupFName));
	strncat_s(ts.SetupFName,sizeof(ts.SetupFName),TempName,_TRUNCATE);

	if (LoadTTSET()) {
		(*ReadIniFile)(ts.SetupFName,&ts);
	}
	FreeTTSET();

#if 0
	ChangeDefaultSet(&ts,NULL);
#endif

	ResetSetup();
}

/* called by the [Setup] Terminal command */
void CVTWindow::SetupTerm()
{
	if (ts.Language==IdJapanese || ts.Language==IdKorean || ts.Language==IdUtf8) {
		ResetCharSet();
	}
	cv.CRSend = ts.CRSend;

	// for russian mode
	cv.RussHost = ts.RussHost;
	cv.RussClient = ts.RussClient;

	if (cv.Ready) {
		if (cv.TelFlag && (ts.TelEcho>0)) {
			TelChangeEcho();
		}
		_free_locale(cv.locale);
		cv.locale = _create_locale(LC_ALL, cv.Locale);
	}

	if ((ts.TerminalWidth!=NumOfColumns) ||
	    (ts.TerminalHeight!=NumOfLines-StatusLine)) {
		LockBuffer();
		HideStatusLine();
		ChangeTerminalSize(ts.TerminalWidth,
		                   ts.TerminalHeight);
		UnlockBuffer();
	}
	else if ((ts.TermIsWin>0) &&
	         ((ts.TerminalWidth!=WinWidth) ||
	          (ts.TerminalHeight!=WinHeight-StatusLine))) {
		BuffChangeWinSize(ts.TerminalWidth,ts.TerminalHeight+StatusLine);
	}

	ChangeTerminalID();
}

void CVTWindow::Startup()
{
	/* auto log */
	/* OnCommOpen �ŊJ�n�����̂ł����ł͊J�n���Ȃ� (2007.5.14 maya) */

	if ((TopicName[0]==0) && (ts.MacroFN[0]!=0)) {
		// start the macro specified in the command line or setup file
		RunMacro(ts.MacroFN,TRUE);
		ts.MacroFN[0] = 0;
	}
	else {// start connection
		if (TopicName[0]!=0) {
			cv.NoMsg=1; /* suppress error messages */
		}
		::PostMessage(HVTWin,WM_USER_COMMSTART,0,0);
	}
}

void CVTWindow::OpenTEK()
{
	ActiveWin = IdTEK;
	if (HTEKWin==NULL) {
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

	if (message == MsgDlgHelp) {
		OnDlgHelp(wParam,lParam);
		return 0;
	}
	else if ((ts.HideTitle>0) &&
	         (message == WM_NCHITTEST)) {
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
	WORD wID = LOWORD(wParam);
	WORD wNotifyCode = HIWORD(wParam);

	if (wNotifyCode==1) {
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
		if (ActiveWin==IdVT) {
			switch (wID) {
				case ID_ACC_NEWCONNECTION:
					if (ts.AcceleratorNewConnection)
						OnFileNewConnection();
					return TRUE;
				case ID_ACC_DUPLICATESESSION:
					// added DisableAcceleratorDuplicateSession (2009.4.6 maya)
					if (!ts.DisableAcceleratorDuplicateSession)
						OnDuplicateSession();
					return TRUE;
				case ID_ACC_CYGWINCONNECTION:
					if (ts.AcceleratorCygwinConnection)
						OnCygwinConnection();
					return TRUE;
				case ID_ACC_DISCONNECT:
					Disconnect(TRUE);
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

	if ((wID>=ID_WINDOW_1) && (wID<ID_WINDOW_1+9)) {
		SelectWin(wID-ID_WINDOW_1);
		return TRUE;
	}
	else {
		if (TTXProcessCommand(HVTWin, wID)) {
			return TRUE;
		}
		else { /* TTPLUG */
			return CFrameWnd::OnCommand(wParam, lParam);
		}
	}
}

void CVTWindow::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	DispSetActive(nState!=WA_INACTIVE);
}

void CVTWindow::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	unsigned int i;
	char Code;

	if (!KeybEnabled || (TalkStatus!=IdTalkKeyb)) {
		return;
	}

	if (MetaKey(ts.MetaKey)) {
		::PostMessage(HVTWin,WM_SYSCHAR,nChar,MAKELONG(nRepCnt,nFlags));
		return;
	}
	Code = nChar;

	if ((ts.Language==IdRussian) &&
	    ((BYTE)Code>=128)) {
		Code = (char)RussConv(ts.RussKeyb,ts.RussClient,(BYTE)Code);
	}

	for (i=1 ; i<=nRepCnt ; i++) {
		CommTextOut(&cv,&Code,1);
		if (ts.LocalEcho>0) {
			CommTextEcho(&cv,&Code,1);
		}
	}

	/* �ŉ��s�ł��������X�N���[������ݒ�̏ꍇ
	   �����[�g�ւ̃L�[���͑��M�ŃX�N���[�������� */
	if (ts.AutoScrollOnlyInBottomLine != 0 && WinOrgY != 0) {
		DispVScroll(SCROLL_BOTTOM, 0);
	}
}

/* copy from ttset.c*/
static void WriteInt2(PCHAR Sect, PCHAR Key, PCHAR FName, int i1, int i2)
{
	char Temp[32];
	_snprintf_s(Temp, sizeof(Temp), _TRUNCATE, "%d,%d", i1, i2);
	WritePrivateProfileString(Sect, Key, Temp, FName);
}

static void SaveVTPos()
{
#define Section "Tera Term"
	if (ts.SaveVTWinPos) {
		/* VT win position */
		WriteInt2(Section, "VTPos", ts.SetupFName, ts.VTPos.x, ts.VTPos.y);

		/* VT terminal size  */
		WriteInt2(Section, "TerminalSize", ts.SetupFName,
		          ts.TerminalWidth, ts.TerminalHeight);
	}
}

void CVTWindow::OnClose()
{
	if ((HTEKWin!=NULL) && ! ::IsWindowEnabled(HTEKWin)) {
		MessageBeep(0);
		return;
	}
	get_lang_msg("MSG_DISCONNECT_CONF", ts.UIMsg, sizeof(ts.UIMsg),
	             "Disconnect?", ts.UILanguageFile);
	if (cv.Ready && (cv.PortType==IdTCPIP) &&
	    ((ts.PortFlag & PF_CONFIRMDISCONN) != 0) &&
	    ! CloseTT &&
	    (::MessageBox(HVTWin, ts.UIMsg, "Tera Term",
	     MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON2)==IDCANCEL)) {
		return;
	}

	FileTransEnd(0);
	ProtoEnd();

	SaveVTPos();
	DestroyWindow();
}

// �STera Term�̏I�����w������
void CVTWindow::OnAllClose()
{
	// �ˑR�I��������Ɗ댯�Ȃ̂ŁA���Ȃ炸���[�U�ɖ₢���킹���o���悤�ɂ���B
	// (2013.8.17 yutaka)
	get_lang_msg("MSG_ALL_TERMINATE_CONF", ts.UIMsg, sizeof(ts.UIMsg),
	             "Terminate ALL Tera Term(s)?", ts.UILanguageFile);
	if (::MessageBox(HVTWin, ts.UIMsg, "Tera Term",
	     MB_OKCANCEL | MB_ICONERROR | MB_DEFBUTTON2)==IDCANCEL) 
		return;

	BroadcastClosingMessage(HVTWin);
}

// �I���₢���킹�Ȃ���Tera Term���I������BOnAllClose()��M�p�B
LONG CVTWindow::OnNonConfirmClose(UINT wParam, LONG lParam)
{
	// ������ ts �̓��e���Ӑ}�I�ɏ��������Ă��A�I�����Ɏ����Z�[�u�����킯�ł͂Ȃ��̂ŁA���ɖ��Ȃ��B
	ts.PortFlag &= ~PF_CONFIRMDISCONN;
	OnClose();
	return 1;
}

void CVTWindow::OnDestroy()
{
	// remove this window from the window list
	UnregWin(HVTWin);

	EndKeyboard();

	/* Disable drag-drop */
	::DragAcceptFiles(HVTWin,FALSE);

	EndDDE();

	if (cv.TelFlag) {
		EndTelnet();
	}
	CommClose(&cv);

	OpenHelp(HH_CLOSE_ALL, 0, ts.UILanguageFile);

	FreeIME();
	FreeTTSET();
	do { }
	while (FreeTTDLG());

	do { }
	while (FreeTTFILE());

	if (HTEKWin != NULL) {
		::DestroyWindow(HTEKWin);
	}

	EndTerm();
	EndDisp();

	FreeBuffer();

	CFrameWnd::OnDestroy();
	TTXEnd(); /* TTPLUG */
}

static LRESULT CALLBACK OnDragDropDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static HFONT DlgDragDropFont = NULL;
	char uimsg[MAX_UIMSG];
	LOGFONT logfont;
	HFONT font;

	switch (msg) {
		case WM_INITDIALOG:
			font = (HFONT)SendMessage(hDlgWnd, WM_GETFONT, 0, 0);
			GetObject(font, sizeof(LOGFONT), &logfont);
			if (get_lang_font("DLG_TAHOMA_FONT", hDlgWnd, &logfont, &DlgDragDropFont, ts.UILanguageFile)) {
				SendDlgItemMessage(hDlgWnd, IDC_SCP_PATH, WM_SETFONT, (WPARAM)DlgDragDropFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDOK, WM_SETFONT, (WPARAM)DlgDragDropFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDCANCEL, WM_SETFONT, (WPARAM)DlgDragDropFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_DAD_STATIC, WM_SETFONT, (WPARAM)DlgDragDropFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDC_DAD_SENDFILE, WM_SETFONT, (WPARAM)DlgDragDropFont, MAKELPARAM(TRUE,0));
			} else {
				DlgDragDropFont = NULL;
			}

			GetWindowText(hDlgWnd, uimsg, sizeof(uimsg));
			get_lang_msg("MSG_DANDD_CONF_TITLE", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
			SetWindowText(hDlgWnd, ts.UIMsg);

			get_lang_msg("MSG_DANDD_CONF", ts.UIMsg, sizeof(ts.UIMsg),
			             "Are you sure that you want to send the file content?", ts.UILanguageFile);
			SetDlgItemText(hDlgWnd, IDC_DAD_STATIC, ts.UIMsg);

			get_lang_msg("FILEDLG_TRANS_TITLE_SENDFILE", ts.UIMsg, sizeof(ts.UIMsg),
						 "Send file", ts.UILanguageFile);
			SetDlgItemText(hDlgWnd, IDOK, ts.UIMsg);

			SendMessage(GetDlgItem(hDlgWnd, IDC_SCP_PATH), WM_SETTEXT, 0, (LPARAM)ts.ScpSendDir);

			// �L�����Z���{�^�����f�t�H���g�ɂ��A���ӎ���Enter�L�[���������Ă��A�������Ȃ��悤�ɂ���B
			SetFocus(GetDlgItem(hDlgWnd, IDCANCEL));
			PostMessage(GetDlgItem(hDlgWnd, IDCANCEL), WM_NEXTDLGCTL, 0, 0L) ;

			// SSH2 �ڑ��ł͂Ȃ��ꍇ�ɂ� "SCP" �𖳌�������B
			if (cv.isSSH != 2) {
				EnableWindow(GetDlgItem(hDlgWnd, IDC_DAD_SENDFILE), FALSE);
				EnableWindow(GetDlgItem(hDlgWnd, IDC_SCP_PATH), FALSE);
				EnableWindow(GetDlgItem(hDlgWnd, IDC_STATIC), FALSE);
			}

			// TRUE�ɂ���ƃ{�^���Ƀt�H�[�J�X��������Ȃ��B
			return FALSE;

		case WM_COMMAND:
			switch (LOWORD(wp)) {
				case IDC_DAD_SENDFILE:
					SendMessage(GetDlgItem(hDlgWnd, IDC_SCP_PATH), WM_GETTEXT, sizeof(ts.ScpSendDir), (LPARAM)ts.ScpSendDir);
					if (DlgDragDropFont != NULL) {
						DeleteObject(DlgDragDropFont);
					}
					EndDialog(hDlgWnd, IDC_DAD_SENDFILE);
					break;

				case IDOK:
					if (DlgDragDropFont != NULL) {
						DeleteObject(DlgDragDropFont);
					}
					EndDialog(hDlgWnd, IDOK);
					break;

				case IDCANCEL:
					if (DlgDragDropFont != NULL) {
						DeleteObject(DlgDragDropFont);
					}
					EndDialog(hDlgWnd, IDCANCEL);
					break;

				default:
					return FALSE;
			}

		default:
			return FALSE;
	}
	return TRUE;
}

void CVTWindow::OnDropFiles(HDROP hDropInfo)
{
	::SetForegroundWindow(HVTWin);
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
					CommTextOut(&cv, ptr, 1);
					if (ts.LocalEcho > 0) {
						CommTextEcho(&cv, ptr, 1);
					}
					ptr++;
				}
				FreeFileVar(&SendVar); // �����Y�ꂸ��

			} else {
				// Confirm send a file when drag and drop (2007.12.28 maya)
				if (ts.ConfirmFileDragAndDrop) {
					// �����Ȃ�t�@�C���̓��e�𑗂荞�ޑO�ɁA���[�U�ɖ₢���킹���s���B(2006.1.21 yutaka)
					// MessageBox��SCP���I���ł���悤�ɂ���B(2008.1.25 yutaka)
					// SCP�p�X���w��ł���悤�Ƀ_�C�A���O�ɕύX�����B(2012.4.11 yutaka)
					int ret;

					ret = DialogBox(hInst, MAKEINTRESOURCE(IDD_DAD_DIALOG),
									HVTWin, (DLGPROC)OnDragDropDlgProc);

					if (ret == IDOK) {   // sendfile
						HelpId = HlpFileSend;
						SendVar->DirLen = 0;
						ts.TransBin = 0;
						FileSendStart();

					} else if (ret == IDC_DAD_SENDFILE) {   // SCP
						typedef int (CALLBACK *PSSH_start_scp)(char *, char *);
						static PSSH_start_scp func = NULL;
						static HMODULE h = NULL;
						char msg[128];

						if (func == NULL) {
							if ( ((h = GetModuleHandle("ttxssh.dll")) == NULL) ) {
								_snprintf_s(msg, sizeof(msg), _TRUNCATE, "GetModuleHandle(\"ttxssh.dll\")) %d", GetLastError());
								goto scp_send_error;
							}
							func = (PSSH_start_scp)GetProcAddress(h, "TTXScpSendfile");
							if (func == NULL) {
								_snprintf_s(msg, sizeof(msg), _TRUNCATE, "GetProcAddress(\"TTXScpSendfile\")) %d", GetLastError());
								goto scp_send_error;
							}
						}

						if (func != NULL) {
							func(SendVar->FullName, ts.ScpSendDir);
							goto send_success;
						} 

scp_send_error:
						::MessageBox(NULL, msg, "Tera Term: scpsend command error", MB_OK | MB_ICONERROR);
send_success:
						FreeFileVar(&SendVar);  // �����Y�ꂸ��

					} else {
						FreeFileVar(&SendVar);

					}
				}
				else {
					SendVar->DirLen = 0;
					ts.TransBin = 0;
					FileSendStart();

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
		case SB_BOTTOM:
			Func = SCROLL_BOTTOM;
			break;
		case SB_ENDSCROLL:
			return;
		case SB_LINEDOWN:
			Func = SCROLL_LINEDOWN;
			break;
		case SB_LINEUP:
			Func = SCROLL_LINEUP;
			break;
		case SB_PAGEDOWN:
			Func = SCROLL_PAGEDOWN;
			break;
		case SB_PAGEUP: 
			Func = SCROLL_PAGEUP;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			Func = SCROLL_POS;
			break;
		case SB_TOP:
			Func = SCROLL_TOP;
			break;
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

	switch (KeyDown(HVTWin,nChar,nRepCnt,nFlags & 0x1ff)) {
	case KEYDOWN_OTHER:
		break;
	case KEYDOWN_CONTROL:
		return;
	case KEYDOWN_COMMOUT:
		/* �ŉ��s�ł��������X�N���[������ݒ�̏ꍇ
		   �����[�g�ւ̃L�[���͑��M�ŃX�N���[�������� */
		if (ts.AutoScrollOnlyInBottomLine != 0 && WinOrgY != 0) {
			DispVScroll(SCROLL_BOTTOM, 0);
		}
		return;
	}

	if (MetaKey(ts.MetaKey) && (nFlags & 0x2000) != 0)
	{
		PeekMessage((LPMSG)&M,HVTWin,WM_CHAR,WM_CHAR,PM_REMOVE);
		/* for Ctrl+Alt+Key combination */
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
	FocusReport(FALSE);
	CFrameWnd::OnKillFocus(pNewWnd);

	if (IsCaretOn()) {
		CaretKillFocus(TRUE);
	}
}

void CVTWindow::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (LButton || MButton || RButton) {
		return;
	}

	DblClkX = point.x;
	DblClkY = point.y;

	if (MouseReport(IdMouseEventBtnDown, IdLeftButton, DblClkX, DblClkY)) {
		return;
	}

	if (BuffUrlDblClk(DblClkX, DblClkY)) { // �u���E�U�Ăяo���̏ꍇ�͉������Ȃ��B (2005.4.3 yutaka)
		return;
	}

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
	if (IgnoreRelease)
		IgnoreRelease = FALSE;
	else
		MouseReport(IdMouseEventBtnUp, IdLeftButton, point.x, point.y);

	if (! LButton) {
		return;
	}

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
	BOOL mousereport;

	if (IgnoreRelease)
		IgnoreRelease = FALSE;
	else
		mousereport = MouseReport(IdMouseEventBtnUp, IdMiddleButton, point.x, point.y);

	if (! MButton) {
		return;
	}

	// added DisablePasteMouseMButton (2008.3.2 maya)
	if (ts.DisablePasteMouseMButton || mousereport) {
		ButtonUp(FALSE);
	}
	else {
		ButtonUp(TRUE);
	}
}

int CVTWindow::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	if ((ts.SelOnActive==0) && (nHitTest==HTCLIENT)) { //disable mouse event for text selection
		IgnoreRelease = TRUE;
		return MA_ACTIVATEANDEAT; //     when window is activated
	}
	else {
		return MA_ACTIVATE;
	}
}

void CVTWindow::OnMouseMove(UINT nFlags, CPoint point)
{
	int i;
	BOOL mousereport;

	if (!IgnoreRelease)
		mousereport = MouseReport(IdMouseEventMove, 0, point.x, point.y);

	if (! (LButton || MButton || RButton)) {
		if (BuffCheckMouseOnURL(point.x, point.y))
			SetMouseCursor("HAND");
		else
			SetMouseCursor(ts.MouseCursorName);
		return;
	}

	if (mousereport) {
		return;
	}

	if (DblClk) {
		i = 2;
	}
	else if (TplClk) {
		i = 3;
	}
	else {
		i = 1;
	}

	if (!ts.SelectOnlyByLButton ||
	    (ts.SelectOnlyByLButton && LButton) ) {
		// SelectOnlyByLButton == TRUE �̂Ƃ��́A���{�^���_�E�����̂ݑI������ (2007.11.21 maya)
		BuffChangeSelect(point.x, point.y,i);
	}
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
	int line, i;

	::ScreenToClient(HVTWin, &pt);

	line = abs(zDelta) / WHEEL_DELTA; // ���C����
	if (line < 1) line = 1;

	// ��X�N���[��������̍s���ɕϊ����� (2008.4.6 yutaka)
	if (line == 1 && ts.MouseWheelScrollLine > 0)
		line *= ts.MouseWheelScrollLine;

	if (MouseReport(IdMouseEventWheel, zDelta<0, pt.x, pt.y))
		return TRUE;

	if (WheelToCursorMode()) {
		if (zDelta < 0) {
			KeyDown(HVTWin, VK_DOWN, line, MapVirtualKey(VK_DOWN, 0) | 0x100);
			KeyUp(VK_DOWN);
		} else {
			KeyDown(HVTWin, VK_UP, line, MapVirtualKey(VK_UP, 0) | 0x100);
			KeyUp(VK_UP);
		}
	} else {
		for (i = 0 ; i < line ; i++) {
			if (zDelta < 0) {
				OnVScroll(SB_LINEDOWN, 0, NULL);
			} else {
				OnVScroll(SB_LINEUP, 0, NULL);
			}
		}
	}

	return (TRUE);
}


void CVTWindow::OnNcLButtonDblClk(UINT nHitTest, CPoint point)
{
	if (! Minimized && (nHitTest == HTCAPTION)) {
		DispRestoreWinSize();
	}
	else {
		CFrameWnd::OnNcLButtonDblClk(nHitTest,point);
	}
}

void CVTWindow::OnNcRButtonDown(UINT nHitTest, CPoint point)
{
	if ((nHitTest==HTCAPTION) &&
	    (ts.HideTitle>0) &&
		AltKey()) {
		::CloseWindow(HVTWin); /* iconize */
	}
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

	if (FirstPaint) {
		if (strlen(TopicName)>0) {
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
	BOOL mousereport;

	if (IgnoreRelease)
		IgnoreRelease = FALSE;
	else
		mousereport = MouseReport(IdMouseEventBtnUp, IdRightButton, point.x, point.y);

	if (! RButton) {
		return;
	}

	// �E�{�^�������ł̃y�[�X�g���֎~���� (2005.3.16 yutaka)
	if (ts.DisablePasteMouseRButton || mousereport) {
		ButtonUp(FALSE);
	} else {
		ButtonUp(TRUE);
	}
}

void CVTWindow::OnSetFocus(CWnd* pOldWnd)
{
	ChangeCaret();
	FocusReport(TRUE);
	CFrameWnd::OnSetFocus(pOldWnd);
}


//
// ���T�C�Y�c�[���`�b�v(based on PuTTY sizetip.c)
// 
static ATOM tip_class = 0;
static HFONT tip_font;
static COLORREF tip_bg;
static COLORREF tip_text;
static HWND tip_wnd = NULL;
static int tip_enabled = 0;

static LRESULT CALLBACK SizeTipWndProc(HWND hWnd, UINT nMsg,
                                       WPARAM wParam, LPARAM lParam)
{

	switch (nMsg) {
		case WM_ERASEBKGND:
			return TRUE;

		case WM_PAINT:
			{
				HBRUSH hbr;
				HGDIOBJ holdbr;
				RECT cr;
				int wtlen;
				LPTSTR wt;
				HDC hdc;

				PAINTSTRUCT ps;
				hdc = BeginPaint(hWnd, &ps);

				SelectObject(hdc, tip_font);
				SelectObject(hdc, GetStockObject(BLACK_PEN));

				hbr = CreateSolidBrush(tip_bg);
				holdbr = SelectObject(hdc, hbr);

				GetClientRect(hWnd, &cr);
				Rectangle(hdc, cr.left, cr.top, cr.right, cr.bottom);

				wtlen = GetWindowTextLength(hWnd);
				wt = (LPTSTR) malloc((wtlen + 1) * sizeof(TCHAR));
				GetWindowText(hWnd, wt, wtlen + 1);

				SetTextColor(hdc, tip_text);
				SetBkColor(hdc, tip_bg);

				TextOut(hdc, cr.left + 3, cr.top + 3, wt, wtlen);

				free(wt);

				SelectObject(hdc, holdbr);
				DeleteObject(hbr);

				EndPaint(hWnd, &ps);
			}
			return 0;

		case WM_NCHITTEST:
			return HTTRANSPARENT;

		case WM_DESTROY:
			DeleteObject(tip_font);
			tip_font = NULL;
			break;

		case WM_SETTEXT:
			{
				LPCTSTR str = (LPCTSTR) lParam;
				SIZE sz;
				HDC hdc = CreateCompatibleDC(NULL);

				SelectObject(hdc, tip_font);
				GetTextExtentPoint32(hdc, str, _tcslen(str), &sz);

				SetWindowPos(hWnd, NULL, 0, 0, sz.cx + 6, sz.cy + 6,
				             SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
				InvalidateRect(hWnd, NULL, FALSE);

				DeleteDC(hdc);
			}
			break;
	}

	return DefWindowProc(hWnd, nMsg, wParam, lParam);
}

static void UpdateSizeTip(HWND src, int cx, int cy)
{
	TCHAR str[32];

	if (!tip_enabled)
		return;

	if (!tip_wnd) {
		NONCLIENTMETRICS nci;

		/* First make sure the window class is registered */

		if (!tip_class) {
			WNDCLASS wc;
			wc.style = CS_HREDRAW | CS_VREDRAW;
			wc.lpfnWndProc = SizeTipWndProc;
			wc.cbClsExtra = 0;
			wc.cbWndExtra = 0;
			wc.hInstance = hInst;
			wc.hIcon = NULL;
			wc.hCursor = NULL;
			wc.hbrBackground = NULL;
			wc.lpszMenuName = NULL;
			wc.lpszClassName = "SizeTipClass";

			tip_class = RegisterClass(&wc);
		}
#if 0
		/* Default values based on Windows Standard color scheme */

		tip_font = GetStockObject(SYSTEM_FONT);
		tip_bg = RGB(255, 255, 225);
		tip_text = RGB(0, 0, 0);
#endif

		/* Prepare other GDI objects and drawing info */

		tip_bg = GetSysColor(COLOR_INFOBK);
		tip_text = GetSysColor(COLOR_INFOTEXT);

		memset(&nci, 0, sizeof(NONCLIENTMETRICS));
		nci.cbSize = sizeof(NONCLIENTMETRICS);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS,
			sizeof(NONCLIENTMETRICS), &nci, 0);
		tip_font = CreateFontIndirect(&nci.lfStatusFont);
	}

	/* Generate the tip text */

	sprintf(str, "%dx%d", cx, cy);

	if (!tip_wnd) {
		HDC hdc;
		SIZE sz;
		RECT wr;
		int ix, iy;
		HMODULE mod;
		HMONITOR hm;

		/* calculate the tip's size */

		hdc = CreateCompatibleDC(NULL);
		GetTextExtentPoint32(hdc, str, _tcslen(str), &sz);
		DeleteDC(hdc);

		GetWindowRect(src, &wr);

		ix = wr.left;
		iy = wr.top - sz.cy;

		if (((mod = GetModuleHandle("user32.dll")) != NULL) &&
		    (GetProcAddress(mod,"MonitorFromPoint") != NULL)) {
			// �}���`���j�^���T�|�[�g����Ă���ꍇ
			POINT p;
			p.x = ix;
			p.y = iy;
			hm = MonitorFromPoint(p, MONITOR_DEFAULTTONULL);
			if (hm == NULL) {
#if 1
				// �c�[���`�b�v���X�N���[������͂ݏo���Ă���ꍇ�̓}�E�X�̂��郂�j�^�ɕ\������
				GetCursorPos(&p);
				hm = MonitorFromPoint(p, MONITOR_DEFAULTTONEAREST);
#else
				// �c�[���`�b�v���X�N���[������͂ݏo���Ă���ꍇ�͍ł��߂����j�^�ɕ\������
				hm = MonitorFromPoint(p, MONITOR_DEFAULTTONEAREST);
#endif
			}
			MONITORINFO mi;
			mi.cbSize = sizeof(MONITORINFO);
			GetMonitorInfo(hm, &mi);
			if (ix < mi.rcMonitor.left + 16) {
				ix = mi.rcMonitor.left + 16;
			}
			if (iy < mi.rcMonitor.top + 16) {
				iy = mi.rcMonitor.top + 16;
			}
		}
		else {
			// �}���`���j�^���T�|�[�g����Ă��Ȃ��ꍇ
			if (ix < 16) {
				ix = 16;
			}
			if (iy < 16) {
				iy = 16;
			}
		}

		/* Create the tip window */

		tip_wnd =
			CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
			MAKEINTRESOURCE(tip_class), str, WS_POPUP, ix,
			iy, sz.cx, sz.cy, NULL, NULL, hInst, NULL);

		ShowWindow(tip_wnd, SW_SHOWNOACTIVATE);

	} else {

		/* Tip already exists, just set the text */

		SetWindowText(tip_wnd, str);
	}
}

static void EnableSizeTip(int bEnable)
{
	if (tip_wnd && !bEnable) {
		DestroyWindow(tip_wnd);
		tip_wnd = NULL;
	}

	tip_enabled = bEnable;
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
	if (Minimized || DontChangeSize) {
		return;
	}

	if (nType == SIZE_MAXIMIZED) {
		ts.TerminalOldWidth = ts.TerminalWidth;
		ts.TerminalOldHeight = ts.TerminalHeight;
	}

	::GetWindowRect(HVTWin,&R);
	w = R.right - R.left;
	h = R.bottom - R.top;
	if (AdjustSize) {
		ResizeWindow(R.left,R.top,w,h,cx,cy);
	}
	else {
		if (ts.FontScaling) {
			int NewFontWidth, NewFontHeight;
			BOOL FontChanged = FALSE;

			NewFontWidth = cx / ts.TerminalWidth;
			NewFontHeight = cy / ts.TerminalHeight;

			if (NewFontWidth - ts.FontDW < 3) {
				NewFontWidth = ts.FontDW + 3;
			}
			if (NewFontWidth != FontWidth) {
				ts.VTFontSize.x = ts.FontDW - NewFontWidth;
				FontWidth = NewFontWidth;
				FontChanged = TRUE;
			}

			if (NewFontHeight - ts.FontDH < 3) {
				NewFontHeight = ts.FontDH + 3;
			}
			if (NewFontHeight != FontHeight) {
				ts.VTFontSize.y = ts.FontDH - NewFontHeight;
				FontHeight = NewFontHeight;
				FontChanged = TRUE;
			}

			w = ts.TerminalWidth;
			h = ts.TerminalHeight;

			if (FontChanged) {
				ChangeFont();
			}
		}
		else {
			w = cx / FontWidth;
			h = cy / FontHeight;
		}

		HideStatusLine();
		BuffChangeWinSize(w,h);
	}

#ifdef WINDOW_MAXMIMUM_ENABLED
	if (nType == SIZE_MAXIMIZED) {
		AdjustSize = 0;
	}
#endif
}

// �u�h���b�O���ɃE�B���h�E�̓��e��\������v�Ƀ`�F�b�N�������Ă���ꍇ�A
// ���T�C�Y���͏��"WM_SIZING"�����ł��邽�߁A���T�C�Y�c�[���`�b�v��
// �ĕ`�悷��B
// (2008.8.1 yutaka)
void CVTWindow::OnSizing(UINT fwSide, LPRECT pRect)
{
	int nWidth;
	int nHeight;
	RECT cr, wr;
	int extra_width, extra_height;
	int w, h;

	::GetWindowRect(HVTWin, &wr);
	::GetClientRect(HVTWin, &cr);

	extra_width = wr.right - wr.left - cr.right + cr.left;
	extra_height = wr.bottom - wr.top - cr.bottom + cr.top;
	nWidth = (pRect->right) - (pRect->left) - extra_width;
	nHeight = (pRect->bottom) - (pRect->top) - extra_height;

	w = nWidth / FontWidth;
	h = nHeight / FontHeight;
	UpdateSizeTip(HVTWin, w, h);
}

void CVTWindow::OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	char e = ESC;
	char Code;
	unsigned int i;

#ifdef WINDOW_MAXMIMUM_ENABLED
	// ALT + x����������� WM_SYSCHAR �����ł���B
	// ALT + Enter�ŃE�B���h�E�̍ő剻 (2005.4.24 yutaka)
	if ((nFlags&0x2000) != 0 && nChar == CR) {
		if (IsZoomed()) { // window is maximum
			ShowWindow(SW_RESTORE);
		} else {
			ShowWindow(SW_MAXIMIZE);
		}
	}
#endif

	if (MetaKey(ts.MetaKey)) {
		if (!KeybEnabled || (TalkStatus!=IdTalkKeyb)) return;
		Code = nChar;
		for (i=1 ; i<=nRepCnt ; i++) {
			switch (ts.Meta8Bit) {
			  case IdMeta8BitRaw:
				Code |= 0x80;
				CommBinaryBuffOut(&cv, &Code, 1);
				if (ts.LocalEcho) {
					CommBinaryEcho(&cv, &Code, 1);
				}
				break;
			  case IdMeta8BitText:
				Code |= 0x80;
				CommTextOut(&cv, &Code, 1);
				if (ts.LocalEcho) {
					CommTextEcho(&cv, &Code, 1);
				}
				break;
			  default:
				CommTextOut(&cv, &e, 1);
				CommTextOut(&cv, &Code, 1);
				if (ts.LocalEcho) {
					CommTextEcho(&cv, &e, 1);
					CommTextEcho(&cv, &Code, 1);
				}
			}
		}
		return;
	}

	CFrameWnd::OnSysChar(nChar, nRepCnt, nFlags);
}

void CVTWindow::OnSysColorChange()
{
	CFrameWnd::OnSysColorChange();
}

void CVTWindow::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (nID==ID_SHOWMENUBAR) {
		ts.PopupMenu = 0;
		SwitchMenu();
	}
	else if (((nID & 0xfff0)==SC_CLOSE) && (cv.PortType==IdTCPIP) &&
	         cv.Open && ! cv.Ready && (cv.ComPort>0)) {
		// now getting host address (see CommOpen() in commlib.c)
		::PostMessage(HVTWin,WM_SYSCOMMAND,nID,lParam);
	}
	else {
		CFrameWnd::OnSysCommand(nID,lParam);
	}
}

void CVTWindow::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if ((nChar==VK_F10) ||
	   ((ts.MetaKey == IdMetaOn) && (nFlags & 0x2000) ||
	    (ts.MetaKey == IdMetaLeft) && (nFlags & 0x2100) == 0x2000 ||
	    (ts.MetaKey == IdMetaRight) && (nFlags & 0x2100) == 0x2100) &&
	   ((MainMenu==NULL) || (nChar!=VK_MENU))) {
		KeyDown(HVTWin,nChar,nRepCnt,nFlags & 0x1ff);
		// OnKeyDown(nChar,nRepCnt,nFlags);
	}
	else {
		CFrameWnd::OnSysKeyDown(nChar,nRepCnt,nFlags);
	}
}

void CVTWindow::OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar==VK_F10) {
		OnKeyUp(nChar,nRepCnt,nFlags);
	}
	else {
		CFrameWnd::OnSysKeyUp(nChar,nRepCnt,nFlags);
	}
}

void CVTWindow::OnTimer(UINT nIDEvent)
{
	POINT Point;
	WORD PortType;
	UINT T;

	if (nIDEvent==IdCaretTimer) {
		if (ts.NonblinkingCursor!=0) {
			T = GetCaretBlinkTime();
			SetCaretBlinkTime(T);
		}
		else {
			::KillTimer(HVTWin,IdCaretTimer);
		}
		return;
	}
	else if (nIDEvent==IdScrollTimer) {
		GetCursorPos(&Point);
		::ScreenToClient(HVTWin,&Point);
		DispAutoScroll(Point);
		if ((Point.x < 0) || (Point.x >= ScreenWidth) ||
			(Point.y < 0) || (Point.y >= ScreenHeight)) {
			::PostMessage(HVTWin,WM_MOUSEMOVE,MK_LBUTTON,MAKELONG(Point.x,Point.y));
		}
		return;
	}
	else if (nIDEvent == IdCancelConnectTimer) {
		// �܂��ڑ����������Ă��Ȃ���΁A�\�P�b�g�������N���[�Y�B
		// CloseSocket()���Ăт������A��������͌ĂׂȂ��̂ŁA����Win32API���R�[������B
		if (!cv.Ready) {
			closesocket(cv.s);
			cv.s = INVALID_SOCKET;  /* �\�P�b�g�����̈��t����B(2010.8.6 yutaka) */
			//::PostMessage(HVTWin, WM_USER_COMMNOTIFY, 0, FD_CLOSE);
		}
	}

	::KillTimer(HVTWin, nIDEvent);

	switch (nIDEvent) {
		case IdDelayTimer:
			cv.CanSend = TRUE;
			break;
		case IdProtoTimer:
			ProtoDlgTimeOut();
		case IdDblClkTimer:
			AfterDblClk = FALSE;
			break;
		case IdComEndTimer:
			if (! CommCanClose(&cv)) {
				// wait if received data remains
				SetTimer(IdComEndTimer,1,NULL);
				break;
			}
			cv.Ready = FALSE;
			if (cv.TelFlag) {
				EndTelnet();
			}
			PortType = cv.PortType;
			CommClose(&cv);
			SetDdeComReady(0);
			if ((PortType==IdTCPIP) &&
				((ts.PortFlag & PF_BEEPONCONNECT) != 0)) {
				MessageBeep(0);
			}
			if ((PortType==IdTCPIP) &&
				(ts.AutoWinClose>0) &&
				::IsWindowEnabled(HVTWin) &&
				((HTEKWin==NULL) || ::IsWindowEnabled(HTEKWin)) ) {
				OnClose();
			}
			else {
				ChangeTitle();
				if (ts.ClearScreenOnCloseConnection) {
					OnEditClearScreen();
				}
			}
			break;
		case IdPrnStartTimer:
			PrnFileStart();
			break;
		case IdPrnProcTimer:
			PrnFileDirectProc();
			break;
	}
}

void CVTWindow::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	int Func;
	SCROLLINFO si;

	switch (nSBCode) {
	case SB_BOTTOM:
		Func = SCROLL_BOTTOM;
		break;
	case SB_ENDSCROLL:
		return;
	case SB_LINEDOWN:
		Func = SCROLL_LINEDOWN;
		break;
	case SB_LINEUP:
		Func = SCROLL_LINEUP;
		break;
	case SB_PAGEDOWN:
		Func = SCROLL_PAGEDOWN;
		break;
	case SB_PAGEUP:
		Func = SCROLL_PAGEUP;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		Func = SCROLL_POS;
		break;
	case SB_TOP:
		Func = SCROLL_TOP;
		break;
	default:
		return;
	}

	// �X�N���[�������W�� 16bit ���� 32bit �֊g������ (2005.10.4 yutaka)
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
	if(BGEnable && BGNoCopyBits) {
		((WINDOWPOS*)lParam)->flags |= SWP_NOCOPYBITS;
	}
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
	EnableSizeTip(1);

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

	EnableSizeTip(0);

	return CFrameWnd::DefWindowProc(WM_EXITSIZEMOVE,wParam,lParam);
}
//-->

LONG CVTWindow::OnIMEComposition(UINT wParam, LONG lParam)
{
	HGLOBAL hstr;
	//LPSTR lpstr;
	wchar_t *lpstr;
	int Len;
	char *mbstr;
	int mlen;

	if (CanUseIME()) {
		hstr = GetConvString(wParam, lParam);
	}
	else {
		hstr = NULL;
	}

	if (hstr!=NULL) {
		//lpstr = (LPSTR)GlobalLock(hstr);
		lpstr = (wchar_t *)GlobalLock(hstr);
		if (lpstr!=NULL) {
			mlen = wcstombs(NULL, lpstr, 0);
			mbstr = (char *)malloc(sizeof(char) * (mlen + 1));
			if (mbstr == NULL) {
				goto skip;
			}
			Len = wcstombs(mbstr, lpstr, mlen + 1);

			// add this string into text buffer of application
			Len = strlen(mbstr);
			if (Len==1) {
				switch (mbstr[0]) {
				case 0x20:
					if (ControlKey()) {
						mbstr[0] = 0; /* Ctrl-Space */
					}
					break;
				case 0x5c: // Ctrl-\ support for NEC-PC98
					if (ControlKey()) {
						mbstr[0] = 0x1c;
					}
					break;
				}
			}
			if (ts.LocalEcho>0) {
				CommTextEcho(&cv,mbstr,Len);
			}
			CommTextOut(&cv,mbstr,Len);

			free(mbstr);
			GlobalUnlock(hstr);
		}
skip:
		GlobalFree(hstr);
		return 0;
	}
	return CFrameWnd::DefWindowProc(WM_IME_COMPOSITION,wParam,lParam);
}

LONG CVTWindow::OnIMEInputChange(UINT wParam, LONG lParam)
{
	ChangeCaret();

	return CFrameWnd::DefWindowProc(WM_INPUTLANGCHANGE,wParam,lParam);
}

LONG CVTWindow::OnIMENotify(UINT wParam, LONG lParam)
{
	if (wParam == IMN_SETOPENSTATUS) {
		ChangeCaret();
	}

	return CFrameWnd::DefWindowProc(WM_IME_NOTIFY,wParam,lParam);
}

// IME�̑O��Q�ƕϊ��@�\�ւ̑Ή�
// MS���炿���Ǝd�l���񎦂���Ă��Ȃ��̂ŁA�A�h�z�b�N�ɂ�邵���Ȃ��炵���B
// cf. http://d.hatena.ne.jp/topiyama/20070703
//     http://ice.hotmint.com/putty/#DOWNLOAD
//     http://27213143.at.webry.info/201202/article_2.html
//     http://webcache.googleusercontent.com/search?q=cache:WzlX3ouMscIJ:anago.2ch.net/test/read.cgi/software/1325573999/82+IMR_DOCUMENTFEED&cd=13&hl=ja&ct=clnk&gl=jp
// (2012.5.9 yutaka)
LONG CVTWindow::OnIMERequest(UINT wParam, LONG lParam)
{
	static int complen, newsize;
	static char comp[512];
	int size, ret;
	char buf[512], newbuf[1024];
	HIMC hIMC;

	// "IME=off"�̏ꍇ�́A�������Ȃ��B
	if (ts.UseIME > 0 &&
		wParam == IMR_DOCUMENTFEED) {
		size = NumOfColumns + 1;   // �J�[�\��������s�̒���+null

		if (lParam == 0) {  // 1��ڂ̌Ăяo��
			// �o�b�t�@�̃T�C�Y��Ԃ��̂݁B
			// ATOK2012�ł͏�� complen=0 �ƂȂ�B
			complen = 0;
			memset(comp, 0, sizeof(comp));
			hIMC = ImmGetContext(HVTWin);
			if (hIMC) {
				ret = ImmGetCompositionString(hIMC, GCS_COMPSTR, comp, sizeof(comp));
				if (ret == IMM_ERROR_NODATA || ret == IMM_ERROR_GENERAL) {
					memset(comp, 0, sizeof(comp));
				}
				complen = strlen(comp);  // w/o null
				ImmReleaseContext(HVTWin, hIMC);
			}
			newsize = size + complen;  // �ϊ��������܂߂��S�̂̒���(including null)

		} else {  // 2��ڂ̌Ăяo��
			//lParam �� RECONVERTSTRING �� ������i�[�o�b�t�@�Ɏg�p����
			RECONVERTSTRING *pReconv   = (RECONVERTSTRING*)lParam;
			char*  pszParagraph        = (char*)pReconv + sizeof(RECONVERTSTRING);
			int cx;

			cx = BuffGetCurrentLineData(buf, sizeof(buf));

			// �J�[�\���ʒu�ɕϊ��������}������B
			memset(newbuf, 0, sizeof(newbuf));
			memcpy(newbuf, buf, cx);
			memcpy(newbuf + cx, comp, complen);
			memcpy(newbuf + cx + complen, buf + cx, size - cx);
			newsize = size + complen;  // �ϊ��������܂߂��S�̂̒���(including null)
	        
			pReconv->dwSize            = sizeof(RECONVERTSTRING);
			pReconv->dwVersion         = 0;
			pReconv->dwStrLen          = newsize - 1;
			pReconv->dwStrOffset       = sizeof(RECONVERTSTRING);
			pReconv->dwCompStrLen      = complen;
			pReconv->dwCompStrOffset   = cx;
			pReconv->dwTargetStrLen    = complen;
			pReconv->dwTargetStrOffset = cx;
	        
			memcpy(pszParagraph, newbuf, newsize);
			//OutputDebugPrintf("cx %d buf [%d:%s] -> [%d:%s]\n", cx, size, buf, newsize, newbuf);
		}
		return (sizeof(RECONVERTSTRING) + newsize);
	}

	return CFrameWnd::DefWindowProc(WM_IME_REQUEST,wParam,lParam);
}

LONG CVTWindow::OnAccelCommand(UINT wParam, LONG lParam)
{
	switch (wParam) {
		case IdHold:
			if (TalkStatus==IdTalkKeyb) {
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
			SelectNextWin(HVTWin,1,FALSE);
			break;
		case IdCmdPrevWin:
			SelectNextWin(HVTWin,-1,FALSE);
			break;
		case IdCmdNextSWin:
			SelectNextWin(HVTWin,1,TRUE);
			break;
		case IdCmdPrevSWin:
			SelectNextWin(HVTWin,-1,TRUE);
			break;
		case IdCmdLocalEcho:
			if (ts.LocalEcho==0) {
				ts.LocalEcho = 1;
			}
			else {
				ts.LocalEcho = 0;
			}
			if (cv.Ready && cv.TelFlag && (ts.TelEcho>0)) {
				TelChangeEcho();
			}
			break;
		case IdCmdDisconnect: // called by TTMACRO
			Disconnect(lParam);
			break;
		case IdCmdLoadKeyMap: // called by TTMACRO
			SetKeyMap();
			break;
		case IdCmdRestoreSetup: // called by TTMACRO
			RestoreSetup();
			break;
		case IdCmdScrollLock:
			ScrollLock = ! ScrollLock;
			break;
	}
	return 0;
}

LONG CVTWindow::OnChangeMenu(UINT wParam, LONG lParam)
{
	HMENU SysMenu;
	BOOL Show, B1, B2;

	Show = (ts.PopupMenu==0) && (ts.HideTitle==0);

// TTXKanjiMenu �̂��߂ɁA���j���[���\������Ă��Ă�
// �ĕ`�悷��悤�ɂ����B (2007.7.14 maya)
	if (Show != (MainMenu!=NULL)) {
		AdjustSize = TRUE;
	}

	if (MainMenu!=NULL) {
		DestroyMenu(MainMenu);
		MainMenu = NULL;
	}

	if (! Show) {
		if (WinMenu!=NULL) {
			DestroyMenu(WinMenu);
		}
		WinMenu = NULL;
	}
	else {
		InitMenu(&MainMenu);
	}

	::SetMenu(HVTWin, MainMenu);
	::DrawMenuBar(HVTWin);

	B1 = ((ts.MenuFlag & MF_SHOWWINMENU)!=0);
	B2 = (WinMenu!=NULL);
	if ((MainMenu!=NULL) &&
	    (B1 != B2)) {
		if (WinMenu==NULL) {
			WinMenu = CreatePopupMenu();
			get_lang_msg("MENU_WINDOW", ts.UIMsg, sizeof(ts.UIMsg),
			             "&Window", ts.UILanguageFile);
			::InsertMenu(MainMenu,ID_HELPMENU,
			             MF_STRING | MF_ENABLED | MF_POPUP | MF_BYPOSITION,
			             (int)WinMenu, ts.UIMsg);
		}
		else {
			RemoveMenu(MainMenu,ID_HELPMENU,MF_BYPOSITION);
			DestroyMenu(WinMenu);
			WinMenu = NULL;
		}
		::DrawMenuBar(HVTWin);
	}

	::GetSystemMenu(HVTWin,TRUE);
	if ((! Show) && ((ts.MenuFlag & MF_NOSHOWMENU)==0)) {
		SysMenu = ::GetSystemMenu(HVTWin,FALSE);
		AppendMenu(SysMenu, MF_SEPARATOR, 0, NULL);
		get_lang_msg("MENU_SHOW_MENUBAR", ts.UIMsg, sizeof(ts.UIMsg),
		             "Show menu &bar", ts.UILanguageFile);
		AppendMenu(SysMenu, MF_STRING, ID_SHOWMENUBAR, ts.UIMsg);
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
	if (TBar == (ts.HideTitle==0)) {
		return 0;
	}

#ifndef WINDOW_MAXMIMUM_ENABLED
	if (ts.HideTitle>0)
		Style = Style & ~(WS_SYSMENU | WS_CAPTION |
		                  WS_MINIMIZEBOX) | WS_BORDER | WS_POPUP;
	else
		Style = Style & ~WS_POPUP | WS_SYSMENU | WS_CAPTION |
	                     WS_MINIMIZEBOX;
#else
	if (ts.HideTitle>0) {
		Style = Style & ~(WS_SYSMENU | WS_CAPTION |
	                      WS_MINIMIZEBOX | WS_MAXIMIZEBOX) | WS_BORDER | WS_POPUP;

#ifdef ALPHABLEND_TYPE2
		if(BGNoFrame) {
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
	::SetWindowPos(HVTWin, NULL, 0, 0, 0, 0,
	               SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	::ShowWindow(HVTWin, SW_SHOW);

	if ((ts.HideTitle==0) && (MainMenu==NULL) &&
	    ((ts.MenuFlag & MF_NOSHOWMENU) == 0)) {
		SysMenu = ::GetSystemMenu(HVTWin,FALSE);
		AppendMenu(SysMenu, MF_SEPARATOR, 0, NULL);
		get_lang_msg("MENU_SHOW_MENUBAR", ts.UIMsg, sizeof(ts.UIMsg),
		             "Show menu &bar", ts.UILanguageFile);
		AppendMenu(SysMenu, MF_STRING, ID_SHOWMENUBAR, ts.UIMsg);
	}
	return 0;
}

LONG CVTWindow::OnCommNotify(UINT wParam, LONG lParam)
{
	switch (LOWORD(lParam)) {
		case FD_READ:  // TCP/IP
			CommProcRRQ(&cv);
			break;
		case FD_CLOSE:
			if (cv.PortType == IdTCPIP) {
				if (TCPLocalEchoUsed) {
					TCPLocalEchoUsed=FALSE;
					ts.LocalEcho = ts.LocalEcho_ini;
				}
				if (TCPCRSendUsed) {
					TCPCRSendUsed = FALSE;
					ts.CRSend = ts.CRSend_ini;
					cv.CRSend = ts.CRSend_ini;
				}
			}
			Connecting = FALSE;
			TCPIPClosed = TRUE;
			// disable transmition
			cv.OutBuffCount = 0;
			cv.LineModeBuffCount = 0;
			cv.FlushLen = 0;
			SetTimer(IdComEndTimer,1,NULL);
			break;
	}
	return 0;
}

LONG CVTWindow::OnCommOpen(UINT wParam, LONG lParam)
{
	CommStart(&cv,lParam,&ts);
#ifndef NO_INET6
	if (ts.PortType == IdTCPIP && cv.RetryWithOtherProtocol == TRUE) {
		Connecting = TRUE;
	}
	else {
		Connecting = FALSE;
	}
#else
	Connecting = FALSE;
#endif /* NO_INET6 */
	ChangeTitle();
	if (! cv.Ready) {
		return 0;
	}

	/* Auto start logging (2007.5.31 maya) */
	if (ts.LogAutoStart && ts.LogFN[0]==0) {
		strncpy_s(ts.LogFN, sizeof(ts.LogFN), ts.LogDefaultName, _TRUNCATE);
	}
	/* ���O�̎悪�L���ŊJ�n���Ă��Ȃ���ΊJ�n���� (2006.9.18 maya) */
	if ((ts.LogFN[0]!=0) && (LogVar==NULL) && NewFileVar(&LogVar)) {
		LogVar->DirLen = 0;
		strncpy_s(LogVar->FullName, sizeof(LogVar->FullName), ts.LogFN, _TRUNCATE);
		LogStart();
	}

	if ((ts.PortType==IdTCPIP) &&
	    ((ts.PortFlag & PF_BEEPONCONNECT) != 0)) {
		MessageBeep(0);
	}

	if (cv.PortType==IdTCPIP) {
		InitTelnet();

		if ((cv.TelFlag) && (ts.TCPPort==ts.TelPort)) {
			// Start telnet option negotiation from this side
			//   if telnet flag is set and port#==default telnet port# (23)
			TelEnableMyOpt(TERMTYPE);

			TelEnableHisOpt(SGA);

			TelEnableMyOpt(SGA);

			if (ts.TelEcho>0)
				TelChangeEcho();
			else
				TelEnableHisOpt(ECHO);

			TelEnableMyOpt(NAWS);
			if (ts.TelBin>0) {
				TelEnableMyOpt(BINARY);
				TelEnableHisOpt(BINARY);
			}

			TelStartKeepAliveThread();
		}
		else if (!ts.DisableTCPEchoCR) {
			if (ts.TCPCRSend>0) {
				TCPCRSendUsed = TRUE;
				ts.CRSend = ts.TCPCRSend;
				cv.CRSend = ts.TCPCRSend;
			}
			if (ts.TCPLocalEcho>0) {
				TCPLocalEchoUsed = TRUE;
				ts.LocalEcho = ts.TCPLocalEcho;
			}
		}
	}

	if (DDELog || FileLog) {
		if (! CreateLogBuf()) {
			if (DDELog) {
				EndDDE();
			}
			if (FileLog) {
				FileTransEnd(OpLog);
			}
		}
	}

	if (BinLog) {
		if (! CreateBinBuf()) {
			FileTransEnd(OpLog);
		}
	}

	SetDdeComReady(1);

	return 0;
}

LONG CVTWindow::OnCommStart(UINT wParam, LONG lParam)
{
	// �����ڑ��������̂Ƃ����ڑ��_�C�A���O���o���悤�ɂ��� (2006.9.15 maya)
	if (((ts.PortType!=IdSerial) && (ts.HostName[0]==0)) ||
	    ((ts.PortType==IdSerial) && (ts.ComAutoConnect == FALSE))) {
		if (ts.HostDialogOnStartup) {
			OnFileNewConnection();
		}
		else {
			SetDdeComReady(0);
		}
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
	if (CloseTT) {
		OnClose();
	}
	return 0;
}

LONG CVTWindow::OnDlgHelp(UINT wParam, LONG lParam)
{
	OpenHelp(HH_HELP_CONTEXT, HelpId, ts.UILanguageFile);
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

LONG CVTWindow::OnChangeTitle(UINT wParam, LONG lParam)
{
	ChangeTitle();
	return 0;
}

void CVTWindow::OnFileNewConnection()
{
//	char Command[MAXPATHLEN], Command2[MAXPATHLEN];
	char Command[MAXPATHLEN + HostNameMaxLength], Command2[MAXPATHLEN + HostNameMaxLength]; // yutaka
	TGetHNRec GetHNRec; /* record for dialog box */

	if (Connecting) return;

	HelpId = HlpFileNewConnection;
	GetHNRec.SetupFN = ts.SetupFName;
	GetHNRec.PortType = ts.PortType;
	GetHNRec.Telnet = ts.Telnet;
	GetHNRec.TelPort = ts.TelPort;
	GetHNRec.TCPPort = ts.TCPPort;
#ifndef NO_INET6
	GetHNRec.ProtocolFamily = ts.ProtocolFamily;
#endif /* NO_INET6 */
	GetHNRec.ComPort = ts.ComPort;
	GetHNRec.MaxComPort = ts.MaxComPort;

	strncpy_s(Command, sizeof(Command),"ttermpro ", _TRUNCATE);
	GetHNRec.HostName = &Command[9];

	if (! LoadTTDLG()) {
		return;
	}

	if ((*GetHostName)(HVTWin,&GetHNRec)) {
		if ((GetHNRec.PortType==IdTCPIP) && LoadTTSET()) {
			if (ts.HistoryList) {
				(*AddHostToList)(ts.SetupFName,GetHNRec.HostName);
			}
			if (ts.JumpList) {
				add_session_to_jumplist(GetHNRec.HostName, GetHNRec.SetupFN);
			}
			FreeTTSET();
		}

		if (! cv.Ready) {
			ts.PortType = GetHNRec.PortType;
			ts.Telnet = GetHNRec.Telnet;
			ts.TCPPort = GetHNRec.TCPPort;
#ifndef NO_INET6
			ts.ProtocolFamily = GetHNRec.ProtocolFamily;
#endif /* NO_INET6 */
			ts.ComPort = GetHNRec.ComPort;

			if ((GetHNRec.PortType==IdTCPIP) &&
				LoadTTSET()) {
				(*ParseParam)(Command, &ts, NULL);
				FreeTTSET();
			}
			SetKeyMap();
			if (ts.MacroFN[0]!=0) {
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
			if (GetHNRec.PortType==IdSerial) {
				char comport[5];
				Command[8] = 0;
				strncat_s(Command,sizeof(Command)," /C=",_TRUNCATE);
				_snprintf_s(comport, sizeof(comport), _TRUNCATE, "%d", GetHNRec.ComPort);
				strncat_s(Command,sizeof(Command),comport,_TRUNCATE);
			}
			else {
				char tcpport[6];
				strncpy_s(Command2, sizeof(Command2), &Command[9], _TRUNCATE);
				Command[9] = 0;
				if (GetHNRec.Telnet==0)
					strncat_s(Command,sizeof(Command)," /T=0",_TRUNCATE);
				else
					strncat_s(Command,sizeof(Command)," /T=1",_TRUNCATE);
				if (GetHNRec.TCPPort<65535) {
					strncat_s(Command,sizeof(Command)," /P=",_TRUNCATE);
					_snprintf_s(tcpport, sizeof(tcpport), _TRUNCATE, "%d", GetHNRec.TCPPort);
					strncat_s(Command,sizeof(Command),tcpport,_TRUNCATE);
				}
#ifndef NO_INET6
				/********************************/
				/* �����Ƀv���g�R������������ */
				/********************************/
				if (GetHNRec.ProtocolFamily == AF_INET) {
					strncat_s(Command,sizeof(Command)," /4",_TRUNCATE);
				} else if (GetHNRec.ProtocolFamily == AF_INET6) {
					strncat_s(Command,sizeof(Command)," /6",_TRUNCATE);
				}
#endif /* NO_INET6 */
				strncat_s(Command,sizeof(Command)," ",_TRUNCATE);
				strncat_s(Command,sizeof(Command),Command2,_TRUNCATE);
			}
			TTXSetCommandLine(Command, sizeof(Command), &GetHNRec); /* TTPLUG */
			WinExec(Command,SW_SHOW);
		}
	}
	else {/* canceled */
		if (! cv.Ready) {
			SetDdeComReady(0);
		}
	}

	FreeTTDLG();
}


// ���łɊJ���Ă���Z�b�V�����̕��������
// (2004.12.6 yutaka)
void CVTWindow::OnDuplicateSession()
{
	char Command[1024];
	char *exec = "ttermpro";
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	
	// ���݂̐ݒ���e�����L�������փR�s�[���Ă���
	CopyTTSetToShmem(&ts);

	if (ts.TCPPort != 23 && (strcmp(ts.HostName, "127.0.0.1") == 0 ||
	    strcmp(ts.HostName, "localhost") == 0)) {
		// localhost�ւ̐ڑ��Ń|�[�g��23�ȊO�̎���cygwin�ڑ��Ƃ݂Ȃ��B
		OnCygwinConnection();
		return;
	} else if (cv.TelFlag) { // telnet
		_snprintf_s(Command, sizeof(Command), _TRUNCATE,
		            "%s %s:%d /DUPLICATE /nossh", 
		            exec, ts.HostName, ts.TCPPort);

	} else if (cv.isSSH) { // SSH
		// �����̏����� TTSSH ���ɂ�点��ׂ� (2004.12.7 yutaka)
		// TTSSH���ł̃I�v�V����������ǉ��B(2005.4.8 yutaka)
		_snprintf_s(Command, sizeof(Command), _TRUNCATE,
		            "%s %s:%d /DUPLICATE", 
		            exec, ts.HostName, ts.TCPPort);

		TTXSetCommandLine(Command, sizeof(Command), NULL); /* TTPLUG */

	} else {
		// telnet/ssh/cygwin�ڑ��ȊO�ł͕������s��Ȃ��B
		return;
	}

	// �Z�b�V�����������s���ہA/K= ������Έ����p�����s���悤�ɂ���B
	// cf. http://sourceforge.jp/ticket/browse.php?group_id=1412&tid=24682
	// (2011.3.27 yutaka)
	if (strlen(ts.KeyCnfFN) > 0) {
		strncat_s(Command, sizeof(Command), " /K=", _TRUNCATE);
		strncat_s(Command, sizeof(Command), ts.KeyCnfFN, _TRUNCATE);
	}

	memset(&si, 0, sizeof(si));
	GetStartupInfo(&si);
	memset(&pi, 0, sizeof(pi));

	if (CreateProcess(NULL, Command, NULL, NULL, FALSE, 0,
	                  NULL, NULL, &si, &pi) == 0) {
		char buf[80];
		char uimsg[MAX_UIMSG];
		get_lang_msg("MSG_ERROR", uimsg, sizeof(uimsg), "ERROR", ts.UILanguageFile);
		get_lang_msg("MSG_EXEC_TT_ERROR", ts.UIMsg, sizeof(ts.UIMsg),
		             "Can't execute Tera Term. (%d)", ts.UILanguageFile);
		_snprintf_s(buf, sizeof(buf), _TRUNCATE, ts.UIMsg, GetLastError());
		::MessageBox(NULL, buf, uimsg, MB_OK | MB_ICONWARNING);
	}
}


//
// Connect to local cygwin
//
void CVTWindow::OnCygwinConnection()
{
	char file[MAX_PATH], *filename;
	char c, *envptr, *envbuff=NULL;
	int envbufflen;
	char *exename = "cygterm.exe";
	char cygterm[MAX_PATH];
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	char uimsg[MAX_UIMSG];

	if (strlen(ts.CygwinDirectory) > 0) {
		if (SearchPath(ts.CygwinDirectory, "bin\\cygwin1", ".dll", sizeof(file), file, &filename) > 0) {
			goto found_dll;
		}
	}

	if (SearchPath(NULL, "cygwin1", ".dll", sizeof(file), file, &filename) > 0) {
		goto found_path;
	}

	for (c = 'C' ; c <= 'Z' ; c++) {
		char tmp[MAX_PATH];
		sprintf(tmp, "%c:\\cygwin\\bin;%c:\\cygwin64\\bin", c, c);
		if (SearchPath(tmp, "cygwin1", ".dll", sizeof(file), file, &filename) > 0) {
			goto found_dll;
		}
	}

	get_lang_msg("MSG_ERROR", uimsg, sizeof(uimsg), "ERROR", ts.UILanguageFile);
	get_lang_msg("MSG_FIND_CYGTERM_DIR_ERROR", ts.UIMsg, sizeof(ts.UIMsg),
	             "Can't find Cygwin directory.", ts.UILanguageFile);
	::MessageBox(NULL, ts.UIMsg, uimsg, MB_OK | MB_ICONWARNING);
	return;

found_dll:;
	envptr = getenv("PATH");
	file[strlen(file)-12] = '\0'; // delete "\\cygwin1.dll"
	if (envptr != NULL) {
		envbufflen = strlen(file) + strlen(envptr) + 7; // "PATH="(5) + ";"(1) + NUL(1)
		if ((envbuff = (char *)malloc(envbufflen)) == NULL) {
			get_lang_msg("MSG_ERROR", uimsg, sizeof(uimsg), "ERROR", ts.UILanguageFile);
			get_lang_msg("MSG_CYGTERM_ENV_ALLOC_ERROR", ts.UIMsg, sizeof(ts.UIMsg),
			             "Can't allocate memory for environment variable.", ts.UILanguageFile);
			::MessageBox(NULL, ts.UIMsg, uimsg, MB_OK | MB_ICONWARNING);
			return;
		}
		_snprintf_s(envbuff, envbufflen, _TRUNCATE, "PATH=%s;%s", file, envptr);
	} else {
		envbufflen = strlen(file) + 6; // "PATH="(5) + NUL(1)
		if ((envbuff = (char *)malloc(envbufflen)) == NULL) {
			get_lang_msg("MSG_ERROR", uimsg, sizeof(uimsg), "ERROR", ts.UILanguageFile);
			get_lang_msg("MSG_CYGTERM_ENV_ALLOC_ERROR", ts.UIMsg, sizeof(ts.UIMsg),
			             "Can't allocate memory for environment variable.", ts.UILanguageFile);
			::MessageBox(NULL, ts.UIMsg, uimsg, MB_OK | MB_ICONWARNING);
			return;
		}
		_snprintf_s(envbuff, envbufflen, _TRUNCATE, "PATH=%s", file);
	}
	_putenv(envbuff);
	if (envbuff) {
		free(envbuff);
		envbuff = NULL;
	}

found_path:;
	memset(&si, 0, sizeof(si));
	GetStartupInfo(&si);
	memset(&pi, 0, sizeof(pi));

	strncpy_s(cygterm, sizeof(cygterm), ts.HomeDir, _TRUNCATE);
	AppendSlash(cygterm, sizeof(cygterm));
	strncat_s(cygterm, sizeof(cygterm), exename, _TRUNCATE);

	if (CreateProcess(NULL, cygterm, NULL, NULL, FALSE, 0,
	                  NULL, NULL, &si, &pi) == 0) {
		get_lang_msg("MSG_ERROR", uimsg, sizeof(uimsg), "ERROR", ts.UILanguageFile);
		get_lang_msg("MSG_EXEC_CYGTERM_ERROR", ts.UIMsg, sizeof(ts.UIMsg),
		             "Can't execute Cygterm.", ts.UILanguageFile);
		::MessageBox(NULL, ts.UIMsg, uimsg, MB_OK | MB_ICONWARNING);
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

	if (CreateProcess(NULL, exename, NULL, NULL, FALSE, 0,
	                  NULL, NULL, &si, &pi) == 0) {
		char buf[80];
		char uimsg[MAX_UIMSG];
		get_lang_msg("MSG_ERROR", uimsg, sizeof(uimsg), "ERROR", ts.UILanguageFile);
		get_lang_msg("MSG_EXEC_TTMENU_ERROR", ts.UIMsg, sizeof(ts.UIMsg),
		             "Can't execute TeraTerm Menu. (%d)", ts.UILanguageFile);
		_snprintf_s(buf, sizeof(buf), _TRUNCATE, ts.UIMsg, GetLastError());
		::MessageBox(NULL, buf, uimsg, MB_OK | MB_ICONWARNING);
	}
}


//
// LogMeTT�̋N��
//
void CVTWindow::OnLogMeInLaunch()
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	if (!isLogMeTTExist()) {
		return;
	}

	memset(&si, 0, sizeof(si));
	GetStartupInfo(&si);
	memset(&pi, 0, sizeof(pi));

	if (CreateProcess(NULL, LogMeTT, NULL, NULL, FALSE, 0,
	                  NULL, NULL, &si, &pi) == 0) {
		char buf[80];
		char uimsg[MAX_UIMSG];
		get_lang_msg("MSG_ERROR", uimsg, sizeof(uimsg), "ERROR", ts.UILanguageFile);
		get_lang_msg("MSG_EXEC_LOGMETT_ERROR", ts.UIMsg, sizeof(ts.UIMsg),
		             "Can't execute LogMeTT. (%d)", ts.UILanguageFile);
		_snprintf_s(buf, sizeof(buf), _TRUNCATE, ts.UIMsg, GetLastError());
		::MessageBox(NULL, buf, uimsg, MB_OK | MB_ICONWARNING);
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
	LOGFONT logfont;
	HFONT font;
	char uimsg[MAX_UIMSG];

	switch (msg) {
		case WM_INITDIALOG:
			//SetDlgItemText(hDlgWnd, IDC_EDIT_COMMENT, "�T���v��");
			// �G�f�B�b�g�R���g���[���Ƀt�H�[�J�X�����Ă�
			SetFocus(GetDlgItem(hDlgWnd, IDC_EDIT_COMMENT));

			font = (HFONT)SendMessage(hDlgWnd, WM_GETFONT, 0, 0);
			GetObject(font, sizeof(LOGFONT), &logfont);
			if (get_lang_font("DLG_SYSTEM_FONT", hDlgWnd, &logfont, &DlgCommentFont, ts.UILanguageFile)) {
				SendDlgItemMessage(hDlgWnd, IDC_EDIT_COMMENT, WM_SETFONT, (WPARAM)DlgCommentFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hDlgWnd, IDOK, WM_SETFONT, (WPARAM)DlgCommentFont, MAKELPARAM(TRUE,0));
			}
			else {
				DlgCommentFont = NULL;
			}

			GetWindowText(hDlgWnd, uimsg, sizeof(uimsg));
			get_lang_msg("DLG_COMMENT_TITLE", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
			SetWindowText(hDlgWnd, ts.UIMsg);
			GetDlgItemText(hDlgWnd, IDOK, uimsg, sizeof(uimsg));
			get_lang_msg("BTN_OK", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
			SetDlgItemText(hDlgWnd, IDOK, ts.UIMsg);

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
					if (DlgCommentFont != NULL) {
						DeleteObject(DlgCommentFont);
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
	ret = DialogBox(hInst, MAKEINTRESOURCE(IDD_COMMENT_DIALOG),
	                HVTWin, (DLGPROC)OnCommentDlgProc);
	if (ret == 0 || ret == -1) {
		ret = GetLastError();
	}

}


// ���O�̉{�� (2005.1.29 yutaka)
void CVTWindow::OnViewLog()
{
	char command[MAX_PATH*2+3]; // command "filename"
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

	_snprintf_s(command, sizeof(command), _TRUNCATE, "%s \"%s\"", ts.ViewlogEditor, file);

	if (CreateProcess(NULL, command, NULL, NULL, FALSE, 0,
	                  NULL, NULL, &si, &pi) == 0) {
		char buf[80];
		char uimsg[MAX_UIMSG];
		get_lang_msg("MSG_ERROR", uimsg, sizeof(uimsg), "ERROR", ts.UILanguageFile);
		get_lang_msg("MSG_VIEW_LOGFILE_ERROR", ts.UIMsg, sizeof(ts.UIMsg),
		             "Can't view logging file. (%d)", ts.UILanguageFile);
		_snprintf_s(buf, sizeof(buf), _TRUNCATE, ts.UIMsg, GetLastError());
		::MessageBox(NULL, buf, uimsg, MB_OK | MB_ICONWARNING);
	}
}


// �B���Ă��郍�O�_�C�A���O��\������ (2008.2.3 maya)
void CVTWindow::OnShowLogDialog()
{
	ShowFTDlg(OpLog);
}


// ���O�̍Đ� (2006.12.13 yutaka)
void CVTWindow::OnReplayLog()
{
	OPENFILENAME ofn;
	char szFile[MAX_PATH];
	char Command[MAX_PATH] = "notepad.exe";
	char *exec = "ttermpro";
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	char uimsg[MAX_UIMSG];

	// �o�C�i�����[�h�ō̎悵�����O�t�@�C����I������
	memset(&ofn, 0, sizeof(OPENFILENAME));
	memset(szFile, 0, sizeof(szFile));
	ofn.lStructSize = get_OPENFILENAME_SIZE();
	ofn.hwndOwner = HVTWin;
	get_lang_msg("FILEDLG_OPEN_LOGFILE_FILTER", ts.UIMsg, sizeof(ts.UIMsg),
	             "all(*.*)\\0*.*\\0\\0", ts.UILanguageFile);
	ofn.lpstrFilter = ts.UIMsg;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = "log";
	get_lang_msg("FILEDLG_OPEN_LOGFILE_TITLE", uimsg, sizeof(uimsg),
	             "Select replay log file with binary mode", ts.UILanguageFile);
	ofn.lpstrTitle = uimsg;
	if(GetOpenFileName(&ofn) == 0) 
		return;


	// "/R"�I�v�V�����t����Tera Term���N������i���O���Đ������j
	_snprintf_s(Command, sizeof(Command), _TRUNCATE,
	            "%s /R=\"%s\"", exec, szFile);

	memset(&si, 0, sizeof(si));
	GetStartupInfo(&si);
	memset(&pi, 0, sizeof(pi));

	if (CreateProcess(NULL, Command, NULL, NULL, FALSE, 0,
	                  NULL, NULL, &si, &pi) == 0) {
		char buf[80];
		get_lang_msg("MSG_ERROR", uimsg, sizeof(uimsg), "ERROR", ts.UILanguageFile);
		get_lang_msg("MSG_EXEC_TT_ERROR", ts.UIMsg, sizeof(ts.UIMsg),
		             "Can't execute Tera Term. (%d)", ts.UILanguageFile);
		_snprintf_s(buf, sizeof(buf), _TRUNCATE, ts.UIMsg, GetLastError());
		::MessageBox(NULL, buf, uimsg, MB_OK | MB_ICONWARNING);
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

void CVTWindow::OnFileYRcv()
{
	HelpId = HlpFileYmodemRecv;
	YMODEMStart(IdYReceive);
}

void CVTWindow::OnFileYSend()
{
	HelpId = HlpFileYmodemSend;
	YMODEMStart(IdYSend);
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
	if (! LoadTTDLG()) {
		return;
	}
	(*ChangeDirectory)(HVTWin,ts.FileDir);
	FreeTTDLG();
}

void CVTWindow::OnFilePrint()
{
	HelpId = HlpFilePrint;
	BuffPrint(FALSE);
}

void CVTWindow::Disconnect(BOOL confirm)
{
	if (! cv.Ready) {
		return;
	}

	if ((cv.PortType==IdTCPIP) &&
	    ((ts.PortFlag & PF_CONFIRMDISCONN) != 0) &&
	    (confirm)) {
		get_lang_msg("MSG_DISCONNECT_CONF", ts.UIMsg, sizeof(ts.UIMsg),
		             "Disconnect?", ts.UILanguageFile);
		if (::MessageBox(HVTWin, ts.UIMsg, "Tera Term",
		                 MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON2)==IDCANCEL) {
			return;
		}
	}

	::PostMessage(HVTWin, WM_USER_COMMNOTIFY, 0, FD_CLOSE);
}

void CVTWindow::OnFileDisconnect()
{
	Disconnect(TRUE);
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
	// add confirm (2008.2.4 yutaka)
	if (CBStartPasteConfirmChange(HVTWin, FALSE)) {
		CBStartPaste(HVTWin, FALSE, BracketedPasteMode(), 0, NULL, 0);
		/* �ŉ��s�ł��������X�N���[������ݒ�̏ꍇ
		   �y�[�X�g�����ŃX�N���[�������� */
		if (ts.AutoScrollOnlyInBottomLine != 0 && WinOrgY != 0) {
			DispVScroll(SCROLL_BOTTOM, 0);
		}
	}
}

void CVTWindow::OnEditPasteCR()
{
	// add confirm (2008.3.11 maya)
	if (CBStartPasteConfirmChange(HVTWin, TRUE)) {
		CBStartPaste(HVTWin, TRUE, BracketedPasteMode(), 0, NULL, 0);
		/* �ŉ��s�ł��������X�N���[������ݒ�̏ꍇ
		   �y�[�X�g�����ŃX�N���[�������� */
		if (ts.AutoScrollOnlyInBottomLine != 0 && WinOrgY != 0) {
			DispVScroll(SCROLL_BOTTOM, 0);
		}
	}
}

void CVTWindow::OnEditClearScreen()
{
	LockBuffer();
	BuffClearScreen();
	if (isCursorOnStatusLine) {
		MoveCursor(0,CursorY);
	}
	else {
		MoveCursor(0,0);
	}
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

void CVTWindow::OnEditSelectAllBuffer()
{
	// Select all of buffer
	POINT p = {0, 0};

	ButtonDown(p, IdLeftButton);
	BuffAllSelect();
	ButtonUp(FALSE);
	ChangeSelectRegion();
}

void CVTWindow::OnEditSelectScreenBuffer()
{
	// Select screen buffer
	POINT p = {0, 0};

	ButtonDown(p, IdLeftButton);
	BuffScreenSelect();
	ButtonUp(FALSE);
	ChangeSelectRegion();
}

void CVTWindow::OnEditCancelSelection()
{
	// Cancel selected buffer
	POINT p = {0, 0};

	ButtonDown(p, IdLeftButton);
	BuffCancelSelection();
	ButtonUp(FALSE);
	ChangeSelectRegion();
}

// Additional settings dialog
//
// (2004.9.5 yutaka) new added
// (2005.2.22 yutaka) changed to Tab Control
// (2008.5.12 maya) changed to PropertySheet
void CVTWindow::OnExternalSetup()
{
	DWORD ret;

	CAddSettingPropSheetDlg CAddSetting("", CWnd::FromHandle(HVTWin));
	CAddSetting.EnableStackedTabs(FALSE);
	ret = CAddSetting.DoModal();
	switch (ret) {
		case -1:
		case IDABORT:
			ret = GetLastError();
			break;
		case IDOK:
#ifdef ALPHABLEND_TYPE2
			BGInitialize();
			BGSetupPrimary(TRUE);
#else
			DispApplyANSIColor();
#endif
			DispSetNearestColors(IdBack, IdFore+8, NULL);
			ChangeWin();
			ChangeFont();
			break;
		default:
			/* nothing to do */
			break;
	}
}

void CVTWindow::OnSetupTerminal()
{
	BOOL Ok;

	if (ts.Language==IdRussian) {
		HelpId = HlpSetupTerminalRuss;
	}
	else {
		HelpId = HlpSetupTerminal;
	}
	if (! LoadTTDLG()) {
		return;
	}
	Ok = (*SetupTerminal)(HVTWin, &ts);
	FreeTTDLG();
	if (Ok) {
		SetupTerm();
	}
}

void CVTWindow::OnSetupWindow()
{
	BOOL Ok;
	char orgTitle[TitleBuffSize];

	HelpId = HlpSetupWindow;
	ts.VTFlag = 1;
	ts.SampleFont = VTFont[0];

	if (! LoadTTDLG()) {
		return;
	}

	strncpy_s(orgTitle, sizeof(orgTitle), ts.Title, _TRUNCATE);
	Ok = (*SetupWin)(HVTWin, &ts);
	FreeTTDLG();

	if (Ok) {
		// Eterm lookfeel�̉�ʏ����X�V���邱�ƂŁA���A���^�C���ł̔w�i�F�ύX��
		// �\�ƂȂ�B(2006.2.24 yutaka)
#ifdef ALPHABLEND_TYPE2
		BGInitialize();
		BGSetupPrimary(TRUE);
#endif

		// �^�C�g�����ύX����Ă�����A�����[�g�^�C�g�����N���A����
		if ((ts.AcceptTitleChangeRequest == IdTitleChangeRequestOverwrite) &&
		    (strcmp(orgTitle, ts.Title) != 0)) {
			cv.TitleRemote[0] = '\0';
		}

		ChangeFont();
		ChangeWin();
	}

}

void CVTWindow::OnSetupFont()
{
	HelpId = HlpSetupFont;
	DispSetupFontDlg();
}

void CVTWindow::OnSetupKeyboard()
{
	BOOL Ok;

	if (ts.Language==IdRussian) {
		HelpId = HlpSetupKeyboardRuss;
	}
	else {
		HelpId = HlpSetupKeyboard;
	}
	if (! LoadTTDLG()) {
		return;
	}
	Ok = (*SetupKeyboard)(HVTWin, &ts);
	FreeTTDLG();

	if (Ok) {
//		ResetKeypadMode(TRUE);
		if ((ts.Language==IdJapanese) || (ts.Language==IdKorean) || (ts.Language==IdUtf8)) //HKS
			ResetIME();
	}
}

void CVTWindow::OnSetupSerialPort()
{
	BOOL Ok;
	HelpId = HlpSetupSerialPort;
	if (! LoadTTDLG()) {
		return;
	}
	Ok = (*SetupSerialPort)(HVTWin, &ts);
	FreeTTDLG();

	if (Ok && ts.ComPort > 0) {
		if (cv.Open) {
			if (ts.ComPort != cv.ComPort) {
				CommClose(&cv);
				CommOpen(HVTWin,&ts,&cv);
			}
			else {
				CommResetSerial(&ts, &cv, ts.ClearComBuffOnOpen);
			}
		}
		else {
			CommOpen(HVTWin,&ts,&cv);
		}
	}
}

void CVTWindow::OnSetupTCPIP()
{
	HelpId = HlpSetupTCPIP;
	if (! LoadTTDLG()) {
		return;
	}
	if ((*SetupTCPIP)(HVTWin, &ts)) {
		TelUpdateKeepAliveInterval();
	}
	FreeTTDLG();
}

void CVTWindow::OnSetupGeneral()
{
	HelpId = HlpSetupGeneral;
	if (! LoadTTDLG()) {
		return;
	}
	if ((*SetupGeneral)(HVTWin,&ts)) {
		ResetCharSet();
		ResetIME();
	}
	FreeTTDLG();
}

void CVTWindow::OnSetupSave()
{
	BOOL Ok;
	char TmpSetupFN[MAX_PATH];
	int ret;

	strncpy_s(TmpSetupFN, sizeof(TmpSetupFN),ts.SetupFName, _TRUNCATE);
	if (! LoadTTFILE()) {
		return;
	}
	HelpId = HlpSetupSave;
	Ok = (*GetSetupFname)(HVTWin,GSF_SAVE,&ts);
	FreeTTFILE();
	if (! Ok) {
		return;
	}

	// �������݂ł��邩�̔��ʂ�ǉ� (2005.11.3 yutaka)
	if ((ret = _access(ts.SetupFName, 0x02)) != 0) {
		if (errno != ENOENT) {  // �t�@�C�������łɑ��݂���ꍇ�̂݃G���[�Ƃ��� (2005.12.13 yutaka)
			char uimsg[MAX_UIMSG];
			get_lang_msg("MSG_TT_ERROR", uimsg, sizeof(uimsg), "Tera Term: ERROR", ts.UILanguageFile);
			get_lang_msg("MSG_SAVESETUP_PERMISSION_ERROR", ts.UIMsg, sizeof(ts.UIMsg),
			             "TERATERM.INI file doesn't have the writable permission.", ts.UILanguageFile);
			MessageBox(ts.UIMsg, uimsg, MB_OK|MB_ICONEXCLAMATION);
			return;
		}
	}

	if (LoadTTSET())
	{
		int w, h;

#ifdef WINDOW_MAXMIMUM_ENABLED
		if (IsZoomed()) {
			w = ts.TerminalWidth;
			h = ts.TerminalHeight;
			ts.TerminalWidth = ts.TerminalOldWidth;
			ts.TerminalHeight = ts.TerminalOldHeight;
		}
#endif

		CopyFile(TmpSetupFN, ts.SetupFName, TRUE);
		/* write current setup values to file */
		(*WriteIniFile)(ts.SetupFName,&ts);
		/* copy host list */
		(*CopyHostList)(TmpSetupFN,ts.SetupFName);
		FreeTTSET();

#ifdef WINDOW_MAXMIMUM_ENABLED
		if (IsZoomed()) {
			ts.TerminalWidth = w;
			ts.TerminalHeight = h;
		}
#endif
	}

#if 0
	ChangeDefaultSet(&ts,NULL);
#endif
}

void CVTWindow::OnSetupRestore()
{
	BOOL Ok;

	HelpId = HlpSetupRestore;
	if (! LoadTTFILE()) {
		return;
	}
	Ok = (*GetSetupFname)(HVTWin,GSF_RESTORE,&ts);
	FreeTTFILE();
	if (Ok) {
		RestoreSetup();
	}
}

void CVTWindow::OnSetupLoadKeyMap()
{
	BOOL Ok;

	HelpId = HlpSetupLoadKeyMap;
	if (! LoadTTFILE()) {
		return;
	}
	Ok = (*GetSetupFname)(HVTWin,GSF_LOADKEY,&ts);
	FreeTTFILE();
	if (! Ok) {
		return;
	}

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

void CVTWindow::OnControlResetRemoteTitle()
{
	cv.TitleRemote[0] = '\0';
	ChangeTitle();
}

void CVTWindow::OnControlAreYouThere()
{
	if (cv.Ready && (cv.PortType==IdTCPIP)) {
		TelSendAYT();
	}
}

void CVTWindow::OnControlSendBreak()
{
	if (cv.Ready)
		switch (cv.PortType) {
			case IdTCPIP:
				// SSH2�ڑ��̏ꍇ�A��p�̃u���[�N�M���𑗐M����B(2010.9.28 yutaka)
				if (cv.isSSH == 2) {
					if (TTXProcessCommand(HVTWin, ID_CONTROL_SENDBREAK)) {
						break;
					}
				} 

				TelSendBreak();
				break;
			case IdSerial:
				CommSendBreak(&cv);
				break;
		}
}

void CVTWindow::OnControlResetPort()
{
	CommResetSerial(&ts, &cv, TRUE);
}

void ApplyBoradCastCommandHisotry(HWND Dialog, char *historyfile)
{
	char EntName[13];
	char Command[HostNameMaxLength+1];
	int i = 1;

	SendDlgItemMessage(Dialog, IDC_COMMAND_EDIT, CB_RESETCONTENT, 0, 0);
	do {
		_snprintf_s(EntName, sizeof(EntName), _TRUNCATE, "Command%d", i);
		GetPrivateProfileString("BroadcastCommands",EntName,"",
		                        Command,sizeof(Command), historyfile);
		if (strlen(Command) > 0) {
			SendDlgItemMessage(Dialog, IDC_COMMAND_EDIT, CB_ADDSTRING,
			                   0, (LPARAM)Command);
		}
		i++;
	} while ((i <= ts.MaxBroadcatHistory) && (strlen(Command)>0));

	SendDlgItemMessage(Dialog, IDC_COMMAND_EDIT, EM_LIMITTEXT,
	                   HostNameMaxLength-1, 0);

	SendDlgItemMessage(Dialog, IDC_COMMAND_EDIT, CB_SETCURSEL,0,0);
}




// �h���b�v�_�E���̒��̃G�f�B�b�g�R���g���[����
// �T�u�N���X�����邽�߂̃E�C���h�E�v���V�[�W��
static WNDPROC OrigBroadcastEditProc; // Original window procedure
static HWND BroadcastWindowList;
static LRESULT CALLBACK BroadcastEditProc(HWND dlg, UINT msg,
                                          WPARAM wParam, LPARAM lParam)
{
	char buf[1024];
	int len;

	switch (msg) {
		case WM_CREATE:
			break;

		case WM_DESTROY:
			break;

		case WM_LBUTTONUP:
			// ���łɃe�L�X�g�����͂���Ă���ꍇ�́A�J�[�\���𖖔��ֈړ�������B
			len = GetWindowText(dlg, buf, sizeof(buf));
			SendMessage(dlg, EM_SETSEL, len, len);
			SetFocus(dlg);
			break;

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
			SetFocus(dlg);
			break;

		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
			{
				int i;
				HWND hd;
				int count;

				if (wParam == 0x0d) {  // Enter key
					SetWindowText(dlg, "");
					SendMessage(dlg, EM_SETSEL, 0, 0);
				}
#if 0
				for (i = 0 ; i < MAXNWIN ; i++) { // 50 = MAXNWIN(@ ttcmn.c)
					hd = GetNthWin(i);
					if (hd == NULL)
						break;

					PostMessage(hd, msg, wParam, lParam);
					//PostMessage(hd, WM_SETFOCUS, NULL, 0);
				}
#else
				count = SendMessage(BroadcastWindowList, LB_GETCOUNT, 0, 0);
				for (i = 0 ; i < count ; i++) {
					if (SendMessage(BroadcastWindowList, LB_GETSEL, i, 0)) {
						hd = GetNthWin(i);
						if (hd) {
							PostMessage(hd, msg, wParam, lParam);
						}
					}
				}
#endif
			}
			break;

		case WM_CHAR:
#if 0
			switch (wParam) {
				case VK_RETURN:
				case VK_ESCAPE:
					// �x�������o�Ȃ��悤�ɂ���
					return FALSE;
			}
			// not break;
#endif
			// ���͂�����������IDC_COMMAND_EDIT�Ɏc��Ȃ��悤�Ɏ̂Ă�
			return FALSE;

		default:
			return CallWindowProc(OrigBroadcastEditProc, dlg, msg, wParam, lParam);
	}

	return FALSE;
}


static int GetApplicationInstanceCount(void)
{
	int i;
	HWND hd;

	for (i = 0 ; i < MAXNWIN ; i++) { // 50 = MAXNWIN(@ ttcmn.c)
		hd = GetNthWin(i);
		if (hd == NULL) {
			break;
		}
	}
	return (i);
}


static void UpdateBroadcastWindowList(HWND hWnd)
{
	int i;
	HWND hd;
    TCHAR szWindowText[256];

	SendMessage(hWnd, LB_RESETCONTENT, 0, 0);

	for (i = 0 ; i < MAXNWIN ; i++) { // 50 = MAXNWIN(@ ttcmn.c)
		hd = GetNthWin(i);
		if (hd == NULL) {
			break;
		}

		GetWindowText(hd, szWindowText, 256);
		SendMessage(hWnd, LB_INSERTSTRING, -1, (LPARAM)szWindowText);
	}

#if 0
	for (i = 0 ; i < SendMessage(BroadcastWindowList, LB_GETCOUNT, 0, 0) ; i++) {
		SendMessage(hWnd, LB_SETSEL, TRUE, i);
	}
#endif
}

/*
 * �STeraTerm�փ��b�Z�[�W�𑗐M����u���[�h�L���X�g���[�h�B
 * "sendbroadcast"�}�N���R�}���h��������p�����B
 */
extern "C"
void SendAllBroadcastMessage(HWND HVTWin, HWND hWnd, int parent_only, char *buf, int buflen)
{
	int i;
	int count;
	HWND hd;
	COPYDATASTRUCT cds;

	// ���ׂĂ�Tera Term�Ƀ��b�Z�[�W�ƃf�[�^�𑗂�
	count = SendMessage(BroadcastWindowList, LB_GETCOUNT, 0, 0);
	for (i = 0 ; i < count ; i++) { 
		hd = NULL;
		if (parent_only) {
			hd = GetParent(hWnd);
			i = MAXNWIN;		// 337: ���������l :P
		} else {
			// ���X�g�{�b�N�X�őI������Ă��邩
			if (SendMessage(BroadcastWindowList, LB_GETSEL, i, 0)) {
				hd = GetNthWin(i);
			}
		}
		if (hd == NULL) {
			continue;
		}

		ZeroMemory(&cds, sizeof(cds));
		cds.dwData = IPC_BROADCAST_COMMAND;
		cds.cbData = buflen;
		cds.lpData = buf;

		// WM_COPYDATA���g���āA�v���Z�X�ԒʐM���s���B
		SendMessage(hd, WM_COPYDATA, (WPARAM)HVTWin, (LPARAM)&cds);

		// ���M��Tera Term�E�B���h�E�ɓK���ȃ��b�Z�[�W�𑗂�B
		// ��������Ȃ��ƁA���荞�񂾃f�[�^�����f����Ȃ��͗l�B
		// (2006.2.7 yutaka)
		PostMessage(hd, WM_SETFOCUS, NULL, 0);
	}

}


/*
 * �C�ӂ�TeraTerm�Q�փ��b�Z�[�W�𑗐M����}���`�L���X�g���[�h�B�����ɂ́A
 * �u���[�h�L���X�g���M���s���A��M���Ń��b�Z�[�W����̑I������B
 * "sendmulticast"�}�N���R�}���h����̂ݗ��p�����B
 */
extern "C"
void SendMulticastMessage(HWND HVTWin, HWND hWnd, char *name, char *buf, int buflen)
{
	int i;
	HWND hd;
	COPYDATASTRUCT cds;
	char *msg = NULL;
	int msglen, nlen;

	/* ���M���b�Z�[�W���\�z����B
	 *
	 * msg
	 * +------+--------------+--+
	 * |name\0|buf           |\0|
	 * +------+--------------+--+
	 * <--------------------->
	 * msglen = strlen(name) + 1 + buflen
	 * buf�̒���ɂ� \0 �͕t���Ȃ��B
	 */
	nlen = strlen(name) + 1;
	msglen = nlen + buflen;
	msg = (char *)malloc(msglen);
	if (msg == NULL) {
		goto error;
	}
	strcpy_s(msg, msglen, name);
	memcpy(msg + nlen, buf, buflen);

	// ���ׂĂ�Tera Term�Ƀ��b�Z�[�W�ƃf�[�^�𑗂�
	for (i = 0 ; i < MAXNWIN ; i++) { // 50 = MAXNWIN(@ ttcmn.c)
		hd = GetNthWin(i);
		if (hd == NULL) {
			break;
		}

		ZeroMemory(&cds, sizeof(cds));
		cds.dwData = IPC_MULTICAST_COMMAND;
		cds.cbData = msglen;
		cds.lpData = msg;

		// WM_COPYDATA���g���āA�v���Z�X�ԒʐM���s���B
		SendMessage(hd, WM_COPYDATA, (WPARAM)HVTWin, (LPARAM)&cds);

		// ���M��Tera Term�E�B���h�E�ɓK���ȃ��b�Z�[�W�𑗂�B
		// ��������Ȃ��ƁA���荞�񂾃f�[�^�����f����Ȃ��͗l�B
		// (2006.2.7 yutaka)
		PostMessage(hd, WM_SETFOCUS, NULL, 0);
	}

error:
	free(msg);
}


extern "C"
void SetMulticastName(char *name)
{
	strncpy_s(ts.MulticastName, sizeof(ts.MulticastName), name, _TRUNCATE);
}

static int CompareMulticastName(char *name)
{
	return strcmp(ts.MulticastName, name);
}

//
// ���ׂẴ^�[�~�i���֓���R�}���h�𑗐M���郂�[�h���X�_�C�A���O�̕\��
// (2005.1.22 yutaka)
//
static LRESULT CALLBACK BroadcastCommandDlgProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	char buf[256 + 3];
	UINT ret;
	LRESULT checked;
	LRESULT history;
	LOGFONT logfont;
	HFONT font;
	char uimsg[MAX_UIMSG];
	char historyfile[MAX_PATH];
	static HWND hwndBroadcast     = NULL; // Broadcast dropdown
	static HWND hwndBroadcastEdit = NULL; // Edit control on Broadcast dropdown
	// for resize
	RECT rc_dlg, rc, rc_ok;
	POINT p;
	static int ok2right, cancel2right, cmdlist2ok, list2bottom, list2right;
	// for update list
	const int list_timer_id = 100;
	const int list_timer_tick = 1000; // msec
	static int prev_instances = 0;
	// for status bar
	static HWND hStatus = NULL;
	static int init_width, init_height;

	switch (msg) {
		case WM_SHOWWINDOW:
			if (wp) {  // show
				// Tera Term window list
				UpdateBroadcastWindowList(GetDlgItem(hWnd, IDC_LIST));
				return TRUE;
			}
			break;

		case WM_INITDIALOG:
			// ���W�I�{�^���̃f�t�H���g�� CR �ɂ���B
			SendMessage(GetDlgItem(hWnd, IDC_RADIO_CR), BM_SETCHECK, BST_CHECKED, 0);
			// �f�t�H���g�Ń`�F�b�N�{�b�N�X�� checked ��Ԃɂ���B
			SendMessage(GetDlgItem(hWnd, IDC_ENTERKEY_CHECK), BM_SETCHECK, BST_CHECKED, 0);
			// history �𔽉f���� (2007.3.3 maya)
			if (ts.BroadcastCommandHistory) {
				SendMessage(GetDlgItem(hWnd, IDC_HISTORY_CHECK), BM_SETCHECK, BST_CHECKED, 0);
			}
			GetDefaultFName(ts.HomeDir, BROADCAST_LOGFILE, historyfile, sizeof(historyfile));
			ApplyBoradCastCommandHisotry(hWnd, historyfile);

			// �G�f�B�b�g�R���g���[���Ƀt�H�[�J�X�����Ă�
			SetFocus(GetDlgItem(hWnd, IDC_COMMAND_EDIT));

			// �T�u�N���X�������ă��A���^�C�����[�h�ɂ��� (2008.1.21 yutaka)
			hwndBroadcast = GetDlgItem(hWnd, IDC_COMMAND_EDIT);
			hwndBroadcastEdit = GetWindow(hwndBroadcast, GW_CHILD);
			OrigBroadcastEditProc = (WNDPROC)GetWindowLong(hwndBroadcastEdit, GWL_WNDPROC);
			SetWindowLong(hwndBroadcastEdit, GWL_WNDPROC, (LONG)BroadcastEditProc);
			// �f�t�H���g��on�B�c���disable�B
			SendMessage(GetDlgItem(hWnd, IDC_REALTIME_CHECK), BM_SETCHECK, BST_CHECKED, 0);  // default on
			EnableWindow(GetDlgItem(hWnd, IDC_HISTORY_CHECK), FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_RADIO_CRLF), FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_RADIO_CR), FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_RADIO_LF), FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_ENTERKEY_CHECK), FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_PARENT_ONLY), FALSE);

			// Tera Term window list
			BroadcastWindowList = GetDlgItem(hWnd, IDC_LIST);
			UpdateBroadcastWindowList(BroadcastWindowList);

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
				SendDlgItemMessage(hWnd, IDC_REALTIME_CHECK, WM_SETFONT, (WPARAM)DlgBroadcastFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hWnd, IDOK, WM_SETFONT, (WPARAM)DlgBroadcastFont, MAKELPARAM(TRUE,0));
				SendDlgItemMessage(hWnd, IDCANCEL, WM_SETFONT, (WPARAM)DlgBroadcastFont, MAKELPARAM(TRUE,0));
			}
			else {
				DlgBroadcastFont = NULL;
			}
			GetWindowText(hWnd, uimsg, sizeof(uimsg));
			get_lang_msg("DLG_BROADCAST_TITLE", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
			SetWindowText(hWnd, ts.UIMsg);
			GetDlgItemText(hWnd, IDC_HISTORY_CHECK, uimsg, sizeof(uimsg));
			get_lang_msg("DLG_BROADCAST_HISTORY", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
			SetDlgItemText(hWnd, IDC_HISTORY_CHECK, ts.UIMsg);
			GetDlgItemText(hWnd, IDC_ENTERKEY_CHECK, uimsg, sizeof(uimsg));
			get_lang_msg("DLG_BROADCAST_ENTER", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
			SetDlgItemText(hWnd, IDC_ENTERKEY_CHECK, ts.UIMsg);
			GetDlgItemText(hWnd, IDC_PARENT_ONLY, uimsg, sizeof(uimsg));
			get_lang_msg("DLG_BROADCAST_PARENTONLY", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
			SetDlgItemText(hWnd, IDC_PARENT_ONLY, ts.UIMsg);
			GetDlgItemText(hWnd, IDC_REALTIME_CHECK, uimsg, sizeof(uimsg));
			get_lang_msg("DLG_BROADCAT_REALTIME", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
			SetDlgItemText(hWnd, IDC_REALTIME_CHECK, ts.UIMsg);
			GetDlgItemText(hWnd, IDOK, uimsg, sizeof(uimsg));
			get_lang_msg("DLG_BROADCAST_SUBMIT", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
			SetDlgItemText(hWnd, IDOK, ts.UIMsg);
			GetDlgItemText(hWnd, IDCANCEL, uimsg, sizeof(uimsg));
			get_lang_msg("BTN_CLOSE", ts.UIMsg, sizeof(ts.UIMsg), uimsg, ts.UILanguageFile);
			SetDlgItemText(hWnd, IDCANCEL, ts.UIMsg);

			// �_�C�A���O�̏����T�C�Y��ۑ�
			GetWindowRect(hWnd, &rc_dlg);
			init_width = rc_dlg.right - rc_dlg.left;
			init_height = rc_dlg.bottom - rc_dlg.top;

			// ���݃T�C�Y����K�v�Ȓl���v�Z
			GetClientRect(hWnd,                                 &rc_dlg);
			p.x = rc_dlg.right;
			p.y = rc_dlg.bottom;
			ClientToScreen(hWnd, &p);

			GetWindowRect(GetDlgItem(hWnd, IDOK),               &rc_ok);
			ok2right = p.x - rc_ok.left;

			GetWindowRect(GetDlgItem(hWnd, IDCANCEL),               &rc);
			cancel2right = p.x - rc.left;

			GetWindowRect(GetDlgItem(hWnd, IDC_COMMAND_EDIT), &rc);
			cmdlist2ok = rc_ok.left - rc.right;

			GetWindowRect(GetDlgItem(hWnd, IDC_LIST), &rc);
			list2bottom = p.y - rc.bottom;
			list2right = p.x - rc.right;

			// ���T�C�Y�A�C�R�����E���ɕ\�����������̂ŁA�X�e�[�^�X�o�[��t����B
			InitCommonControls();
			hStatus = CreateStatusWindow(
				WS_CHILD | WS_VISIBLE |
				CCS_BOTTOM | SBARS_SIZEGRIP, NULL, hWnd, 1);

			// ���X�g�X�V�^�C�}�[�̊J�n
			SetTimer(hWnd, list_timer_id, list_timer_tick, NULL);

			return FALSE;

		case WM_COMMAND:
			switch (wp) {
			case IDC_ENTERKEY_CHECK | (BN_CLICKED << 16):
				// �`�F�b�N�̗L���ɂ��A���W�I�{�^���̗L���E���������߂�B
				checked = SendMessage(GetDlgItem(hWnd, IDC_ENTERKEY_CHECK), BM_GETCHECK, 0, 0);
				if (checked & BST_CHECKED) { // ���s�R�[�h����
					EnableWindow(GetDlgItem(hWnd, IDC_RADIO_CRLF), TRUE);
					EnableWindow(GetDlgItem(hWnd, IDC_RADIO_CR), TRUE);
					EnableWindow(GetDlgItem(hWnd, IDC_RADIO_LF), TRUE);

				} else {
					EnableWindow(GetDlgItem(hWnd, IDC_RADIO_CRLF), FALSE);
					EnableWindow(GetDlgItem(hWnd, IDC_RADIO_CR), FALSE);
					EnableWindow(GetDlgItem(hWnd, IDC_RADIO_LF), FALSE);
				}
				return TRUE;

			case IDC_REALTIME_CHECK | (BN_CLICKED << 16):
				checked = SendMessage(GetDlgItem(hWnd, IDC_REALTIME_CHECK), BM_GETCHECK, 0, 0);
				if (checked & BST_CHECKED) { // check����
					// new handler
					hwndBroadcast = GetDlgItem(hWnd, IDC_COMMAND_EDIT);
					hwndBroadcastEdit = GetWindow(hwndBroadcast, GW_CHILD);
					OrigBroadcastEditProc = (WNDPROC)GetWindowLong(hwndBroadcastEdit, GWL_WNDPROC);
					SetWindowLong(hwndBroadcastEdit, GWL_WNDPROC, (LONG)BroadcastEditProc);

					EnableWindow(GetDlgItem(hWnd, IDC_HISTORY_CHECK), FALSE);
					EnableWindow(GetDlgItem(hWnd, IDC_RADIO_CRLF), FALSE);
					EnableWindow(GetDlgItem(hWnd, IDC_RADIO_CR), FALSE);
					EnableWindow(GetDlgItem(hWnd, IDC_RADIO_LF), FALSE);
					EnableWindow(GetDlgItem(hWnd, IDC_ENTERKEY_CHECK), FALSE);
					EnableWindow(GetDlgItem(hWnd, IDC_PARENT_ONLY), FALSE);
					EnableWindow(GetDlgItem(hWnd, IDC_LIST), TRUE);  // true
				} else {
					// restore old handler
					SetWindowLong(hwndBroadcastEdit, GWL_WNDPROC, (LONG)OrigBroadcastEditProc);

					EnableWindow(GetDlgItem(hWnd, IDC_HISTORY_CHECK), TRUE);
					EnableWindow(GetDlgItem(hWnd, IDC_RADIO_CRLF), TRUE);
					EnableWindow(GetDlgItem(hWnd, IDC_RADIO_CR), TRUE);
					EnableWindow(GetDlgItem(hWnd, IDC_RADIO_LF), TRUE);
					EnableWindow(GetDlgItem(hWnd, IDC_ENTERKEY_CHECK), TRUE);
					EnableWindow(GetDlgItem(hWnd, IDC_PARENT_ONLY), TRUE);
					EnableWindow(GetDlgItem(hWnd, IDC_LIST), TRUE);  // true
				}
				return TRUE;
			}

			switch (LOWORD(wp)) {
				case IDOK:
					{
						memset(buf, 0, sizeof(buf));

						// realtime mode�̏ꍇ�AEnter key�̂ݑ���B
						// cf. http://logmett.com/forum/viewtopic.php?f=8&t=1601
						// (2011.3.14 hirata)
						checked = SendMessage(GetDlgItem(hWnd, IDC_REALTIME_CHECK), BM_GETCHECK, 0, 0);
						if (checked & BST_CHECKED) { // check����
							strncpy_s(buf, sizeof(buf), "\n", _TRUNCATE);
							SetDlgItemText(hWnd, IDC_COMMAND_EDIT, "");
							goto skip;
						}

						ret = GetDlgItemText(hWnd, IDC_COMMAND_EDIT, buf, 256 - 1);
						if (ret == 0) { // error
							memset(buf, 0, sizeof(buf));
						}

						// �u���[�h�L���X�g�R�}���h�̗�����ۑ� (2007.3.3 maya)
						history = SendMessage(GetDlgItem(hWnd, IDC_HISTORY_CHECK), BM_GETCHECK, 0, 0);
						if (history) {
							GetDefaultFName(ts.HomeDir, BROADCAST_LOGFILE, historyfile, sizeof(historyfile));
							if (LoadTTSET()) {
								(*AddValueToList)(historyfile, buf, "BroadcastCommands", "Command",
												  ts.MaxBroadcatHistory);
								FreeTTSET();
							}
							ApplyBoradCastCommandHisotry(hWnd, historyfile);
							ts.BroadcastCommandHistory = TRUE;
						}
						else {
							ts.BroadcastCommandHistory = FALSE;
						}
						checked = SendMessage(GetDlgItem(hWnd, IDC_ENTERKEY_CHECK), BM_GETCHECK, 0, 0);
						if (checked & BST_CHECKED) { // ���s�R�[�h����
							if (SendMessage(GetDlgItem(hWnd, IDC_RADIO_CRLF), BM_GETCHECK, 0, 0) & BST_CHECKED) {
								strncat_s(buf, sizeof(buf), "\r\n", _TRUNCATE);

							} else if (SendMessage(GetDlgItem(hWnd, IDC_RADIO_CR), BM_GETCHECK, 0, 0) & BST_CHECKED) {
								strncat_s(buf, sizeof(buf), "\r", _TRUNCATE);

							} else if (SendMessage(GetDlgItem(hWnd, IDC_RADIO_LF), BM_GETCHECK, 0, 0) & BST_CHECKED) {
								strncat_s(buf, sizeof(buf), "\n", _TRUNCATE);

							} else {
								strncat_s(buf, sizeof(buf), "\r", _TRUNCATE);

							}
						}

skip:;
						// 337: 2007/03/20 �`�F�b�N����Ă�����e�E�B���h�E�ɂ̂ݑ��M
						checked = SendMessage(GetDlgItem(hWnd, IDC_PARENT_ONLY), BM_GETCHECK, 0, 0);

						SendAllBroadcastMessage(HVTWin, hWnd, checked, buf, strlen(buf));
					}

					// ���[�h���X�_�C�A���O�͈�x���������ƁA�A�v���P�[�V�������I������܂�
					// �j������Ȃ��̂ŁA�ȉ��́u�E�B���h�E�v���V�[�W���߂��v�͕s�v�Ǝv����B(yutaka)
#if 0
					SetWindowLong(hwndBroadcastEdit, GWL_WNDPROC, (LONG)OrigBroadcastEditProc);
#endif

					//EndDialog(hDlgWnd, IDOK);
					return TRUE;

				case IDCANCEL:
					EndDialog(hWnd, 0);
					//DestroyWindow(hWnd);

					return TRUE;

				case IDC_COMMAND_EDIT:
					if (HIWORD(wp) == CBN_DROPDOWN) {
						GetDefaultFName(ts.HomeDir, BROADCAST_LOGFILE, historyfile, sizeof(historyfile));
						ApplyBoradCastCommandHisotry(hWnd, historyfile);
					}
					return FALSE;

				case IDC_LIST:
					// ��ʓI�ȃA�v���P�[�V�����Ɠ������슴���������邽�߁A
					// �uSHIFT+�N���b�N�v�ɂ��A���I�ȑI�����T�|�[�g����B
					// (2009.9.28 yutaka)
					if (HIWORD(wp) == LBN_SELCHANGE && ShiftKey()) {
						int i, cur, prev;

						cur = ListBox_GetCurSel(BroadcastWindowList);
						prev = -1;
						for (i = cur - 1 ; i >= 0 ; i--) {
							if (ListBox_GetSel(BroadcastWindowList, i)) {
								prev = i;
								break;
							}
						}
						if (prev != -1) {
							// ���łɑI���ς݂̉ӏ�������΁A��������A���I������B
							for (i = prev ; i < cur ; i++) {
								ListBox_SetSel(BroadcastWindowList, TRUE, i);
							}
						}
					}

					return FALSE;

				default:
					return FALSE;
			}
			break;

		case WM_CLOSE:
			//DestroyWindow(hWnd);
			EndDialog(hWnd, 0);
			if (DlgBroadcastFont != NULL) {
				DeleteObject(DlgBroadcastFont);
			}
			return TRUE;

		case WM_SIZE:
			{
				// �Ĕz�u
				int dlg_w, dlg_h;
				RECT rc_dlg;
				RECT rc;
				POINT p;

				// �V�����_�C�A���O�̃T�C�Y�𓾂�
				GetClientRect(hWnd,                                 &rc_dlg);
				dlg_w = rc_dlg.right;
				dlg_h = rc_dlg.bottom;

				// OK button
				GetWindowRect(GetDlgItem(hWnd, IDOK), &rc);
				p.x = rc.left;
				p.y = rc.top;
				ScreenToClient(hWnd, &p);
				SetWindowPos(GetDlgItem(hWnd, IDOK), 0,
							 dlg_w - ok2right, p.y, 0, 0,
							 SWP_NOSIZE | SWP_NOZORDER);

				// Cancel button
				GetWindowRect(GetDlgItem(hWnd, IDCANCEL), &rc);
				p.x = rc.left;
				p.y = rc.top;
				ScreenToClient(hWnd, &p);
				SetWindowPos(GetDlgItem(hWnd, IDCANCEL), 0,
							 dlg_w - cancel2right, p.y, 0, 0,
							 SWP_NOSIZE | SWP_NOZORDER);

				// Command Edit box
				GetWindowRect(GetDlgItem(hWnd, IDC_COMMAND_EDIT), &rc);
				p.x = rc.left;
				p.y = rc.top;
				ScreenToClient(hWnd, &p);
				SetWindowPos(GetDlgItem(hWnd, IDC_COMMAND_EDIT), 0,
							 0, 0, dlg_w - p.x - ok2right - cmdlist2ok, p.y,
							 SWP_NOMOVE | SWP_NOZORDER);

				// List Edit box
				GetWindowRect(GetDlgItem(hWnd, IDC_LIST), &rc);
				p.x = rc.left;
				p.y = rc.top;
				ScreenToClient(hWnd, &p);
				SetWindowPos(GetDlgItem(hWnd, IDC_LIST), 0,
							 0, 0, dlg_w - p.x - list2right , dlg_h - p.y - list2bottom,
							 SWP_NOMOVE | SWP_NOZORDER);

				// status bar
				SendMessage(hStatus , msg , wp , lp);
			}
			return TRUE;

		case WM_GETMINMAXINFO:
			{
				// �_�C�A���O�̏����T�C�Y��菬�����ł��Ȃ��悤�ɂ���
				LPMINMAXINFO lpmmi;
				lpmmi = (LPMINMAXINFO)lp;
				lpmmi->ptMinTrackSize.x = init_width;
				lpmmi->ptMinTrackSize.y = init_height;
			}
			return FALSE;

		case WM_TIMER:
			{
				int n;

				if (wp != list_timer_id)
					break;

				n = GetApplicationInstanceCount();
				if (n != prev_instances) {
					prev_instances = n;
					UpdateBroadcastWindowList(BroadcastWindowList);	
				}
			}
			return TRUE;

		case WM_VKEYTOITEM:
			// ���X�g�{�b�N�X�ŃL�[����(CTRL+A)���ꂽ��A�S�I���B
			if ((HWND)lp == BroadcastWindowList) {
				if (ControlKey() && LOWORD(wp) == 'A') {
					int i, n;

					//OutputDebugPrintf("msg %x wp %x lp %x\n", msg, wp, lp);
					n = GetApplicationInstanceCount();
					for (i = 0 ; i < n ; i++) {
						ListBox_SetSel(BroadcastWindowList, TRUE, i);
					}
				}
			}
			return TRUE;

		default:
			//OutputDebugPrintf("msg %x wp %x lp %x\n", msg, wp, lp);
			return FALSE;
	}
	return TRUE;
}

void CVTWindow::OnControlBroadcastCommand(void)
{
	// TODO: ���[�h���X�_�C�A���O�̃n���h���́A�e�v���Z�X�� DestroyWindow() API�Ŕj������
	// �K�v�����邪�A������OS�C���Ƃ���B
	static HWND hDlgWnd = NULL;
	RECT prc, rc;
	LONG x, y;

	if (hDlgWnd != NULL) {
		goto activate;
	}

	hDlgWnd = CreateDialog(hInst, MAKEINTRESOURCE(IDD_BROADCAST_DIALOG),
	                       HVTWin, (DLGPROC)BroadcastCommandDlgProc);

	if (hDlgWnd == NULL) {
		return;
	}

	// �_�C�A���O���E�B���h�E�̐^��ɔz�u���� (2008.1.25 yutaka)
	GetWindowRect(&prc);
	::GetWindowRect(hDlgWnd, &rc);
	x = prc.left;
	y = prc.top - (rc.bottom - rc.top);
	if (y < 0) {
		y = 0;
	}
	::SetWindowPos(hDlgWnd, NULL, x, y,  0, 0, SWP_NOSIZE | SWP_NOZORDER);

activate:;
	::ShowWindow(hDlgWnd, SW_SHOW);
}

// WM_COPYDATA�̎�M
LONG CVTWindow::OnReceiveIpcMessage(UINT wParam, LONG lParam)
{
	COPYDATASTRUCT *cds;
	char *buf, *msg, *name;
	int buflen, msglen, nlen;
	int sending = 0;

	if (!cv.Ready) {
		return 0;
	}

	if (!ts.AcceptBroadcast) { // 337: 2007/03/20
		return 0;
	}

	// �����M�f�[�^������ꍇ�͐�ɑ��M����
	// �f�[�^�ʂ������ꍇ�͑��M������Ȃ��\��������
	if (TalkStatus == IdTalkCB) {
		CBSend();
	}
	// ���M�\�ȏ�ԂłȂ���΃G���[
	if (TalkStatus != IdTalkKeyb) {
		return 0;
	}

	cds = (COPYDATASTRUCT *)lParam;
	msglen = cds->cbData;
	msg = (char *)cds->lpData;
	if (cds->dwData == IPC_BROADCAST_COMMAND) {
		buf = msg;
		buflen = msglen;
		sending = 1;

	} else if (cds->dwData == IPC_MULTICAST_COMMAND) {
		name = msg;
		nlen = strlen(name) + 1;
		buf = msg + nlen;
		buflen = msglen - nlen; 

		// �}���`�L���X�g�����`�F�b�N����
		if (CompareMulticastName(name) == 0) {  // ����
			sending = 1;
		}
	}

	if (sending) {
		// �[���֕�����𑗂荞��
		// DDE�ʐM�Ɏg���֐��ɕύX�B(2006.2.7 yutaka)
		CBStartPaste(HVTWin, FALSE, BracketedPasteMode(), TermWidthMax/*CBBufSize*/, buf, buflen);
		// ���M�f�[�^������ꍇ�͑��M����
		if (TalkStatus == IdTalkCB) {
			CBSend();
		}
	}

	return 1; // ���M�ł����ꍇ��1��Ԃ�
}



void CVTWindow::OnControlOpenTEK()
{
	OpenTEK();
}

void CVTWindow::OnControlCloseTEK()
{
	if ((HTEKWin==NULL) ||
		! ::IsWindowEnabled(HTEKWin)) {
		MessageBeep(0);
	}
	else {
		::DestroyWindow(HTEKWin);
	}
}

void CVTWindow::OnControlMacro()
{
	RunMacro(NULL,FALSE);
}

void CVTWindow::OnShowMacroWindow()
{
	RunMacro(NULL,FALSE);
}

void CVTWindow::OnWindowWindow()
{
	BOOL Close;

	HelpId = HlpWindowWindow;
	if (! LoadTTDLG()) {
		return;
	}
	(*WindowWindow)(HVTWin,&Close);
	FreeTTDLG();
	if (Close) {
		OnClose();
	}
}

void CVTWindow::OnWindowMinimizeAll()
{
	ShowAllWin(SW_MINIMIZE);
}

void CVTWindow::OnWindowCascade()
{
	ShowAllWinCascade(HVTWin);
}

void CVTWindow::OnWindowStacked()
{
	ShowAllWinStacked(HVTWin);
}

void CVTWindow::OnWindowSidebySide()
{
	ShowAllWinSidebySide(HVTWin);
}

void CVTWindow::OnWindowRestoreAll()
{
	ShowAllWin(SW_RESTORE);
}

void CVTWindow::OnWindowUndo()
{
	UndoAllWin();
}

void CVTWindow::OnHelpIndex()
{
	OpenHelp(HH_DISPLAY_TOPIC, 0, ts.UILanguageFile);
}

void CVTWindow::OnHelpAbout()
{
	if (! LoadTTDLG()) {
		return;
	}
	(*AboutDialog)(HVTWin);
	FreeTTDLG();
}
