// Tera Term
// Copyright(C) 1994-1998 T. Teranishi
// All rights reserved.

// TTMACRO.EXE, TTL parser

#include "teraterm.h"
#include <string.h>
#include "ttmdlg.h"
#include "ttmparse.h"


WORD TTLStatus = 0;
TStrVal LineBuff;
WORD LinePtr;
WORD LineLen;

#define MaxNumOfIntVar (LONG)128
#define MaxNumOfStrVar (LONG)128
#define MaxNumOfLabVar (LONG)256

#define IntVarIdOff (LONG)0
#define StrVarIdOff (IntVarIdOff+MaxNumOfIntVar)
#define LabVarIdOff (StrVarIdOff+MaxNumOfStrVar)
#define MaxNumOfName (MaxNumOfIntVar+MaxNumOfStrVar+MaxNumOfLabVar)
#define NameBuffLen MaxNumOfName*MaxNameLen
#define StrBuffLen MaxNumOfStrVar*MaxStrLen

static int IntVal[MaxNumOfIntVar];
static BINT LabVal[MaxNumOfLabVar];
static BYTE LabLevel[MaxNumOfLabVar];

static HANDLE HNameBuff;
static PCHAR NameBuff;
static HANDLE HStrBuff;
static PCHAR StrBuff;
static WORD IntVarCount, StrVarCount, LabVarCount;


BOOL InitVar()
{
  HNameBuff = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,NameBuffLen);
  if (HNameBuff==NULL) return FALSE;
  NameBuff = NULL;

  HStrBuff = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,StrBuffLen);
  if (HStrBuff==NULL) return FALSE;
  StrBuff = NULL;

  IntVarCount = 0;
  LabVarCount = 0;
  StrVarCount = 0;
  return TRUE;
}

void EndVar()
{
  UnlockVar();
  GlobalFree(HNameBuff);
  GlobalFree(HStrBuff);
}

void DispErr(WORD Err)
{
  char Msg[41];
  int i;

  switch (Err) {
    case ErrCloseParent: strcpy(Msg,"\")\" expected."); break;
    case ErrCantCall: strcpy(Msg,"Can't call sub."); break;
    case ErrCantConnect: strcpy(Msg,"Can't link macro."); break;
    case ErrCantOpen: strcpy(Msg,"Can't open file."); break;
    case ErrDivByZero: strcpy(Msg,"Divide by zero."); break;
    case ErrInvalidCtl: strcpy(Msg,"Invalid control."); break;
    case ErrLabelAlreadyDef: strcpy(Msg,"Label already defined."); break;
    case ErrLabelReq: strcpy(Msg,"Label requiered."); break;
    case ErrLinkFirst: strcpy(Msg,"Link macro first."); break;
    case ErrStackOver: strcpy(Msg,"Stack overflow."); break;
    case ErrSyntax: strcpy(Msg,"Syntax error."); break;
    case ErrTooManyLabels: strcpy(Msg,"Too many labels."); break;
    case ErrTooManyVar: strcpy(Msg,"Too many variables."); break;
    case ErrTypeMismatch: strcpy(Msg,"Type mismatch."); break;
    case ErrVarNotInit: strcpy(Msg,"Variable not initialized."); break;
  }

  i = OpenErrDlg(Msg,LineBuff);
  if (i==IDOK) TTLStatus = IdTTLEnd;
}

void LockVar()
{
  if (NameBuff==NULL)
    NameBuff = GlobalLock(HNameBuff);
  if (NameBuff==NULL)
    PostQuitMessage(0);

  if (StrBuff==NULL)
    StrBuff = GlobalLock(HStrBuff);
  if (StrBuff==NULL)
    PostQuitMessage(0);
}

void UnlockVar()
{
  if (NameBuff!=NULL)
    GlobalUnlock(HNameBuff);
  NameBuff = NULL;

  if (StrBuff!=NULL)
    GlobalUnlock(HStrBuff);
  StrBuff = NULL;
}

