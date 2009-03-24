/* Tera Term */
/* TERATERM.EXE, IME interface */

#include "teraterm.h"
#include "tttypes.h"
#include <stdlib.h>
#include <string.h>
#include <imm.h>

#include "ttwinman.h"
#include "ttcommon.h"

#include "ttime.h"
#include "ttlib.h"

#ifdef TERATERM32
#ifndef _IMM_
  #define _IMM_

  typedef DWORD HIMC;

  typedef struct tagCOMPOSITIONFORM {
    DWORD dwStyle;
    POINT ptCurrentPos;
    RECT  rcArea;
  } COMPOSITIONFORM, *PCOMPOSITIONFORM, NEAR *NPCOMPOSITIONFORM, FAR *LPCOMPOSITIONFORM;
#endif //_IMM_

#define GCS_RESULTSTR 0x0800

typedef LONG (WINAPI *TImmGetCompositionString)
	(HIMC, DWORD, LPVOID, DWORD);
typedef HIMC (WINAPI *TImmGetContext)(HWND);
typedef BOOL (WINAPI *TImmReleaseContext)(HWND, HIMC);
typedef BOOL (WINAPI *TImmSetCompositionFont)(HIMC, LPLOGFONTA);
typedef BOOL (WINAPI *TImmSetCompositionWindow)(HIMC, LPCOMPOSITIONFORM);

static TImmGetCompositionString PImmGetCompositionString;
static TImmGetContext PImmGetContext;
static TImmReleaseContext PImmReleaseContext;
static TImmSetCompositionFont PImmSetCompositionFont;
static TImmSetCompositionWindow PImmSetCompositionWindow;

#else

typedef struct tagIMESTRUCT {
  UINT	 fnc;
  WPARAM wParam;
  UINT	 wCount;
  UINT	 dchSource;
  UINT	 dchDest;
  LPARAM lParam1;
  LPARAM lParam2;
  LPARAM lParam3;
} IMESTRUCT;
typedef IMESTRUCT FAR  *LPIMESTRUCT;

#define MCW_DEFAULT 0x00
#define MCW_WINDOW  0x02
#define IME_SETCONVERSIONWINDOW 0x08
#define IME_SETCONVERSIONFONTEX 0x19

typedef LRESULT (WINAPI *TSendIMEMessageEx)
  (HWND, LPARAM);

typedef BOOL (WINAPI *TWINNLSEnableIME)
  (HWND HWin, BOOL bEnable);

static TSendIMEMessageEx PSendIMEMessageEx;
static TWINNLSEnableIME PWINNLSEnableIME;

#endif

static HANDLE HIMEDLL = NULL;
static LOGFONT lfIME;


