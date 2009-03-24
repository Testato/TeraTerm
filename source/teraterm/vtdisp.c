/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TERATERM.EXE, VT terminal display routines */
#include "teraterm.h"
#include "tttypes.h"
#include "string.h"

#include "ttwinman.h"
#include "ttime.h"
#include "ttdialog.h"
#include "ttcommon.h"

#include "vtdisp.h"

#include <locale.h>

#define CurWidth 2

int WinWidth, WinHeight;
static BOOL Active = FALSE;
static BOOL CompletelyVisible;
HFONT VTFont[AttrFontMask+1];
int FontHeight, FontWidth, ScreenWidth, ScreenHeight;
BOOL AdjustSize;
BOOL DontChangeSize=FALSE;
static int CRTWidth, CRTHeight;
int CursorX, CursorY;

// --- scrolling status flags
int WinOrgX, WinOrgY, NewOrgX, NewOrgY;

int NumOfLines, NumOfColumns;
int PageStart, BuffEnd;

static BOOL CursorOnDBCS = FALSE;
static LOGFONT VTlf;
static BOOL SaveWinSize = FALSE;
static int WinWidthOld, WinHeightOld;
static HBRUSH Background;
static COLORREF ANSIColor[16];
static int Dx[256];

// caret variables
static int CaretStatus;
static BOOL CaretEnabled = TRUE;

// ---- device context and status flags
static HDC VTDC = NULL; /* Device context for VT window */
static BYTE DCAttr, DCAttr2;
static BOOL DCReverse;
static HFONT DCPrevFont;

// scrolling
static int ScrollCount = 0;
static int dScroll = 0;
static int SRegionTop;
static int SRegionBottom;

void InitDisp()
{
  HDC TmpDC;
  int i;

  TmpDC = GetDC(NULL);

#ifndef NO_ANSI_COLOR_EXTENSION
  for (i = IdBack ; i <= IdFore+8 ; i++)
    ANSIColor[i] = ts.ANSIColor[i];
  if ((ts.ColorFlag & CF_USETEXTCOLOR)!=0) // use background color for "Black"
    ANSIColor[IdBack ]   = ts.VTColor[1];
  if ((ts.ColorFlag & CF_USETEXTCOLOR)!=0) // use text color for "white"
    ANSIColor[IdFore ]   = ts.VTColor[0];
#else /* NO_ANSI_COLOR_EXTENSION */
  if ((ts.ColorFlag & CF_USETEXTCOLOR)==0)
    ANSIColor[IdBack ]   = RGB(  0,  0,  0);
  else // use background color for "Black"
    ANSIColor[IdBack ]   = ts.VTColor[1];
  ANSIColor[IdRed  ]     = RGB(255,  0,  0);
  ANSIColor[IdGreen]     = RGB(  0,255,  0);
  ANSIColor[IdYellow]    = RGB(255,255,  0);
  ANSIColor[IdBlue]      = RGB(  0,  0,255);
  ANSIColor[IdMagenta]   = RGB(255,  0,255);
  ANSIColor[IdCyan]      = RGB(  0,255,255);
  if ((ts.ColorFlag & CF_USETEXTCOLOR)==0)
    ANSIColor[IdFore ]   = RGB(255,255,255);
  else // use text color for "white"
    ANSIColor[IdFore ]   = ts.VTColor[0];

  ANSIColor[IdBack+8]    = RGB(128,128,128);
  ANSIColor[IdRed+8]     = RGB(128,  0,  0);
  ANSIColor[IdGreen+8]   = RGB(  0,128,  0);
  ANSIColor[IdYellow+8]	 = RGB(128,128,  0);
  ANSIColor[IdBlue+8]    = RGB(  0,  0,128);
  ANSIColor[IdMagenta+8] = RGB(128,  0,128);
  ANSIColor[IdCyan+8]    = RGB(  0,128,128);
  ANSIColor[IdFore+8]    = RGB(192,192,192);
#endif /* NO_ANSI_COLOR_EXTENSION */

  for (i = IdBack ; i <= IdFore+8 ; i++)
    ANSIColor[i] = GetNearestColor(TmpDC, ANSIColor[i]);

  /* background paintbrush */
  Background = CreateSolidBrush(ts.VTColor[1]);
  /* CRT width & height */
  CRTWidth = GetDeviceCaps(TmpDC,HORZRES);
  CRTHeight = GetDeviceCaps(TmpDC,VERTRES);

  ReleaseDC(NULL, TmpDC);

  if ((ts.VTPos.x > CRTWidth) || (ts.VTPos.y > CRTHeight))
  {
    ts.VTPos.x = CW_USEDEFAULT;
    ts.VTPos.y = CW_USEDEFAULT;
  }

  if ((ts.TEKPos.x > CRTWidth) || (ts.TEKPos.y > CRTHeight))
  {
    ts.TEKPos.x = CW_USEDEFAULT;
    ts.TEKPos.y = CW_USEDEFAULT;
  }
}