BOOL CheckReservedWord(PCHAR Str, LPWORD WordId)
{
  *WordId = 0;

  if (stricmp(Str,"beep")==0) *WordId = RsvBeep;
  else if (stricmp(Str,"bplusrecv")==0) *WordId = RsvBPlusRecv;
  else if (stricmp(Str,"bplussend")==0) *WordId = RsvBPlusSend;
  else if (stricmp(Str,"call")==0) *WordId = RsvCall;
  else if (stricmp(Str,"changedir")==0) *WordId = RsvChangeDir;
  else if (stricmp(Str,"clearscreen")==0) *WordId = RsvClearScreen;
  else if (stricmp(Str,"closesbox")==0) *WordId = RsvCloseSBox;
  else if (stricmp(Str,"closett")==0) *WordId = RsvCloseTT;
  else if (stricmp(Str,"code2str")==0) *WordId = RsvCode2Str;
  else if (stricmp(Str,"connect")==0) *WordId = RsvConnect;
  else if (stricmp(Str,"delpassword")==0) *WordId = RsvDelPassword;
  else if (stricmp(Str,"disconnect")==0) *WordId = RsvDisconnect;
  else if (stricmp(Str,"else")==0) *WordId = RsvElse;
  else if (stricmp(Str,"elseif")==0) *WordId = RsvElseIf;
  else if (stricmp(Str,"enablekeyb")==0) *WordId = RsvEnableKeyb;
  else if (stricmp(Str,"end")==0) *WordId = RsvEnd;
  else if (stricmp(Str,"endif")==0) *WordId = RsvEndIf;
  else if (stricmp(Str,"endwhile")==0) *WordId = RsvEndWhile;
  else if (stricmp(Str,"exec")==0) *WordId = RsvExec;
  else if (stricmp(Str,"execcmnd")==0) *WordId = RsvExecCmnd;
  else if (stricmp(Str,"exit")==0) *WordId = RsvExit;
  else if (stricmp(Str,"fileclose")==0) *WordId = RsvFileClose;
  else if (stricmp(Str,"fileconcat")==0) *WordId = RsvFileConcat;
  else if (stricmp(Str,"filecopy")==0) *WordId = RsvFileCopy;
  else if (stricmp(Str,"filecreate")==0) *WordId = RsvFileCreate;
  else if (stricmp(Str,"filedelete")==0) *WordId = RsvFileDelete;
  else if (stricmp(Str,"filemarkptr")==0) *WordId = RsvFileMarkPtr;
  else if (stricmp(Str,"fileopen")==0) *WordId = RsvFileOpen;
  else if (stricmp(Str,"filereadln")==0) *WordId = RsvFileReadln;
  else if (stricmp(Str,"filerename")==0) *WordId = RsvFileRename;
  else if (stricmp(Str,"filesearch")==0) *WordId = RsvFileSearch;
  else if (stricmp(Str,"fileseek")==0) *WordId = RsvFileSeek;
  else if (stricmp(Str,"fileseekback")==0) *WordId = RsvFileSeekBack;
  else if (stricmp(Str,"filestrseek")==0) *WordId = RsvFileStrSeek;
  else if (stricmp(Str,"filestrseek2")==0) *WordId = RsvFileStrSeek2;
  else if (stricmp(Str,"filewrite")==0) *WordId = RsvFileWrite;
  else if (stricmp(Str,"filewriteln")==0) *WordId = RsvFileWriteLn;
  else if (stricmp(Str,"findclose")==0) *WordId = RsvFindClose;
  else if (stricmp(Str,"findfirst")==0) *WordId = RsvFindFirst;
  else if (stricmp(Str,"findnext")==0) *WordId = RsvFindNext;
  else if (stricmp(Str,"flushrecv")==0) *WordId = RsvFlushRecv;
  else if (stricmp(Str,"for")==0) *WordId = RsvFor;
  else if (stricmp(Str,"getdate")==0) *WordId = RsvGetDate;
  else if (stricmp(Str,"getdir")==0) *WordId = RsvGetDir;
  else if (stricmp(Str,"getenv")==0) *WordId = RsvGetEnv;
  else if (stricmp(Str,"getpassword")==0) *WordId = RsvGetPassword;
  else if (stricmp(Str,"gettime")==0) *WordId = RsvGetTime;
  else if (stricmp(Str,"gettitle")==0) *WordId = RsvGetTitle;
  else if (stricmp(Str,"goto")==0) *WordId = RsvGoto;
  else if (stricmp(Str,"if")==0) *WordId = RsvIf;
  else if (stricmp(Str,"include")==0) *WordId = RsvInclude ;
  else if (stricmp(Str,"inputbox")==0) *WordId = RsvInputBox;
  else if (stricmp(Str,"int2str")==0) *WordId = RsvInt2Str;
  else if (stricmp(Str,"kmtfinish")==0) *WordId = RsvKmtFinish;
  else if (stricmp(Str,"kmtget")==0) *WordId = RsvKmtGet;
  else if (stricmp(Str,"kmtrecv")==0) *WordId = RsvKmtRecv;
  else if (stricmp(Str,"kmtsend")==0) *WordId = RsvKmtSend;
  else if (stricmp(Str,"loadkeymap")==0) *WordId = RsvLoadKeyMap;
  else if (stricmp(Str,"logclose")==0) *WordId = RsvLogClose;
  else if (stricmp(Str,"logopen")==0) *WordId = RsvLogOpen;
  else if (stricmp(Str,"logpause")==0) *WordId = RsvLogPause;
  else if (stricmp(Str,"logstart")==0) *WordId = RsvLogStart;
  else if (stricmp(Str,"logwrite")==0) *WordId = RsvLogWrite;
  else if (stricmp(Str,"makepath")==0) *WordId = RsvMakePath;
  else if (stricmp(Str,"messagebox")==0) *WordId = RsvMessageBox;
  else if (stricmp(Str,"next")==0) *WordId = RsvNext;
  else if (stricmp(Str,"passwordbox")==0) *WordId = RsvPasswordBox;
  else if (stricmp(Str,"pause")==0) *WordId = RsvPause;
  else if (stricmp(Str,"quickvanrecv")==0) *WordId = RsvQuickVANRecv;
  else if (stricmp(Str,"quickvansend")==0) *WordId = RsvQuickVANSend;
  else if (stricmp(Str,"recvln")==0) *WordId = RsvRecvLn;
  else if (stricmp(Str,"restoresetup")==0) *WordId = RsvRestoreSetup;
  else if (stricmp(Str,"return")==0) *WordId = RsvReturn;
  else if (stricmp(Str,"send")==0) *WordId = RsvSend;
  else if (stricmp(Str,"sendbreak")==0) *WordId = RsvSendBreak;
  else if (stricmp(Str,"sendfile")==0) *WordId = RsvSendFile;
  else if (stricmp(Str,"sendkcode")==0) *WordId = RsvSendKCode;
  else if (stricmp(Str,"sendln")==0) *WordId = RsvSendLn;
  else if (stricmp(Str,"setdate")==0) *WordId = RsvSetDate;
  else if (stricmp(Str,"setdir")==0) *WordId = RsvSetDir;
  else if (stricmp(Str,"setdlgpos")==0) *WordId = RsvSetDlgPos;
  else if (stricmp(Str,"setecho")==0) *WordId = RsvSetEcho;
  else if (stricmp(Str,"setexitcode")==0) *WordId = RsvSetExitCode;
  else if (stricmp(Str,"setsync")==0) *WordId = RsvSetSync;
  else if (stricmp(Str,"settime")==0) *WordId = RsvSetTime;
  else if (stricmp(Str,"settitle")==0) *WordId = RsvSetTitle;
  else if (stricmp(Str,"show")==0) *WordId = RsvShow;
  else if (stricmp(Str,"showtt")==0) *WordId = RsvShowTT;
  else if (stricmp(Str,"statusbox")==0) *WordId = RsvStatusBox;
  else if (stricmp(Str,"str2code")==0) *WordId = RsvStr2Code;
  else if (stricmp(Str,"str2int")==0) *WordId = RsvStr2Int;
  else if (stricmp(Str,"strcompare")==0) *WordId = RsvStrCompare;
  else if (stricmp(Str,"strconcat")==0) *WordId = RsvStrConcat;
  else if (stricmp(Str,"strcopy")==0) *WordId = RsvStrCopy;
  else if (stricmp(Str,"strlen")==0) *WordId = RsvStrLen;
  else if (stricmp(Str,"strscan")==0) *WordId = RsvStrScan;
  else if (stricmp(Str,"testlink")==0) *WordId = RsvTestLink;
  else if (stricmp(Str,"then")==0) *WordId = RsvThen;
  else if (stricmp(Str,"unlink")==0) *WordId = RsvUnlink;
  else if (stricmp(Str,"wait")==0) *WordId = RsvWait;
  else if (stricmp(Str,"waitevent")==0) *WordId = RsvWaitEvent;
  else if (stricmp(Str,"waitln")==0) *WordId = RsvWaitLn;
  else if (stricmp(Str,"waitrecv")==0) *WordId = RsvWaitRecv;
  else if (stricmp(Str,"while")==0) *WordId = RsvWhile;
  else if (stricmp(Str,"xmodemrecv")==0) *WordId = RsvXmodemRecv;
  else if (stricmp(Str,"xmodemsend")==0) *WordId = RsvXmodemSend;
  else if (stricmp(Str,"yesnobox")==0) *WordId = RsvYesNoBox;
  else if (stricmp(Str,"zmodemrecv")==0) *WordId = RsvZmodemRecv;
  else if (stricmp(Str,"zmodemsend")==0) *WordId = RsvZmodemSend;

  else if (stricmp(Str,"not")==0) *WordId = RsvNot;
  else if (stricmp(Str,"and")==0) *WordId = RsvAnd;
  else if (stricmp(Str,"or")==0) *WordId = RsvOr;
  else if (stricmp(Str,"xor")==0) *WordId = RsvXor;

  return (*WordId!=0);
}

