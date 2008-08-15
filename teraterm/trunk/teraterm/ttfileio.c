/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TERATERM.EXE, TTSET interface */
#include "teraterm.h"
#include "tttypes.h"

#include "ttfileio.h"
#include "ttplug.h" /* TTPLUG */

TReadFile PReadFile;
TWriteFile PWriteFile;
TCreateFile PCreateFile;
TCloseFile PCloseFile;

void InitFileIO()
{
  PReadFile = ReadFile;
  PWriteFile = WriteFile;
  PCreateFile = CreateFile;
  PCloseFile = CloseHandle;
  return;
}