void EndDisp()
{
  int i, j;

  if (VTDC!=NULL) DispReleaseDC();

  /* Delete fonts */
  for (i = 0 ; i <= AttrFontMask; i++)
  {
    for (j = i+1 ; j <= AttrFontMask ; j++)
      if (VTFont[j]==VTFont[i])
        VTFont[j] = 0;
    if (VTFont[i]!=0) DeleteObject(VTFont[i]);
  }

  if (Background!=0)
  {
	DeleteObject(Background);
	Background = 0;
  }
}

void DispReset()
{
  /* Cursor */
  CursorX = 0;
  CursorY = 0;

  /* Scroll status */
  ScrollCount = 0;
  dScroll = 0;

  if (IsCaretOn()) CaretOn();
  DispEnableCaret(TRUE); // enable caret
}

void DispConvWinToScreen
  (int Xw, int Yw, int *Xs, int *Ys, PBOOL Right)
// Converts window coordinate to screen cordinate
//   Xs: horizontal position in window coordinate (pixels)
//   Ys: vertical
//  Output
//	 Xs, Ys: screen coordinate
//   Right: TRUE if the (Xs,Ys) is on the right half of
//			 a character cell.
{
  if (Xs!=NULL)
	*Xs = Xw / FontWidth + WinOrgX;
  *Ys = Yw / FontHeight + WinOrgY;
  if ((Xs!=NULL) && (Right!=NULL))
    *Right = (Xw - (*Xs-WinOrgX)*FontWidth) >= FontWidth/2;
}

void SetLogFont()
{
  memset(&VTlf, 0, sizeof(LOGFONT));
  VTlf.lfWeight = FW_NORMAL;
  VTlf.lfItalic = 0;
  VTlf.lfUnderline = 0;
  VTlf.lfStrikeOut = 0;
  VTlf.lfWidth = ts.VTFontSize.x;
  VTlf.lfHeight = ts.VTFontSize.y;
  VTlf.lfCharSet = ts.VTFontCharSet;
  VTlf.lfOutPrecision  = OUT_CHARACTER_PRECIS;
  VTlf.lfClipPrecision = CLIP_CHARACTER_PRECIS;
  VTlf.lfQuality       = DEFAULT_QUALITY;
  VTlf.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
  strcpy(VTlf.lfFaceName,ts.VTFont);
}

void ChangeFont()
{
  int i, j;
  TEXTMETRIC Metrics;
  HDC TmpDC;

  /* Delete Old Fonts */
  for (i = 0 ; i <= AttrFontMask ; i++)
  {
    for (j = i+1 ; j <= AttrFontMask ; j++)
      if (VTFont[j]==VTFont[i])
        VTFont[j] = 0;
    if (VTFont[i]!=0)
      DeleteObject(VTFont[i]);
  }

  /* Normal Font */
  SetLogFont();
  VTFont[0] = CreateFontIndirect(&VTlf);

  /* set IME font */
  SetConversionLogFont(&VTlf);

  TmpDC = GetDC(HVTWin);

  SelectObject(TmpDC, VTFont[0]);
  GetTextMetrics(TmpDC, &Metrics); 
  FontWidth = Metrics.tmAveCharWidth + ts.FontDW;
  FontHeight = Metrics.tmHeight + ts.FontDH;

  ReleaseDC(HVTWin,TmpDC);

  /* Underline */
  VTlf.lfUnderline = 1;
  VTFont[AttrUnder] = CreateFontIndirect(&VTlf);

  if (ts.EnableBold>0)
  {
    /* Bold */
    VTlf.lfUnderline = 0;
    VTlf.lfWeight = FW_BOLD;
    VTFont[AttrBold] = CreateFontIndirect(&VTlf);
    /* Bold + Underline */
    VTlf.lfUnderline = 1;
    VTFont[AttrBold | AttrUnder] = CreateFontIndirect(&VTlf);
  }
  else {
    VTFont[AttrBold] = VTFont[AttrDefault];
    VTFont[AttrBold | AttrUnder] = VTFont[AttrUnder];
  }

  /* Special font */
  VTlf.lfWeight = FW_NORMAL;
  VTlf.lfUnderline = 0;
  VTlf.lfWidth = FontWidth + 1; /* adjust width */
  VTlf.lfHeight = FontHeight;
  VTlf.lfCharSet = SYMBOL_CHARSET;

  strcpy(VTlf.lfFaceName,"Tera Special");
  VTFont[AttrSpecial] = CreateFontIndirect(&VTlf);
  VTFont[AttrSpecial | AttrBold] = VTFont[AttrSpecial];
  VTFont[AttrSpecial | AttrUnder] = VTFont[AttrSpecial];
  VTFont[AttrSpecial | AttrBold | AttrUnder] = VTFont[AttrSpecial];

  SetLogFont();

  for (i = 0 ; i <= 255; i++)
    Dx[i] = FontWidth;
}