BOOL LoadIME()
{
  BOOL Err;
  PTTSet tempts;
#ifndef NO_I18N
  char uimsg[MAX_UIMSG];
#endif

#ifdef TERATERM32
  if (HIMEDLL != NULL) return TRUE;
  HIMEDLL = LoadLibrary("IMM32.DLL");
  if (HIMEDLL == NULL)
  {
#else
  if (HIMEDLL >= HINSTANCE_ERROR) return TRUE;
  HIMEDLL = LoadLibrary("WINNLS.DLL");
  if (HIMEDLL < HINSTANCE_ERROR)
  {
#endif
#ifndef NO_I18N
    strcpy(uimsg, "Tera Term: Error");
    get_lang_msg("MSG_TT_ERROR", uimsg, ts.UILanguageFile);
    strcpy(ts.UIMsg, "Can't use IME");
    get_lang_msg("MSG_USE_IME_ERROR", ts.UIMsg, ts.UILanguageFile);
    MessageBox(0,ts.UIMsg,uimsg,MB_ICONEXCLAMATION);
#else
    MessageBox(0,"Can't use IME",
      "Tera Term: Error",MB_ICONEXCLAMATION);
#endif
    WritePrivateProfileString("Tera Term","IME","off",ts.SetupFName);
    ts.UseIME = 0;
    tempts = (PTTSet)malloc(sizeof(TTTSet));
    if (tempts!=NULL)
    {
      GetDefaultSet(tempts);
      tempts->UseIME = 0;
      ChangeDefaultSet(tempts,NULL);
      free(tempts);
    }
    return FALSE;
  }

  Err = FALSE;

#ifdef TERATERM32
  PImmGetCompositionString = (TImmGetCompositionString)GetProcAddress(
    HIMEDLL, "ImmGetCompositionStringA");
  if (PImmGetCompositionString==NULL) Err = TRUE;

  PImmGetContext = (TImmGetContext)GetProcAddress(
    HIMEDLL, "ImmGetContext");
  if (PImmGetContext==NULL) Err = TRUE;

  PImmReleaseContext = (TImmReleaseContext)GetProcAddress(
    HIMEDLL, "ImmReleaseContext");
  if (PImmReleaseContext==NULL) Err = TRUE;

  PImmSetCompositionFont = (TImmSetCompositionFont)GetProcAddress(
    HIMEDLL, "ImmSetCompositionFontA");
  if (PImmSetCompositionFont==NULL) Err = TRUE;

  PImmSetCompositionWindow = (TImmSetCompositionWindow)GetProcAddress(
    HIMEDLL, "ImmSetCompositionWindow");
  if (PImmSetCompositionWindow==NULL) Err = TRUE;
#else
  PSendIMEMessageEx = (TSendIMEMessageEx)GetProcAddress(HIMEDLL, "SendIMEMessageEx");
  if (PSendIMEMessageEx==NULL) Err = TRUE;

  PWINNLSEnableIME = (TWINNLSEnableIME)GetProcAddress(HIMEDLL, "WINNLSEnableIME");
  if (PWINNLSEnableIME==NULL) Err = TRUE;
#endif	
  if ( Err )
  {
    FreeLibrary(HIMEDLL);
    HIMEDLL = NULL;
    return FALSE;
  }
  else
    return TRUE;
}

void FreeIME()
{
  HANDLE HTemp;
#ifndef TERATERM32
  MSG Msg;
#endif

#ifdef TERATERM32
  if (HIMEDLL==NULL) return;
#else
  if (HIMEDLL<HINSTANCE_ERROR) return;
#endif
  HTemp = HIMEDLL;
  HIMEDLL = NULL;

  /* position of conv. window -> default */
  SetConversionWindow(HVTWin,-1,0);
#ifdef TERATERM32
  Sleep(1); // for safety
#else
  PeekMessage(&Msg,NULL,0,0,PM_NOREMOVE);
#endif
  FreeLibrary(HTemp);
}

BOOL CanUseIME()
{
#ifdef TERATERM32
  return (HIMEDLL != NULL);
#else
  return (HIMEDLL >= HINSTANCE_ERROR);
#endif
}

void SetConversionWindow(HWND HWin, int X, int Y)
{
#ifdef TERATERM32
  HIMC	hIMC;
  COMPOSITIONFORM cf;
#else
  HANDLE HIME;
  HANDLE HIMElf;
  LPLOGFONT PIMElf;
  LPIMESTRUCT PIME;
#endif

#ifdef TERATERM32
  if (HIMEDLL == NULL) return;
// Adjust the position of conversion window
  hIMC = (*PImmGetContext)(HVTWin);
  if (X>=0)
  {
    cf.dwStyle = CFS_POINT;
    cf.ptCurrentPos.x = X;
    cf.ptCurrentPos.y = Y;
  }
  else
    cf.dwStyle = CFS_DEFAULT;
  (*PImmSetCompositionWindow)(hIMC,&cf);

  // Set font for the conversion window
  (*PImmSetCompositionFont)(hIMC,&lfIME);
  (*PImmReleaseContext)(HVTWin,hIMC);
#else
  if (HIMEDLL < HINSTANCE_ERROR) return;
  HIME = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE,sizeof(IMESTRUCT));

  /* Set position of conversion window */
  PIME = (LPIMESTRUCT)GlobalLock(HIME);
  (*PIME).fnc = IME_SETCONVERSIONWINDOW;
  if (X<0) (*PIME).wParam = MCW_DEFAULT;
      else (*PIME).wParam = MCW_WINDOW;
  (*PIME).lParam1 = MAKELONG(X,Y);
  GlobalUnlock(HIME);
  PSendIMEMessageEx(HWin, (LPARAM)HIME);

  /* Set font of conversion window */
  HIMElf = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE,sizeof(LOGFONT));
  PIMElf = (PLOGFONT)GlobalLock(HIMElf);
  memcpy(PIMElf,&lfIME,sizeof(LOGFONT));
  GlobalUnlock(HIMElf);

  PIME = (LPIMESTRUCT)GlobalLock(HIME);
  (*PIME).fnc = IME_SETCONVERSIONFONTEX;
  if (X<0) (*PIME).lParam1 = NULL;
      else (*PIME).lParam1 = (LPARAM)HIMElf;
  GlobalUnlock(HIME);
  PSendIMEMessageEx(HWin, (LPARAM)HIME);

  GlobalFree(HIME);
  GlobalFree(HIMElf);
#endif
}

void SetConversionLogFont(PLOGFONT lf)
{
  memcpy(&lfIME,lf,sizeof(LOGFONT));
}

#ifdef TERATERM32
HGLOBAL GetConvString(UINT wParam, LPARAM lParam)
{
	HIMC hIMC;
	HGLOBAL hstr = NULL;
	//LPSTR lpstr;
	wchar_t *lpstr;
	DWORD dwSize;

	if (HIMEDLL==NULL) return NULL;
	hIMC = (*PImmGetContext)(HVTWin);
	if (hIMC==0) return NULL;

	if ((lParam & GCS_RESULTSTR)==0) 
		goto skip;

	// Get the size of the result string.
	//dwSize = (*PImmGetCompositionString)(hIMC, GCS_RESULTSTR, NULL, 0);
	dwSize = ImmGetCompositionStringW(hIMC, GCS_RESULTSTR, NULL, 0);
	dwSize += sizeof(WCHAR);
	hstr = GlobalAlloc(GHND,dwSize);
	if (hstr != NULL)
	{
//		lpstr = (LPSTR)GlobalLock(hstr);
		lpstr = GlobalLock(hstr);
		if (lpstr != NULL)
		{
#if 0
			// Get the result strings that is generated by IME into lpstr.
			(*PImmGetCompositionString)
				(hIMC, GCS_RESULTSTR, lpstr, dwSize);
#else
			ImmGetCompositionStringW(hIMC, GCS_RESULTSTR, lpstr, dwSize);
#endif
			GlobalUnlock(hstr);
		}
		else {
			GlobalFree(hstr);
			hstr = NULL;
		}
	}

skip:
	(*PImmReleaseContext)(HVTWin, hIMC);
	return hstr;
}
#endif
