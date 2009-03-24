// Tera Term
// Copyright(C) 1994-1998 T. Teranishi
// All rights reserved.

// TTMACRO.EXE, DDE routines

#include "teraterm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ddeml.h>
#include "ttmdlg.h"
#include "ttmparse.h"
#include "ttmmsg.h"

#include "ttmdde.h"

#include "ttl.h"

// Oniguruma: Regular expression library
#define ONIG_EXTERN extern
#include "oniguruma.h"
#undef ONIG_EXTERN

BOOL Linked = FALSE;
WORD ComReady = 0;
int OutLen;

  // for "WaitRecv" command
TStrVal Wait2Str;
BOOL Wait2Found;

#define ServiceName "TERATERM"
#define ItemName "DATA"
#define ItemName2 "PARAM"

#define OutBufSize 512
#define RingBufSize 4096
#define RCountLimit 3072

static HWND HMainWin = NULL;
#ifndef TERATERM32
static PFNCALLBACK DdeCallbackPtr = NULL;
#endif
static DWORD Inst = 0;
static HCONV ConvH = 0;

static HSZ Service = 0;
static HSZ Topic = 0;
static HSZ Item = 0;
static HSZ Item2 = 0;

// sync mode
static BOOL SyncMode = FALSE;
static BOOL SyncSent;

static BOOL QuoteFlag;
static char OutBuf[OutBufSize];

static char RingBuf[RingBufSize];
static int RBufStart = 0;
static int RBufPtr = 0;
static int RBufCount = 0;

  // for 'Wait' command
static PCHAR PWaitStr[10];
static int WaitStrLen[10];
static int WaitCount[10];
  // for "WaitRecv" command
static TStrVal Wait2SubStr;
static int Wait2Count, Wait2Len;
static int Wait2SubLen, Wait2SubPos;
 //  waitln & recvln
static TStrVal RecvLnBuff;
static int RecvLnPtr = 0;
static BYTE RecvLnLast = 0;

// regex action flag
enum regex_type RegexActionType;


// ring buffer
void Put1Byte(BYTE b)
{
  RingBuf[RBufPtr] = b;
  RBufPtr++;
  if (RBufPtr>=RingBufSize) RBufPtr = RBufPtr-RingBufSize;
  if (RBufCount>=RingBufSize)
  {
    RBufCount = RingBufSize;
    RBufStart = RBufPtr;
  }
  else
    RBufCount++;
}

BOOL Read1Byte(LPBYTE b)
{
  if (RBufCount<=0)
    return FALSE;

  *b = RingBuf[RBufStart];
  RBufStart++;
  if (RBufStart>=RingBufSize)
    RBufStart = RBufStart-RingBufSize;
  RBufCount--;
  if (QuoteFlag)
  {
    *b= *b-1;
    QuoteFlag = FALSE;
  }
  else 
    QuoteFlag = (*b==0x01);

  return (! QuoteFlag);
}

HDDEDATA AcceptData(HDDEDATA ItemHSz, HDDEDATA Data)
{
  HDDEDATA DH;
  PCHAR DPtr;
  DWORD DSize;
  int i;

  if (DdeCmpStringHandles((HSZ)ItemHSz, Item) != 0) return 0;

  DH = Data;
  if (DH==0)
    DH = DdeClientTransaction(NULL,0,ConvH,Item,CF_OEMTEXT,XTYP_REQUEST,1000,NULL);

  if (DH==0) return (HDDEDATA)DDE_FACK;
  DPtr = DdeAccessData(DH,&DSize);
  if (DPtr==NULL) return (HDDEDATA)DDE_FACK;
  DSize = strlen(DPtr);
  for (i=0; i<(signed)DSize; i++)
    Put1Byte((BYTE)DPtr[i]);
  DdeUnaccessData(DH);
  DdeFreeDataHandle(DH);
  SyncSent = FALSE;

  return (HDDEDATA)DDE_FACK;
}

