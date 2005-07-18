// Tera Term
// Copyright(C) 1994-1998 T. Teranishi
// All rights reserved.

// TTMACRO.EXE, Macro file buffer

#include "teraterm.h"
#include <string.h>
#include "ttmparse.h"
#include "ttlib.h"

#include "ttmbuff.h"

int EndWhileFlag;

#ifdef TERATERM32
  #define MAXBUFFLEN 2147483647
#else
  #define MAXBUFFLEN 32767
#endif

#define MAXNESTLEVEL 10

static int INest;
static HANDLE BuffHandle[MAXNESTLEVEL];
static PCHAR Buff[MAXNESTLEVEL];
static BINT BuffLen[MAXNESTLEVEL];
static BINT BuffPtr[MAXNESTLEVEL];

#define MAXSP 10

// Control type
  // Call
#define CtlCall  1
  // For ... Next
#define CtlFor   2
  // While ... EndWhile
#define CtlWhile 3

// Control stack
#define INVALIDPTR (BINT)-1
static BINT PtrStack[MAXSP];
static int LevelStack[MAXSP];
static int TypeStack[MAXSP];
static int SP; // Stack pointer
static BINT LineStart;
static BOOL NextFlag;

static int LineNo;  // line number (2005.7.18 yutaka)
static int NextLineNo;  // line number (2005.7.18 yutaka)

void InitLineNo(void)
{
	LineNo = 1;
	NextLineNo = 1;
}

int GetLineNo(void)
{
	return (LineNo);
}


BOOL LoadMacroFile(PCHAR FileName, int IBuff)
{
  int F;

  if ((FileName[0]==0) || (IBuff>MAXNESTLEVEL-1)) return FALSE;
  if (BuffHandle[IBuff]!=0) return FALSE;
  BuffPtr[IBuff] = 0;

  // get file length
  BuffLen[IBuff] = GetFSize(FileName);
  if (BuffLen[IBuff]==0) return FALSE;
  if (BuffLen[IBuff]>MAXBUFFLEN) return FALSE;

  F = _lopen(FileName,0);
  if (F<=0) return FALSE;
  BuffHandle[IBuff] = GlobalAlloc(GMEM_MOVEABLE, BuffLen[IBuff]);
  if (BuffHandle[IBuff]!=NULL)
  {
    Buff[IBuff] = GlobalLock(BuffHandle[IBuff]);
    if (Buff[IBuff]!=NULL)
    {
      _lread(F, Buff[IBuff], BuffLen[IBuff]);
      _lclose(F);
      GlobalUnlock(BuffHandle[IBuff]);
      return TRUE;
    }
    else {
      GlobalFree(BuffHandle[IBuff]);
      BuffHandle[IBuff] = 0;
    }
  }
  _lclose(F);
  return FALSE;
}

BOOL GetRawLine()
{
	int i;
	BYTE b;

	LineStart = BuffPtr[INest];
	Buff[INest] = GlobalLock(BuffHandle[INest]);
	if (Buff[INest]==NULL) return FALSE;

	if (BuffPtr[INest]<BuffLen[INest])
		b = (Buff[INest])[BuffPtr[INest]];

	i = 0;
	while ((BuffPtr[INest]<BuffLen[INest]) &&
		((b>=0x20) || (b==0x09)))
	{
		LineBuff[i] = b;
		i++;
		BuffPtr[INest]++;
		if (BuffPtr[INest]<BuffLen[INest])
			b = (Buff[INest])[BuffPtr[INest]];
	}
	LineBuff[i] = 0;
	LinePtr = 0;
	LineLen = strlen(LineBuff);

	LineNo = NextLineNo; // current line number

	while ((BuffPtr[INest]<BuffLen[INest]) &&
		(b<0x20) && (b!=0x09))
	{
		BuffPtr[INest]++;
		if (BuffPtr[INest]<BuffLen[INest])
			b = (Buff[INest])[BuffPtr[INest]];

		// ‰üs‚Ì”»’è (2005.7.18 yutaka)
		if (b == 0x0A) { // LF
			NextLineNo++;    // countup line number
		}
	}
	GlobalUnlock(BuffHandle[INest]);
	return ((LineLen>0) || (BuffPtr[INest]<BuffLen[INest]));
}

BOOL GetNewLine()
{
	BOOL Ok;
	BYTE b;

	do {
		Ok = GetRawLine();
		if (! Ok && (INest>0))
			do {
				CloseBuff(INest);
				INest--;
				Ok = GetRawLine();
			} while (!Ok && (INest>0));
		if (! Ok) return FALSE;

		b = GetFirstChar();
		LinePtr--;
	} while ((b==0) || (b==':'));

	return TRUE;
}

