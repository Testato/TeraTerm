/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TTMACRO.EXE, dialog boxes */

#include "stdafx.h"
#include "teraterm.h"
#include <direct.h>
#ifdef TERATERM32
#include "ttm_res.h"
#else
#include "ttm_re16.h"
#endif
#include "ttlib.h"
#include <commdlg.h>
#include "errdlg.h"
#include "inpdlg.h"
#include "msgdlg.h"
#include "statdlg.h"

#define MaxStrLen 256

extern "C" {
char HomeDir[MAXPATHLEN];
char FileName[MAXPATHLEN];
char TopicName[11];
char ShortName[MAXPATHLEN];
char Param2[MAXPATHLEN];
char Param3[MAXPATHLEN];
BOOL SleepFlag;
}

static int DlgPosX = -1000;
static int DlgPosY = 0;

static PStatDlg StatDlg = NULL;

extern "C" {
BOOL NextParam(PCHAR Param, int *i, PCHAR Buff, int BuffSize)
{
  int j;
  char c, q;
  BOOL Quoted;
    
  if ( *i >= (int)strlen(Param)) return FALSE;
  j = 0;

  while (Param[*i]==' ')
    (*i)++;
 
  c = Param[*i];
  Quoted = ((c=='"') || (c=='\''));
  q = 0;
  if (Quoted)
  {
    q = c; 
   (*i)++;
    c = Param[*i];
  }
  (*i)++;
  while ((c!=0) && (c!=q) && (Quoted || (c!=' ')) &&
	 (Quoted || (c!=';')) && (j<BuffSize-1))
  {
    Buff[j] = c;
    j++;
    c = Param[*i];
    (*i)++;
  }
  if (! Quoted && (c==';'))
    (*i)--;

  Buff[j] = 0;
  return (strlen(Buff)>0);
}
}

extern "C" {
void ParseParam(PBOOL IOption, PBOOL VOption)
{
  int i, j, k;
  char Param[MAXPATHLEN];
  char Temp[MAXPATHLEN];

  // Get home directory
  GetModuleFileName(AfxGetInstanceHandle(),FileName,sizeof(FileName));
  ExtractDirName(FileName,HomeDir);
  _chdir(HomeDir);

  // Get command line parameters
  FileName[0] = 0;
  TopicName[0] = 0;
  Param2[0] = 0;
  Param3[0] = 0;
  SleepFlag = FALSE;
  *IOption = FALSE;
  *VOption = FALSE;
#ifdef TERATERM32
  strcpy(Param,GetCommandLine());
  i = 0;
  // the first term shuld be executable filename of TTMACRO
  NextParam(Param, &i, Temp, sizeof(Temp));
#else
  i = (int)*(LPBYTE)MAKELP(GetCurrentPDB(),0x80);
  memcpy(Param,MAKELP(GetCurrentPDB(),0x81),i);
  Param[i] = 0;
  i = 0;
#endif
  j = 0;

  while (NextParam(Param, &i, Temp, sizeof(Temp)))
  {
    if (_strnicmp(Temp,"/D=",3)==0) // DDE option
      strcpy(TopicName,&Temp[3]);
    else if (_strnicmp(Temp,"/I",2)==0)
      *IOption = TRUE;
    else if (_strnicmp(Temp,"/S",2)==0)
      SleepFlag = TRUE;
    else if (_strnicmp(Temp,"/V",2)==0)
      *VOption = TRUE;
    else {
      j++;
      if (j==1)
	strcpy(FileName,Temp);
      else if (j==2)
	strcpy(Param2,Temp);
      else if (j==3)
	strcpy(Param3,Temp);
    }
  }

  if (FileName[0]=='*')
    FileName[0] = 0;
  else if (FileName[0]!=0)
  {
    if (GetFileNamePos(FileName,&j,&k))
    {
      FitFileName(&FileName[k],".TTL");
      strcpy(ShortName,&FileName[k]);
      if (j==0)
      {
	strcpy(FileName,HomeDir);
	AppendSlash(FileName);
	strcat(FileName,ShortName);
      }
    }
    else
      FileName[0] = 0;
  }
}
}

extern "C" {
BOOL GetFileName(HWND HWin)
{
  char FNFilter[31];
  OPENFILENAME FNameRec;

  if (FileName[0]!=0) return FALSE;

  memset(FNFilter, 0, sizeof(FNFilter));
  memset(&FNameRec, 0, sizeof(OPENFILENAME));
  strcpy(FNFilter, "Macro files (*.ttl)");
  strcpy(&(FNFilter[strlen(FNFilter)+1]), "*.ttl");

  FNameRec.lStructSize	 = sizeof(OPENFILENAME);
  FNameRec.hwndOwner	 = HWin;
  FNameRec.lpstrFilter	 = FNFilter;
  FNameRec.nFilterIndex  = 1;
  FNameRec.lpstrFile  = FileName;
  FNameRec.nMaxFile  = sizeof(FileName);
  FNameRec.lpstrInitialDir = HomeDir;
  FNameRec.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
  FNameRec.lpstrDefExt = "TTL";
  FNameRec.lpstrTitle = "MACRO: Open macro";
  if (GetOpenFileName(&FNameRec))
    strcpy(ShortName,&(FileName[FNameRec.nFileOffset]));
  else
    FileName[0] = 0;

  if (FileName[0]==0)
  {
    ShortName[0] = 0;
    return FALSE;
  }
  else
    return TRUE;
}
}

extern "C" {
void SetDlgPos(int x, int y)
{
  DlgPosX = x;
  DlgPosY = y;
  if (StatDlg!=NULL) // update status box position
    StatDlg->Update(NULL,NULL,DlgPosX,DlgPosY);
}
}

extern "C" {
void OpenInpDlg(PCHAR Buff, PCHAR Text, PCHAR Caption,
				BOOL Paswd)
{
  CInpDlg InpDlg(Buff,Text,Caption,Paswd,DlgPosX,DlgPosY);
  InpDlg.DoModal();
}
}

extern "C" {
int OpenErrDlg(PCHAR Msg, PCHAR Line)
{
  CErrDlg ErrDlg(Msg,Line,DlgPosX,DlgPosY);
  return ErrDlg.DoModal();
}
}

extern "C" {
int OpenMsgDlg(PCHAR Text, PCHAR Caption, BOOL YesNo)
{
  CMsgDlg MsgDlg(Text,Caption,YesNo,DlgPosX,DlgPosY);
  return MsgDlg.DoModal();
}
}

extern "C" {
void OpenStatDlg(PCHAR Text, PCHAR Caption)
{
  if (StatDlg==NULL)
  {
    StatDlg = new CStatDlg();
    StatDlg->Create(Text,Caption,DlgPosX,DlgPosY);
  }
  else // if status box already exists,
       // update text and caption only.
    StatDlg->Update(Text,Caption,32767,0);
}
}

extern "C" {
void CloseStatDlg()
{
  if (StatDlg==NULL) return;
  StatDlg->DestroyWindow();
  StatDlg = NULL;
}
}