void ResetIME()
{
  /* reset language for communication */
  cv.Language = ts.Language;

  /* reset IME */
  if (ts.Language==IdJapanese)
  {
    if (ts.UseIME==0)
      FreeIME();
    else if (! LoadIME())
      ts.UseIME = 0;

	if (ts.UseIME>0)
	{
		if (ts.IMEInline>0)
			SetConversionLogFont(&VTlf);
		else
			SetConversionWindow(HVTWin,-1,0);
	}
  }
  else
    FreeIME();

  if (IsCaretOn()) CaretOn();
}

void ChangeCaret()
{
  UINT T;

  if (! Active) return;
  if (CaretEnabled)
  {
	DestroyCaret();
	switch (ts.CursorShape) {
	  case IdVCur:
		CreateCaret(HVTWin, 0, CurWidth, FontHeight);
		break;
	  case IdHCur:
		CreateCaret(HVTWin, 0, FontWidth, CurWidth);
		break;
	}
	CaretStatus = 1;
  }
  CaretOn();
  if (CaretEnabled &&
	  (ts.NonblinkingCursor!=0))
  {
    T = GetCaretBlinkTime() * 2 / 3;
    SetTimer(HVTWin,IdCaretTimer,T,NULL);
  }
}

void CaretOn()
// Turn on the cursor
{
  int CaretX, CaretY, H;

  if (! Active) return;

  CaretX = (CursorX-WinOrgX)*FontWidth;
  CaretY = (CursorY-WinOrgY)*FontHeight;

  if ((ts.Language==IdJapanese) &&
      CanUseIME() && (ts.IMEInline>0))
    /* set IME conversion window pos. & font */
    SetConversionWindow(HVTWin,CaretX,CaretY);

  if (! CaretEnabled) return;

  if (ts.CursorShape!=IdVCur)
  {
    if (ts.CursorShape==IdHCur)
    {
     CaretY = CaretY+FontHeight-CurWidth;
     H = CurWidth;
    }
    else H = FontHeight;

    DestroyCaret();
    if (CursorOnDBCS)
      CreateCaret(HVTWin, 0, FontWidth*2, H); /* double width caret */
    else
      CreateCaret(HVTWin, 0, FontWidth, H); /* single width caret */
    CaretStatus = 1;
  }

  SetCaretPos(CaretX,CaretY);

  while (CaretStatus > 0)
  {
    ShowCaret(HVTWin);
    CaretStatus--;
  }

}

void CaretOff()
{
  if (! Active) return;
  if (CaretStatus == 0)
  {
    HideCaret(HVTWin);
    CaretStatus++;
  }
}

void DispDestroyCaret()
{
  DestroyCaret();
  if (ts.NonblinkingCursor!=0)
	KillTimer(HVTWin,IdCaretTimer);
}

BOOL IsCaretOn()
// check if caret is on
{
  return (Active && (CaretStatus==0));
}

void DispEnableCaret(BOOL On)
{
  if (! On) CaretOff();
  CaretEnabled = On;
}

BOOL IsCaretEnabled()
{
  return CaretEnabled;
}

void DispSetCaretWidth(BOOL DW)
{
  /* TRUE if cursor is on a DBCS character */
  CursorOnDBCS = DW;
}

void DispChangeWinSize(int Nx, int Ny)
{
  LONG W,H,dW,dH;
  RECT R;

  if (SaveWinSize)
  {
    WinWidthOld = WinWidth;
    WinHeightOld = WinHeight;
    SaveWinSize = FALSE;
  }
  else {
    WinWidthOld = NumOfColumns;
    WinHeightOld = NumOfLines;
  }

  WinWidth = Nx;
  WinHeight = Ny;

  ScreenWidth = WinWidth*FontWidth;
  ScreenHeight = WinHeight*FontHeight;

  AdjustScrollBar();

  GetWindowRect(HVTWin,&R);
  W = R.right-R.left;
  H = R.bottom-R.top;
  GetClientRect(HVTWin,&R);
  dW = ScreenWidth - R.right + R.left;
  dH = ScreenHeight - R.bottom + R.top;
  
  if ((dW!=0) || (dH!=0))
  {
    AdjustSize = TRUE;
    SetWindowPos(HVTWin,HWND_TOP,0,0,W+dW,H+dH,SWP_NOMOVE);
  }
  else
    InvalidateRect(HVTWin,NULL,FALSE);
}

void ResizeWindow(int x, int y, int w, int h, int cw, int ch)
{
  int dw,dh, NewX, NewY;
  POINT Point;

  if (! AdjustSize) return;
  dw = ScreenWidth - cw;
  dh = ScreenHeight - ch;
  if ((dw!=0) || (dh!=0))
    SetWindowPos(HVTWin,HWND_TOP,x,y,w+dw,h+dh,SWP_NOMOVE);
  else {
    AdjustSize = FALSE;

    NewX = x;
    NewY = y;
    if (x+w > CRTWidth)
    {
      NewX = CRTWidth-w;
      if (NewX < 0) NewX = 0;
    }
    if (y+h > CRTHeight)
    {
      NewY = CRTHeight-h;
      if (NewY < 0) NewY = 0;
    }
    if ((NewX!=x) || (NewY!=y))
      SetWindowPos(HVTWin,HWND_TOP,NewX,NewY,w,h,SWP_NOSIZE);

    Point.x = 0;
    Point.y = ScreenHeight;
    ClientToScreen(HVTWin,&Point);
    CompletelyVisible = (Point.y <= CRTHeight);
    if (IsCaretOn()) CaretOn();
  }
}