BOOL RegisterLabels(int IBuff)
{
  BYTE b;
  TName LabName;
  WORD Err;
  WORD VarType, VarId;

  Buff[IBuff] = GlobalLock(BuffHandle[IBuff]);
  if (Buff[IBuff]==NULL) return FALSE;
  BuffPtr[IBuff] = 0;

  while (GetRawLine())
  {
    Err = 0;

    b = GetFirstChar();
    if (b==':')
    {
      if (GetLabelName(LabName) && (GetFirstChar()==0))
      {
        if (CheckVar(LabName,&VarType,&VarId))
          Err = ErrLabelAlreadyDef;
        else
          if (! NewLabVar(LabName,BuffPtr[IBuff],(WORD)IBuff))
            Err = ErrTooManyLabels;
      }
      else
        Err = ErrSyntax;
    }

    if (Err>0) DispErr(Err);
  }
  BuffPtr[IBuff] = 0;
  InitLineNo(); // (2005.7.18 yutaka)
  GlobalUnlock(BuffHandle[IBuff]);
  return TRUE;
}

BOOL InitBuff(PCHAR FileName)
{
  int i;

  SP = 0;
  NextFlag = FALSE;
  EndWhileFlag = 0;
  for (i=0 ; i<=MAXNESTLEVEL-1 ; i++)
    BuffHandle[i] = 0;
  INest = 0;
  if (! LoadMacroFile(FileName, INest)) return FALSE;
  if (! RegisterLabels(INest)) return FALSE;
  return TRUE;
}

void CloseBuff(int IBuff)
{
  int i;

  DelLabVar((WORD)IBuff);
  for (i=IBuff ; i<=MAXNESTLEVEL-1 ; i++)
  {
    if (BuffHandle[i]!=NULL)
    {
      GlobalUnlock(BuffHandle[i]);
      GlobalFree(BuffHandle[i]);
    }
    BuffHandle[i] = NULL;
  }

  while ((SP>0) && (LevelStack[SP-1]>=IBuff))
    SP--;
}

void JumpToLabel(int ILabel)
{
  BINT Ptr;
  WORD Level;

  CopyLabel((WORD)ILabel, &Ptr,&Level);
  if (Level < INest)
  {
    INest = Level;
    CloseBuff(INest+1);
  }
  BuffPtr[INest] = Ptr;
}

WORD CallToLabel(int ILabel)
{
  BINT Ptr;
  WORD Level;

  CopyLabel((WORD)ILabel, &Ptr,&Level);
  if (Level != INest)
    return ErrCantCall;

  if (SP>=MAXSP)
    return ErrStackOver;

  PtrStack[SP] = BuffPtr[INest];
  LevelStack[SP] = INest;
  TypeStack[SP] = CtlCall;
  SP++;

  BuffPtr[INest] = Ptr;
  return 0;
}

WORD ReturnFromSub()
{
  if ((SP<1) ||
      (TypeStack[SP-1]!=CtlCall)) return ErrInvalidCtl;

  SP--;
  if (LevelStack[SP] < INest)
  {
    INest = LevelStack[SP];
    CloseBuff(INest+1);
  }
  BuffPtr[INest] = PtrStack[SP];
  return 0;
}

BOOL BuffInclude(PCHAR FileName)
{
  if (INest>=MAXNESTLEVEL-1) return FALSE;
  INest++;
  if (LoadMacroFile(FileName, INest))
  {
    if (RegisterLabels(INest))
      return TRUE;
    else {
      CloseBuff(INest);
      INest--;
    }
  }
  else
    INest--;

  return FALSE;
}

BOOL ExitBuffer()
{
  if (INest<1) return FALSE;
  CloseBuff(INest);
  INest--;
  return TRUE;
}

int SetForLoop()
{
  if (SP>=MAXSP)
    return ErrStackOver;

  PtrStack[SP] = LineStart;
  LevelStack[SP] = INest;
  TypeStack[SP] = CtlFor;
  SP++;
  return 0;
}

void LastForLoop()
{
  if ((SP<1) || (TypeStack[SP-1]!=CtlFor)) return;
  PtrStack[SP-1] = INVALIDPTR;  
}

BOOL CheckNext()
{
  if (NextFlag)
  {
    NextFlag = FALSE;
    return TRUE;
  }
  return FALSE;
}

int NextLoop()
{
  if ((SP<1) || (TypeStack[SP-1]!=CtlFor))
    return ErrInvalidCtl;

  NextFlag = (PtrStack[SP-1]!=INVALIDPTR);
  if (! NextFlag) // exit from loop
  {
    SP--;
    return 0;
  }
  if (LevelStack[SP-1] < INest)
  {
    INest = LevelStack[SP-1];
    CloseBuff(INest+1);
  }
  BuffPtr[INest] = PtrStack[SP-1];
  return 0;
}

int SetWhileLoop()
{
  if (SP>=MAXSP)
    return ErrStackOver;

  PtrStack[SP] = LineStart;
  LevelStack[SP] = INest;
  TypeStack[SP] = CtlWhile;
  SP++;
  return 0;
}

void EndWhileLoop()
{
  EndWhileFlag = 1;
}

int BackToWhile()
{
  if ((SP<1) || (TypeStack[SP-1]!=CtlWhile))
    return ErrInvalidCtl;

  SP--;
  if (LevelStack[SP] < INest)
  {
    INest = LevelStack[SP];
    CloseBuff(INest+1);
  }
  BuffPtr[INest] = PtrStack[SP];
  return 0;
}