BYTE GetFirstChar()
{
  BYTE b;

  if (LinePtr<LineLen)
    b = LineBuff[LinePtr];
  else return 0;

  while ((LinePtr<LineLen) && ((b==0x20) || (b==0x09)))
  {
    LinePtr++;
    if (LinePtr<LineLen) b = LineBuff[LinePtr];
  }
  if ((b>0x20) && (b!=';'))
  {
    LinePtr++;
    return b;
  }
  return 0;
}

BOOL GetIdentifier(PCHAR Name)
{
  int i;
  BYTE b;

  memset(Name,0,MaxNameLen);

  b = GetFirstChar();
  if (b==0) return FALSE;

  // Check first character of identifier
  if (((b<'A') || (b>'Z')) &&
      (b!='_') &&
      ((b<'a') || (b>'z')))
  {
    LinePtr--;
    return FALSE;
  }

  Name[0] = b;
  i = 1;

  if (LinePtr<LineLen) b = LineBuff[LinePtr];
  while ((LinePtr<LineLen) &&
	 ( (b>='0') && (b<='9') ||
	   (b>='A') && (b<='Z') ||
	   (b>='_') ||
	   (b>='a') && (b<='z') ) )
  {
    if (i<MaxNameLen-1)
    {
      Name[i] = b;
      i++;
    }
    LinePtr++;
    if (LinePtr<LineLen) b = LineBuff[LinePtr];
  }
  return TRUE;
}