void PaintWindow(HDC PaintDC, RECT PaintRect, BOOL fBkGnd,
		 int* Xs, int* Ys, int* Xe, int* Ye)
//  Paint window with background color &
//  convert paint region from window coord. to screen coord.
//  Called from WM_PAINT handler
//    PaintRect: Paint region in window coordinate
//    Return:
//	*Xs, *Ys: upper left corner of the region
//		    in screen coord.
//	*Xe, *Ye: lower right
{
  if (VTDC!=NULL)
	DispReleaseDC();
  VTDC = PaintDC;
  DCPrevFont = SelectObject(VTDC, VTFont[0]);
  DispInitDC();
  if (fBkGnd)
    FillRect(VTDC, &PaintRect,Background);

  *Xs = PaintRect.left / FontWidth + WinOrgX;
  *Ys = PaintRect.top / FontHeight + WinOrgY;
  *Xe = (PaintRect.right-1) / FontWidth + WinOrgX;
  *Ye = (PaintRect.bottom-1) / FontHeight + WinOrgY;
}

void DispEndPaint()
{
  if (VTDC==NULL) return;
  SelectObject(VTDC,DCPrevFont);
  VTDC = NULL;
}

void DispClearWin()
{
  InvalidateRect(HVTWin,NULL,FALSE);

  ScrollCount = 0;
  dScroll = 0;
  if (WinHeight > NumOfLines)
    DispChangeWinSize(NumOfColumns,NumOfLines);
  else {
    if ((NumOfLines==WinHeight) && (ts.EnableScrollBuff>0))
    {
      SetScrollRange(HVTWin,SB_VERT,0,1,FALSE);
    }
    else 
      SetScrollRange(HVTWin,SB_VERT,0,NumOfLines-WinHeight,FALSE);

    SetScrollPos(HVTWin,SB_HORZ,0,TRUE);
    SetScrollPos(HVTWin,SB_VERT,0,TRUE);
  }
  if (IsCaretOn()) CaretOn();
}

void DispChangeBackground()
{
  DispReleaseDC();
  if (Background != NULL) DeleteObject(Background);
  Background = CreateSolidBrush(ts.VTColor[1]);

  InvalidateRect(HVTWin,NULL,TRUE);
}

void DispChangeWin()
{
  /* Change window caption */
  ChangeTitle();

  /* Menu bar / Popup menu */
  SwitchMenu();

  SwitchTitleBar();

  /* Change caret shape */
  ChangeCaret();

  if ((ts.ColorFlag & CF_USETEXTCOLOR)==0)
  {
#ifndef NO_ANSI_COLOR_EXTENSION
    ANSIColor[IdFore ]   = ts.ANSIColor[IdFore ];
    ANSIColor[IdBack ]   = ts.ANSIColor[IdBack ];
#else /* NO_ANSI_COLOR_EXTENSION */
    ANSIColor[IdFore ]   = RGB(255,255,255);
    ANSIColor[IdBack ]   = RGB(  0,  0,  0);
#endif /* NO_ANSI_COLOR_EXTENSION */
  }
  else { // use text (background) color for "white (black)"
    ANSIColor[IdFore ]   = ts.VTColor[0];
    ANSIColor[IdBack ]   = ts.VTColor[1];
  }

  /* change background color */
  DispChangeBackground();
}

void DispInitDC()
{
  if (VTDC==NULL)
  {
    VTDC = GetDC(HVTWin);
    DCPrevFont = SelectObject(VTDC, VTFont[0]);
  }
  else
    SelectObject(VTDC, VTFont[0]);
  SetTextColor(VTDC, ts.VTColor[0]);
  SetBkColor(VTDC, ts.VTColor[1]);
  SetBkMode(VTDC,OPAQUE);
  DCAttr = AttrDefault;
  DCAttr2 = AttrDefault2;
  DCReverse = FALSE;
}

void DispReleaseDC()
{
  if (VTDC==NULL) return;
  SelectObject(VTDC, DCPrevFont);
  ReleaseDC(HVTWin,VTDC);
  VTDC = NULL;
}

