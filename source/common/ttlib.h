/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* useful routines */

#ifdef __cplusplus
extern "C" {
#endif

BOOL GetFileNamePos(PCHAR PathName, int far *DirLen, int far *FNPos);
BOOL ExtractFileName(PCHAR PathName, PCHAR FileName);
BOOL ExtractDirName(PCHAR PathName, PCHAR DirName);
void FitFileName(PCHAR FileName, PCHAR DefExt);
void AppendSlash(PCHAR Path);
void Str2Hex(PCHAR Str, PCHAR Hex, int Len, int MaxHexLen, BOOL ConvSP);
BYTE ConvHexChar(BYTE b);
int Hex2Str(PCHAR Hex, PCHAR Str, int MaxLen);
BOOL DoesFileExist(PCHAR FName);
long GetFSize(PCHAR FName);
void uint2str(UINT i, PCHAR Str, int len);
#ifdef WIN32
void QuoteFName(PCHAR FName);
#endif

#ifdef __cplusplus
}
#endif
