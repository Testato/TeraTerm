/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TERATERM.EXE, DDE routines */
#include "teraterm.h"
#include "tttypes.h"
#include <stdio.h>
#include <string.h>
#include <ddeml.h>
#include "ttwinman.h"
#include "ttftypes.h"
#include "filesys.h"
#include "clipboar.h"
#include "ttsetup.h"
#include "telnet.h"
#include "ttlib.h"

#include "ttdde.h"

#define ServiceName "TERATERM"
#define ItemName "DATA"
#define ItemName2 "PARAM"

char TopicName[21] = "";
HCONV ConvH = 0;
BOOL AdvFlag = FALSE;
BOOL CloseTT = FALSE;

static BOOL DdeCmnd = FALSE;

#ifndef TERATERM32
static FARPROC DdeCallbackPtr = NULL;
#endif

static DWORD Inst = 0;
static HSZ Service = 0;
static HSZ Topic = 0;
static HSZ Item = 0;
static HSZ Item2 = 0;
static HWND HWndDdeCli = NULL;

static StartupFlag = FALSE;

// for sync mode
static BOOL SyncMode = FALSE;
static BOOL SyncRecv;
static LONG SyncFreeSpace;

static char ParamFileName[256];
static WORD ParamBinaryFlag;
static WORD ParamAppendFlag;
static WORD ParamXmodemOpt;

#define CBBufSize 300

void GetClientHWnd(PCHAR HWndStr)
{
  int i;
  BYTE b;
  LONG HCli;

  HCli = 0;
  i = 0;
  b = HWndStr[0];
  while (b > 0)
  {
    if (b <= 0x39)
      HCli = (HCli << 4) + (b-0x30);
    else
      HCli = (HCli << 4) + (b-0x37);
    i++;
    b = HWndStr[i];
  }
  HWndDdeCli = (HWND)HCli;
}

void Byte2HexStr(BYTE b, LPSTR HexStr)
{
  if (b<0xa0)
    HexStr[0] = 0x30 + (b >> 4);
  else
    HexStr[0] = 0x37 + (b >> 4);
  if ((b & 0x0f) < 0x0a)
    HexStr[1] = 0x30 + (b & 0x0f);
  else
    HexStr[1] = 0x37 + (b & 0x0f);
  HexStr[2] = 0;
}

void SetTopic()
{
#ifdef TERATERM32
  WORD w;

  w = HIWORD(HVTWin);
  Byte2HexStr(HIBYTE(w),&(TopicName[0]));
  Byte2HexStr(LOBYTE(w),&(TopicName[2]));
  w = LOWORD(HVTWin);
  Byte2HexStr(HIBYTE(w),&(TopicName[4]));
  Byte2HexStr(LOBYTE(w),&(TopicName[6]));
#else
  Byte2HexStr(HIBYTE(HVTWin),&(TopicName[0]));
  Byte2HexStr(LOBYTE(HVTWin),&(TopicName[2]));
#endif 
}

HDDEDATA WildConnect(HSZ ServiceHsz, HSZ TopicHsz, UINT ClipFmt)
{
  HSZPAIR Pairs[2];
  BOOL Ok;

  Pairs[0].hszSvc  = Service;
  Pairs[0].hszTopic = Topic;
  Pairs[1].hszSvc = NULL;
  Pairs[1].hszTopic = NULL;

  Ok = FALSE;

  if ((ServiceHsz == 0) && (TopicHsz == 0))
    Ok = TRUE;
  else
    if ((TopicHsz == 0) &&
	(DdeCmpStringHandles(Service, ServiceHsz) == 0))
      Ok = TRUE;
    else
      if ((DdeCmpStringHandles(Topic, TopicHsz) == 0) &&
	  (ServiceHsz == 0))
	Ok = TRUE;

  if (Ok)
    return DdeCreateDataHandle(Inst, (LPBYTE)(&Pairs), sizeof(Pairs),
      0, NULL, ClipFmt, 0);
  else
    return 0;
}

  BOOL DDEGet1(LPBYTE b)
  {
    if (cv.DCount <= 0) return FALSE;
    *b = ((LPSTR)cv.LogBuf)[cv.DStart];
    cv.DStart++;
    if (cv.DStart>=InBuffSize)
      cv.DStart = cv.DStart-InBuffSize;
    cv.DCount--;
    return TRUE;
  }

  LONG DDEGetDataLen()
  {
    BYTE b;
    LONG Len;
    int Start, Count;

    Len = cv.DCount;
    Start = cv.DStart;
    Count = cv.DCount;
    while (Count>0)
    {
      b = ((LPSTR)cv.LogBuf)[Start];
      if ((b==0x00) || (b==0x01)) Len++;
      Start++;
      if (Start>=InBuffSize) Start = Start-InBuffSize;
      Count--;
    }

    return Len;
  }