void DispSetupDC(BYTE Attr, BYTE Attr2, BOOL Reverse)
// Setup device context
//   Attr, Attr2: character attribute 1 & 2
//   Reverse: true if text is selected (reversed) by mouse
{
  COLORREF TextColor, BackColor;
  int i, j;

  if (VTDC==NULL)  DispInitDC();

  if ((DCAttr==Attr) && (DCAttr2==Attr2) &&
      (DCReverse==Reverse)) return;
  DCAttr = Attr;
  DCAttr2 = Attr2;
  DCReverse = Reverse;
     
  SelectObject(VTDC, VTFont[Attr & AttrFontMask]);

  if ((ts.ColorFlag & CF_FULLCOLOR) == 0)
  {
	if ((Attr & AttrBlink) != 0)
	{
	  TextColor = ts.VTBlinkColor[0];
	  BackColor = ts.VTBlinkColor[1];
	}
	else if ((Attr & AttrBold) != 0)
	{
	  TextColor = ts.VTBoldColor[0];
	  BackColor = ts.VTBoldColor[1];
	}
	else {
	  if ((Attr2 & Attr2Fore) != 0)
	  {
		j = Attr2 & Attr2ForeMask;
		TextColor = ANSIColor[j];
	  }
	  else
		TextColor = ts.VTColor[0];

	  if ((Attr2 & Attr2Back) != 0)
	  {
		j = (Attr2 & Attr2BackMask) >> SftAttrBack;
		BackColor = ANSIColor[j];
	  }
	  else
		BackColor = ts.VTColor[1];
	}
  }
  else { // full color
	if ((Attr2 & Attr2Fore) != 0)
	{
	  if ((Attr & AttrBold) != 0)
		i = 0;
	  else
		i = 8;
	  j = Attr2 & Attr2ForeMask;
	  if (j==0)
		j = 8 - i + j;
	  else
		j = i + j;
	  TextColor = ANSIColor[j];
	}
	else if ((Attr & AttrBlink) != 0)
	  TextColor = ts.VTBlinkColor[0];
	else if ((Attr & AttrBold) != 0)
	  TextColor = ts.VTBoldColor[0];          
	else
	  TextColor = ts.VTColor[0];

	if ((Attr2 & Attr2Back) != 0)
	{
	  if ((Attr & AttrBlink) != 0)
		i = 0;
	  else
		i = 8;
	  j = (Attr2 & Attr2BackMask) >> SftAttrBack;
	  if (j==0)
		j = 8 - i + j;
	  else
		j = i + j;
	  BackColor = ANSIColor[j];
	}
	else if ((Attr & AttrBlink) != 0)
	  BackColor = ts.VTBlinkColor[1];
	else if ((Attr & AttrBold) != 0)
	  BackColor = ts.VTBoldColor[1];          
	else
	  BackColor = ts.VTColor[1];
  }

  if (Reverse != ((Attr & AttrReverse) != 0))
  {
    SetTextColor(VTDC,BackColor);
    SetBkColor(  VTDC,TextColor);
  }
  else {
    SetTextColor(VTDC,TextColor);
    SetBkColor(  VTDC,BackColor);
  }
}

#if 1
// 当面はこちらの関数を使う。(2004.11.4 yutaka)
void DispStr(PCHAR Buff, int Count, int Y, int* X)
// Display a string
//   Buff: points the string
//   Y: vertical position in window cordinate
//  *X: horizontal position
// Return:
//  *X: horizontal position shifted by the width of the string
{
  RECT RText;

  if ((ts.Language==IdRussian) &&
      (ts.RussClient!=ts.RussFont))
    RussConvStr(ts.RussClient,ts.RussFont,Buff,Count);

  RText.top = Y;
  RText.bottom = Y+FontHeight;
  RText.left = *X;
  RText.right = *X + Count*FontWidth;
  ExtTextOut(VTDC,*X+ts.FontDX,Y+ts.FontDY,
             ETO_CLIPPED | ETO_OPAQUE,
             &RText,Buff,Count,&Dx[0]);
  *X = RText.right;

  if ((ts.Language==IdRussian) &&
      (ts.RussClient!=ts.RussFont))
    RussConvStr(ts.RussFont,ts.RussClient,Buff,Count);
}

#else
void DispStr(PCHAR Buff, int Count, int Y, int* X)
// Display a string
//   Buff: points the string
//   Y: vertical position in window cordinate
//  *X: horizontal position
// Return:
//  *X: horizontal position shifted by the width of the string
{
	RECT RText;
	wchar_t *wc;
	int len, wclen;
	CHAR ch;

#if 0
#include <crtdbg.h>
	_CrtSetBreakAlloc(52);
	Buff[0] = 0x82;
	Buff[1] = 0xe4;
	Buff[2] = 0x82;
	Buff[3] = 0xbd;
	Buff[4] = 0x82;
	Buff[5] = 0xa9;
	Count = 6;
#endif

	setlocale(LC_ALL, ts.Locale);

	ch = Buff[Count];
	Buff[Count] = 0;
	len = mbstowcs(NULL, Buff, 0);

	wc = malloc(sizeof(wchar_t) * (len + 1));
	if (wc == NULL)
		return;
	wclen = mbstowcs(wc, Buff, len + 1);
	Buff[Count] = ch;

	if ((ts.Language==IdRussian) &&
		(ts.RussClient!=ts.RussFont))
		RussConvStr(ts.RussClient,ts.RussFont,Buff,Count);

	RText.top = Y;
	RText.bottom = Y+FontHeight;
	RText.left = *X;
	RText.right = *X + Count*FontWidth; // 

	// Unicodeで出力する。
#if 1
	// UTF-8環境において、tcshがEUC出力した場合、画面に何も表示されないことがある。
	// マウスでドラッグしたり、ログファイルへ保存してみると、文字化けした文字列を
	// 確認することができる。(2004.8.6 yutaka)
	ExtTextOutW(VTDC,*X+ts.FontDX,Y+ts.FontDY,
		ETO_CLIPPED | ETO_OPAQUE,
		&RText, wc, wclen, NULL);
//		&RText, wc, wclen, &Dx[0]);
#else
	TextOutW(VTDC, *X+ts.FontDX, Y+ts.FontDY, wc, wclen);

#endif

	*X = RText.right;

	if ((ts.Language==IdRussian) &&
		(ts.RussClient!=ts.RussFont))
		RussConvStr(ts.RussFont,ts.RussClient,Buff,Count);

	free(wc);
}
#endif


