/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TERATERM.EXE, Clipboard routines */
#include "teraterm.h"
#include "tttypes.h"
#include <string.h>

#include "ttwinman.h"
#include "ttcommon.h"

#include "clipboar.h"

// for clipboard copy
static HGLOBAL CBCopyHandle = NULL;
static PCHAR CBCopyPtr = NULL;

// for clipboard paste
static HGLOBAL CBMemHandle = NULL;
static PCHAR CBMemPtr = NULL;
static LONG CBMemPtr2 = 0;
static BOOL CBAddCR = FALSE;
static BYTE CBByte;
static BOOL CBRetrySend;
static BOOL CBRetryEcho;
static BOOL CBSendCR;
static BOOL CBDDE;

PCHAR CBOpen(LONG MemSize)
{
  if (MemSize==0) return (NULL);
  if (CBCopyHandle!=NULL) return (NULL);
  CBCopyPtr = NULL;
  CBCopyHandle = GlobalAlloc(GMEM_MOVEABLE, MemSize);
  if (CBCopyHandle == NULL) 
    MessageBeep(0);
  else {
    CBCopyPtr = GlobalLock(CBCopyHandle);
    if (CBCopyPtr == NULL)
    {
      GlobalFree(CBCopyHandle);
      CBCopyHandle = NULL;
      MessageBeep(0);
    }
  }
  return (CBCopyPtr);
}

void CBClose()
{
  BOOL Empty;
  if (CBCopyHandle==NULL) return;

  Empty = FALSE;
  if (CBCopyPtr!=NULL)
    Empty = (CBCopyPtr[0]==0);

  GlobalUnlock(CBCopyHandle);
  CBCopyPtr = NULL;

  if (OpenClipboard(HVTWin))
  {
    EmptyClipboard();
    if (! Empty)
      SetClipboardData(CF_TEXT, CBCopyHandle);
    CloseClipboard();
  }
  CBCopyHandle = NULL;
}

void CBStartPaste(HWND HWin, BOOL AddCR,
		  int BuffSize, PCHAR DataPtr, int DataSize)
//
//  DataPtr and DataSize are used only for DDE
//	  For clipboard, BuffSize should be 0
//	  DataSize should be <= BuffSize
{
  UINT Cf;

  if (! cv.Ready) return;
  if (TalkStatus!=IdTalkKeyb) return;

  CBAddCR = AddCR;

  if (BuffSize==0) // for clipboard
  {
    if (IsClipboardFormatAvailable(CF_TEXT))
      Cf = CF_TEXT;
    else if (IsClipboardFormatAvailable(CF_OEMTEXT))
      Cf = CF_OEMTEXT;
    else return;
  }

  CBMemHandle = NULL;
  CBMemPtr = NULL;
  CBMemPtr2 = 0;
  CBDDE = FALSE;
  if (BuffSize==0) //clipboard
  {
    if (OpenClipboard(HWin))
      CBMemHandle = GetClipboardData(Cf);
    if (CBMemHandle!=NULL) TalkStatus=IdTalkCB;
  }
  else { // dde
    CBMemHandle = GlobalAlloc(GHND,BuffSize);
    if (CBMemHandle != NULL)
    {
      CBDDE = TRUE;
      CBMemPtr = GlobalLock(CBMemHandle);
      if (CBMemPtr != NULL)
      {
	memcpy(CBMemPtr,DataPtr,DataSize);
	GlobalUnlock(CBMemHandle);
	CBMemPtr=NULL;
	TalkStatus=IdTalkCB;
      }
    }
  }
  CBRetrySend = FALSE;
  CBRetryEcho = FALSE;
  CBSendCR = FALSE;
  if (TalkStatus != IdTalkCB) CBEndPaste();
}

void CBSend()
{
  int c;
  BOOL EndFlag;

  if (CBMemHandle==NULL) return;

  if (CBRetrySend)
  {
    CBRetryEcho = (ts.LocalEcho>0);
    c = CommTextOut(&cv,(PCHAR)&CBByte,1);
    CBRetrySend = (c==0);
    if (CBRetrySend) return;
  }

  if (CBRetryEcho)
  {
    c = CommTextEcho(&cv,(PCHAR)&CBByte,1);
    CBRetryEcho = (c==0);
    if (CBRetryEcho) return;
  }

  CBMemPtr = GlobalLock(CBMemHandle);
  if (CBMemPtr==NULL) return;
  
  do {
    if (CBSendCR && (CBMemPtr[CBMemPtr2]==0x0a))
      CBMemPtr2++;

    EndFlag = (CBMemPtr[CBMemPtr2]==0);
    if (! EndFlag)
    {
      CBByte = CBMemPtr[CBMemPtr2];
      CBMemPtr2++;
// Decoding characters which are encoded by MACRO
//   to support NUL character sending
//
//  [encoded character] --> [decoded character]
//         01 01        -->     00
//         01 02        -->     01
      if (CBByte==0x01) /* 0x01 from MACRO */
      {
        CBByte = CBMemPtr[CBMemPtr2];
        CBMemPtr2++;
        CBByte = CBByte - 1; // character just after 0x01
      }
    }
    else if (CBAddCR)
    {
	  EndFlag = FALSE;
      CBAddCR = FALSE;
      CBByte = 0x0d;
    }
    else {
      CBEndPaste();
      return;
    }

    if (! EndFlag)
    {
      c = CommTextOut(&cv,(PCHAR)&CBByte,1);
      CBSendCR = (CBByte==0x0D);
      CBRetrySend = (c==0);
      if ((! CBRetrySend) &&
	  (ts.LocalEcho>0))
      {
	c = CommTextEcho(&cv,(PCHAR)&CBByte,1);
	CBRetryEcho = (c==0);
      }
    }
    else
      c=0;
  }
  while (c>0);

  if (CBMemPtr!=NULL)
  {
    GlobalUnlock(CBMemHandle);
    CBMemPtr=NULL;
  }
}

void CBEndPaste()
{
  TalkStatus = IdTalkKeyb;

  if (CBMemHandle!=NULL)
  {
    if (CBMemPtr!=NULL)
      GlobalUnlock(CBMemHandle);
    if (CBDDE)
      GlobalFree(CBMemHandle);
  }
  if (!CBDDE) CloseClipboard();

  CBDDE = FALSE;
  CBMemHandle = NULL;
  CBMemPtr = NULL;
  CBMemPtr2 = 0;
  CBAddCR = FALSE;
}