HDDEDATA AcceptRequest(HSZ ItemHSz)
{
  BYTE b;
  BOOL Unlock;
  HDDEDATA DH;
  LPSTR DP;
  int i;
  LONG Len;

  if ((! DDELog) || (ConvH==0)) return 0;

  if (DdeCmpStringHandles(ItemHSz, Item2) == 0) // item "PARAM"
    DH = DdeCreateDataHandle(Inst,ParamFileName,sizeof(ParamFileName),0,
			     Item2,CF_OEMTEXT,0);
  else if (DdeCmpStringHandles(ItemHSz, Item) == 0) // item "DATA"
  {
    if (cv.HLogBuf==0) return 0;

    if (cv.LogBuf==NULL)
    {
      Unlock = TRUE;
      cv.LogBuf = GlobalLock(cv.HLogBuf);
      if (cv.LogBuf == NULL) return 0;
    }
    else Unlock = FALSE;

    Len = DDEGetDataLen();
    if ((SyncMode) &&
	(SyncFreeSpace<Len))
      Len = SyncFreeSpace;

    DH = DdeCreateDataHandle(Inst,NULL,Len+2,0,
			     Item,CF_OEMTEXT,0);
    DP = DdeAccessData(DH,NULL);
    if (DP != NULL)
    {
      i = 0;
      while (i < Len)
      {
	if (DDEGet1(&b)) {
	  if ((b==0x00) || (b==0x01))
	  {
	    DP[i] = 0x01;
	    DP[i+1] = b + 1;
	    i = i + 2;
	  }
	  else {
	    DP[i] = b;
	    i++;
	  }
	}
	else
	  Len = 0;
      }
      DP[i] = 0;
      DdeUnaccessData(DH);
    }

    if (Unlock)
    {
      GlobalUnlock(cv.HLogBuf);
      cv.LogBuf = NULL;
    }
  }
  else
    return 0;

  return DH;
}

HDDEDATA AcceptPoke(HSZ ItemHSz, UINT ClipFmt,
  HDDEDATA Data)
{
  LPSTR DataPtr;
  DWORD DataSize;

  if ((TalkStatus != IdTalkKeyb) ||
      (ConvH==0)) return DDE_FNOTPROCESSED;

  if ((ClipFmt!=CF_TEXT) && (ClipFmt!=CF_OEMTEXT)) return DDE_FNOTPROCESSED;

  if (DdeCmpStringHandles(ItemHSz, Item) != 0) return DDE_FNOTPROCESSED;

  DataPtr = DdeAccessData(Data,&DataSize);
  if (DataPtr==NULL) return DDE_FNOTPROCESSED;
  CBStartPaste(NULL,FALSE,CBBufSize,DataPtr,DataSize);
  DdeUnaccessData(Data);
  if (TalkStatus==IdTalkCB)
    return (HDDEDATA)DDE_FACK;
  else
    return DDE_FNOTPROCESSED;
}

WORD HexStr2Word(PCHAR Str)
{
  int i;
  BYTE b;
  WORD w;

  for (i=0; i<=3; i++)
  {
    b = Str[i];
    if (b <= 0x39)
      w = (w << 4) + (b-0x30);
    else
      w = (w << 4) + (b-0x37);
  }
  return w;
}

#define CmdSetHWnd	' '
#define CmdSetFile	'!'
#define CmdSetBinary	'"'
#define CmdSetAppend	'#'
#define CmdSetXmodemOpt '$'
#define CmdSetSync	'%'