void DispEraseCurToEnd(int YEnd)
{
  RECT R;

  if (VTDC==NULL) DispInitDC();
  R.left = 0;
  R.right = ScreenWidth;
  R.top = (CursorY+1-WinOrgY)*FontHeight;
  R.bottom = (YEnd+1-WinOrgY)*FontHeight;
  FillRect(VTDC,&R,Background);
  R.left = (CursorX-WinOrgX)*FontWidth;
  R.bottom = R.top;
  R.top = R.bottom-FontHeight;
  FillRect(VTDC,&R,Background);
}

void DispEraseHomeToCur(int YHome)
{
  RECT R;

  if (VTDC==NULL) DispInitDC();
  R.left = 0;
  R.right = ScreenWidth;
  R.top = (YHome-WinOrgY)*FontHeight;
  R.bottom = (CursorY-WinOrgY)*FontHeight;
  FillRect(VTDC,&R,Background);
  R.top = R.bottom;
  R.bottom = R.top + FontHeight;
  R.right = (CursorX+1-WinOrgX)*FontWidth;
  FillRect(VTDC,&R,Background);
}

void DispEraseCharsInLine(int XStart, int Count)
{
  RECT R;

  if (VTDC==NULL) DispInitDC();
  R.top = (CursorY-WinOrgY)*FontHeight;
  R.bottom = R.top+FontHeight;
  R.left = (XStart-WinOrgX)*FontWidth;
  R.right = R.left + Count * FontWidth;
  FillRect(VTDC,&R,Background);
}

BOOL DispDeleteLines(int Count, int YEnd)
// return value:
//	 TRUE  - screen is successfully updated
//   FALSE - screen is not updated
{
  RECT R;

  if (Active && CompletelyVisible &&
      (YEnd+1-WinOrgY <= WinHeight))
  {
	R.left = 0;
	R.right = ScreenWidth;
	R.top = (CursorY-WinOrgY)*FontHeight;
	R.bottom = (YEnd+1-WinOrgY)*FontHeight;
	ScrollWindow(HVTWin,0,-FontHeight*Count,&R,&R);
	UpdateWindow(HVTWin);
	return TRUE;
  }
  else
	return FALSE;
}

BOOL DispInsertLines(int Count, int YEnd)
// return value:
//	 TRUE  - screen is successfully updated
//   FALSE - screen is not updated
{
  RECT R;

  if (Active && CompletelyVisible &&
      (CursorY >= WinOrgY))
  {
    R.left = 0;
    R.right = ScreenWidth;
    R.top = (CursorY-WinOrgY)*FontHeight;
    R.bottom = (YEnd+1-WinOrgY)*FontHeight;
    ScrollWindow(HVTWin,0,FontHeight*Count,&R,&R);
	UpdateWindow(HVTWin);
    return TRUE;
  }
  else
	return FALSE;
}

BOOL IsLineVisible(int* X, int* Y)
//  Check the visibility of a line
//	called from UpdateStr()
//    *X, *Y: position of a character in the line. screen coord.
//    Return: TRUE if the line is visible.
//	*X, *Y:
//	  If the line is visible
//	    position of the character in window coord.
//	  Otherwise
//	    no change. same as input value.
{
  if ((dScroll != 0) &&
      (*Y>=SRegionTop) &&
      (*Y<=SRegionBottom))
  {
    *Y = *Y + dScroll;
    if ((*Y<SRegionTop) || (*Y>SRegionBottom))
      return FALSE;
  }

  if ((*Y<WinOrgY) ||
      (*Y>=WinOrgY+WinHeight))
    return FALSE;

  /* screen coordinate -> window coordinate */
  *X = (*X-WinOrgX)*FontWidth;
  *Y = (*Y-WinOrgY)*FontHeight;
  return TRUE;
}

//-------------- scrolling functions --------------------

