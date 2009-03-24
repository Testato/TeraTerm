// Tera Term
// Copyright(C) 1994-1998 T. Teranishi
// All rights reserved.

// TTMACRO.EXE, misc routines

#include "teraterm.h"
#include "ttlib.h"
#include <string.h>
#include <direct.h>

static char CurrentDir[MAXPATHLEN];

void CalcTextExtent(HDC DC, PCHAR Text, LPSIZE s)
{
  int W, H, i, i0;
  char Temp[256];
  DWORD dwExt;

  W = 0;
  H = 0;
  i = 0;
  do {
    i0 = i;
    while ((Text[i]!=0) &&
	   (Text[i]!=0x0d) &&
	   (Text[i]!=0x0a))
      i++;
    memcpy(Temp,&Text[i0],i-i0);
    Temp[i-i0] = 0;
    if (Temp[0]==0)
    {
     Temp[0] = 0x20;
     Temp[1] = 0;
    }
    dwExt = GetTabbedTextExtent(DC,Temp,strlen(Temp),0,NULL);
    s->cx = LOWORD(dwExt);
    s->cy = HIWORD(dwExt);
    if (s->cx > W) W = s->cx;
    H = H + s->cy;
    if (Text[i]!=0)
    {
      i++;
      if ((Text[i]==0x0a) &&
	  (Text[i-1]==0x0d))
	i++;
    }
  } while (Text[i]!=0);
  if ((i-i0 == 0) && (H > s->cy)) H = H - s->cy;
  s->cx = W;
  s->cy = H;
}

void TTMGetDir(PCHAR Dir)
{
  strcpy(Dir,CurrentDir);
}

void TTMSetDir(PCHAR Dir)
{
  char Temp[MAXPATHLEN];

  getcwd(Temp,sizeof(Temp));
  chdir(CurrentDir);
  chdir(Dir);
  getcwd(CurrentDir,sizeof(CurrentDir));
  chdir(Temp);
}

void GetAbsPath(PCHAR FName)
{
  int i, j;
  char Temp[MAXPATHLEN];

  if ((! GetFileNamePos(FName,&i,&j)) ||
      (i>0)) return;
  strcpy(Temp,FName);
  strcpy(FName,CurrentDir);
  AppendSlash(FName);
  strcat(FName,Temp);
}
