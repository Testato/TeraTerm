// Tera Term
// Copyright(C) 1994-1998 T. Teranishi
// All rights reserved.

// TTMACRO.EXE, misc routines

#ifdef __cplusplus
extern "C" {
#endif

#ifdef I18N
extern char UILanguageFile[MAX_PATH];
#endif

void CalcTextExtent(HDC DC, PCHAR Text, LPSIZE s);
void TTMGetDir(PCHAR Dir);
void TTMSetDir(PCHAR Dir);
void GetAbsPath(PCHAR FName);

#ifdef __cplusplus
}
#endif