BOOL GetReservedWord(LPWORD WordId)
{
  TName Name;
  WORD P;

  P = LinePtr;
  if (! GetIdentifier(Name)) return FALSE;
  if (! CheckReservedWord(Name,WordId))
  {
    LinePtr = P;
    return FALSE;
  }
  if (0 < *WordId)
    return TRUE;
  else
    LinePtr = P;
  return FALSE;
}

BOOL GetOperator(LPWORD WordId)
{
  WORD P;
  BYTE b;

  P = LinePtr;
  b = GetFirstChar();
  switch (b) {
    case 0: return FALSE;
    case '*': *WordId = RsvMul; break;
    case '+': *WordId = RsvPlus; break;
    case '-': *WordId = RsvMinus; break;
    case '/': *WordId = RsvDiv; break;
    case '%': *WordId = RsvMod; break;
    case '<': *WordId = RsvLT; break;
    case '=': *WordId = RsvEQ; break;
    case '>': *WordId = RsvGT; break;
    default:
      LinePtr--;
      if (! GetReservedWord(WordId) || (*WordId < RsvOperator))
      {
	LinePtr = P;
	return FALSE;
      }
  }

  if (((*WordId==RsvLT) || (*WordId==RsvGT)) &&
      (LinePtr<LineLen))
  {
    b = LineBuff[LinePtr];
    if (b=='=')
    {
      if (*WordId==RsvLT)
	*WordId=RsvLE;
      else
	*WordId=RsvGE;
      LinePtr++;
    }
    else if ((b=='>') && (*WordId==RsvLT))
    {
      *WordId = RsvNE;
      LinePtr++;
    }
  }
  return TRUE;
}