#define CmdBPlusRecv	'&'
#define CmdBPlusSend	'\''
#define CmdChangeDir	'('
#define CmdClearScreen	')'
#define CmdCloseWin	'*'
#define CmdConnect	'+'
#define CmdDisconnect	','
#define CmdEnableKeyb	'-'
#define CmdGetTitle	'.'
#define CmdInit 	'/'
#define CmdKmtFinish	'0'
#define CmdKmtGet	'1'
#define CmdKmtRecv	'2'
#define CmdKmtSend	'3'
#define CmdLoadKeyMap	'4'
#define CmdLogClose	'5'
#define CmdLogOpen	'6'
#define CmdLogPause	'7'
#define CmdLogStart	'8'
#define CmdLogWrite	'9'
#define CmdQVRecv	':'
#define CmdQVSend	';'
#define CmdRestoreSetup	'<'
#define CmdSendBreak	'='
#define CmdSendFile	'>'
#define CmdSendKCode	'?'
#define CmdSetEcho	'@'
#define CmdSetTitle	'A'
#define CmdShowTT	'B'
#define CmdXmodemSend	'C'
#define CmdXmodemRecv	'D'
#define CmdZmodemSend	'E'
#define CmdZmodemRecv	'F'

HDDEDATA AcceptExecute(HSZ TopicHSz, HDDEDATA Data)
{
  char Command[260];
  char Temp[MAXPATHLEN];
  int i;
  WORD w, c;

  if ((ConvH==0) ||
      (DdeCmpStringHandles(TopicHSz, Topic) != 0) ||
      (DdeGetData(Data,Command,sizeof(Command),0) == 0))
    return DDE_FNOTPROCESSED;

  switch (Command[0]) {
    case CmdSetHWnd:
      GetClientHWnd(&Command[1]);
      if (cv.Ready)
	SetDdeComReady(1);
      break;
    case CmdSetFile:
      strcpy(ParamFileName,&(Command[1]));
      break;
    case CmdSetBinary:
      ParamBinaryFlag = Command[1] & 1;
      break;
    case CmdSetAppend:
      ParamAppendFlag = Command[1] & 1;
      break;
    case CmdSetXmodemOpt:
      ParamXmodemOpt = Command[1] & 3;
      if (ParamXmodemOpt==0) ParamXmodemOpt = 1;
      break;
    case CmdSetSync:
      if (sscanf(&(Command[1]),"%u",&SyncFreeSpace)!=1)
	SyncFreeSpace = 0;
      SyncMode = (SyncFreeSpace>0);
      SyncRecv = TRUE;
      break;
    case CmdBPlusRecv:
      if ((FileVar==NULL) && NewFileVar(&FileVar))
      {
	FileVar->NoMsg = TRUE;
	DdeCmnd = TRUE;
	BPStart(IdBPReceive);
      }
      else
	return DDE_FNOTPROCESSED;
      break;
    case CmdBPlusSend:
      if ((FileVar==NULL) && NewFileVar(&FileVar))
      {
	FileVar->DirLen = 0;
	strcpy(FileVar->FullName,ParamFileName);
	FileVar->NumFname = 1;
	FileVar->NoMsg = TRUE;
	DdeCmnd = TRUE;
	BPStart(IdBPSend);
      }
      else
	return DDE_FNOTPROCESSED;
      break;
    case CmdChangeDir:
      strcpy(ts.FileDir,ParamFileName);
      break;
    case CmdClearScreen:
      switch (ParamFileName[0]) {
        case '0':
          PostMessage(HVTWin,WM_USER_ACCELCOMMAND,IdCmdEditCLS,0);
	  break;
        case '1':
          PostMessage(HVTWin,WM_USER_ACCELCOMMAND,IdCmdEditCLB,0);
	  break;
        case '2':
          PostMessage(HTEKWin,WM_USER_ACCELCOMMAND,IdCmdEditCLS,0);
	  break;
      }
      break;
    case CmdCloseWin:
      CloseTT = TRUE;
      break;
    case CmdConnect:
      if (cv.Open) {
	if (cv.Ready)
	  SetDdeComReady(1);
	break;
      }
      strcpy(Temp,"a "); // dummy exe name
      strcat(Temp,ParamFileName);
      if (LoadTTSET())
	(*ParseParam)(Temp, &ts, NULL);
      FreeTTSET();
      cv.NoMsg = 1; /* suppress error messages */
      PostMessage(HVTWin,WM_USER_COMMSTART,0,0);
      break;
    case CmdDisconnect:
      PostMessage(HVTWin,WM_USER_ACCELCOMMAND,IdCmdDisconnect,0);
      break;
    case CmdEnableKeyb:
      KeybEnabled = (ParamBinaryFlag!=0);
      break;
    case CmdGetTitle:
      // title is transfered later by XTYP_REQUEST
      strcpy(ParamFileName,ts.Title);
      break;
    case CmdInit: // initialization signal from TTMACRO
      if (StartupFlag) // in case of startup macro
      { // TTMACRO is waiting for connecting to the host
	if ((ts.PortType==IdSerial) ||
	    (ts.HostName[0]!=0))
	{
	  cv.NoMsg = 1;
	  // start connecting
	  PostMessage(HVTWin,WM_USER_COMMSTART,0,0);
	}
	else  // notify TTMACRO that I can not connect
	  SetDdeComReady(0);
	StartupFlag = FALSE;
      }
      break;
    case CmdKmtFinish:
    case CmdKmtRecv:
      if ((FileVar==NULL) && NewFileVar(&FileVar))
      {
	FileVar->NoMsg = TRUE;
	DdeCmnd = TRUE;
	if (Command[0]==CmdKmtFinish)
	  i = IdKmtFinish;
	else
	  i = IdKmtReceive;
	KermitStart(i);
      }
      else
	return DDE_FNOTPROCESSED;
      break;
    case CmdKmtGet:
    case CmdKmtSend:
      if ((FileVar==NULL) && NewFileVar(&FileVar))
      {
	FileVar->DirLen = 0;
	strcpy(FileVar->FullName,ParamFileName);
	FileVar->NumFname = 1;
	FileVar->NoMsg = TRUE;
	DdeCmnd = TRUE;
	if (Command[0]==CmdKmtGet)
	  i = IdKmtGet;
	else
	  i = IdKmtSend;
	KermitStart(i);
      }
      else
	return DDE_FNOTPROCESSED;
      break;
    case CmdLoadKeyMap:
      strcpy(ts.KeyCnfFN,ParamFileName);
      PostMessage(HVTWin,WM_USER_ACCELCOMMAND,IdCmdLoadKeyMap,0);
      break;
    case CmdLogClose:
      if (LogVar != NULL) FileTransEnd(OpLog);
      break;
    case CmdLogOpen:
      if ((LogVar==NULL) && NewFileVar(&LogVar))
      {
	LogVar->DirLen = 0;
	LogVar->NoMsg = TRUE;
	strcpy(LogVar->FullName,ParamFileName);
	ParseStrftimeFileName(LogVar->FullName);
	ts.TransBin = ParamBinaryFlag;
	ts.Append = ParamAppendFlag;
	LogStart();
      }
      else
	return DDE_FNOTPROCESSED;
      break;
    case CmdLogPause:
      FLogChangeButton(TRUE);
      break;
    case CmdLogStart:
      FLogChangeButton(FALSE);
      break;
    case CmdLogWrite:
      if (LogVar != NULL)
      {
	_lwrite(LogVar->FileHandle,
	ParamFileName,strlen(ParamFileName));
	LogVar->ByteCount =
	  LogVar->ByteCount + strlen(ParamFileName);
	FLogRefreshNum();
      }
      break;
    case CmdQVRecv:
      if ((FileVar==NULL) && NewFileVar(&FileVar))
      {
	FileVar->NoMsg = TRUE;
	DdeCmnd = TRUE;
	QVStart(IdQVReceive);
      }
      else
	return DDE_FNOTPROCESSED;
      break;
    case CmdQVSend:
      if ((FileVar==NULL) && NewFileVar(&FileVar))
      {
	FileVar->DirLen = 0;
	strcpy(FileVar->FullName,ParamFileName);
	FileVar->NumFname = 1;
	FileVar->NoMsg = TRUE;
	DdeCmnd = TRUE;
	QVStart(IdQVSend);
      }
      else
	return DDE_FNOTPROCESSED;
      break;
    case CmdRestoreSetup:
      strcpy(ts.SetupFName,ParamFileName);
      PostMessage(HVTWin,WM_USER_ACCELCOMMAND,IdCmdRestoreSetup,0);
      break;
    case CmdSendBreak:
      PostMessage(HVTWin,WM_USER_ACCELCOMMAND,IdBreak,0);
      break;
    case CmdSendFile:
      if ((SendVar==NULL) && NewFileVar(&SendVar))
      {
	SendVar->DirLen = 0;
	strcpy(SendVar->FullName,ParamFileName);
	ts.TransBin = ParamBinaryFlag;
	SendVar->NoMsg = TRUE;
	DdeCmnd = TRUE;
	FileSendStart();
      }
      else
	return DDE_FNOTPROCESSED;
      break;
    case CmdSendKCode:
      w = HexStr2Word(ParamFileName);
      c = HexStr2Word(&ParamFileName[4]);
      PostMessage(HVTWin,WM_USER_KEYCODE,w,(LPARAM)c);
      break;
    case CmdSetEcho:
      ts.LocalEcho = ParamBinaryFlag;
      if (cv.Ready && cv.TelFlag && (ts.TelEcho>0))
	TelChangeEcho();
      break;
    case CmdSetTitle:
      strcpy(ts.Title,ParamFileName);
      ChangeTitle();
      break;
    case CmdShowTT:
      switch (ParamFileName[0]) {
        case '-': ShowWindow(HVTWin,SW_HIDE); break;
        case '0': ShowWindow(HVTWin,SW_MINIMIZE); break;
        case '1': ShowWindow(HVTWin,SW_RESTORE); break;
        case '2': ShowWindow(HTEKWin,SW_HIDE); break;
        case '3': ShowWindow(HTEKWin,SW_MINIMIZE); break;
        case '4':
	  PostMessage(HVTWin,WM_USER_ACCELCOMMAND,IdCmdCtrlOpenTEK,0);
	  break;
        case '5':
	  PostMessage(HVTWin,WM_USER_ACCELCOMMAND,IdCmdCtrlCloseTEK,0);
	  break;
      }
      break;
    case CmdXmodemRecv:
      if ((FileVar==NULL) && NewFileVar(&FileVar))
      {
	FileVar->DirLen = 0;
	strcpy(FileVar->FullName,ParamFileName);
	ts.XmodemOpt = ParamXmodemOpt;
	ts.XmodemBin = ParamBinaryFlag;
	FileVar->NoMsg = TRUE;
	DdeCmnd = TRUE;
	XMODEMStart(IdXReceive);
      }
      else
	return DDE_FNOTPROCESSED;
      break;
    case CmdXmodemSend:
      if ((FileVar==NULL) && NewFileVar(&FileVar))
      {
	FileVar->DirLen = 0;
	strcpy(FileVar->FullName,ParamFileName);
	ts.XmodemOpt = ParamXmodemOpt;
	FileVar->NoMsg = TRUE;
	DdeCmnd = TRUE;
	XMODEMStart(IdXSend);
      }
      else
	return DDE_FNOTPROCESSED;
      break;
    case CmdZmodemRecv:
      if ((FileVar==NULL) && NewFileVar(&FileVar))
      {
	FileVar->NoMsg = TRUE;
	DdeCmnd = TRUE;
	ZMODEMStart(IdZReceive);
      }
      else
	return DDE_FNOTPROCESSED;
      break;
    case CmdZmodemSend:
      if ((FileVar==NULL) && NewFileVar(&FileVar))
      {
	FileVar->DirLen = 0;
	strcpy(FileVar->FullName,ParamFileName);
	FileVar->NumFname = 1;
	ts.XmodemBin = ParamBinaryFlag;
	FileVar->NoMsg = TRUE;
	DdeCmnd = TRUE;
	ZMODEMStart(IdZSend);
      }
      else
	return DDE_FNOTPROCESSED;
      break;
    default:
      return DDE_FNOTPROCESSED;
  }
  return (HDDEDATA)DDE_FACK;
}

