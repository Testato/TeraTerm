/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TERATERM.EXE, keyboard routines */

#include "teraterm.h"
#include "tttypes.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ttlib.h"
#include "ttsetup.h"
#include "ttcommon.h"
#include "ttwinman.h"
#include "ttdde.h"

#include "keyboard.h"

BOOL AutoRepeatMode;
BOOL AppliKeyMode, AppliCursorMode;
BOOL DebugFlag = FALSE;

static char FuncKeyStr[IdUDK20-IdUDK6+1][FuncKeyStrMax];
static int FuncKeyLen[IdUDK20-IdUDK6+1];

  /*keyboard status*/
static int PreviousKey;

  /*key code map*/
static PKeyMap KeyMap = NULL;

// Ctrl-\ support for NEC-PC98
static short VKBackslash;

#ifndef VK_PROCESSKEY
#define VK_PROCESSKEY 0xE5
#endif

void SetKeyMap()
{
  char TempDir[MAXPATHLEN];
  char TempName[MAXPATHLEN];

  if ( strlen(ts.KeyCnfFN)==0 ) return;
  ExtractFileName(ts.KeyCnfFN,TempName);
  ExtractDirName(ts.KeyCnfFN,TempDir);
  if (TempDir[0]==0)
    strcpy(TempDir,ts.HomeDir);
  FitFileName(TempName,".CNF");

  strcpy(ts.KeyCnfFN,TempDir);
  AppendSlash(ts.KeyCnfFN);
  strcat(ts.KeyCnfFN,TempName);

  if ( KeyMap==NULL )
    KeyMap = (PKeyMap)malloc(sizeof(TKeyMap));
  if ( KeyMap!=NULL )
  {
    if ( LoadTTSET() )
      (*ReadKeyboardCnf)(ts.KeyCnfFN, KeyMap, TRUE);
    FreeTTSET();
  }
  if ((stricmp(TempDir,ts.HomeDir)==0) &&
      (stricmp(TempName,"KEYBOARD.CNF")==0))
  {
     ChangeDefaultSet(NULL,KeyMap);
     free(KeyMap);
     KeyMap = NULL;
  }
}

void ClearUserKey()
{
  int i;

  i = 0;
  while (i <= IdUDK20-IdUDK6)
  {
    FuncKeyLen[i] = 0;
    i++;
  }
}

void DefineUserKey(int NewKeyId, PCHAR NewKeyStr, int NewKeyLen)
{
  if ((NewKeyLen==0) || (NewKeyLen>FuncKeyStrMax)) return;

  if ((NewKeyId>=17) && (NewKeyId<=21))
    NewKeyId = NewKeyId-17;
  else if ((NewKeyId>=23) && (NewKeyId<=26))
    NewKeyId = NewKeyId-18;
  else if ((NewKeyId>=28) && (NewKeyId<=29))
    NewKeyId = NewKeyId-19;
  else if ((NewKeyId>=31) && (NewKeyId<=34))
    NewKeyId = NewKeyId-20;
  else
    return;

  memcpy(&FuncKeyStr[NewKeyId][0], NewKeyStr, NewKeyLen);
  FuncKeyLen[NewKeyId] = NewKeyLen;
}