BOOL GetLabelName(PCHAR Name)
{
  int i;
  BYTE b;

  memset(Name,0,MaxNameLen);

  b = GetFirstChar();
  if (b==0) return FALSE;
  Name[0] = b;

  i = 1;
  if (LinePtr<LineLen) b = LineBuff[LinePtr];
  while ((LinePtr<LineLen) &&
	 ( (b>='0') && (b<='9') ||
	   (b>='A') && (b<='Z') ||
	   (b>='_') ||
	   (b>='a') && (b<='z') ) )
  {
    if (i<MaxNameLen-1)
    {
      Name[i] = b;
      i++;
    }
    LinePtr++;
    if (LinePtr<LineLen) b = LineBuff[LinePtr];
  }

  return (strlen(Name)>0);
}

  int GetQuotedStr(PCHAR Str, BYTE q, LPWORD i)
  {
    BYTE b;

    b=0;
    if (LinePtr<LineLen) b = LineBuff[LinePtr];
    while ((LinePtr<LineLen) && (b>=' ') && (b!=q))
    {
      if (*i<MaxStrLen-1)
      {
	Str[*i] = b;
	(*i)++;
      }

      LinePtr++;
      if (LinePtr<LineLen) b = LineBuff[LinePtr];
    }
    if (b==q)
      if (LinePtr<LineLen) LinePtr++;
    else
      return (ErrSyntax);

    return 0;
  }

  WORD GetCharByCode(PCHAR Str, LPWORD i)
  {
    BYTE b;
    WORD n;

    b=0;
    n = 0;
    if (LinePtr<LineLen) b = LineBuff[LinePtr];
    if (((b<'0') || (b>'9')) &&
	(b!='$')) return ErrSyntax;

    if (b!='$')
    { /* decimal */
      while ((LinePtr<LineLen) && (b>='0') && (b<='9'))
      {
	n = n * 10 + b - 0x30;
	LinePtr++;
	if (LinePtr<LineLen) b = LineBuff[LinePtr];
      }
    }
    else { /* hexadecimal */
      LinePtr++;
      if (LinePtr<LineLen) b = LineBuff[LinePtr];
      while ((LinePtr<LineLen) &&
	     ((b>='0') && (b<='9') ||
	      (b>='A') && (b<='F') ||
	      (b>='a') && (b<='f')))
      {
	if (b>='a')
	  b = b - 0x57;
	else if (b>='A')
	  b = b - 0x37;
	else
	  b = b - 0x30;
	n = n * 16 + b;
	LinePtr++;
	if (LinePtr<LineLen) b = LineBuff[LinePtr];
      }
    }

    if ((n==0) || (n>255)) return ErrSyntax;

    if (*i<MaxStrLen-1)
    {
      Str[*i] = (char)n;
      (*i)++;
    }
    return 0;
  }

BOOL GetString(PCHAR Str, LPWORD Err)
{
  BYTE q;
  WORD i;

  *Err = 0;
  memset(Str,0,MaxStrLen);

  q = GetFirstChar();
  if (q==0) return FALSE;
  LinePtr--;
  if ((q!=0x22) && (q!=0x27) && (q!='#'))
    return FALSE;

  i = 0;
  while (((q==0x22) || (q==0x27) || (q=='#')) && (*Err==0))
  {
    LinePtr++;
    switch (q) {
      case 0x22:
      case 0x27: *Err = GetQuotedStr(Str,q,&i); break;
      case '#': *Err = GetCharByCode(Str,&i); break;
    }
    q = LineBuff[LinePtr];
  }
  return TRUE;
}

BOOL GetNumber(int far *Num)
{
  BYTE b;

  *Num = 0;

  b = GetFirstChar();
  if (b==0) return FALSE;
  if ((b>='0') && (b<='9'))
  { /* decimal constant */
    *Num = b-0x30;
    if (LinePtr<LineLen) b = LineBuff[LinePtr];
    while ((LinePtr<LineLen) && (b>='0') && (b<='9'))
    {
      *Num = *Num * 10 - 0x30 + b;
      LinePtr++;
      if (LinePtr<LineLen) b = LineBuff[LinePtr];
    }
  }
  else if (b=='$')
  { /* hexadecimal constant */
    if (LinePtr<LineLen) b = LineBuff[LinePtr];
    while ((LinePtr<LineLen) &&
	   ((b>='0') && (b<='9') ||
	    (b>='A') && (b<='F') ||
	    (b>='a') && (b<='f')))
    {
      if (b>='a')
	b = b - 0x57;
      else if (b>='A')
	b = b - 0x37;
      else
	b = b - 0x30;
      *Num = *Num * 16 + b;
      LinePtr++;
      if (LinePtr<LineLen) b = LineBuff[LinePtr];
    }
  }
  else {
    LinePtr--;
    return FALSE;
  }
  return TRUE;
}