void AdjustScrollBar() /* called by ChangeWindowSize() */
{
  LONG XRange, YRange;
  int ScrollPosX, ScrollPosY;

  if (NumOfColumns-WinWidth>0)
    XRange = NumOfColumns-WinWidth;
  else
    XRange = 0;

  if (BuffEnd-WinHeight>0)
    YRange = BuffEnd-WinHeight;
  else
    YRange = 0;

  ScrollPosX = GetScrollPos(HVTWin,SB_HORZ);
  ScrollPosY = GetScrollPos(HVTWin,SB_VERT);
  if (ScrollPosX > XRange)
    ScrollPosX = XRange;
  if (ScrollPosY > YRange)
    ScrollPosY = YRange;

  WinOrgX = ScrollPosX;
  WinOrgY = ScrollPosY-PageStart;
  NewOrgX = WinOrgX;
  NewOrgY = WinOrgY;

  DontChangeSize = TRUE;

  SetScrollRange(HVTWin,SB_HORZ,0,XRange,FALSE);

  if ((YRange == 0) && (ts.EnableScrollBuff>0))
  {
    SetScrollRange(HVTWin,SB_VERT,0,1,FALSE);
  }
  else {
    SetScrollRange(HVTWin,SB_VERT,0,YRange,FALSE);
  }

  SetScrollPos(HVTWin,SB_HORZ,ScrollPosX,TRUE);
  SetScrollPos(HVTWin,SB_VERT,ScrollPosY,TRUE);

  DontChangeSize = FALSE;  
}

void DispScrollToCursor(int CurX, int CurY)
{
  if (CurX < NewOrgX)
    NewOrgX = CurX;
  else if (CurX >= NewOrgX+WinWidth)
    NewOrgX = CurX + 1 - WinWidth;

  if (CurY < NewOrgY)
    NewOrgY = CurY;
  else if (CurY >= NewOrgY+WinHeight)
    NewOrgY = CurY + 1 - WinHeight;
}

void DispScrollNLines(int Top, int Bottom, int Direction)
//  Scroll a region of the window by Direction lines
//    updates window if necessary
//  Top: top line of scroll region
//  Bottom: bottom line
//  Direction: +: forward, -: backward
{
  if ((dScroll*Direction <0) ||
      (dScroll*Direction >0) &&
      ((SRegionTop!=Top) ||
       (SRegionBottom!=Bottom)))
    DispUpdateScroll();
  SRegionTop = Top;
  SRegionBottom = Bottom;
  dScroll = dScroll + Direction;
  if (Direction>0)
    DispCountScroll(Direction);
  else
    DispCountScroll(-Direction);
}

void DispCountScroll(int n)
{
  ScrollCount = ScrollCount + n;
  if (ScrollCount>=ts.ScrollThreshold) DispUpdateScroll();
}

void DispUpdateScroll()
{
  int d;
  RECT R;

  ScrollCount = 0;

  /* Update partial scroll */
  if (dScroll != 0)
  {
    d = dScroll * FontHeight;
    R.left = 0;
    R.right = ScreenWidth;
    R.top = (SRegionTop-WinOrgY)*FontHeight;
    R.bottom = (SRegionBottom+1-WinOrgY)*FontHeight;
    ScrollWindow(HVTWin,0,-d,&R,&R);
    if ((SRegionTop==0) && (dScroll>0))
	{ // update scroll bar if BuffEnd is changed
	  if ((BuffEnd==WinHeight) &&
          (ts.EnableScrollBuff>0))
        SetScrollRange(HVTWin,SB_VERT,0,1,TRUE);
      else
        SetScrollRange(HVTWin,SB_VERT,0,BuffEnd-WinHeight,FALSE);
      SetScrollPos(HVTWin,SB_VERT,WinOrgY+PageStart,TRUE);
	}
    dScroll = 0;
  }

  /* Update normal scroll */
  if (NewOrgX < 0) NewOrgX = 0;
  if (NewOrgX>NumOfColumns-WinWidth)
    NewOrgX = NumOfColumns-WinWidth;
  if (NewOrgY < -PageStart) NewOrgY = -PageStart;
  if (NewOrgY>BuffEnd-WinHeight-PageStart)
    NewOrgY = BuffEnd-WinHeight-PageStart;

  if ((NewOrgX==WinOrgX) &&
      (NewOrgY==WinOrgY)) return;

  if (NewOrgX==WinOrgX)
  {
    d = (NewOrgY-WinOrgY) * FontHeight;
    ScrollWindow(HVTWin,0,-d,NULL,NULL);
  }
  else if (NewOrgY==WinOrgY)
  {
    d = (NewOrgX-WinOrgX) * FontWidth;
    ScrollWindow(HVTWin,-d,0,NULL,NULL);
  }
  else
    InvalidateRect(HVTWin,NULL,TRUE);

  /* Update scroll bars */
  if (NewOrgX!=WinOrgX)
    SetScrollPos(HVTWin,SB_HORZ,NewOrgX,TRUE);

  if (NewOrgY!=WinOrgY)
  {
    if ((BuffEnd==WinHeight) &&
        (ts.EnableScrollBuff>0))
      SetScrollRange(HVTWin,SB_VERT,0,1,TRUE);
    else
      SetScrollRange(HVTWin,SB_VERT,0,BuffEnd-WinHeight,FALSE);
    SetScrollPos(HVTWin,SB_VERT,NewOrgY+PageStart,TRUE);
  }

  WinOrgX = NewOrgX;
  WinOrgY = NewOrgY;

  if (IsCaretOn()) CaretOn();
}