BOOL KeyDown(HWND HWin, WORD VKey, WORD Count, WORD Scan)
{
  WORD Key;
  MSG M;
  BYTE KeyState[256];
  BOOL Single, Control;
  int i;
  int CodeCount;
  int CodeLength;
  char Code[MAXPATHLEN];
  WORD CodeType;
  WORD wId;

  if (VKey==VK_PROCESSKEY) return TRUE;

  if ((VKey==VK_SHIFT) ||
      (VKey==VK_CONTROL) ||
      (VKey==VK_MENU)) return TRUE;

  /* debug mode */
  if ((ts.Debug>0) && (VKey == VK_ESCAPE) &&
      ShiftKey())
  {
    MessageBeep(0);
    DebugFlag = ! DebugFlag;
    CodeCount = 0;
    PeekMessage((LPMSG)&M,HWin,WM_CHAR,WM_CHAR,PM_REMOVE);
    return TRUE;
  }

  if (! AutoRepeatMode && (PreviousKey==VKey))
  {
    PeekMessage((LPMSG)&M,HWin,WM_CHAR,WM_CHAR,PM_REMOVE);
    return TRUE;
  }

  PreviousKey = VKey;

  if (Scan==0)
    Scan = MapVirtualKey(VKey,0);

  Single = TRUE;
  Control = TRUE;
  if (ShiftKey())
  {
    Scan = Scan | 0x200;
    Single = FALSE;
    Control = FALSE;
  }

  if (ControlKey())
  {
    Scan = Scan | 0x400;
    Single = FALSE;
  }
  else
    Control = FALSE;

  if (AltKey())
  {
    Scan = Scan | 0x800;
    if (ts.MetaKey==0)
    {
      Single = FALSE;
      Control = FALSE;
    }
  }

  CodeCount = Count;
  CodeLength = 0;
  CodeType = IdBinary;

  if ((VKey!=VK_DELETE) || (ts.DelKey==0))
    /* Windows keycode -> Tera Term keycode */
    Key = GetKeyCode(KeyMap,Scan);
  else
    Key = 0;

  if (Key==0)
  {
    switch (VKey) {
      case VK_BACK:
	if (Control)
	{
	  CodeLength = 1;
	  if (ts.BSKey==IdDEL)
	    Code[0] = 0x08;
	  else
	    Code[0] = 0x7F;
	}
        else if (Single)
	{
	  CodeLength = 1;
	  if (ts.BSKey==IdDEL)
	    Code[0] = 0x7F;
	  else
	    Code[0] = 0x08;
	}
	break;
      case VK_RETURN: /* CR Key */
	if (Single)
	{
	  CodeType = IdText; // do new-line conversion
	  CodeLength = 1;
	  Code[0] = 0x0D;
	}
	break;
      case VK_SPACE:
	if (Control)
	{ // Ctrl-Space -> NUL
	  CodeLength = 1;
	  Code[0] = 0;
	}
	break;
      case VK_DELETE:
	if ((Single) && (ts.DelKey>0))
	{ // DEL character
	  CodeLength = 1;
	  Code[0] = 0x7f;
	}
	break;
      default:
	if ((VKey==VKBackslash) && Control)
	{ // Ctrl-\ support for NEC-PC98
	  CodeLength = 1;
	  Code[0] = 0x1c;
	}
    }
    if ((ts.MetaKey>0) && (CodeLength==1) &&
	AltKey())
    {
      Code[1] = Code[0];
      Code[0] = 0x1b;
      CodeLength = 2;
      PeekMessage((LPMSG)&M,HWin,WM_SYSCHAR,WM_SYSCHAR,PM_REMOVE);
    }
  }
  else if ((IdUDK6<=Key) && (Key<=IdUDK20) &&
	   (FuncKeyLen[Key-IdUDK6]>0))
  {
    memcpy(Code,&FuncKeyStr[Key-IdUDK6][0],FuncKeyLen[Key-IdUDK6]);
    CodeLength = FuncKeyLen[Key-IdUDK6];
    CodeType = IdBinary;
  }
  else
    GetKeyStr(HWin,KeyMap,Key,AppliKeyMode,AppliCursorMode,
	      Code,&CodeLength,&CodeType);

  if (CodeLength==0) return FALSE;

  if (VKey==VK_NUMLOCK)
  {
    /* keep NumLock LED status */
    GetKeyboardState((PBYTE)KeyState);
    KeyState[VK_NUMLOCK] = KeyState[VK_NUMLOCK] ^ 1;
    SetKeyboardState((PBYTE)KeyState);
  }

  PeekMessage((LPMSG)&M,HWin,WM_CHAR,WM_CHAR,PM_REMOVE);

  if (KeybEnabled)
  {
    switch (CodeType) {
      case IdBinary:
	if (TalkStatus==IdTalkKeyb)
	{
	  for (i = 1 ; i <= CodeCount ; i++)
	  {
	    CommBinaryOut(&cv,Code,CodeLength);
	    if (ts.LocalEcho>0)
	      CommBinaryEcho(&cv,Code,CodeLength);
	  }
	}
	break;
      case IdText:
	if (TalkStatus==IdTalkKeyb)
	{
	  for (i = 1 ; i <= CodeCount ; i++)
	  {
	    if (ts.LocalEcho>0)
	      CommTextEcho(&cv,Code,CodeLength);
	    CommTextOut(&cv,Code,CodeLength);
	  }
	}
	break;
      case IdMacro:
	Code[CodeLength] = 0;
	RunMacro(Code,FALSE);
	break;
      case IdCommand:
	Code[CodeLength] = 0;
	if (sscanf(Code, "%d", &wId) == 1)
#ifdef TERATERM32
	  PostMessage(HWin,WM_COMMAND,MAKELONG(wId,0),0);
#else
	  PostMessage(HWin,WM_COMMAND,wId,0);
#endif
	break;
    }
  }
  return TRUE;
}