BOOL CheckVar(PCHAR Name, LPWORD VarType, LPWORD VarId)
{
  int i;
  long P;

  *VarType = TypUnknown;

  i = 0;
  while (i<IntVarCount)
  {
    P = (i+IntVarIdOff)*MaxNameLen;
    if (stricmp(&NameBuff[P],Name)==0)
    {
      *VarType = TypInteger;
      *VarId = (WORD)i;
      return TRUE;
    }
    i++;
  }

  i = 0;
  while (i<StrVarCount)
  {
    P = (i+StrVarIdOff)*MaxNameLen;
    if (stricmp(&NameBuff[P],Name)==0)
    {
      *VarType = TypString;
      *VarId = i;
      return TRUE;
    }
    i++;
  }

  i = 0;
  while (i<LabVarCount)
  {
    P = (i+LabVarIdOff)*MaxNameLen;
    if (stricmp(&NameBuff[P],Name)==0)
    {
      *VarType = TypLabel;
      *VarId = i;
      return TRUE;
    }
    i++;
  }

  return FALSE;
}

BOOL NewIntVar(PCHAR Name, int InitVal)
{
  long P;

  if (IntVarCount>=MaxNumOfIntVar) return FALSE;
  P = (IntVarIdOff+IntVarCount)*MaxNameLen;
  strcpy(&NameBuff[P],Name);
  IntVal[IntVarCount] = InitVal;
  IntVarCount++;
  return TRUE;
}

BOOL NewStrVar(PCHAR Name, PCHAR InitVal)
{
  long P;

  if (StrVarCount>=MaxNumOfStrVar) return FALSE;
  P = (StrVarIdOff+StrVarCount)*MaxNameLen;
  strcpy(&NameBuff[P],Name);
  P = StrVarCount*MaxStrLen;
  strcpy(&StrBuff[P],InitVal);
  StrVarCount++;
  return TRUE;
}

BOOL NewLabVar(PCHAR Name, BINT InitVal, WORD ILevel)
{
  long P;

  if (LabVarCount>=MaxNumOfLabVar) return FALSE;

  P = (LabVarIdOff+LabVarCount)*MaxNameLen;
  strcpy(&(NameBuff[P]),Name);
  LabVal[LabVarCount] = InitVal;
  LabLevel[LabVarCount] = LOBYTE(ILevel);
  LabVarCount++;
  return TRUE;
}

void DelLabVar(WORD ILevel)
{
  while ((LabVarCount>0) && (LabLevel[LabVarCount-1]>=ILevel))
    LabVarCount--;
}

void CopyLabel(WORD ILabel, BINT far *Ptr, LPWORD Level)
{
  *Ptr = LabVal[ILabel];
  *Level = (WORD)LabLevel[ILabel];
}

BOOL GetFactor(LPWORD ValType, int far *Val, LPWORD Err)
{
  TName Name;
  WORD P, Id;

  P = LinePtr;
  *Err = 0;
  if (GetIdentifier(Name))
  {
    if (CheckReservedWord(Name,&Id))
    {
      if (Id==RsvNot)
      {
	if (GetFactor(ValType,Val,Err))
	{
	  if ((*Err==0) && (*ValType!=TypInteger))
	    *Err = ErrTypeMismatch;
	  *Val = ~ *Val;
	}
	else
	  *Err = ErrSyntax;
      }
      else
	*Err = ErrSyntax;
    }
    else if (CheckVar(Name, ValType, &Id))
    {
      switch (*ValType) {
	case TypInteger: *Val = IntVal[Id]; break;
	case TypString: *Val = Id; break;
      }
    }
    else
      *Err = ErrVarNotInit;
  }
  else if (GetNumber(Val))
    *ValType = TypInteger;
  else if (GetOperator(&Id))
  {
    if ((Id==RsvPlus) || (Id==RsvMinus))
    {
      if (GetFactor(ValType,Val,Err))
      {
	if ((*Err==0) && (*ValType != TypInteger))
	  *Err = ErrTypeMismatch;
	if (Id==RsvMinus) *Val = -(*Val);
      }
      else
	*Err = ErrSyntax;
    }
    else
      *Err = ErrSyntax;
  }
  else if (GetFirstChar()=='(')
  {
    if (GetExpression(ValType,Val,Err))
    {
      if ((*Err==0) && (GetFirstChar()!=')'))
	*Err = ErrCloseParent;
    }
    else
      *Err = ErrSyntax;
  }
  else {
    *Err = 0;
    return FALSE;
  }

  if (*Err!=0) LinePtr = P;
  return TRUE;
}