HDDEDATA CALLBACK DdeCallbackProc(UINT CallType, UINT Fmt, HCONV Conv,
  HSZ HSz1, HSZ HSz2, HDDEDATA Data, DWORD Data1, DWORD Data2)
{
  HDDEDATA Result;

  if (Inst==0) return 0;
  Result = 0;

  switch (CallType) {
    case XTYP_WILDCONNECT:
      Result = WildConnect(HSz2, HSz1, Fmt);
      break;
    case XTYP_CONNECT:
      if (Conv == 0)
      {
	if ((DdeCmpStringHandles(Topic, HSz1) == 0) &&
	    (DdeCmpStringHandles(Service, HSz2) == 0))
	{
	  if (cv.Ready)
	    SetDdeComReady(1);
	  Result = (HDDEDATA)TRUE;
	}
      }
      break;
    case XTYP_CONNECT_CONFIRM:
      ConvH = Conv;
      break;
    case XTYP_ADVREQ:
    case XTYP_REQUEST:
      Result = AcceptRequest(HSz2);
      break;
    case XTYP_POKE:
      Result = AcceptPoke(HSz2, Fmt, Data);
    case XTYP_ADVSTART:
      if ((ConvH!=0) &&
	  (DdeCmpStringHandles(Topic, HSz1) == 0) &&
	  (DdeCmpStringHandles(Item, HSz2) == 0) &&
	  ! AdvFlag)
      {
	AdvFlag = TRUE;
	Result = (HDDEDATA)TRUE;
      }
      break;
    case XTYP_ADVSTOP:
      if ((ConvH!=0) &&
	  (DdeCmpStringHandles(Topic, HSz1) == 0) &&
	  (DdeCmpStringHandles(Item, HSz2) == 0) &&
	  AdvFlag)
      {
	AdvFlag = FALSE;
	Result = (HDDEDATA)TRUE;
      }
      break;
    case XTYP_DISCONNECT:
      ConvH = 0;
      PostMessage(HVTWin,WM_USER_DDEEND,0,0);
      break;
    case XTYP_EXECUTE:
      Result = AcceptExecute(HSz1, Data);
  }  /* switch (CallType) */

  return Result;
}

