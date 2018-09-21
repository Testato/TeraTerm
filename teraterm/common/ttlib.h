/*
 * Copyright (C) 1994-1998 T. Teranishi
 * (C) 2006-2018 TeraTerm Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* useful routines */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#if defined(TTPCMN_DLL)
#if !defined(DllExport)
#define DllExport __declspec(dllexport)
#endif
#elif defined(TTPCMN_IMPORT)
#if !defined(DllExport)
#define DllExport __declspec(dllimport)
#endif
#else
#undef DllExport
#define DllExport	// direct link
#endif

BOOL GetFileNamePos(PCHAR PathName, int *DirLen, int *FNPos);
DllExport BOOL ExtractFileName(PCHAR PathName, PCHAR FileName, int destlen);
DllExport BOOL ExtractDirName(PCHAR PathName, PCHAR DirName);
void FitFileName(PCHAR FileName, int destlen, const char *DefExt);
void AppendSlash(PCHAR Path, int destlen);
void DeleteSlash(PCHAR Path);
void Str2Hex(PCHAR Str, PCHAR Hex, int Len, int MaxHexLen, BOOL ConvSP);
BYTE ConvHexChar(BYTE b);
int Hex2Str(PCHAR Hex, PCHAR Str, int MaxLen);
BOOL DoesFileExist(PCHAR FName);
BOOL DoesFolderExist(PCHAR FName);
long GetFSize(PCHAR FName);
long GetFMtime(PCHAR FName);
BOOL SetFMtime(PCHAR FName, DWORD mtime);
void uint2str(UINT i, PCHAR Str, int destlen, int len);
#ifdef WIN32
void QuoteFName(PCHAR FName);
#endif
int isInvalidFileNameChar(PCHAR FName);
#define deleteInvalidFileNameChar(name) replaceInvalidFileNameChar(name, 0)
DllExport void replaceInvalidFileNameChar(PCHAR FName, unsigned char c);
int isInvalidStrftimeChar(PCHAR FName);
void deleteInvalidStrftimeChar(PCHAR FName);
void ParseStrftimeFileName(PCHAR FName, int destlen);
void ConvFName(PCHAR HomeDir, PCHAR Temp, int templen, PCHAR DefExt, PCHAR FName, int destlen);
void RestoreNewLine(PCHAR Text);
BOOL GetNthString(PCHAR Source, int Nth, int Size, PCHAR Dest);
void GetNthNum(PCHAR Source, int Nth, int *Num);
int GetNthNum2(PCHAR Source, int Nth, int defval);
DllExport void GetDownloadFolder(char *dest, int destlen);
void WINAPI GetDefaultFName(const char *home, const char *file, char *dest, int destlen);
void GetDefaultSetupFName(char *home, char *dest, int destlen);
void GetUILanguageFile(char *buf, int buflen);
void GetOnOffEntryInifile(char *entry, char *buf, int buflen);
void get_lang_msg(const char *key, PCHAR buf, int buf_len, const char *def, const char *iniFile);
int get_lang_font(PCHAR key, HWND dlg, PLOGFONT logfont, HFONT *font, const char *iniFile);
DllExport BOOL doSelectFolder(HWND hWnd, char *path, int pathlen, char *def, char *msg);
DllExport void OutputDebugPrintf(const char *fmt, ...);
DllExport BOOL is_NT4();
int get_OPENFILENAME_SIZE();
DllExport BOOL IsWindows95();
DllExport BOOL IsWindowsMe();
DllExport BOOL IsWindowsNT4();
DllExport BOOL IsWindowsNTKernel();
DllExport BOOL IsWindows2000();
DllExport BOOL IsWindows2000OrLater();
DllExport BOOL IsWindowsVistaOrLater();
DllExport BOOL IsWindows7OrLater();
DllExport BOOL HasMultiMonitorSupport();
DllExport BOOL HasGetAdaptersAddresses();
DllExport BOOL HasDnsQuery();
DllExport BOOL HasBalloonTipSupport();
int KanjiCode2List(int lang, int kcode);
int List2KanjiCode(int lang, int kcode);
int KanjiCodeTranslate(int lang, int kcode);
DllExport char *mctimelocal(char *format, BOOL utc_flag);
char *strelapsed(DWORD start_time);

DllExport void b64encode(PCHAR dst, int dsize, PCHAR src, int len);
DllExport int b64decode(PCHAR dst, int dsize, PCHAR src);

DllExport PCHAR WINAPI GetParam(PCHAR buff, int size, PCHAR param);
DllExport void WINAPI DequoteParam(PCHAR dest, int dest_len, PCHAR src);
void WINAPI DeleteComment(PCHAR dest, int dest_size, PCHAR src);

void split_buffer(char *buffer, int delimiter, char **head, char **body);
BOOL GetPositionOnWindow(
	HWND hWnd, const POINT *point,
	BOOL *InWindow, BOOL *InClient, BOOL *InTitleBar);

#define CheckFlag(var, flag)	(((var) & (flag)) != 0)

#ifdef __cplusplus
}
#endif