BOOL GetTerm(LPWORD ValType, int far *Val, LPWORD Err)
{
  WORD P1, P2, Type1, Type2, Er;
  int Val1, Val2;
  WORD WId;

  P1 = LinePtr;
  if (! GetFactor(&Type1,&Val1,&Er)) return FALSE;
  *ValType = Type1;
  *Val = Val1;
  *Err = Er;
  if (Er!=0)
  {
    LinePtr = P1;
    return TRUE;
  }
  if (Type1!=TypInteger) return TRUE;

  do {
    P2 = LinePtr;
    if (! GetOperator(&WId)) return TRUE;

    switch (WId) {
      case RsvAnd:
      case RsvMul:
      case RsvDiv:
      case RsvMod: break;
      default:
	LinePtr = P2;
	return TRUE;
    }

    if (! GetFactor(&Type2,&Val2,&Er))
    {
      *Err = ErrSyntax;
      LinePtr = P1;
      return TRUE;
    }

    if (Er!=0)
    {
      *Err = Er;
      LinePtr = P1;
      return TRUE;
    }

    if (Type2!=TypInteger)
    {
      *Err = ErrTypeMismatch;
      LinePtr = P1;
      return TRUE;
    }

    switch (WId) {
      case RsvAnd: Val1 = Val1 & Val2; break;
      case RsvMul: Val1 = Val1 * Val2; break;
      case RsvDiv:
	if (Val2!=0)
	  Val1 = Val1 / Val2;
	else {
	  *Err = ErrDivByZero;
	  LinePtr = P1;
	  return TRUE;
	}
      case RsvMod:
	if (Val2!=0)
	  Val1 = Val1 % Val2;
	else {
	  *Err = ErrDivByZero;
	  LinePtr = P1;
	  return TRUE;
	}
    }

    *Val = Val1;
  } while (TRUE);
}

BOOL GetSimpleExpression(LPWORD ValType, int far *Val, LPWORD Err)
{
  WORD P1, P2, Type1,Type2, Er;
  int Val1,Val2;
  WORD WId;

  P1 = LinePtr;
  if (! GetTerm(&Type1,&Val1,&Er)) return FALSE;
  *ValType = Type1;
  *Val = Val1;
  *Err = Er;
  if (Er!=0)
  {
    LinePtr = P1;
    return TRUE;
  }
  if (Type1!=TypInteger) return TRUE;

  do {
    P2 = LinePtr;
    if (! GetOperator(&WId)) return TRUE;

    switch (WId) {
      case RsvOr:
      case RsvXor:
      case RsvPlus:
      case RsvMinus: break;
      default:
	LinePtr = P2;
	return TRUE;
    }

    if (! GetTerm(&Type2,&Val2,&Er))
    {
      *Err = ErrSyntax;
      LinePtr = P1;
      return TRUE;
    }

    if (Er!=0)
    {
      *Err = Er;
      LinePtr = P1;
      return TRUE;
    }

    if (Type2!=TypInteger)
    {
      *Err = ErrTypeMismatch;
      LinePtr = P1;
      return TRUE;
    }

    switch (WId) {
      case RsvOr:    Val1 = Val1 | Val2; break;
      case RsvXor:   Val1 = Val1 ^ Val2; break;
      case RsvPlus:  Val1 = Val1 + Val2; break;
      case RsvMinus: Val1 = Val1 - Val2; break;
    }
    *Val = Val1;
  } while (TRUE);
}