BOOL InitDDE()
{
  BOOL Ok;

  if (ConvH != 0) return FALSE;

  Ok = TRUE;

#ifdef TERATERM32
  if (DdeInitialize(&Inst,(PFNCALLBACK)DdeCallbackProc,0,0) == DMLERR_NO_ERROR)
#else
  DdeCallbackPtr = (PFNCALLBACK)MakeProcInstance((FARPROC)DdeCallbackProc,
    (HINSTANCE)GetWindowWord(HVTWin,GWW_HINSTANCE));
  if ((DdeCallbackPtr!=NULL) &&
      (DdeInitialize(&Inst,DdeCallbackPtr,0,0) == DMLERR_NO_ERROR))
#endif
  {
    Service= DdeCreateStringHandle(Inst, ServiceName, CP_WINANSI);
    Topic  = DdeCreateStringHandle(Inst, TopicName, CP_WINANSI);
    Item   = DdeCreateStringHandle(Inst, ItemName, CP_WINANSI);
    Item2  = DdeCreateStringHandle(Inst, ItemName2, CP_WINANSI);

    Ok = DdeNameService(Inst, Service, 0, DNS_REGISTER) != 0;
  }
  else
    Ok = FALSE;

  SyncMode = FALSE;  
  CloseTT = FALSE;
  StartupFlag = FALSE;
  DDELog = FALSE;

  if (Ok)
  {
    Ok = CreateLogBuf();
    if (Ok) DDELog = TRUE;
  }

  if (! Ok) EndDDE();
  return Ok;
}

