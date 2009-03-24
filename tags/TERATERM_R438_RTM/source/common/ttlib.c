/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* misc. routines  */
#include "teraterm.h"
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>

#ifndef TERATERM32
  #define CharNext AnsiNext
  #define CharPrev AnsiPrev
#endif

BOOL GetFileNamePos(PCHAR PathName, int far *DirLen, int far *FNPos)
{
  BYTE b;
  LPTSTR Ptr, DirPtr, FNPtr, PtrOld;

  *DirLen = 0;
  *FNPos = 0;
  if (PathName==NULL) return FALSE;

  if ((strlen(PathName)>=2) && (PathName[1]==':'))
    Ptr = &PathName[2];
  else
    Ptr = PathName;
  if (Ptr[0]=='\\') Ptr = CharNext(Ptr);

  DirPtr = Ptr;
  FNPtr = Ptr;
  while (Ptr[0]!=0)
  {
    b = Ptr[0];
    PtrOld = Ptr;
    Ptr = CharNext(Ptr);
    switch (b) {
      case ':': return FALSE;
      case '\\':
	DirPtr = PtrOld;
	FNPtr = Ptr;
	break;
    }
  }
  *DirLen = DirPtr-PathName;
  *FNPos = FNPtr-PathName;
  return TRUE;
}

BOOL ExtractFileName(PCHAR PathName, PCHAR FileName)
{
  int i, j;

  if (FileName==NULL) return FALSE;
  if (! GetFileNamePos(PathName,&i,&j)) return FALSE;
  strcpy(FileName,&PathName[j]);
  return (strlen(FileName)>0);
}

BOOL ExtractDirName(PCHAR PathName, PCHAR DirName)
{
  int i, j;

  if (DirName==NULL) return FALSE;
  if (! GetFileNamePos(PathName,&i,&j)) return FALSE;
  memmove(DirName,PathName,i); // do not use memcpy
  DirName[i] = 0;
  return TRUE;
}

/* fit a filename to the windows-filename format */
/* FileName must contain filename part only. */
void FitFileName(PCHAR FileName, PCHAR DefExt)
{
  int i, j, NumOfDots;
  char Temp[MAXPATHLEN];
  BYTE b;
#ifndef TERATERM32
  int NameLen;
#endif
  
  NumOfDots = 0;
  i = 0;
  j = 0;
  /* filename started with a dot is illeagal */
  if (FileName[0]=='.')
  {
    Temp[0] = '_';  /* insert an underscore char */
    j++;
  }

  do {
    b = FileName[i];
    i++;
#ifdef TERATERM32
    if (b=='.') NumOfDots++;
#else
    if (b=='.')
    {
      NumOfDots++;
      NameLen = j;
    }
#endif
    if ((b!=0) &&
	(j < MAXPATHLEN-1))
    {
      Temp[j] = b;
      j++;
    }
  } while (b!=0);
  Temp[j] = 0;

#ifdef TERATERM32
  if ((NumOfDots==0) &&
      (DefExt!=NULL))
    strcat(Temp,DefExt); /* add the default extension */

  strcpy(FileName,Temp);
#else
  if (NumOfDots==0)
  {
    NameLen = j;
    if (DefExt!=NULL)
      strcat(Temp,DefExt); /* add the default extension */
  }
  for (i=0; i<=NameLen-1; i++)
    if (Temp[i]=='.') /* convert dots in the filename */
      Temp[i] = '_'; /* to underscores. */
  strcpy(FileName,Temp);
  if (NameLen>8)
    FileName[8] = 0;
  else
    FileName[NameLen] = 0;
  strncat(FileName,&Temp[NameLen],12-sizeof(FileName));
#endif
}

// Append a slash to the end of a path name
void AppendSlash(PCHAR Path)
{
  if (strcmp(CharPrev((LPCTSTR)Path,
	(LPCTSTR)(&Path[strlen(Path)])),
	"\\") != 0)
    strcat(Path,"\\");
}

void Str2Hex(PCHAR Str, PCHAR Hex, int Len, int MaxHexLen, BOOL ConvSP)
{
  BYTE b, low;
  int i, j;

  if (ConvSP)
    low = 0x20;
  else
    low = 0x1F;

  j = 0;
  for (i=0; i<=Len-1; i++)
  {
    b = Str[i];
    if ((b!='$') && (b>low) && (b<0x7f))
    {
      if (j < MaxHexLen)
      {
	Hex[j] = b;
	j++;
      }
    }
    else {
      if (j < MaxHexLen-2)
      {
	Hex[j] = '$';
	j++;
	if (b<=0x9f)
	  Hex[j] = (char)((b >> 4) + 0x30);
	else
	  Hex[j] = (char)((b >> 4) + 0x37);
	j++;
	if ((b & 0x0f) <= 0x9)
	  Hex[j] = (char)((b & 0x0f) + 0x30);
	else
	  Hex[j] = (char)((b & 0x0f) + 0x37);
	j++;
      }
    }
  }
  Hex[j] = 0;
}

BYTE ConvHexChar(BYTE b)
{
  if ((b>='0') && (b<='9'))
    return (b - 0x30);
  else if ((b>='A') && (b<='F'))
    return (b - 0x37);
  else if ((b>='a') && (b<='f'))
    return (b - 0x57);
  else
    return 0;
}

int Hex2Str(PCHAR Hex, PCHAR Str, int MaxLen)
{
  BYTE b, c;
  int i, imax, j;

  j = 0;
  imax = strlen(Hex);
  i = 0;
  while ((i < imax) && (j<MaxLen))
  {
    b = Hex[i];
    if (b=='$')
    {
      i++;
      if (i < imax)
	c = Hex[i];
      else
	c = 0x30;
      b = ConvHexChar(c) << 4;
      i++;
      if (i < imax)
	c = Hex[i];
      else
	c = 0x30;
      b = b + ConvHexChar(c);
    };

    Str[j] = b;
    j++;
    i++;
  }
  if (j<MaxLen) Str[j] = 0;

  return j;
}

BOOL DoesFileExist(PCHAR FName)
{ // check if a file exists or not
  struct _stat st;

  return (_stat(FName,&st)==0);
}

long GetFSize(PCHAR FName)
{
  struct _stat st;

  if (_stat(FName,&st)==-1) return 0;
  return (long)st.st_size;
}

void uint2str(UINT i, PCHAR Str, int len)
{
  char Temp[20];

  sprintf(Temp,"%u",i);
  Temp[len] = 0;
  strcpy(Str,Temp);
}

#ifdef TERATERM32
void QuoteFName(PCHAR FName)
{
  int i;

  if (FName[0]==0) return;
  if (strchr(FName,' ')==NULL) return;
  i = strlen(FName);
  memmove(&(FName[1]),FName,i);
  FName[0] = '\"';
  FName[i+1] = '\"';
  FName[i+2] = 0;
}
#endif