BOOL GetExpression(LPWORD ValType, int far *Val, LPWORD Err)
{
  WORD P1,P2, Type1,Type2, Er;
  int Val1, Val2;
  WORD WId;

  P1 = LinePtr;
  if (! GetSimpleExpression(&Type1,&Val1,&Er))
  {
    LinePtr = P1;
    return FALSE;
  }
  *ValType = Type1;
  *Val = Val1;
  *Err = Er;
  if (Er!=0)
  {
    LinePtr = P1;
    return TRUE;
  }
  if (Type1!=TypInteger) return TRUE;

  P2 = LinePtr;
  if (! GetOperator(&WId)) return TRUE;

  switch (WId) {
    case RsvLT:
    case RsvEQ:
    case RsvGT:
    case RsvLE:
    case RsvNE:
    case RsvGE: break;
    default:
      LinePtr = P2;
      return TRUE;
  }

  if (! GetSimpleExpression(&Type2,&Val2,&Er))
  {
    *Err = ErrSyntax;
    LinePtr = P1;
    return TRUE;
  }

  if (Er!=0)
  {
    *Err = Er;
    LinePtr = P1;
    return TRUE;
  }

  if (Type2!=TypInteger)
  {
    *Err = ErrTypeMismatch;
    return TRUE;
  }

  *Val = 0;
  switch (WId) {
    case RsvLT: if (Val1 <Val2) *Val = 1; break;
    case RsvEQ: if (Val1==Val2) *Val = 1; break;
    case RsvGT: if (Val1 >Val2) *Val = 1; break;
    case RsvLE: if (Val1<=Val2) *Val = 1; break;
    case RsvNE: if (Val1!=Val2) *Val = 1; break;
    case RsvGE: if (Val1>=Val2) *Val = 1; break;
  }
  return TRUE;
}

void GetIntVal(int far *Val, LPWORD Err)
{
  WORD ValType;

  if (*Err != 0) return;
  if (! GetExpression(&ValType,Val,Err))
  {
    *Err = ErrSyntax;
    return;
  }
  if (*Err!=0) return;
  if (ValType!=TypInteger)
    *Err = ErrTypeMismatch;
}

void SetIntVal(WORD VarId, int Val)
{
  IntVal[VarId] = Val;
}

int CopyIntVal(WORD VarId)
{
  return IntVal[VarId];
}

void GetIntVar(LPWORD VarId, LPWORD Err)
{
  TName Name;
  WORD VarType;

  if (*Err!=0) return;

  if (GetIdentifier(Name))
  {
    if (CheckVar(Name,&VarType,VarId))
    {
      if (VarType!=TypInteger)
	*Err = ErrTypeMismatch;
    }
    else {
      if (NewIntVar(Name,0))
	CheckVar(Name,&VarType,VarId);
      else
	*Err = ErrTooManyVar;
    }
  }
  else
    *Err = ErrSyntax;
}

void GetStrVal(PCHAR Str, LPWORD Err)
{
  WORD VarType;
  int VarId;

  Str[0] = 0;
  if (*Err!=0) return;

  if (GetString(Str,Err))
    return;
  else if (GetExpression(&VarType,&VarId,Err))
  {
    if (*Err!=0) return;
    if (VarType!=TypString)
      *Err = ErrTypeMismatch;
    else
      strcpy(Str,&StrBuff[VarId*MaxStrLen]);
  }
  else
    *Err = ErrSyntax;
}

void GetStrVar(LPWORD VarId, LPWORD Err)
{
  TName Name;
  WORD VarType;

  if (*Err!=0) return;

  if (GetIdentifier(Name))
  {
    if (CheckVar(Name,&VarType,VarId))
    {
      if (VarType!=TypString)
	*Err = ErrTypeMismatch;
    }
    else {
      if (NewStrVar(Name,""))
	CheckVar(Name,&VarType,VarId);
      else
	*Err = ErrTooManyVar;
    }
  }
  else
    *Err = ErrSyntax;
}

void SetStrVal(WORD VarId, PCHAR Str)
{
  strcpy(&StrBuff[VarId*MaxStrLen],Str);
}

PCHAR StrVarPtr(WORD VarId)
{
  return (&StrBuff[VarId*MaxStrLen]);
}