void SendDDEReady()
{
  GetClientHWnd(TopicName);
  PostMessage(HWndDdeCli,WM_USER_DDEREADY,0,0);
}

void EndDDE()
{
  DWORD Temp;

  if (ConvH != 0)
    DdeDisconnect(ConvH);
  ConvH = 0;  
  SyncMode = FALSE;
  StartupFlag = FALSE;

  Temp = Inst;
  if (Inst != 0)
  {
    Inst = 0;
    DdeNameService(Temp, Service, 0, DNS_UNREGISTER);
    if (Service != 0)
      DdeFreeStringHandle(Temp, Service);
    Service = 0;
    if (Topic != 0)
      DdeFreeStringHandle(Temp, Topic);
    Topic = 0;
    if (Item != 0)
      DdeFreeStringHandle(Temp, Item);
    Item = 0;
    if (Item2 != 0)
      DdeFreeStringHandle(Temp, Item2);
    Item2 = 0;

    DdeUninitialize(Temp);
#ifndef TERATERM32
    if (DdeCallbackPtr != NULL)
      FreeProcInstance((FARPROC)DdeCallbackPtr);
    DdeCallbackPtr = NULL;
#endif
  }
  TopicName[0] = 0;
  
  if (HWndDdeCli!=NULL)
    PostMessage(HWndDdeCli,WM_USER_DDECMNDEND,0,0);
  HWndDdeCli = NULL;
  AdvFlag = FALSE;

  DDELog = FALSE;
  FreeLogBuf();
  cv.NoMsg = 0;
}