// CallBack Procedure for DDEML
HDDEDATA CALLBACK DdeCallbackProc(UINT CallType, UINT Fmt, HCONV Conv,
  HSZ hsz1, HSZ hsz2, HDDEDATA Data, DWORD Data1, DWORD Data2)
{
  if (Inst==0) return 0;
  switch (CallType) {
    case XTYP_REGISTER: break;
    case XTYP_UNREGISTER: break;
    case XTYP_XACT_COMPLETE: break;
    case XTYP_ADVDATA:
      return (AcceptData((HDDEDATA)hsz2,Data));
    case XTYP_DISCONNECT:
      ConvH = 0;
      Linked = FALSE;
      PostMessage(HMainWin,WM_USER_DDEEND,0,0);
      break;
  }
  return 0;
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

void Word2HexStr(WORD w, LPSTR HexStr)
{
  Byte2HexStr(HIBYTE(w),HexStr);
  Byte2HexStr(LOBYTE(w),&HexStr[2]);
}

BOOL InitDDE(HWND HWin)
{
  int i;
  WORD w;
  char Cmd[10];

  HMainWin = HWin;
  Linked = FALSE;
  SyncMode = FALSE;
  OutLen = 0;
  RBufStart = 0;
  RBufPtr = 0;
  RBufCount = 0;
  QuoteFlag = FALSE;
  for (i = 0 ; i<=9 ; i++)
  {
    PWaitStr[i] = NULL;
    WaitStrLen[i] = 0;
    WaitCount[i] = 0;
  }

#ifdef TERATERM32
  if (DdeInitialize(&Inst, (PFNCALLBACK)DdeCallbackProc,
	APPCMD_CLIENTONLY |
	CBF_SKIP_REGISTRATIONS |
	CBF_SKIP_UNREGISTRATIONS,0)
      != DMLERR_NO_ERROR) return FALSE;
#else
  DdeCallbackPtr = (PFNCALLBACK)MakeProcInstance((FARPROC)DdeCallbackProc,
    (HINSTANCE)GetWindowWord(HWin,GWW_HINSTANCE));
  if (DdeCallbackPtr==NULL) return FALSE;
  if (DdeInitialize(&Inst, DdeCallbackPtr,
	APPCMD_CLIENTONLY |
	CBF_SKIP_REGISTRATIONS |
	CBF_SKIP_UNREGISTRATIONS,0)
      != DMLERR_NO_ERROR) 
  {
    FreeProcInstance((FARPROC)DdeCallbackPtr);
    DdeCallbackPtr = NULL;
    return FALSE;
  }
#endif

  Service= DdeCreateStringHandle(Inst, ServiceName, CP_WINANSI);
  Topic  = DdeCreateStringHandle(Inst, TopicName, CP_WINANSI);
  Item	 = DdeCreateStringHandle(Inst, ItemName, CP_WINANSI);
  Item2  = DdeCreateStringHandle(Inst, ItemName2, CP_WINANSI);
  if ((Service==0) || (Topic==0) ||
      (Item==0) || (Item2==0)) return FALSE;

  ConvH = DdeConnect(Inst, Service, Topic, NULL);
  if (ConvH == 0) return FALSE;
  Linked = TRUE;

  Cmd[0] = CmdSetHWnd;
#ifdef TERATERM32
  w = HIWORD(HWin);
  Word2HexStr(w,&(Cmd[1]));
  w = LOWORD(HWin);
  Word2HexStr(w,&(Cmd[5]));
#else
  w = HWin;
  Word2HexStr(w,&(Cmd[1]));
#endif

  DdeClientTransaction(Cmd,strlen(Cmd)+1,ConvH,0,
    CF_OEMTEXT,XTYP_EXECUTE,1000,NULL);

  DdeClientTransaction(NULL,0,ConvH,Item,
    CF_OEMTEXT,XTYP_ADVSTART,1000,NULL);

  return TRUE;
}

void EndDDE()
{
  DWORD Temp;

  Linked = FALSE;
  SyncMode = FALSE;

  ConvH = 0;
  TopicName[0] = 0;

  Temp = Inst;
  if (Inst != 0)
  {
    Inst = 0;
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

    DdeUninitialize(Temp);  // Ignore the return value
  }
#ifndef TERATERM32
  if (DdeCallbackPtr != NULL)
    FreeProcInstance((FARPROC)DdeCallbackPtr);
  DdeCallbackPtr = NULL;
#endif
}

void DDEOut1Byte(BYTE B)
{
  if (((B==0x00) || (B==0x01)) &&
      (OutLen < OutBufSize-2))
  {
// Character encoding
//   to support NUL character sending
//
//  [char to be sent] --> [encoded character]
//	   00	      -->     01 01
//	   01	      -->     01 02
//
    OutBuf[OutLen] = 0x01;
    OutBuf[OutLen+1] = B+1;
    OutLen = OutLen + 2;
  }
  else if (OutLen < OutBufSize-1)
  {
    OutBuf[OutLen] = B;
    OutLen++;
  }
}

void DDEOut(PCHAR B)
{
  int i;

  i = strlen(B);
  if (OutLen+i > OutBufSize-1)
    i = OutBufSize-1 - OutLen;
  memcpy(&(OutBuf[OutLen]),B,i);
  OutLen = OutLen + i;
}

void DDESend()
{
  if ((! Linked) || (OutLen==0)) return;
  OutBuf[OutLen] = 0;
  if (DdeClientTransaction(OutBuf,OutLen+1,ConvH,Item,CF_OEMTEXT,XTYP_POKE,1000,NULL)
      != 0)
    OutLen = 0;
}

void ClearRecvLnBuff()
{
  RecvLnPtr = 0;
  RecvLnLast = 0;
}

void PutRecvLnBuff(BYTE b)
{
  if (RecvLnLast==0x0a)
    ClearRecvLnBuff();
  if (RecvLnPtr < sizeof(RecvLnBuff)-1)
    RecvLnBuff[RecvLnPtr++] = b;
  RecvLnLast = b;
}

PCHAR GetRecvLnBuff()
{
  if ((RecvLnPtr>0) &&
      RecvLnBuff[RecvLnPtr-1]==0x0a)
  {
    RecvLnPtr--;
    if ((RecvLnPtr>0) &&
	RecvLnBuff[RecvLnPtr-1]==0x0d)
      RecvLnPtr--;
  }
  RecvLnBuff[RecvLnPtr] = 0;
  ClearRecvLnBuff();
  return RecvLnBuff;
}

void FlushRecv()
{
  ClearRecvLnBuff();
  RBufStart = 0;
  RBufPtr = 0;
  RBufCount = 0;
}

void ClearWait()
{
  int i;

  for (i = 0 ; i <= 9 ; i++)
  {
    if (PWaitStr[i]!=NULL)
      free(PWaitStr[i]);
    PWaitStr[i] = NULL;
    WaitStrLen[i] = 0;
    WaitCount[i] = 0;
  }

  RegexActionType = REGEX_NONE; // regex disabled
}

void SetWait(int Index, PCHAR Str)
{
  if (PWaitStr[Index-1]!=NULL)
    free(PWaitStr[Index-1]);
  PWaitStr[Index-1] = malloc(strlen(Str)+1);
  strcpy(PWaitStr[Index-1],Str);
  WaitStrLen[Index-1] = strlen(Str);
  WaitCount[Index-1] = 0;
}

int CmpWait(int Index, PCHAR Str)
{
  if (PWaitStr[Index-1]!=NULL)
    return strcmp(PWaitStr[Index-1],Str);
  else
    return 1;
}

void SetWait2(PCHAR Str, int Len, int Pos)
{
  strcpy(Wait2SubStr,Str);
  Wait2SubLen = strlen(Wait2SubStr);

  if (Len<1)
    Wait2Len = 0;
  else if (Len>MaxStrLen-1)
    Wait2Len = MaxStrLen-1;
  else
    Wait2Len = Len;

  if (Wait2Len<Wait2SubLen)
    Wait2Len = Wait2SubLen;

  if (Pos<1)
    Wait2SubPos = 1;
  else if (Pos>Wait2Len-Wait2SubLen+1)
    Wait2SubPos = Wait2Len-Wait2SubLen+1;
  else
    Wait2SubPos = Pos;

  Wait2Count = 0;
  Wait2Str[0] = 0;
  Wait2Found = (Wait2SubStr[0]==0);
}


// 正規表現によるパターンマッチを行う（Oniguruma使用）
int FindRegexStringOne(char *regex, int regex_len, char *target, int target_len)
{
	int r;
	unsigned char *start, *range, *end;
	regex_t* reg;
	OnigErrorInfo einfo;
	OnigRegion *region;
	UChar* pattern = (UChar* )regex;
	UChar* str     = (UChar* )target;
	int matched = 0;
	char ch;
	int mstart, mend;


	r = onig_new(&reg, pattern, pattern + regex_len,
		ONIG_OPTION_DEFAULT, ONIG_ENCODING_ASCII, ONIG_SYNTAX_DEFAULT, &einfo);
	if (r != ONIG_NORMAL) {
		char s[ONIG_MAX_ERROR_MESSAGE_LEN];
		onig_error_code_to_str(s, r, &einfo);
		fprintf(stderr, "ERROR: %s\n", s);
		return -1;
	}

	region = onig_region_new();

	end   = str + target_len;
	start = str;
	range = end;
	r = onig_search(reg, str, end, start, range, region, ONIG_OPTION_NONE);
	if (r >= 0) {
		int i;

		for (i = 0; i < region->num_regs; i++) {
			mstart = region->beg[i];
			mend = region->end[i];

			// マッチパターン全体を matchstr へ格納する
			if (i == 0) {
				ch = target[mend];
				target[mend] = 0; // null terminate
				LockVar();
				SetMatchStr(target + mstart);
				UnlockVar();
				target[mend] = ch;  // restore

			} else { // 残りのマッチパターンは groupmatchstr[1-9] へ格納する
				ch = target[mend];
				target[mend] = 0; // null terminate
				LockVar();
				SetGroupMatchStr(i, target + mstart);
				UnlockVar();
				target[mend] = ch;  // restore

			}

		}

		matched = 1;
	}
	else if (r == ONIG_MISMATCH) {
		fprintf(stderr, "search fail\n");
	}
	else { /* error */
		char s[ONIG_MAX_ERROR_MESSAGE_LEN];
		onig_error_code_to_str(s, r);
		fprintf(stderr, "ERROR: %s\n", s);
		return -1;
	}

	onig_region_free(region, 1 /* 1:free self, 0:free contents only */);
	onig_free(reg);
	onig_end();

	return (matched);
}

// 正規表現によるパターンマッチを行う
int FindRegexString(void)
{
	int i, Found = 0;
	PCHAR Str;

	if (RegexActionType == REGEX_NONE)
		return 0;  // not match

	if (RecvLnPtr == 0)
		return 0;  // not match

	for (i = 9 ; i >= 0 ; i--) {
		Str = PWaitStr[i]; // regex pattern
		if (Str!=NULL) {
			if (FindRegexStringOne(Str, strlen(Str), RecvLnBuff, RecvLnPtr) > 0) { // matched
				Found = i+1;

				// マッチした行を inputstr へ格納する
				LockVar();
				SetInputStr(GetRecvLnBuff());  // ここでバッファがクリアされる
				UnlockVar();

				break;
			}
		}
	}

	return (Found);
}


// 'wait': 
// ttmacro process sleeps to wait specified word(s).
// 
// 'waitregex': 
// ttmacro process sleeps to wait specified word(s) with regular expression.
//
// add 'waitregex' command (2005.10.5 yutaka)
int Wait()
{
	BYTE b;
	int i, Found, ret;
	PCHAR Str;

	Found = 0;
	while ((Found==0) && Read1Byte(&b))
	{
		if (b == 0x0a) { // 改行が来たら、バッファをクリアするのでその前にパターンマッチを行う。(waitregex command)
			ret = FindRegexString();
			if (ret > 0) {
				Found = ret;
				break;
			}
		}

		PutRecvLnBuff(b);

		if (RegexActionType == REGEX_NONE) { // 正規表現なしの場合は1バイトずつ検索する(wait command)
			for (i = 9 ; i >= 0 ; i--)
			{
				Str = PWaitStr[i];
				if (Str!=NULL)
				{
					if ((BYTE)Str[WaitCount[i]]==b)
						WaitCount[i]++;
					else if (WaitCount[i]>0) {
						WaitCount[i] = 0;
						if ((BYTE)Str[0]==b)
							WaitCount[i] = 1;
					}
					if (WaitCount[i]==WaitStrLen[i])
						Found = i+1;
				}
			}
		}
	}

	// 改行なしで文字列が流れてくる場合にもパターンマッチを試みる
	if (Found == 0) {
		ret = FindRegexString();
		if (ret > 0) {
			Found = ret;
		}
	}

	if (Found>0) ClearWait();
	SendSync();

	return Found;
}

BOOL Wait2()
{
  BYTE b;

  while (! (Wait2Found && (Wait2Count==Wait2Len))
	 && Read1Byte(&b))
  {
    if (Wait2Count>=Wait2Len)
    {
      memmove(Wait2Str,&(Wait2Str[1]),Wait2Len-1);
      Wait2Str[Wait2Len-1] = b;
      Wait2Count = Wait2Len;
    }
    else {
      Wait2Str[Wait2Count] = b;
      Wait2Count++;
    }
    Wait2Str[Wait2Count] = 0;
    if ((! Wait2Found) &&
	(Wait2Count>=Wait2SubPos+Wait2SubLen-1))
      Wait2Found =
	(strncmp(&(Wait2Str[Wait2SubPos-1]),
		 Wait2SubStr,Wait2SubLen)==0);
  }
  SendSync();

  return (Wait2Found && (Wait2Count==Wait2Len));
}

void SetFile(PCHAR FN)
{
  char Cmd[256];

  Cmd[0] = CmdSetFile;
  strcpy(&(Cmd[1]),FN);
  DdeClientTransaction(Cmd,strlen(Cmd)+1,ConvH,0,CF_OEMTEXT,XTYP_EXECUTE,1000,NULL);
}

void SetBinary(int BinFlag)
{
  char Cmd[3];

  Cmd[0] = CmdSetBinary;
  Cmd[1] = 0x30 + (BinFlag & 1);
  Cmd[2] = 0;
  DdeClientTransaction(Cmd,strlen(Cmd)+1,ConvH,0,CF_OEMTEXT,XTYP_EXECUTE,1000,NULL);
}

void SetAppend(int AppendFlag)
{
  char Cmd[3];

  Cmd[0] = CmdSetAppend;
  Cmd[1] = 0x30 + (AppendFlag & 1);
  Cmd[2] = 0;
  DdeClientTransaction(Cmd,strlen(Cmd)+1,ConvH,0,CF_OEMTEXT,XTYP_EXECUTE,1000,NULL);
}

void SetXOption(int XOption)
{
  char Cmd[3];

  if ((XOption<1) || (XOption>3)) XOption = 1;
  Cmd[0] = CmdSetXmodemOpt;
  Cmd[1] = 0x30 + XOption;
  Cmd[2] = 0;
  DdeClientTransaction(Cmd,strlen(Cmd)+1,ConvH,0,CF_OEMTEXT,XTYP_EXECUTE,1000,NULL);
}

void SendSync()
{
  char Cmd[10];
  int i;

  if (! Linked) return;
  if (! SyncMode) return;
  if (SyncSent) return;
  if (RBufCount>=RCountLimit) return;

  // notify free buffer space to Tera Term
  i = RingBufSize - RBufCount;
  if (i<1) i = 1;
  if (i>RingBufSize) i = RingBufSize;
  SyncSent = TRUE;

  Cmd[0] = CmdSetSync;
  sprintf(&Cmd[1],"%u",i);
  DdeClientTransaction(Cmd,strlen(Cmd)+1,ConvH,0,CF_OEMTEXT,XTYP_EXECUTE,1000,NULL);
}

void SetSync(BOOL OnFlag)
{
  char Cmd[10];
  int i;

  if (! Linked) return;
  if (SyncMode==OnFlag) return;
  SyncMode = OnFlag;

  if (OnFlag) { // sync mode on
    // notify free buffer space to Tera Term
    i = RingBufSize - RBufCount;
    if (i<1) i = 1;
    if (i>RingBufSize) i = RingBufSize;
    SyncSent = TRUE;
  }
  else // sync mode off
    i = 0;

  Cmd[0] = CmdSetSync;
  sprintf(&Cmd[1],"%u",i);
  DdeClientTransaction(Cmd,strlen(Cmd)+1,ConvH,0,CF_OEMTEXT,XTYP_EXECUTE,1000,NULL);
}

WORD SendCmnd(char OpId, int WaitFlag)
/* WaitFlag  should be 0 or IdTTLWaitCmndEnd or IdTTLWaitCmndResult. */
{
  char Cmd[2];

  if (! Linked)
    return ErrLinkFirst;

  if (WaitFlag!=0) TTLStatus = WaitFlag;
  Cmd[0] = OpId;
  Cmd[1] = 0;
  if (DdeClientTransaction(Cmd,strlen(Cmd)+1,ConvH,0,CF_OEMTEXT,XTYP_EXECUTE,5000,NULL)==0)
    TTLStatus = IdTTLRun;

  return 0;
}

WORD GetTTParam(char OpId, PCHAR Param)
{
  HDDEDATA Data;
  PCHAR DataPtr;

  if (! Linked)
    return ErrLinkFirst;

  SendCmnd(OpId,0);
  Data = DdeClientTransaction(NULL,0,ConvH,Item2,CF_OEMTEXT,XTYP_REQUEST,5000,NULL);
  if (Data==0) return 0;
  DataPtr = (PCHAR)DdeAccessData(Data,NULL);
  if (DataPtr!=NULL)
  {
    strcpy(Param,DataPtr);
    DdeUnaccessData(Data);
  }
  DdeFreeDataHandle(Data);

  return 0;
}