void DispScrollHomePos()
{
  NewOrgX = 0;
  NewOrgY = 0;
  DispUpdateScroll();
}

void DispAutoScroll(POINT p)
{
  int X, Y;

  X = (p.x + FontWidth / 2) / FontWidth;
  Y = p.y / FontHeight;
  if (X<0)
    NewOrgX = WinOrgX + X;
  else if (X>=WinWidth)
    NewOrgX = NewOrgX + X - WinWidth + 1;
  if (Y<0)
    NewOrgY = WinOrgY + Y;
  else if (Y>=WinHeight)
    NewOrgY = NewOrgY + Y - WinHeight + 1;

  DispUpdateScroll();
}

void DispHScroll(int Func, int Pos)
{
  switch (Func) {
	case SCROLL_BOTTOM:
      NewOrgX = NumOfColumns-WinWidth;
      break;
	case SCROLL_LINEDOWN: NewOrgX = WinOrgX + 1; break;
	case SCROLL_LINEUP: NewOrgX = WinOrgX - 1; break;
	case SCROLL_PAGEDOWN:
      NewOrgX = WinOrgX + WinWidth - 1;
      break;
	case SCROLL_PAGEUP:
      NewOrgX = WinOrgX - WinWidth + 1;
      break;
	case SCROLL_POS: NewOrgX = Pos; break;
	case SCROLL_TOP: NewOrgX = 0; break;
  }
  DispUpdateScroll();
}

void DispVScroll(int Func, int Pos)
{
  switch (Func) {
	case SCROLL_BOTTOM:
      NewOrgY = BuffEnd-WinHeight-PageStart;
      break;
	case SCROLL_LINEDOWN: NewOrgY = WinOrgY + 1; break;
	case SCROLL_LINEUP: NewOrgY = WinOrgY - 1; break;
	case SCROLL_PAGEDOWN:
      NewOrgY = WinOrgY + WinHeight - 1;
      break;
	case SCROLL_PAGEUP:
      NewOrgY = WinOrgY - WinHeight + 1;
      break;
	case SCROLL_POS: NewOrgY = Pos-PageStart; break;
	case SCROLL_TOP: NewOrgY = -PageStart; break;
  }
  DispUpdateScroll();
}

//-------------- end of scrolling functions --------

void DispSetupFontDlg()
//  Popup the Setup Font dialogbox and
//  reset window
{
  BOOL Ok;

  if (! LoadTTDLG()) return;
  SetLogFont();
  Ok = ChooseFontDlg(HVTWin,&VTlf,&ts);
  FreeTTDLG();
  if (! Ok) return;

  strcpy(ts.VTFont,VTlf.lfFaceName);
  ts.VTFontSize.x = VTlf.lfWidth;
  ts.VTFontSize.y = VTlf.lfHeight;
  ts.VTFontCharSet = VTlf.lfCharSet;

  ChangeFont();

  DispChangeWinSize(WinWidth,WinHeight);

  ChangeCaret();
}

void DispRestoreWinSize()
//  Restore window size by double clik on caption bar
{
  if (ts.TermIsWin>0) return;

  if ((WinWidth==NumOfColumns) && (WinHeight==NumOfLines))
  {
    if (WinWidthOld > NumOfColumns)
      WinWidthOld = NumOfColumns;
    if (WinHeightOld > BuffEnd)
      WinHeightOld = BuffEnd;
    DispChangeWinSize(WinWidthOld,WinHeightOld);
  }
  else {
    SaveWinSize = TRUE;
    DispChangeWinSize(NumOfColumns,NumOfLines);
  }
}

void DispSetWinPos()
{
  int CaretX, CaretY;
  POINT Point;
  RECT R;

  GetWindowRect(HVTWin,&R);
  ts.VTPos.x = R.left;
  ts.VTPos.y = R.top;

  if (CanUseIME() && (ts.IMEInline > 0))
  {
    CaretX = (CursorX-WinOrgX)*FontWidth;
    CaretY = (CursorY-WinOrgY)*FontHeight;
    /* set IME conversion window pos. */
    SetConversionWindow(HVTWin,CaretX,CaretY);
  }

  Point.x = 0;
  Point.y = ScreenHeight;
  ClientToScreen(HVTWin,&Point);
  CompletelyVisible = (Point.y <= CRTHeight);
}

void DispSetActive(BOOL ActiveFlag)
{
  Active = ActiveFlag;
  if (Active)
  {
    SetFocus(HVTWin);
    ActiveWin = IdVT;
  }
  else {
    if ((ts.Language==IdJapanese) &&
        CanUseIME())
      /* position & font of conv. window -> default */
      SetConversionWindow(HVTWin,-1,0);
  }
}