void DDEAdv()
{
  if ((ConvH==0) ||
      (! AdvFlag) ||
      (cv.DCount==0))
    return;

  if ((! SyncMode) ||
      SyncMode && SyncRecv)
  {
    if (SyncFreeSpace<10)
      SyncFreeSpace=0;
    else
      SyncFreeSpace=SyncFreeSpace-10;
    DdePostAdvise(Inst,Topic,Item);
    SyncRecv = FALSE;
  }
}

void EndDdeCmnd(int Result)
{
  if ((ConvH==0) || (HWndDdeCli==NULL) || (! DdeCmnd)) return;
  PostMessage(HWndDdeCli,WM_USER_DDECMNDEND,(WPARAM)Result,0);
  DdeCmnd = FALSE;
}

void SetDdeComReady(WORD Ready)
{
  if (HWndDdeCli==NULL) return;
  PostMessage(HWndDdeCli,WM_USER_DDECOMREADY,Ready,0);
}

void RunMacro(PCHAR FName, BOOL Startup)
//  FName: macro filename
//  Startup: TRUE in case of startup macro execution.
//		  In this case, the connection to the host will
//		  made after the link to TT(P)MACRO is established.
{
	int i;
	char Cmnd[MAXPATHLEN+40];
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	DWORD pri = NORMAL_PRIORITY_CLASS;

	SetTopic();
	if (! InitDDE()) return;
#ifdef TERATERM32
	strcpy(Cmnd,"TTPMACRO /D=");
#else
	strcpy(Cmnd,"TTMACRO /D=");
#endif
	strcat(Cmnd,TopicName);
	if (FName!=NULL)
	{
		strcat(Cmnd," ");
		i = strlen(Cmnd);
		strcat(Cmnd,FName);
#ifdef TERATERM32
		QuoteFName(&Cmnd[i]);
#endif
	}

	StartupFlag = Startup;
	if (Startup)
		strcat(Cmnd," /S"); // "startup" flag

#if 0
	if (WinExec(Cmnd,SW_MINIMIZE) < 32)
		EndDDE();
#else

	// ログ採取中も下げないことにする。(2005.8.14 yutaka)
#if 0
	// TeraTerm本体でログ採取中にマクロを実行すると、マクロの動作が停止することが
	// あるため、プロセスの優先度を1つ下げて実行させる。(2004/9/5 yutaka)
	// ログ採取中のみに下げる。(2004/11/28 yutaka)
	if (FileLog || BinLog) {
		pri = BELOW_NORMAL_PRIORITY_CLASS;
	}
	// 暫定処置として、常に下げることにする。(2005/5/15 yutaka)
	// マクロによるtelnet自動ログインが失敗することがあるので、下げないことにする。(2005/5/23 yutaka)
	pri = BELOW_NORMAL_PRIORITY_CLASS;
#endif

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	GetStartupInfo(&si);
	si.wShowWindow = SW_MINIMIZE;

	if (CreateProcess(
		NULL,
		Cmnd,
		NULL, 
		NULL,
		FALSE,
		pri,
		NULL, NULL,
		&si, &pi) == 0) {
			EndDDE();
	}
#endif
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2005/08/14 12:41:04  yutakakn
 * ログ採取中には ttpmacro.exe のプロセス優先度を下げないようにした。
 *
 * Revision 1.4  2005/05/23 14:48:57  yutakakn
 * ttpmacro.exeのプロセス優先度を下げないようにした。
 *
 * Revision 1.3  2005/05/15 09:23:19  yutakakn
 * 暫定処置として ttermpro.exe のプロセス優先度は常に下げることにした。
 *
 * Revision 1.2  2004/11/28 13:57:30  yutakakn
 * TeraTerm本体からのマクロ実行において、ログ採取時のみにプロセス優先度を下げるようにした。
 *
 */