void KeyCodeSend(WORD KCode, WORD Count)
{
  WORD Key;
  int i, CodeLength;
  char Code[MAXPATHLEN];
  WORD CodeType;
  WORD Scan, VKey, State;
  BOOL Single, Control;
  DWORD dw;
  BOOL Ok;
  HWND HWin;

  if (ActiveWin==IdTEK)
    HWin = HTEKWin;
  else
    HWin = HVTWin;

  CodeLength = 0;
  CodeType = IdBinary;
  Key = GetKeyCode(KeyMap,KCode);
  if (Key==0)
  {
    Scan = KCode & 0x1FF;
    VKey = MapVirtualKey(Scan,1);
    State = 0;
    Single = TRUE;
    Control = TRUE;
    if ((KCode & 512) != 0)
    { /* shift */
      State = State | 2; /* bit 1 */
      Single = FALSE;
      Control = FALSE;
    }
    
    if ((KCode & 1024) != 0)
    { /* control */
      State = State | 4; /* bit 2 */
      Single = FALSE;
    }
    else
      Control = FALSE;

    if ((KCode & 2048) != 0)
    { /* alt */
      State = State | 16; /* bit 4 */
      Single = FALSE;
      Control = FALSE;
    }

    switch (VKey) {
      case VK_BACK:
	if (Control)
	{
	  CodeLength = 1;
	  if (ts.BSKey==IdDEL)
	    Code[0] = 0x08;
	  else
	    Code[0] = 0x7F;
	}
        else if (Single)
	{
	  CodeLength = 1;
	  if (ts.BSKey==IdDEL)
	    Code[0] = 0x7F;
	  else
	    Code[0] = 0x08;
	}
	break;
      case VK_RETURN: /* CR Key */
	if (Single)
	{
	  CodeType = IdText; // do new-line conversion
	  CodeLength = 1;
	  Code[0] = 0x0D;
	}
	break;
      case VK_SPACE:
	if (Control)
	{ // Ctrl-Space -> NUL
	  CodeLength = 1;
	  Code[0] = 0;
	}
	break;
      case VK_DELETE:
	if ((Single) && (ts.DelKey>0))
	{ // DEL character
	  CodeLength = 1;
	  Code[0] = 0x7f;
	}
	break;
      default:
	if ((VKey==VKBackslash) && Control)
	{ // Ctrl-\ support for NEC-PC98
	  CodeLength = 1;
	  Code[0] = 0x1c;
	}
    }

    if (CodeLength==0)
    {
      i = -1;
      do {
	i++;
	dw = OemKeyScan((WORD)i);
	Ok = (LOWORD(dw)==Scan) &&
	     (HIWORD(dw)==State);
      } while ((i<255) && ! Ok);
      if (Ok)
      {
	CodeType = IdText;
	CodeLength = 1;
	Code[0] = (char)i;
      }
    }
  }
  else if ((IdUDK6<=Key) && (Key<=IdUDK20) &&
	   (FuncKeyLen[Key-IdUDK6]>0))
  {
    memcpy(Code,&FuncKeyStr[Key-IdUDK6][0],FuncKeyLen[Key-IdUDK6]);
    CodeLength = FuncKeyLen[Key-IdUDK6];
    CodeType = IdBinary;
  }
  else
    GetKeyStr(HWin,KeyMap,Key,AppliKeyMode,AppliCursorMode,
	      Code,&CodeLength,&CodeType);

  if (CodeLength==0) return;
  if (TalkStatus==IdTalkKeyb)
  {
    switch (CodeType) {
      case IdBinary:
	for (i = 1 ; i <= Count ; i++)
	{
	  CommBinaryOut(&cv,Code,CodeLength);
	  if (ts.LocalEcho>0)
	    CommBinaryEcho(&cv,Code,CodeLength);
	}
	break;
      case IdText:
	for (i = 1 ; i <= Count ; i++)
	{
	  if (ts.LocalEcho>0)
	    CommTextEcho(&cv,Code,CodeLength);
	  CommTextOut(&cv,Code,CodeLength);
	}
	break;
      case IdMacro:
	Code[CodeLength] = 0;
	RunMacro(Code,FALSE);
	break;
    }
  }
}

void KeyUp(WORD VKey)
{
  if (PreviousKey == VKey) PreviousKey = 0;
}

BOOL ShiftKey()
{
  return ((GetAsyncKeyState(VK_SHIFT) & 0xFFFFFF80) != 0);
}

BOOL ControlKey()
{
  return ((GetAsyncKeyState(VK_CONTROL) & 0xFFFFFF80) != 0);
}

BOOL AltKey()
{
  return ((GetAsyncKeyState(VK_MENU) & 0xFFFFFF80) != 0);
}

void InitKeyboard()
{
  KeyMap = NULL;
  ClearUserKey();
  PreviousKey = 0;
  VKBackslash = LOBYTE(VkKeyScan('\\'));
}

void EndKeyboard()
{
  if (KeyMap != NULL)
    free(KeyMap);
}
