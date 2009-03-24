/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */
/* IPv6 modification is Copyright(C) 2000 Jun-ya Kato <kato@win6.jp> */

/* TTDLG.DLL, dialog boxes */
#include "teraterm.h"
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <direct.h>
#include <commdlg.h>
#include "tttypes.h"
#include "ttlib.h"
#include "dlglib.h"
#include "ttcommon.h"
#ifdef TERATERM32
  #include "dlg_res.h"
#else
  #include "dlg_re16.h"
#endif
#ifdef INET6
#include <winsock2.h>
static char FAR * ProtocolFamilyList[] = { "UNSPEC", "IPv6", "IPv4", NULL };
#endif /* INET6 */

static HANDLE hInst;

static PCHAR far NLList[] = {"CR","CR+LF",NULL};
static PCHAR far TermList[] =
  {"VT100","VT101","VT102","VT282","VT320","VT382",NULL};
static WORD TermJ_Term[] = {1,1,2,3,3,4,4,5,6};
static WORD Term_TermJ[] = {1,3,4,7,8,9};
static PCHAR far TermListJ[] =
  {"VT100","VT100J","VT101","VT102","VT102J","VT220J",
   "VT282","VT320","VT382",NULL};
static PCHAR far KanjiList[] = {"SJIS","EUC","JIS", "UTF-8", "UTF-8m", NULL};
static PCHAR far KanjiListSend[] = {"SJIS","EUC","JIS", "UTF-8", NULL};
static PCHAR far KanjiInList[] = {"^[$@","^[$B",NULL};
static PCHAR far KanjiOutList[] = {"^[(B","^[(J",NULL};
static PCHAR far KanjiOutList2[] = {"^[(B","^[(J","^[(H",NULL};
static PCHAR far RussList[] = {"Windows","KOI8-R","CP 866","ISO 8859-5",NULL};
static PCHAR far RussList2[] = {"Windows","KOI8-R",NULL};
static PCHAR far LocaleList[] = {"japanese","chinese", "chinese-simplified", "chinese-traditional", NULL};

BOOL CALLBACK TermDlg(HWND Dialog, UINT Message, WPARAM wParam, LPARAM lParam)
{
	PTTSet ts;
	WORD w;
	//  char Temp[HostNameMaxLength + 1]; // 81(yutaka)
	char Temp[81]; // 81(yutaka)

	switch (Message) {
	case WM_INITDIALOG:
		ts = (PTTSet)lParam;
		SetWindowLong(Dialog, DWL_USER, lParam);

		SetDlgItemInt(Dialog,IDC_TERMWIDTH,ts->TerminalWidth,FALSE);
		SendDlgItemMessage(Dialog, IDC_TERMWIDTH, EM_LIMITTEXT,3, 0);

		SetDlgItemInt(Dialog,IDC_TERMHEIGHT,ts->TerminalHeight,FALSE);
		SendDlgItemMessage(Dialog, IDC_TERMHEIGHT, EM_LIMITTEXT,3, 0);

		SetRB(Dialog,ts->TermIsWin,IDC_TERMISWIN,IDC_TERMISWIN);
		SetRB(Dialog,ts->AutoWinResize,IDC_TERMRESIZE,IDC_TERMRESIZE);
		if ( ts->TermIsWin>0 )
			DisableDlgItem(Dialog,IDC_TERMRESIZE,IDC_TERMRESIZE);

		SetDropDownList(Dialog, IDC_TERMCRRCV, NLList, ts->CRReceive);
		SetDropDownList(Dialog, IDC_TERMCRSEND, NLList, ts->CRSend);

		if ( ts->Language!=IdJapanese ) /* non-Japanese mode */
		{
			if ((ts->TerminalID>=1) &&
				(ts->TerminalID<=9))
				w = TermJ_Term[ts->TerminalID-1];
			else
				w = 1;
			SetDropDownList(Dialog, IDC_TERMID, TermList, w);
		}
		else
			SetDropDownList(Dialog, IDC_TERMID, TermListJ, ts->TerminalID);

		SetRB(Dialog,ts->LocalEcho,IDC_TERMLOCALECHO,IDC_TERMLOCALECHO);

		if ((ts->FTFlag & FT_BPAUTO)!=0)
			DisableDlgItem(Dialog,IDC_TERMANSBACKTEXT,IDC_TERMANSBACK);
		else {
			Str2Hex(ts->Answerback,Temp,ts->AnswerbackLen,
				sizeof(Temp)-1,FALSE);
			SetDlgItemText(Dialog, IDC_TERMANSBACK, Temp);
			SendDlgItemMessage(Dialog, IDC_TERMANSBACK, EM_LIMITTEXT,
				sizeof(Temp) - 1, 0);
		}

		SetRB(Dialog,ts->AutoWinSwitch,IDC_TERMAUTOSWITCH,IDC_TERMAUTOSWITCH);

		if (ts->Language==IdJapanese)
		{
			SetDropDownList(Dialog, IDC_TERMKANJI, KanjiList, ts->KanjiCode);
			if ( ts->KanjiCode!=IdJIS )
				DisableDlgItem(Dialog,IDC_TERMKANA,IDC_TERMKANA);
			SetRB(Dialog,ts->JIS7Katakana,IDC_TERMKANA,IDC_TERMKANA);
			SetDropDownList(Dialog, IDC_TERMKANJISEND, KanjiListSend, ts->KanjiCodeSend);
			if ( ts->KanjiCodeSend!=IdJIS )
				DisableDlgItem(Dialog,IDC_TERMKANASEND,IDC_TERMKOUT);
			SetRB(Dialog,ts->JIS7KatakanaSend,IDC_TERMKANASEND,IDC_TERMKANASEND);
			SetDropDownList(Dialog,IDC_TERMKIN,KanjiInList,ts->KanjiIn);
			if ((ts->TermFlag & TF_ALLOWWRONGSEQUENCE)!=0)
				SetDropDownList(Dialog,IDC_TERMKOUT,KanjiOutList2,ts->KanjiOut);
			else
				SetDropDownList(Dialog,IDC_TERMKOUT,KanjiOutList,ts->KanjiOut);

			// ロケール用テキストボックス
			SetDlgItemText(Dialog, IDC_LOCALE_EDIT, ts->Locale);
			SendDlgItemMessage(Dialog, IDC_LOCALE_EDIT, EM_LIMITTEXT, sizeof(ts->Locale), 0);

			SetDlgItemInt(Dialog, IDC_CODEPAGE_EDIT, ts->CodePage, FALSE);
			//SendDlgItemMessage(Dialog, IDC_CODEPAGE_EDIT, EM_LIMITTEXT, 16, 0);

		}
		else if (ts->Language==IdRussian)
		{
			SetDropDownList(Dialog,IDC_TERMRUSSHOST,RussList,ts->RussHost);
			SetDropDownList(Dialog,IDC_TERMRUSSCLIENT,RussList,ts->RussClient);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
	case IDOK:
		ts = (PTTSet)GetWindowLong(Dialog,DWL_USER);

		if ( ts!=NULL )
		{
			ts->TerminalWidth = GetDlgItemInt(Dialog,IDC_TERMWIDTH,NULL,FALSE);
			if ( ts->TerminalWidth<1 ) ts->TerminalWidth = 1;
			if ( ts->TerminalWidth>500 ) ts->TerminalWidth = 500;

			ts->TerminalHeight = GetDlgItemInt(Dialog,IDC_TERMHEIGHT,NULL,FALSE);
			if ( ts->TerminalHeight<1 ) ts->TerminalHeight = 1;
			if ( ts->TerminalHeight>500 ) ts->TerminalHeight = 500;

			GetRB(Dialog,&ts->TermIsWin,IDC_TERMISWIN,IDC_TERMISWIN);
			GetRB(Dialog,&ts->AutoWinResize,IDC_TERMRESIZE,IDC_TERMRESIZE);

			ts->CRReceive = (WORD)GetCurSel(Dialog, IDC_TERMCRRCV);
			ts->CRSend = (WORD)GetCurSel(Dialog, IDC_TERMCRSEND);

			w = (WORD)GetCurSel(Dialog, IDC_TERMID);
			if ( ts->Language!=IdJapanese ) /* non-Japanese mode */
			{
				if ((w==0) || (w>6)) w = 1;
				w = Term_TermJ[w-1];
			}
			ts->TerminalID = w;

			GetRB(Dialog,&ts->LocalEcho,IDC_TERMLOCALECHO,IDC_TERMLOCALECHO);

			if ((ts->FTFlag & FT_BPAUTO)==0)
			{
				GetDlgItemText(Dialog, IDC_TERMANSBACK,Temp,sizeof(Temp));
				ts->AnswerbackLen =
					Hex2Str(Temp,ts->Answerback,sizeof(ts->Answerback));
			}

			GetRB(Dialog,&ts->AutoWinSwitch,IDC_TERMAUTOSWITCH,IDC_TERMAUTOSWITCH);

			if (ts->Language==IdJapanese)
			{
				BOOL ret;

				ts->KanjiCode = (WORD)GetCurSel(Dialog, IDC_TERMKANJI);
				GetRB(Dialog,&ts->JIS7Katakana,IDC_TERMKANA,IDC_TERMKANA);
				ts->KanjiCodeSend = (WORD)GetCurSel(Dialog, IDC_TERMKANJISEND);
				GetRB(Dialog,&ts->JIS7KatakanaSend,IDC_TERMKANASEND,IDC_TERMKANASEND);
				ts->KanjiIn = (WORD)GetCurSel(Dialog, IDC_TERMKIN);
				ts->KanjiOut = (WORD)GetCurSel(Dialog, IDC_TERMKOUT);

				GetDlgItemText(Dialog, IDC_LOCALE_EDIT, ts->Locale, sizeof(ts->Locale));
				ts->CodePage = GetDlgItemInt(Dialog, IDC_CODEPAGE_EDIT, &ret, FALSE);
			}
			else if (ts->Language==IdRussian)
			{
				ts->RussHost = (WORD)GetCurSel(Dialog, IDC_TERMRUSSHOST);
				ts->RussClient = (WORD)GetCurSel(Dialog, IDC_TERMRUSSCLIENT);
			}
		}
		EndDialog(Dialog, 1);
		return TRUE;

	case IDCANCEL:
		EndDialog(Dialog, 0);
		return TRUE;

	case IDC_TERMISWIN:
		GetRB(Dialog,&w,IDC_TERMISWIN,IDC_TERMISWIN);
		if ( w==0 )
			EnableDlgItem(Dialog,IDC_TERMRESIZE,IDC_TERMRESIZE);
		else
			DisableDlgItem(Dialog,IDC_TERMRESIZE,IDC_TERMRESIZE);
		break;

	case IDC_TERMKANJI:
		w = (WORD)GetCurSel(Dialog, IDC_TERMKANJI);
		if (w==IdJIS)
			EnableDlgItem(Dialog,IDC_TERMKANA,IDC_TERMKANA);
		else
			DisableDlgItem(Dialog,IDC_TERMKANA,IDC_TERMKANA);
		break;

	case IDC_TERMKANJISEND:
		w = (WORD)GetCurSel(Dialog, IDC_TERMKANJISEND);
		if (w==IdJIS)
			EnableDlgItem(Dialog,IDC_TERMKANASEND,IDC_TERMKOUT);
		else
			DisableDlgItem(Dialog,IDC_TERMKANASEND,IDC_TERMKOUT);
		break;

	case IDC_TERMHELP:
		PostMessage(GetParent(Dialog),WM_USER_DLGHELP2,0,0);
		break;
		}
	}
	return FALSE;
}

  void DispSample(HWND Dialog, PTTSet ts, int IAttr)
  {
    int i,x,y;
    COLORREF Text, Back;
    int DX[3];
    TEXTMETRIC Metrics;
    RECT Rect, TestRect;
    int FW,FH;
    HDC DC;

    DC = GetDC(Dialog);
    Text = RGB(ts->TmpColor[IAttr][0],
	       ts->TmpColor[IAttr][1],
	       ts->TmpColor[IAttr][2]);
    Text = GetNearestColor(DC, Text);
    Back = RGB(ts->TmpColor[IAttr][3],
	       ts->TmpColor[IAttr][4],
	       ts->TmpColor[IAttr][5]);
    Back = GetNearestColor(DC, Back);
    SetTextColor(DC, Text);
    SetBkColor(DC, Back);
    SelectObject(DC,ts->SampleFont);
    GetTextMetrics(DC, &Metrics);
    FW = Metrics.tmAveCharWidth;
    FH = Metrics.tmHeight;
    for (i = 0 ; i <= 2 ; i++) DX[i] = FW;
    GetClientRect(Dialog,&Rect);
    TestRect.left = Rect.left + (int)((Rect.right-Rect.left)*0.65);
    TestRect.right = Rect.left + (int)((Rect.right-Rect.left)*0.93);
    TestRect.top = Rect.top + (int)((Rect.bottom-Rect.top)*0.54);
    TestRect.bottom = Rect.top + (int)((Rect.bottom-Rect.top)*0.94);
    x = (int)((TestRect.left+TestRect.right) / 2 - FW * 1.5);
    y = (TestRect.top+TestRect.bottom-FH) / 2;
    ExtTextOut(DC, x,y, ETO_CLIPPED | ETO_OPAQUE,
	       &TestRect, "ABC", 3, &(DX[0]));
    ReleaseDC(Dialog,DC);
  }

  void ChangeColor(HWND Dialog, PTTSet ts, int IAttr, int IOffset)
  {
    SetDlgItemInt(Dialog,IDC_WINRED,ts->TmpColor[IAttr][IOffset],FALSE);
    SetDlgItemInt(Dialog,IDC_WINGREEN,ts->TmpColor[IAttr][IOffset+1],FALSE);
    SetDlgItemInt(Dialog,IDC_WINBLUE,ts->TmpColor[IAttr][IOffset+2],FALSE);

    DispSample(Dialog,ts,IAttr);
  }

  void ChangeSB
    (HWND Dialog, PTTSet ts, int IAttr, int IOffset)
  {
    HWND HRed, HGreen, HBlue;

    HRed = GetDlgItem(Dialog, IDC_WINREDBAR);
    HGreen = GetDlgItem(Dialog, IDC_WINGREENBAR);
    HBlue = GetDlgItem(Dialog, IDC_WINBLUEBAR);
    
    SetScrollPos(HRed,SB_CTL,ts->TmpColor[IAttr][IOffset+0],TRUE);
    SetScrollPos(HGreen,SB_CTL,ts->TmpColor[IAttr][IOffset+1],TRUE);
    SetScrollPos(HBlue,SB_CTL,ts->TmpColor[IAttr][IOffset+2],TRUE);
    ChangeColor(Dialog,ts,IAttr,IOffset);
  }

  void RestoreVar(HWND Dialog, PTTSet ts, int *IAttr, int *IOffset)
  {
    WORD w;

    GetRB(Dialog,&w,IDC_WINTEXT,IDC_WINBACK);
    if (w==2) *IOffset = 3;
	 else *IOffset = 0;
    if ((ts!=NULL) && (ts->VTFlag>0))
    {
      *IAttr = GetCurSel(Dialog,IDC_WINATTR);
      if (*IAttr>0) (*IAttr)--;
    }
    else
      *IAttr = 0;
  }

BOOL CALLBACK WinDlg(HWND Dialog, UINT Message, WPARAM wParam, LPARAM lParam)
{
  PTTSet ts;
  HWND Wnd, HRed, HGreen, HBlue;
  int IAttr, IOffset;
  WORD i, pos, ScrollCode, NewPos;
  HDC DC;

  switch (Message) {
    case WM_INITDIALOG:
      ts = (PTTSet)lParam;
      SetWindowLong(Dialog, DWL_USER, lParam);

      SetDlgItemText(Dialog, IDC_WINTITLE, ts->Title);
      SendDlgItemMessage(Dialog, IDC_WINTITLE, EM_LIMITTEXT,
			 sizeof(ts->Title)-1, 0);

      SetRB(Dialog,ts->HideTitle,IDC_WINHIDETITLE,IDC_WINHIDETITLE);
      SetRB(Dialog,ts->PopupMenu,IDC_WINHIDEMENU,IDC_WINHIDEMENU);
      if ( ts->HideTitle>0 )
	DisableDlgItem(Dialog,IDC_WINHIDEMENU,IDC_WINHIDEMENU);

      if (ts->VTFlag>0)
      {
	SetDlgItemText(Dialog, IDC_WINCOLOREMU,"Full &color");
	if ((ts->ColorFlag & CF_FULLCOLOR)!=0)
	  i = 1;
	else
	  i = 0;
	SetRB(Dialog,i,IDC_WINCOLOREMU,IDC_WINCOLOREMU);
	ShowDlgItem(Dialog,IDC_WINSCROLL1,IDC_WINSCROLL3);
	SetRB(Dialog,ts->EnableScrollBuff,IDC_WINSCROLL1,IDC_WINSCROLL1);
	SetDlgItemInt(Dialog,IDC_WINSCROLL2,ts->ScrollBuffSize,FALSE);

	// 入力最大桁数を 5 から 8 へ拡張 (2004.11.28 yutaka)
	SendDlgItemMessage(Dialog, IDC_WINSCROLL2, EM_LIMITTEXT, 8, 0);

	if ( ts->EnableScrollBuff==0 )
	  DisableDlgItem(Dialog,IDC_WINSCROLL2,IDC_WINSCROLL3);
	for (i = 0 ; i <= 1 ; i++)
	{
	  ts->TmpColor[0][i*3]	 = GetRValue(ts->VTColor[i]);
	  ts->TmpColor[0][i*3+1] = GetGValue(ts->VTColor[i]);
	  ts->TmpColor[0][i*3+2] = GetBValue(ts->VTColor[i]);
	  ts->TmpColor[1][i*3]	 = GetRValue(ts->VTBoldColor[i]);
	  ts->TmpColor[1][i*3+1] = GetGValue(ts->VTBoldColor[i]);
	  ts->TmpColor[1][i*3+2] = GetBValue(ts->VTBoldColor[i]);
	  ts->TmpColor[2][i*3]	 = GetRValue(ts->VTBlinkColor[i]);
	  ts->TmpColor[2][i*3+1] = GetGValue(ts->VTBlinkColor[i]);
	  ts->TmpColor[2][i*3+2] = GetBValue(ts->VTBlinkColor[i]);
      /* begin - ishizaki */
	  ts->TmpColor[3][i*3]	 = GetRValue(ts->URLColor[i]);
	  ts->TmpColor[3][i*3+1] = GetGValue(ts->URLColor[i]);
	  ts->TmpColor[3][i*3+2] = GetBValue(ts->URLColor[i]);
      /* end - ishizaki */
	}
	ShowDlgItem(Dialog,IDC_WINATTRTEXT,IDC_WINATTR);
	SendDlgItemMessage(Dialog, IDC_WINATTR, CB_ADDSTRING,
			   0, (LPARAM)"Normal");
	SendDlgItemMessage(Dialog, IDC_WINATTR, CB_ADDSTRING,
			   0, (LPARAM)"Bold");
	SendDlgItemMessage(Dialog, IDC_WINATTR, CB_ADDSTRING,
			   0, (LPARAM)"Blink");
    /* begin - ishizaki */
	SendDlgItemMessage(Dialog, IDC_WINATTR, CB_ADDSTRING,
			   0, (LPARAM)"URL");
    /* end - ishizaki */
	SendDlgItemMessage(Dialog, IDC_WINATTR, CB_SETCURSEL,
			   0,0);
      }
      else {
	for (i = 0 ; i <=1 ; i++)
	{
	  ts->TmpColor[0][i*3]	 = GetRValue(ts->TEKColor[i]);
	  ts->TmpColor[0][i*3+1] = GetGValue(ts->TEKColor[i]);
	  ts->TmpColor[0][i*3+2] = GetBValue(ts->TEKColor[i]);
	}
	SetRB(Dialog,ts->TEKColorEmu,IDC_WINCOLOREMU,IDC_WINCOLOREMU);
      }
      SetRB(Dialog,1,IDC_WINTEXT,IDC_WINBACK);

      SetRB(Dialog,ts->CursorShape,IDC_WINBLOCK,IDC_WINHORZ);

      IAttr = 0;
      IOffset = 0;

      HRed = GetDlgItem(Dialog, IDC_WINREDBAR);
      SetScrollRange(HRed,SB_CTL,0,255,TRUE);

      HGreen = GetDlgItem(Dialog, IDC_WINGREENBAR);
      SetScrollRange(HGreen,SB_CTL,0,255,TRUE);

      HBlue = GetDlgItem(Dialog, IDC_WINBLUEBAR);
      SetScrollRange(HBlue,SB_CTL,0,255,TRUE);

      ChangeSB(Dialog,ts,IAttr,IOffset);

      return TRUE;

    case WM_COMMAND:
      ts = (PTTSet)GetWindowLong(Dialog,DWL_USER);
      RestoreVar(Dialog,ts,&IAttr,&IOffset);
      switch (LOWORD(wParam)) {
	case IDOK:
	  if ( ts!=NULL )
	  {
	    GetDlgItemText(Dialog,IDC_WINTITLE,ts->Title,sizeof(ts->Title));
	    GetRB(Dialog,&ts->HideTitle,IDC_WINHIDETITLE,IDC_WINHIDETITLE);
	    GetRB(Dialog,&ts->PopupMenu,IDC_WINHIDEMENU,IDC_WINHIDEMENU);
	    DC = GetDC(Dialog);
	    if (ts->VTFlag>0)
	    {
	      GetRB(Dialog,&i,IDC_WINCOLOREMU,IDC_WINCOLOREMU);
	      if (i!=0)
		ts->ColorFlag |= CF_FULLCOLOR;
	      else
		ts->ColorFlag &= ~(WORD)CF_FULLCOLOR;
	      GetRB(Dialog,&ts->EnableScrollBuff,IDC_WINSCROLL1,IDC_WINSCROLL1);
	      if ( ts->EnableScrollBuff>0 )
	      {
		ts->ScrollBuffSize =
		  GetDlgItemInt(Dialog,IDC_WINSCROLL2,NULL,FALSE);
	      }
	      for (i = 0 ; i <= 1 ; i++)
	      {
		ts->VTColor[i] =
		  RGB(ts->TmpColor[0][i*3],
		      ts->TmpColor[0][i*3+1],
		      ts->TmpColor[0][i*3+2]);
		ts->VTBoldColor[i] =
		  RGB(ts->TmpColor[1][i*3],
		      ts->TmpColor[1][i*3+1],
		      ts->TmpColor[1][i*3+2]);
		ts->VTBlinkColor[i] =
		  RGB(ts->TmpColor[2][i*3],
		      ts->TmpColor[2][i*3+1],
		      ts->TmpColor[2][i*3+2]);
        /* begin - ishizaki */
		ts->URLColor[i] =
		  RGB(ts->TmpColor[3][i*3],
		      ts->TmpColor[3][i*3+1],
		      ts->TmpColor[3][i*3+2]);
        /* end - ishizaki */
		ts->VTColor[i] = GetNearestColor(DC,ts->VTColor[i]);
		ts->VTBoldColor[i] = GetNearestColor(DC,ts->VTBoldColor[i]);
		ts->VTBlinkColor[i] = GetNearestColor(DC,ts->VTBlinkColor[i]);
        /* begin - ishizaki */
		ts->URLColor[i] = GetNearestColor(DC,ts->URLColor[i]);
        /* end - ishizaki */
	      }
	    }
	    else {
	      for (i = 0 ; i <= 1 ; i++)
	      {
		ts->TEKColor[i] =
		  RGB(ts->TmpColor[0][i*3],
		      ts->TmpColor[0][i*3+1],
		      ts->TmpColor[0][i*3+2]);
		ts->TEKColor[i] = GetNearestColor(DC,ts->TEKColor[i]);
	      }
	      GetRB(Dialog,&ts->TEKColorEmu,IDC_WINCOLOREMU,IDC_WINCOLOREMU);
	    }
	    ReleaseDC(Dialog,DC);

	    GetRB(Dialog,&ts->CursorShape,IDC_WINBLOCK,IDC_WINHORZ);

	  }
	  EndDialog(Dialog, 1);
	  return TRUE;

	case IDCANCEL:
	  EndDialog(Dialog, 0);
	  return TRUE;

	case IDC_WINHIDETITLE:
	  GetRB(Dialog,&i,IDC_WINHIDETITLE,IDC_WINHIDETITLE);
	  if (i>0)
	    DisableDlgItem(Dialog,IDC_WINHIDEMENU,IDC_WINHIDEMENU);
	  else
	    EnableDlgItem(Dialog,IDC_WINHIDEMENU,IDC_WINHIDEMENU);
	  break;

	case IDC_WINSCROLL1:
	  if ( ts==NULL ) return TRUE;
	  GetRB(Dialog,&i,IDC_WINSCROLL1,IDC_WINSCROLL1);
	  if ( i>0 )
	    EnableDlgItem(Dialog,IDC_WINSCROLL2,IDC_WINSCROLL3);
	  else
	    DisableDlgItem(Dialog,IDC_WINSCROLL2,IDC_WINSCROLL3);
	  break;

	case IDC_WINTEXT:
	  if ( ts==NULL ) return TRUE;
	  IOffset = 0;
	  ChangeSB(Dialog,ts,IAttr,IOffset);
	  break;
	  
	case IDC_WINBACK:
	  if ( ts==NULL ) return TRUE;
	  IOffset = 3;
	  ChangeSB(Dialog,ts,IAttr,IOffset);
	  break;

	case IDC_WINREV:
	  if ( ts==NULL ) return TRUE;
	  i = ts->TmpColor[IAttr][0];
	  ts->TmpColor[IAttr][0] = ts->TmpColor[IAttr][3];
	  ts->TmpColor[IAttr][3] = i;
	  i = ts->TmpColor[IAttr][1];
	  ts->TmpColor[IAttr][1] = ts->TmpColor[IAttr][4];
	  ts->TmpColor[IAttr][4] = i;
	  i = ts->TmpColor[IAttr][2];
	  ts->TmpColor[IAttr][2] = ts->TmpColor[IAttr][5];
	  ts->TmpColor[IAttr][5] = i;

	  ChangeSB(Dialog,ts,IAttr,IOffset);
	  break;

	case IDC_WINATTR:
	  if ( ts!=NULL ) ChangeSB(Dialog,ts,IAttr,IOffset);
	  break;

	case IDC_WINHELP:
	  PostMessage(GetParent(Dialog),WM_USER_DLGHELP2,0,0);
	  break;
      }
      break;

    case WM_PAINT:
      ts = (PTTSet)GetWindowLong(Dialog,DWL_USER);
      if ( ts==NULL ) return TRUE;
      RestoreVar(Dialog,ts,&IAttr,&IOffset);
      DispSample(Dialog,ts,IAttr);
      break;

    case WM_HSCROLL:
      ts = (PTTSet)GetWindowLong(Dialog,DWL_USER);
      if (ts==NULL) return TRUE;
      RestoreVar(Dialog,ts,&IAttr,&IOffset);
      HRed = GetDlgItem(Dialog, IDC_WINREDBAR);
      HGreen = GetDlgItem(Dialog, IDC_WINGREENBAR);
      HBlue = GetDlgItem(Dialog, IDC_WINBLUEBAR);
#ifdef TERATERM32
      Wnd = (HWND)lParam;
      ScrollCode = LOWORD(wParam);
      NewPos = HIWORD(wParam);
#else
      Wnd = (HWND)HIWORD(lParam);
      ScrollCode = wParam;
      NewPos = LOWORD(lParam);
#endif
      if ( Wnd == HRed ) i = IOffset;
      else if ( Wnd == HGreen ) i = IOffset + 1;
      else if ( Wnd == HBlue ) i = IOffset + 2;
      else return TRUE;
      pos = ts->TmpColor[IAttr][i];
      switch (ScrollCode) {
	case SB_BOTTOM:
	  pos = 255;
	  break;
	case SB_LINEDOWN:
	  if (pos<255) pos++;
	  break;
	case SB_LINEUP:
	  if (pos>0) pos--;
	  break;
	case SB_PAGEDOWN:
	  pos = pos + 16;
	  break;
	case SB_PAGEUP:
	  pos = pos - 16;
	  break;
	case SB_THUMBPOSITION:
	  pos = NewPos;
	  break;
	case SB_THUMBTRACK:
	  pos = NewPos;
	  break;
	case SB_TOP:
	  pos = 0;
	  break;
	default:
	  return TRUE;
      }
      if (pos > 255) pos = 255;
      ts->TmpColor[IAttr][i] = pos;
      SetScrollPos(Wnd,SB_CTL,pos,TRUE);
      ChangeColor(Dialog,ts,IAttr,IOffset);
      return FALSE;
  }
  return FALSE;
}

BOOL CALLBACK KeybDlg(HWND Dialog, UINT Message, WPARAM wParam, LPARAM lParam)
{
  PTTSet ts;

  switch (Message) {
    case WM_INITDIALOG:
      ts = (PTTSet)lParam;
      SetWindowLong(Dialog, DWL_USER, lParam);

      SetRB(Dialog,ts->BSKey-1,IDC_KEYBBS,IDC_KEYBBS);
      SetRB(Dialog,ts->DelKey,IDC_KEYBDEL,IDC_KEYBDEL);
      SetRB(Dialog,ts->MetaKey,IDC_KEYBMETA,IDC_KEYBMETA);
      if (ts->Language==IdRussian)
      {
	ShowDlgItem(Dialog,IDC_KEYBKEYBTEXT,IDC_KEYBKEYB);
	SetDropDownList(Dialog, IDC_KEYBKEYB, RussList2, ts->RussKeyb);
      }
      return TRUE;

    case WM_COMMAND:
      switch (LOWORD(wParam)) {
	case IDOK:
	  ts = (PTTSet)GetWindowLong(Dialog,DWL_USER);
	  if ( ts!=NULL )
	  {
	    GetRB(Dialog,&ts->BSKey,IDC_KEYBBS,IDC_KEYBBS);
	    ts->BSKey++;
	    GetRB(Dialog,&ts->DelKey,IDC_KEYBDEL,IDC_KEYBDEL);
	    GetRB(Dialog,&ts->MetaKey,IDC_KEYBMETA,IDC_KEYBMETA);
	    if (ts->Language==IdRussian)
	      ts->RussKeyb = (WORD)GetCurSel(Dialog, IDC_KEYBKEYB);
	  }
	  EndDialog(Dialog, 1);
	  return TRUE;

	case IDCANCEL:
	  EndDialog(Dialog, 0);
	  return TRUE;

	case IDC_KEYBHELP:
	  PostMessage(GetParent(Dialog),WM_USER_DLGHELP2,0,0);
      }
  }
  return FALSE;
}

static PCHAR far BaudList[] =
  {"110","300","600","1200","2400","4800","9600",
   "14400","19200","38400","57600","115200",NULL};
static PCHAR far DataList[] = {"7 bit","8 bit",NULL};
static PCHAR far ParityList[] = {"even","odd","none",NULL};
static PCHAR far StopList[] = {"1 bit","2 bit",NULL};
static PCHAR far FlowList[] = {"Xon/Xoff","hardware","none",NULL};

BOOL CALLBACK SerialDlg(HWND Dialog, UINT Message, WPARAM wParam, LPARAM lParam)
{
  PTTSet ts;
  int i;
  char Temp[6];

  switch (Message) {
    case WM_INITDIALOG:
      ts = (PTTSet)lParam;
      SetWindowLong(Dialog, DWL_USER, lParam);

      strcpy(Temp,"COM");

      for (i=1;i<=ts->MaxComPort;i++)
      {
	uint2str(i,&Temp[3],2);
	SendDlgItemMessage(Dialog, IDC_SERIALPORT, CB_ADDSTRING,
			   0, (LPARAM)Temp);
      }
      if (ts->ComPort<=ts->MaxComPort)
	i = ts->ComPort-1;
      else
	i = 0;
      SendDlgItemMessage(Dialog, IDC_SERIALPORT, CB_SETCURSEL,i,0);

      SetDropDownList(Dialog, IDC_SERIALBAUD, BaudList, ts->Baud);
      SetDropDownList(Dialog, IDC_SERIALDATA, DataList, ts->DataBit);
      SetDropDownList(Dialog, IDC_SERIALPARITY, ParityList, ts->Parity);
      SetDropDownList(Dialog, IDC_SERIALSTOP, StopList, ts->StopBit);
      SetDropDownList(Dialog, IDC_SERIALFLOW, FlowList, ts->Flow);

      SetDlgItemInt(Dialog,IDC_SERIALDELAYCHAR,ts->DelayPerChar,FALSE);
      SendDlgItemMessage(Dialog, IDC_SERIALDELAYCHAR, EM_LIMITTEXT,4, 0);

      SetDlgItemInt(Dialog,IDC_SERIALDELAYLINE,ts->DelayPerLine,FALSE);
      SendDlgItemMessage(Dialog, IDC_SERIALDELAYLINE, EM_LIMITTEXT,4, 0);

      return TRUE;

    case WM_COMMAND:
      switch (LOWORD(wParam)) {
	case IDOK:
	  ts = (PTTSet)GetWindowLong(Dialog,DWL_USER);
	  if ( ts!=NULL )
	  {
	    ts->ComPort = (WORD)GetCurSel(Dialog, IDC_SERIALPORT);
	    ts->Baud = (WORD)GetCurSel(Dialog, IDC_SERIALBAUD);
	    ts->DataBit = (WORD)GetCurSel(Dialog, IDC_SERIALDATA);
	    ts->Parity = (WORD)GetCurSel(Dialog, IDC_SERIALPARITY);
	    ts->StopBit = (WORD)GetCurSel(Dialog, IDC_SERIALSTOP);
	    ts->Flow = (WORD)GetCurSel(Dialog, IDC_SERIALFLOW);

	    ts->DelayPerChar = GetDlgItemInt(Dialog,IDC_SERIALDELAYCHAR,NULL,FALSE);

	    ts->DelayPerLine = GetDlgItemInt(Dialog,IDC_SERIALDELAYLINE,NULL,FALSE);

	    ts->PortType = IdSerial;
	  }
	  EndDialog(Dialog, 1);
	  return TRUE;

	case IDCANCEL:
	  EndDialog(Dialog, 0);
	  return TRUE;

	case IDC_SERIALHELP:
	  PostMessage(GetParent(Dialog),WM_USER_DLGHELP2,0,0);
      }
  }
  return FALSE;
}

BOOL CALLBACK TCPIPDlg(HWND Dialog, UINT Message, WPARAM wParam, LPARAM lParam)
{
  PTTSet ts;
  char EntName[7];
  char TempHost[HostNameMaxLength+1];
  UINT i, Index;
  WORD w;
  BOOL Ok;

  switch (Message) {
    case WM_INITDIALOG:
      ts = (PTTSet)lParam;
      SetWindowLong(Dialog, DWL_USER, lParam);

      SendDlgItemMessage(Dialog, IDC_TCPIPHOST, EM_LIMITTEXT,
			 HostNameMaxLength-1, 0);

      strcpy(EntName,"Host");

      i = 1;
      do {
	uint2str(i,&EntName[4],2);
	GetPrivateProfileString("Hosts",EntName,"",
				TempHost,sizeof(TempHost),ts->SetupFName);
	if (strlen(TempHost) > 0)
	  SendDlgItemMessage(Dialog, IDC_TCPIPLIST, LB_ADDSTRING,
			     0, (LPARAM)TempHost);
	i++;
      } while ((i <= 99) && (strlen(TempHost)>0));

      /* append a blank item to the bottom */
      TempHost[0] = 0;
      SendDlgItemMessage(Dialog, IDC_TCPIPLIST, LB_ADDSTRING,
		         0, (LPARAM)TempHost);

      SetRB(Dialog,ts->HistoryList,IDC_TCPIPHISTORY,IDC_TCPIPHISTORY);

      SetRB(Dialog,ts->AutoWinClose,IDC_TCPIPAUTOCLOSE,IDC_TCPIPAUTOCLOSE);

      SetDlgItemInt(Dialog,IDC_TCPIPPORT,ts->TCPPort,FALSE);

      SetRB(Dialog,ts->Telnet,IDC_TCPIPTELNET,IDC_TCPIPTELNET);

      SetDlgItemText(Dialog, IDC_TCPIPTERMTYPE, ts->TermType);

      SendDlgItemMessage(Dialog, IDC_TCPIPTERMTYPE, EM_LIMITTEXT,
			 sizeof(ts->TermType)-1, 0);

      if ( ts->Telnet==0 )
	DisableDlgItem(Dialog,IDC_TCPIPTERMTYPELABEL,IDC_TCPIPTERMTYPE);

      return TRUE;

    case WM_COMMAND:
      switch (LOWORD(wParam)) {
	case IDOK:
	  ts = (PTTSet)GetWindowLong(Dialog,DWL_USER);
	  if (ts!=NULL)
	  {
	    WritePrivateProfileString("Hosts",NULL,NULL,ts->SetupFName);

	    Index = SendDlgItemMessage(Dialog,IDC_TCPIPLIST,LB_GETCOUNT,0,0);
	    if (Index==(UINT)LB_ERR)
	      Index = 0;
	    else
	      Index--;
	    if (Index>99) Index = 99;

	    strcpy(EntName,"Host");
	    for (i = 1 ; i <= Index ; i++)
	    {
	      SendDlgItemMessage(Dialog, IDC_TCPIPLIST, LB_GETTEXT,
				 i-1, (LPARAM)TempHost);
	      uint2str(i,&EntName[4],2);
	      WritePrivateProfileString("Hosts",EntName,TempHost,ts->SetupFName);
	    }

	    GetRB(Dialog,&ts->HistoryList,IDC_TCPIPHISTORY,IDC_TCPIPHISTORY);

	    GetRB(Dialog,&ts->AutoWinClose,IDC_TCPIPAUTOCLOSE,IDC_TCPIPAUTOCLOSE);

	    ts->TCPPort = GetDlgItemInt(Dialog,IDC_TCPIPPORT,&Ok,FALSE);
	    if (! Ok) ts->TCPPort = ts->TelPort;

	    GetRB(Dialog,&ts->Telnet,IDC_TCPIPTELNET,IDC_TCPIPTELNET);

	    GetDlgItemText(Dialog, IDC_TCPIPTERMTYPE, ts->TermType,
			   sizeof(ts->TermType));
	  }
	  EndDialog(Dialog, 1);
	  return TRUE;

	case IDCANCEL:
	  EndDialog(Dialog, 0);
	  return TRUE;

	case IDC_TCPIPHOST:
#ifdef TERATERM32
	  if (HIWORD(wParam)==EN_CHANGE)
#else
	  if (HIWORD(lParam)==EN_CHANGE)
#endif
	  {
	    GetDlgItemText(Dialog, IDC_TCPIPHOST, TempHost, sizeof(TempHost));
	    if (strlen(TempHost)==0)
	      DisableDlgItem(Dialog,IDC_TCPIPADD,IDC_TCPIPADD);
	    else
	      EnableDlgItem(Dialog,IDC_TCPIPADD,IDC_TCPIPADD);
	  }
	  break;

	case IDC_TCPIPADD:
	  GetDlgItemText(Dialog, IDC_TCPIPHOST, TempHost, sizeof(TempHost));
	  if (strlen(TempHost)>0)
	  {
	    Index = SendDlgItemMessage(Dialog,IDC_TCPIPLIST,LB_GETCURSEL,0,0);
	    if (Index==(UINT)LB_ERR) Index = 0;

	    SendDlgItemMessage(Dialog, IDC_TCPIPLIST, LB_INSERTSTRING,
			       Index, (LPARAM)TempHost);

	    SetDlgItemText(Dialog, IDC_TCPIPHOST, 0);
	    SetFocus(GetDlgItem(Dialog, IDC_TCPIPHOST));
	  }
	  break;

	case IDC_TCPIPLIST:
#ifdef TERATERM32
	  if (HIWORD(wParam)==LBN_SELCHANGE)
#else
	  if (HIWORD(lParam)==LBN_SELCHANGE)
#endif
	  {
	    i = SendDlgItemMessage(Dialog,IDC_TCPIPLIST,LB_GETCOUNT,0,0);
	    Index = SendDlgItemMessage(Dialog, IDC_TCPIPLIST, LB_GETCURSEL, 0, 0);
	    if ((i<=1) || (Index==(UINT)LB_ERR) ||
		(Index==i-1))
	      DisableDlgItem(Dialog,IDC_TCPIPUP,IDC_TCPIPDOWN);
	    else {
	      EnableDlgItem(Dialog,IDC_TCPIPREMOVE,IDC_TCPIPREMOVE);
	      if (Index==0)
		DisableDlgItem(Dialog,IDC_TCPIPUP,IDC_TCPIPUP);
	      else
		EnableDlgItem(Dialog,IDC_TCPIPUP,IDC_TCPIPUP);
	      if (Index>=i-2)
		DisableDlgItem(Dialog,IDC_TCPIPDOWN,IDC_TCPIPDOWN);
	      else
		EnableDlgItem(Dialog,IDC_TCPIPDOWN,IDC_TCPIPDOWN);
	    }
	  }
	  break;

	case IDC_TCPIPUP:
	case IDC_TCPIPDOWN:
	  i = SendDlgItemMessage(Dialog,IDC_TCPIPLIST,LB_GETCOUNT,0,0);
	  Index = SendDlgItemMessage(Dialog, IDC_TCPIPLIST, LB_GETCURSEL, 0, 0);
	  if (Index==(UINT)LB_ERR) return TRUE;
	  if (LOWORD(wParam)==IDC_TCPIPDOWN) Index++;
	  if ((Index==0) || (Index>=i-1)) return TRUE;
	  SendDlgItemMessage(Dialog, IDC_TCPIPLIST, LB_GETTEXT,
			     Index, (LPARAM)TempHost);
	  SendDlgItemMessage(Dialog, IDC_TCPIPLIST, LB_DELETESTRING,
			     Index, 0);
	  SendDlgItemMessage(Dialog, IDC_TCPIPLIST, LB_INSERTSTRING,
			     Index-1, (LPARAM)TempHost);
	  if (LOWORD(wParam)==IDC_TCPIPUP) Index--;
	  SendDlgItemMessage(Dialog, IDC_TCPIPLIST, LB_SETCURSEL,Index,0);
	  if (Index==0)
	    DisableDlgItem(Dialog,IDC_TCPIPUP,IDC_TCPIPUP);
	  else
	    EnableDlgItem(Dialog,IDC_TCPIPUP,IDC_TCPIPUP);
	  if (Index>=i-2)
	    DisableDlgItem(Dialog,IDC_TCPIPDOWN,IDC_TCPIPDOWN);
	  else
	    EnableDlgItem(Dialog,IDC_TCPIPDOWN,IDC_TCPIPDOWN);
	  SetFocus(GetDlgItem(Dialog, IDC_TCPIPLIST));
	  break;

	case IDC_TCPIPREMOVE:
	  i = SendDlgItemMessage(Dialog,IDC_TCPIPLIST,LB_GETCOUNT,0,0);
	  Index = SendDlgItemMessage(Dialog,IDC_TCPIPLIST,LB_GETCURSEL, 0, 0);
	  if ((Index==(UINT)LB_ERR) ||
	      (Index==i-1)) return TRUE;
	  SendDlgItemMessage(Dialog, IDC_TCPIPLIST, LB_GETTEXT,
			     Index, (LPARAM)TempHost);
	  SendDlgItemMessage(Dialog, IDC_TCPIPLIST, LB_DELETESTRING,
			     Index, 0);
	  SetDlgItemText(Dialog, IDC_TCPIPHOST, TempHost);
	  DisableDlgItem(Dialog,IDC_TCPIPUP,IDC_TCPIPDOWN);
	  SetFocus(GetDlgItem(Dialog, IDC_TCPIPHOST));
	  break;

	case IDC_TCPIPTELNET:
	  GetRB(Dialog,&w,IDC_TCPIPTELNET,IDC_TCPIPTELNET);
	  if (w==1)
	  {
	    EnableDlgItem(Dialog,IDC_TCPIPTERMTYPELABEL,IDC_TCPIPTERMTYPE);
	    ts = (PTTSet)GetWindowLong(Dialog,DWL_USER);
	    if (ts!=NULL)
	      SetDlgItemInt(Dialog,IDC_TCPIPPORT,ts->TelPort,FALSE);
	  }
	  else
	    DisableDlgItem(Dialog,IDC_TCPIPTERMTYPELABEL,IDC_TCPIPTERMTYPE);
	  break;

	case IDC_TCPIPHELP:
	  PostMessage(GetParent(Dialog),WM_USER_DLGHELP2,0,0);
	  break;
      }
  }
  return FALSE;
}

BOOL CALLBACK HostDlg(HWND Dialog, UINT Message, WPARAM wParam, LPARAM lParam)
{
  PGetHNRec GetHNRec;
  char EntName[7];
  char TempHost[HostNameMaxLength+1];
  WORD i, j, w;
  BOOL Ok;

  switch (Message) {
    case WM_INITDIALOG:
      GetHNRec = (PGetHNRec)lParam;
      SetWindowLong(Dialog, DWL_USER, lParam);

      if ( GetHNRec->PortType==IdFile )
	GetHNRec->PortType = IdTCPIP;
      SetRB(Dialog,GetHNRec->PortType,IDC_HOSTTCPIP,IDC_HOSTSERIAL);

      strcpy(EntName,"Host");

      i = 1;
      do {
	uint2str(i,&EntName[4],2);
	GetPrivateProfileString("Hosts",EntName,"",
				TempHost,sizeof(TempHost),GetHNRec->SetupFN);
	if ( strlen(TempHost) > 0 )
	  SendDlgItemMessage(Dialog, IDC_HOSTNAME, CB_ADDSTRING,
			     0, (LPARAM)TempHost);
	i++;
      } while ((i <= 99) && (strlen(TempHost)>0));

      SendDlgItemMessage(Dialog, IDC_HOSTNAME, EM_LIMITTEXT,
			 HostNameMaxLength-1, 0);

      SendDlgItemMessage(Dialog, IDC_HOSTNAME, CB_SETCURSEL,0,0);

      SetRB(Dialog,GetHNRec->Telnet,IDC_HOSTTELNET,IDC_HOSTTELNET);
      SendDlgItemMessage(Dialog, IDC_HOSTTCPPORT, EM_LIMITTEXT,5,0);
      SetDlgItemInt(Dialog,IDC_HOSTTCPPORT,GetHNRec->TCPPort,FALSE);
#ifdef INET6
      for (i=0; ProtocolFamilyList[i]; ++i)
      {
        SendDlgItemMessage(Dialog, IDC_HOSTTCPPROTOCOL, CB_ADDSTRING,
			   0, (LPARAM)ProtocolFamilyList[i]);
      }
      SendDlgItemMessage(Dialog, IDC_HOSTTCPPROTOCOL, EM_LIMITTEXT,
                         ProtocolFamilyMaxLength-1, 0);
      SendDlgItemMessage(Dialog, IDC_HOSTTCPPROTOCOL, CB_SETCURSEL,0,0);
#endif /* INET6 */

      j = 0;
      w = 1;
      strcpy(EntName,"COM");
      for (i=1; i<=GetHNRec->MaxComPort ;i++)
      {
	if (((GetCOMFlag() >> (i-1)) & 1)==0)
	{
	  uint2str(i,&EntName[3],2);
	  SendDlgItemMessage(Dialog, IDC_HOSTCOM, CB_ADDSTRING,
			     0, (LPARAM)EntName);
	  j++;
	  if (GetHNRec->ComPort==i) w = j;
	}
      }
      if (j>0)
	SendDlgItemMessage(Dialog, IDC_HOSTCOM, CB_SETCURSEL,w-1,0);
      else /* All com ports are already used */
	GetHNRec->PortType = IdTCPIP;

      if ( GetHNRec->PortType==IdTCPIP )
	DisableDlgItem(Dialog,IDC_HOSTCOMLABEL,IDC_HOSTCOM);
#ifndef INET6
      else
	DisableDlgItem(Dialog,IDC_HOSTNAMELABEL,IDC_HOSTTCPPORT);
#else
      else {
        DisableDlgItem(Dialog,IDC_HOSTNAMELABEL,IDC_HOSTTCPPORT);
        DisableDlgItem(Dialog,IDC_HOSTTCPPROTOCOLLABEL,IDC_HOSTTCPPROTOCOL);
      }
#endif /* INET6 */
 
      return TRUE;

    case WM_COMMAND:
      switch (LOWORD(wParam)) {
	case IDOK:
	  GetHNRec = (PGetHNRec)GetWindowLong(Dialog,DWL_USER);
	  if ( GetHNRec!=NULL )
	  {
#ifdef INET6
            char afstr[BUFSIZ];
#endif /* INET6 */
	    GetRB(Dialog,&GetHNRec->PortType,IDC_HOSTTCPIP,IDC_HOSTSERIAL);
	    if ( GetHNRec->PortType==IdTCPIP )
	      GetDlgItemText(Dialog, IDC_HOSTNAME, GetHNRec->HostName, HostNameMaxLength);
	    else
	      GetHNRec->HostName[0] = 0;
	    GetRB(Dialog,&GetHNRec->Telnet,IDC_HOSTTELNET,IDC_HOSTTELNET);
	    i = GetDlgItemInt(Dialog,IDC_HOSTTCPPORT,&Ok,FALSE);
	    if (Ok) GetHNRec->TCPPort = i;
#ifdef INET6
#define getaf(str) \
((strcmp((str), "IPv6") == 0) ? AF_INET6 : \
 ((strcmp((str), "IPv4") == 0) ? AF_INET : AF_UNSPEC))
            memset(afstr, 0, sizeof(afstr));
            GetDlgItemText(Dialog, IDC_HOSTTCPPROTOCOL, afstr, sizeof(afstr));
            GetHNRec->ProtocolFamily = getaf(afstr);
#endif /* INET6 */
	    memset(EntName,0,sizeof(EntName));
	    GetDlgItemText(Dialog, IDC_HOSTCOM, EntName, sizeof(EntName)-1);
	    GetHNRec->ComPort = (BYTE)(EntName[3])-0x30;
	    if (strlen(EntName)>4)
	      GetHNRec->ComPort = GetHNRec->ComPort*10 + (BYTE)(EntName[4])-0x30;
	  }
	  EndDialog(Dialog, 1);
	  return TRUE;

	case IDCANCEL:
	  EndDialog(Dialog, 0);
	  return TRUE;

	case IDC_HOSTTCPIP:
	  EnableDlgItem(Dialog,IDC_HOSTNAMELABEL,IDC_HOSTTCPPORT);
#ifdef INET6
          EnableDlgItem(Dialog,IDC_HOSTTCPPROTOCOLLABEL,IDC_HOSTTCPPROTOCOL);
#endif /* INET6 */
	  DisableDlgItem(Dialog,IDC_HOSTCOMLABEL,IDC_HOSTCOM);
	  return TRUE;

	case IDC_HOSTSERIAL:
	  EnableDlgItem(Dialog,IDC_HOSTCOMLABEL,IDC_HOSTCOM);
	  DisableDlgItem(Dialog,IDC_HOSTNAMELABEL,IDC_HOSTTCPPORT);
#ifdef INET6
          DisableDlgItem(Dialog,IDC_HOSTTCPPROTOCOLLABEL,IDC_HOSTTCPPROTOCOL);
#endif /* INET6 */
	  break;

	case IDC_HOSTTELNET:
	  GetRB(Dialog,&i,IDC_HOSTTELNET,IDC_HOSTTELNET);
	  if ( i==1 )
	  {
	    GetHNRec = (PGetHNRec)GetWindowLong(Dialog,DWL_USER);
	    if ( GetHNRec!=NULL )
	      SetDlgItemInt(Dialog,IDC_HOSTTCPPORT,GetHNRec->TelPort,FALSE);
	  }
	  break;

	case IDC_HOSTHELP:
	  PostMessage(GetParent(Dialog),WM_USER_DLGHELP2,0,0);
      }
  }
  return FALSE;
}

BOOL CALLBACK DirDlg(HWND Dialog, UINT Message, WPARAM wParam, LPARAM lParam)
{
  PCHAR CurDir;
  char HomeDir[MAXPATHLEN];
  char TmpDir[MAXPATHLEN];
  RECT R;
  HDC TmpDC;
  SIZE s;
  HWND HDir, HOk, HCancel, HHelp;
  POINT D, B;
  int WX, WY, WW, WH, CW, DW, DH, BW, BH;

  switch (Message) {
    case WM_INITDIALOG:
      CurDir = (PCHAR)(lParam);
      SetWindowLong(Dialog, DWL_USER, lParam);

      SetDlgItemText(Dialog, IDC_DIRCURRENT, CurDir);
      SendDlgItemMessage(Dialog, IDC_DIRNEW, EM_LIMITTEXT,
			 MAXPATHLEN-1, 0);

// adjust dialog size
      // get size of current dir text
      HDir = GetDlgItem(Dialog, IDC_DIRCURRENT);
      GetWindowRect(HDir,&R);
      D.x = R.left;
      D.y = R.top;
      ScreenToClient(Dialog,&D);
      DH = R.bottom-R.top;  
      TmpDC = GetDC(Dialog);
#ifdef TERATERM32
      GetTextExtentPoint32(TmpDC,CurDir,strlen(CurDir),&s);
#else
      GetTextExtentPoint(TmpDC,CurDir,strlen(CurDir),&s);
#endif
      ReleaseDC(Dialog,TmpDC);
      DW = s.cx + s.cx/10;

      // get button size
      HOk = GetDlgItem(Dialog, IDOK);
      HCancel = GetDlgItem(Dialog, IDCANCEL);
      HHelp = GetDlgItem(Dialog, IDC_DIRHELP);
      GetWindowRect(HHelp,&R);
      B.x = R.left;
      B.y = R.top;
      ScreenToClient(Dialog,&B);
      BW = R.right-R.left;
      BH = R.bottom-R.top;

      // calc new dialog size
      GetWindowRect(Dialog,&R);
      WX = R.left;
      WY = R.top;
      WW = R.right-R.left;
      WH = R.bottom-R.top;
      GetClientRect(Dialog,&R);
      CW = R.right-R.left;
      if (D.x+DW < CW) DW = CW-D.x;
      WW = WW + D.x+DW - CW;

      // resize current dir text
      MoveWindow(HDir,D.x,D.y,DW,DH,TRUE);
      // move buttons
      MoveWindow(HOk,(D.x+DW-4*BW)/2,B.y,BW,BH,TRUE);
      MoveWindow(HCancel,(D.x+DW-BW)/2,B.y,BW,BH,TRUE);
      MoveWindow(HHelp,(D.x+DW+2*BW)/2,B.y,BW,BH,TRUE);
      // resize edit box
      HDir = GetDlgItem(Dialog, IDC_DIRNEW);
      GetWindowRect(HDir,&R);
      D.x = R.left;
      D.y = R.top;
      ScreenToClient(Dialog,&D);
      DW = R.right-R.left;
      if (DW<s.cx) DW = s.cx;
	MoveWindow(HDir,D.x,D.y,DW,R.bottom-R.top,TRUE);
      // resize dialog
      MoveWindow(Dialog,WX,WY,WW,WH,TRUE);

      return TRUE;

    case WM_COMMAND:
      switch (LOWORD(wParam)) {
	case IDOK:
	  CurDir = (PCHAR)GetWindowLong(Dialog,DWL_USER);
	  if ( CurDir!=NULL )
	  {
	    getcwd(HomeDir,sizeof(HomeDir));
	    chdir(CurDir);
	    GetDlgItemText(Dialog, IDC_DIRNEW, TmpDir,
			   sizeof(TmpDir));
	    if ( strlen(TmpDir)>0 )
	    {
	      if (chdir(TmpDir) != 0)
	      {
		MessageBox(Dialog,"Cannot find directory",
		  "Tera Term: Error",MB_ICONEXCLAMATION);
		chdir(HomeDir);
		return TRUE;
	      }
	      getcwd(CurDir,MAXPATHLEN);
	    }
	    chdir(HomeDir);
	  }
	  EndDialog(Dialog, 1);
	  return TRUE;

	case IDCANCEL:
	  EndDialog(Dialog, 0);
	  return TRUE;

	case IDC_DIRHELP:
	  PostMessage(GetParent(Dialog),WM_USER_DLGHELP2,0,0);
      }
  }
  return FALSE;
}

// 実行ファイルからバージョン情報を得る (2005.2.28 yutaka)
static void get_file_version(char *exefile, int *major, int *minor, int *release, int *build)
{
	typedef struct {
		WORD wLanguage;
		WORD wCodePage;
	} LANGANDCODEPAGE, *LPLANGANDCODEPAGE;
	LPLANGANDCODEPAGE lplgcode;
	UINT unLen;
	DWORD size;
	char *buf = NULL;
	BOOL ret;
	int i;
	char fmt[80];
	char *pbuf;

	size = GetFileVersionInfoSize(exefile, NULL);
	if (size == 0) {
		goto error;
	}
	buf = malloc(size);
	ZeroMemory(buf, size);

	if (GetFileVersionInfo(exefile, 0, size, buf) == FALSE) {
		goto error;
	}

	ret = VerQueryValue(buf,
			"\\VarFileInfo\\Translation", 
			(LPVOID *)&lplgcode, &unLen);
	if (ret == FALSE)
		goto error;

	for (i = 0 ; i < (int)(unLen / sizeof(LANGANDCODEPAGE)) ; i++) {
		_snprintf(fmt, sizeof(fmt), "\\StringFileInfo\\%04x%04x\\FileVersion", 
			lplgcode[i].wLanguage, lplgcode[i].wCodePage);
		VerQueryValue(buf, fmt, &pbuf, &unLen);
		if (unLen > 0) { // get success
			int n, a, b, c, d;

			n = sscanf(pbuf, "%d, %d, %d, %d", &a, &b, &c, &d);
			if (n == 4) { // convert success
				*major = a;
				*minor = b;
				*release = c;
				*build = d;
				break;
			}
		}
	}

	free(buf);
	return;

error:
	free(buf);
	*major = *minor = *release = *build = 0;
}


//
// static textに書かれたURLをダブルクリックすると、ブラウザが起動するようにする。
// based on sakura editor 1.5.2.1 # CDlgAbout.cpp
// (2005.4.7 yutaka)
//

typedef struct {
	WNDPROC proc;
	BOOL mouseover;
	HFONT font;
	HWND hWnd;
	int timer_done;
} url_subclass_t;

static url_subclass_t author_url_class, forum_url_class;

// static textに割り当てるプロシージャ
static LRESULT CALLBACK UrlWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	url_subclass_t *parent = (url_subclass_t *)GetWindowLongPtr( hWnd, GWLP_USERDATA );
	HDC hdc;
	POINT pt;
	RECT rc;

	switch (msg) {
#if 0
	case WM_SETCURSOR:
		{
		// カーソル形状変更
		HCURSOR hc;

		hc = (HCURSOR)LoadImage(NULL,
				MAKEINTRESOURCE(IDC_HAND),
				IMAGE_CURSOR,
				0,
				0,
				LR_DEFAULTSIZE | LR_SHARED);
		if (hc != NULL) {
			SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG_PTR)hc);
		}
		return (LRESULT)0;
		}
#endif

	case WM_LBUTTONDBLCLK:
		{
		char url[128];

		// get URL
		SendMessage(hWnd, WM_GETTEXT , sizeof(url), (LPARAM)url);
		// kick WWW browser
	    ShellExecute(NULL, NULL, url, NULL, NULL,SW_SHOWNORMAL);
		}
		break;

	case WM_MOUSEMOVE:
		{
		BOOL bHilighted;
		pt.x = LOWORD( lParam );
		pt.y = HIWORD( lParam );
		GetClientRect( hWnd, &rc );
		bHilighted = PtInRect( &rc, pt );

		if (parent->mouseover != bHilighted) {
			parent->mouseover = bHilighted;
			InvalidateRect( hWnd, NULL, TRUE );
			if (parent->timer_done == 0) {
				parent->timer_done = 1;
				SetTimer( hWnd, 1, 200, NULL );
			}
		}

		}
		break;

	case WM_TIMER:
		// URLの上にマウスカーソルがあるなら、システムカーソルを変更する。
		if (author_url_class.mouseover || forum_url_class.mouseover) {
			HCURSOR hc;
			//SetCapture(hWnd);

			hc = (HCURSOR)LoadImage(NULL,
					MAKEINTRESOURCE(IDC_HAND),
					IMAGE_CURSOR,
					0,
					0,
					LR_DEFAULTSIZE | LR_SHARED);

			SetSystemCursor(CopyCursor(hc), 32512 /* OCR_NORMAL */);    // 矢印
			SetSystemCursor(CopyCursor(hc), 32513 /* OCR_IBEAM */);     // Iビーム

		} else {
			//ReleaseCapture();
			// マウスカーソルを元に戻す。
			SystemParametersInfo(SPI_SETCURSORS, 0, NULL, 0);

		}

		// カーソルがウィンドウ外にある場合にも WM_MOUSEMOVE を送る
		GetCursorPos( &pt );
		ScreenToClient( hWnd, &pt );
		GetClientRect( hWnd, &rc );
		if( !PtInRect( &rc, pt ) )
			SendMessage( hWnd, WM_MOUSEMOVE, 0, MAKELONG( pt.x, pt.y ) );
		break;

	case WM_PAINT: 
		{
		// ウィンドウの描画
		PAINTSTRUCT ps;
		HFONT hFont;
		HFONT hOldFont;
		TCHAR szText[512];

		hdc = BeginPaint( hWnd, &ps );

		// 現在のクライアント矩形、テキスト、フォントを取得する
		GetClientRect( hWnd, &rc );
		GetWindowText( hWnd, szText, 512 );
		hFont = (HFONT)SendMessage( hWnd, WM_GETFONT, (WPARAM)0, (LPARAM)0 );

		// テキスト描画
		SetBkMode( hdc, TRANSPARENT );
		SetTextColor( hdc, parent->mouseover ? RGB( 0x84, 0, 0 ): RGB( 0, 0, 0xff ) );
		hOldFont = (HFONT)SelectObject( hdc, (HGDIOBJ)hFont );
		TextOut( hdc, 2, 0, szText, lstrlen( szText ) );
		SelectObject( hdc, (HGDIOBJ)hOldFont );

		// フォーカス枠描画
		if( GetFocus() == hWnd )
			DrawFocusRect( hdc, &rc );

		EndPaint( hWnd, &ps );
		return 0;
		}

	case WM_ERASEBKGND:
		hdc = (HDC)wParam;
		GetClientRect( hWnd, &rc );

		// 背景描画
		if( parent->mouseover ){
			// ハイライト時背景描画
			SetBkColor( hdc, RGB( 0xff, 0xff, 0 ) );
			ExtTextOut( hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL );
		}else{
			// 親にWM_CTLCOLORSTATICを送って背景ブラシを取得し、背景描画する
			HBRUSH hbr;
			HBRUSH hbrOld;
			hbr = (HBRUSH)SendMessage( GetParent( hWnd ), WM_CTLCOLORSTATIC, wParam, (LPARAM)hWnd );
			hbrOld = (HBRUSH)SelectObject( hdc, hbr );
			FillRect( hdc, &rc, hbr );
			SelectObject( hdc, hbrOld );
		}
		return (LRESULT)1;

	case WM_DESTROY:
		// 後始末
		SetWindowLongPtr( hWnd, GWLP_WNDPROC, (LONG_PTR)parent->proc );
		if( parent->font != NULL )
			DeleteObject( parent->font );

		// マウスカーソルを元に戻す。
		SystemParametersInfo(SPI_SETCURSORS, 0, NULL, 0);
		return (LRESULT)0;
	}

	return CallWindowProc( parent->proc, hWnd, msg, wParam, lParam );
}

// static textにプロシージャを設定し、サブクラス化する。
static void do_subclass_window(HWND hWnd, url_subclass_t *parent)
{
	HFONT hFont;
	LOGFONT lf;
	LONG_PTR lptr;

	//SetCapture(hWnd);

	if (!IsWindow(hWnd))
		return;

	// 親のプロシージャをサブクラスから参照できるように、ポインタを登録しておく。
	lptr = SetWindowLongPtr( hWnd, GWLP_USERDATA, (LONG_PTR)parent );
	// サブクラスのプロシージャを登録する。
	parent->proc = (WNDPROC)SetWindowLongPtr( hWnd, GWLP_WNDPROC, (LONG_PTR)UrlWndProc);

	// 下線を付ける
	hFont = (HFONT)SendMessage( hWnd, WM_GETFONT, (WPARAM)0, (LPARAM)0 );
	GetObject( hFont, sizeof(lf), &lf );
	lf.lfUnderline = TRUE;
	parent->font = hFont = CreateFontIndirect( &lf ); // 不要になったら削除すること
	if (hFont != NULL)
		SendMessage( hWnd, WM_SETFONT, (WPARAM)hFont, (LPARAM)FALSE );

	parent->hWnd = hWnd;
	parent->timer_done = 0;
}


#ifdef WATCOM
  #pragma off (unreferenced);
#endif
BOOL CALLBACK AboutDlg(HWND Dialog, UINT Message, WPARAM wParam, LPARAM lParam)
#ifdef WATCOM
  #pragma on (unreferenced);
#endif
{
	int a, b, c, d;
	char buf[30];

	switch (Message) {
	case WM_INITDIALOG:
		// TeraTermのバージョンを設定する (2005.2.28 yutaka)
		// __argv[0]では WinExec() したプロセスから参照できないようなので削除。(2005.3.14 yutaka)
		get_file_version("ttermpro.exe", &a, &b, &c, &d);
		_snprintf(buf, sizeof(buf), "Version %d.%d", a, b);
		SendMessage(GetDlgItem(Dialog, IDC_TT_VERSION), WM_SETTEXT, 0, (LPARAM)buf);

		// static textをサブクラス化する。ただし、tabstop, notifyプロパティを有効にしておかないと
		// メッセージが拾えない。(2005.4.5 yutaka)
		do_subclass_window(GetDlgItem(Dialog, IDC_AUTHOR_URL), &author_url_class);
		do_subclass_window(GetDlgItem(Dialog, IDC_FORUM_URL), &forum_url_class);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			EndDialog(Dialog, 1);
			return TRUE;

		case IDCANCEL:
			EndDialog(Dialog, 0);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

static PCHAR far LangList[] = {"English","Japanese","Russian",NULL};

BOOL CALLBACK GenDlg(HWND Dialog, UINT Message, WPARAM wParam, LPARAM lParam)
{
  PTTSet ts;
  WORD w;
  char Temp[6];

  switch (Message) {
    case WM_INITDIALOG:
      ts = (PTTSet)lParam;
      SetWindowLong(Dialog, DWL_USER, lParam);

      SendDlgItemMessage(Dialog, IDC_GENPORT, CB_ADDSTRING,
			 0, (LPARAM)"TCP/IP");
      strcpy(Temp,"COM");
      for (w=1;w<=ts->MaxComPort;w++)
      {
	uint2str(w,&Temp[3],2);
	SendDlgItemMessage(Dialog, IDC_GENPORT, CB_ADDSTRING,
			   0, (LPARAM)Temp);
      }
      if (ts->PortType==IdSerial)
      {
	if (ts->ComPort <= ts->MaxComPort)
	  w = ts->ComPort;
	else
	  w = 1; // COM1
      }
      else
	w = 0; // TCP/IP
      SendDlgItemMessage(Dialog, IDC_GENPORT, CB_SETCURSEL,w,0);

      if ((ts->MenuFlag & MF_NOLANGUAGE)==0)
      {
	ShowDlgItem(Dialog,IDC_GENLANGLABEL,IDC_GENLANG);
	SetDropDownList(Dialog, IDC_GENLANG, LangList, ts->Language);
      }
      return TRUE;

    case WM_COMMAND:
      switch (LOWORD(wParam)) {
	case IDOK:
	  ts = (PTTSet)GetWindowLong(Dialog,DWL_USER);
	  if (ts!=NULL)
	  {
	    w = (WORD)GetCurSel(Dialog, IDC_GENPORT);
	    if (w>1)
	    {
	      ts->PortType = IdSerial;
	      ts->ComPort = w-1;
	    }
	    else
	      ts->PortType = IdTCPIP;
	    if ((ts->MenuFlag & MF_NOLANGUAGE)==0)
	      ts->Language = (WORD)GetCurSel(Dialog, IDC_GENLANG);
	  }
	  EndDialog(Dialog, 1);
	  return TRUE;

	case IDCANCEL:
	  EndDialog(Dialog, 0);
	  return TRUE;

	case IDC_GENHELP:
	  PostMessage(GetParent(Dialog),WM_USER_DLGHELP2,0,0);
      }
  }
  return FALSE;
}

BOOL CALLBACK WinListDlg(HWND Dialog, UINT Message, WPARAM wParam, LPARAM lParam)
{
  PBOOL Close;
  int n;
  HWND Hw;

  switch (Message) {
    case WM_INITDIALOG:
      Close = (PBOOL)lParam;
      SetWindowLong(Dialog, DWL_USER, lParam);
      SetWinList(GetParent(Dialog),Dialog,IDC_WINLISTLIST);
      return TRUE;

    case WM_COMMAND:
      switch (LOWORD(wParam)) {
	case IDOK:
	  n = SendDlgItemMessage(Dialog,IDC_WINLISTLIST,
	    LB_GETCURSEL, 0, 0);
	  if (n!=CB_ERR)
	    SelectWin(n);
	  EndDialog(Dialog, 1);
	  return TRUE;

	case IDCANCEL:
	  EndDialog(Dialog, 0);
	  return TRUE;
	case IDC_WINLISTLIST:
#ifdef TERATERM32
	  if (HIWORD(wParam)==LBN_DBLCLK)
#else
	  if (HIWORD(lParam)==LBN_DBLCLK)
#endif
	    PostMessage(Dialog,WM_COMMAND,IDOK,0);
	  break;
	case IDC_WINLISTCLOSE:
	  n = SendDlgItemMessage(Dialog,IDC_WINLISTLIST,
	    LB_GETCURSEL, 0, 0);
	  if (n==CB_ERR) break;
	  Hw = GetNthWin(n);
	  if (Hw!=GetParent(Dialog))
	  {
	    if (! IsWindowEnabled(Hw))
	    {
	      MessageBeep(0);
	      break;
	    }
	    SendDlgItemMessage(Dialog,IDC_WINLISTLIST,
	      LB_DELETESTRING,n,0);
	    PostMessage(Hw,WM_SYSCOMMAND,SC_CLOSE,0);
	  }
	  else {
	    Close = (PBOOL)GetWindowLong(Dialog,DWL_USER);
	    if (Close!=NULL) *Close = TRUE;
	    EndDialog(Dialog, 1);
	    return TRUE;
	  }
	  break;
	case IDC_WINLISTHELP:
	  PostMessage(GetParent(Dialog),WM_USER_DLGHELP2,0,0);
      }
  }
  return FALSE;
}

BOOL FAR PASCAL SetupTerminal(HWND WndParent, PTTSet ts)
{
#ifndef TERATERM32
  DLGPROC TermProc;
  BOOL Ok;
#endif

  int i;

  if (ts->Language==IdJapanese) // Japanese mode
    i = IDD_TERMDLGJ;
  else if (ts->Language==IdRussian) // Russian mode
    i = IDD_TERMDLGR;
  else
    i = IDD_TERMDLG;

#ifdef TERATERM32
  return
    (BOOL)DialogBoxParam(hInst,
      MAKEINTRESOURCE(i),
      WndParent, TermDlg, (LPARAM)ts);
#else
  TermProc = MakeProcInstance(TermDlg, hInst);
  Ok = (BOOL)DialogBoxParam(hInst,
    MAKEINTRESOURCE(i),
    WndParent, TermProc, (LPARAM)ts);
  FreeProcInstance(TermProc);
  return Ok;
#endif
}

BOOL FAR PASCAL SetupWin(HWND WndParent, PTTSet ts)
{
#ifndef TERATERM32
  DLGPROC WinProc;
  BOOL Ok;
#endif

#ifdef TERATERM32
  return
    (BOOL)DialogBoxParam(hInst,
      MAKEINTRESOURCE(IDD_WINDLG),
      WndParent, WinDlg, (LPARAM)ts);
#else
  WinProc = MakeProcInstance(WinDlg, hInst);
  Ok = (BOOL)DialogBoxParam(hInst,
    MAKEINTRESOURCE(IDD_WINDLG),
    WndParent, WinProc, (LPARAM)ts);
  FreeProcInstance(WinProc);
  return Ok;
#endif
}

BOOL FAR PASCAL SetupKeyboard(HWND WndParent, PTTSet ts)
{
#ifndef TERATERM32
  DLGPROC KeybProc;
  BOOL Ok;
#endif

#ifdef TERATERM32
  return
    (BOOL)DialogBoxParam(hInst,
      MAKEINTRESOURCE(IDD_KEYBDLG),
      WndParent, KeybDlg, (LPARAM)ts);
#else
  KeybProc = MakeProcInstance(KeybDlg, hInst);
  Ok = (BOOL)DialogBoxParam(hInst,
    MAKEINTRESOURCE(IDD_KEYBDLG),
    WndParent, KeybProc, (LPARAM)ts);
  FreeProcInstance(KeybProc);
  return Ok;
#endif
}

BOOL FAR PASCAL SetupSerialPort(HWND WndParent, PTTSet ts)
{
#ifndef TERATERM32
  DLGPROC SerialProc;
  BOOL Ok;
#endif

#ifdef TERATERM32
  return
    (BOOL)DialogBoxParam(hInst,
      MAKEINTRESOURCE(IDD_SERIALDLG),
      WndParent, SerialDlg, (LPARAM)ts);
#else
  SerialProc = MakeProcInstance(SerialDlg, hInst);
  Ok = (BOOL)DialogBoxParam(hInst,
    MAKEINTRESOURCE(IDD_SERIALDLG),
    WndParent, SerialProc, (LPARAM)ts);
  FreeProcInstance(SerialProc);
  return Ok;
#endif
}

BOOL FAR PASCAL SetupTCPIP(HWND WndParent, PTTSet ts)
{
#ifndef TERATERM32
  DLGPROC TCPIPProc;
  BOOL Ok;
#endif

#ifdef TERATERM32
  return
    (BOOL)DialogBoxParam(hInst,
      MAKEINTRESOURCE(IDD_TCPIPDLG),
      WndParent, TCPIPDlg, (LPARAM)ts);
#else
  TCPIPProc = MakeProcInstance(TCPIPDlg, hInst);
  Ok = (BOOL)DialogBoxParam(hInst,
    MAKEINTRESOURCE(IDD_TCPIPDLG),
    WndParent, TCPIPProc, (LPARAM)ts);
  FreeProcInstance(TCPIPProc);
  return Ok;
#endif
}

BOOL FAR PASCAL GetHostName(HWND WndParent, PGetHNRec GetHNRec)
{
#ifndef TERATERM32
  DLGPROC HostProc;
  BOOL Ok;
#endif

#ifdef TERATERM32
  return
    (BOOL)DialogBoxParam(hInst,
      MAKEINTRESOURCE(IDD_HOSTDLG),
      WndParent, HostDlg, (LPARAM)GetHNRec);
#else
  HostProc = MakeProcInstance(HostDlg, hInst);
  Ok = (BOOL)DialogBoxParam(hInst,
    MAKEINTRESOURCE(IDD_HOSTDLG),
    WndParent, HostProc, (LPARAM)GetHNRec);
  FreeProcInstance(HostProc);
  return Ok;
#endif
}

BOOL FAR PASCAL ChangeDirectory(HWND WndParent, PCHAR CurDir)
{
#ifndef TERATERM32
  DLGPROC DirProc;
  BOOL Ok;
#endif

#ifdef TERATERM32
  return
    (BOOL)DialogBoxParam(hInst,
      MAKEINTRESOURCE(IDD_DIRDLG),
      WndParent, DirDlg, (LPARAM)CurDir);
#else
  DirProc = MakeProcInstance(DirDlg, hInst);
  Ok = (BOOL)DialogBoxParam(hInst,
    MAKEINTRESOURCE(IDD_DIRDLG),
    WndParent, DirProc, (LPARAM)CurDir);
  FreeProcInstance(DirProc);
  return Ok;
#endif
}

BOOL FAR PASCAL AboutDialog(HWND WndParent)
{
#ifndef TERATERM32
  DLGPROC AboutProc;
  BOOL Ok;
#endif
#ifdef TERATERM32
  return
    (BOOL)DialogBox(hInst,
      MAKEINTRESOURCE(IDD_ABOUTDLG),
      WndParent, AboutDlg);
#else
  AboutProc = MakeProcInstance(AboutDlg, hInst);
  Ok = (BOOL)DialogBox(hInst,
    MAKEINTRESOURCE(IDD_ABOUTDLG),
    WndParent, AboutProc);
  FreeProcInstance(AboutProc);
  return Ok;
#endif
}

BOOL CALLBACK TFontHook(HWND Dialog, UINT Message, WPARAM wParam, LPARAM lParam)
{
  LPCHOOSEFONT cf;
  PTTSet ts;

  switch (Message) {
    case WM_INITDIALOG:
      cf = (LPCHOOSEFONT)lParam;
      ts = (PTTSet)(cf->lCustData);
      SetWindowLong(Dialog, DWL_USER, (LPARAM)ts);
      ShowDlgItem(Dialog,IDC_FONTBOLD,IDC_FONTBOLD);
      SetRB(Dialog,ts->EnableBold,IDC_FONTBOLD,IDC_FONTBOLD);
      if (ts->Language==IdRussian)
      {
	ShowDlgItem(Dialog,IDC_FONTCHARSET1,IDC_FONTCHARSET2);
	SetDropDownList(Dialog,IDC_FONTCHARSET2,RussList,ts->RussFont);
      }
      SetFocus(GetDlgItem(Dialog,1136));
      break;
    case WM_COMMAND:
      switch (LOWORD(wParam)) {
	case IDOK:
	  ts = (PTTSet)GetWindowLong(Dialog,DWL_USER);
	  if (ts!=NULL)
	  {
	    GetRB(Dialog,&ts->EnableBold,IDC_FONTBOLD,IDC_FONTBOLD);
	    if (ts->Language==IdRussian)
	      ts->RussFont = (WORD)GetCurSel(Dialog, IDC_FONTCHARSET2);
	  }
	  break;
	case IDCANCEL:
	  break;
      }
  }
  return FALSE;
}

#ifndef TERATERM32
  typedef UINT (CALLBACK *LPCFHOOKPROC)(HWND,UINT,WPARAM,LPARAM);
#endif

BOOL FAR PASCAL ChooseFontDlg(HWND WndParent, LPLOGFONT LogFont, PTTSet ts)
{
  CHOOSEFONT cf;
  BOOL Ok;

  memset(&cf, 0, sizeof(CHOOSEFONT));
  cf.lStructSize = sizeof(CHOOSEFONT);
  cf.hwndOwner = WndParent;
  cf.lpLogFont = LogFont;
  cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT |
	     CF_FIXEDPITCHONLY | CF_SHOWHELP | CF_ENABLETEMPLATE;
  if (ts!=NULL)
  {
    cf.Flags = cf.Flags | CF_ENABLEHOOK;
#ifdef TERATERM32
    cf.lpfnHook = (LPCFHOOKPROC)(&TFontHook);
#else
    cf.lpfnHook = (LPCFHOOKPROC)MakeProcInstance(TFontHook, hInst);
#endif
    cf.lCustData = (DWORD)ts;
  }
  cf.lpTemplateName = MAKEINTRESOURCE(IDD_FONTDLG);
  cf.nFontType = REGULAR_FONTTYPE;
  cf.hInstance = hInst;
  Ok = ChooseFont(&cf);
#ifndef TERATERM32
  FreeProcInstance(cf.lpfnHook);
#endif
  return Ok;
}

BOOL FAR PASCAL SetupGeneral(HWND WndParent, PTTSet ts)
{
#ifndef TERATERM32
  DLGPROC GenProc;
  BOOL Ok;
#endif

#ifdef TERATERM32
  return
    (BOOL)DialogBoxParam(hInst,
      MAKEINTRESOURCE(IDD_GENDLG),
      WndParent, (DLGPROC)&GenDlg, (LPARAM)ts);
#else
  GenProc = MakeProcInstance(GenDlg, hInst);
  Ok = (BOOL)DialogBoxParam(hInst,
    MAKEINTRESOURCE(IDD_GENDLG),
    WndParent, GenProc, (LPARAM)ts);
  FreeProcInstance(GenProc);
  return Ok;
#endif
}

BOOL FAR PASCAL WindowWindow(HWND WndParent, PBOOL Close)
{
#ifndef TERATERM32
  DLGPROC WinListProc;
  BOOL Ok;
#endif

  *Close = FALSE;
#ifdef TERATERM32
  return
    (BOOL)DialogBoxParam(hInst,
      MAKEINTRESOURCE(IDD_WINLISTDLG),
      WndParent,
      (DLGPROC)&WinListDlg, (LPARAM)Close);
#else
  WinListProc = MakeProcInstance(WinListDlg, hInst);
  Ok = (BOOL)DialogBoxParam(hInst,
    MAKEINTRESOURCE(IDD_WINLISTDLG),
    WndParent, WinListProc, (LPARAM)Close);
  FreeProcInstance(WinListProc);
  return Ok;
#endif
}

#ifdef TERATERM32
#ifdef WATCOM
  #pragma off (unreferenced);
#endif
BOOL WINAPI DllMain(HANDLE hInstance,
		    ULONG ul_reason_for_call,
		    LPVOID lpReserved)
#ifdef WATCOM
  #pragma on (unreferenced);
#endif
{
  hInst = hInstance;
  switch (ul_reason_for_call) {
    case DLL_THREAD_ATTACH:
      /* do thread initialization */
      break;
    case DLL_THREAD_DETACH:
      /* do thread cleanup */
      break;
    case DLL_PROCESS_ATTACH:
      /* do process initialization */
      break;
    case DLL_PROCESS_DETACH:
      /* do process cleanup */
      break;
  }
  return TRUE;
}
#else
#ifdef WATCOM
#pragma off (unreferenced);
#endif
int CALLBACK LibMain(HANDLE hInstance, WORD wDataSegment,
		     WORD wHeapSize, LPSTR lpszCmdLine)
#ifdef WATCOM
#pragma on (unreferenced);
#endif
{
  hInst = hInstance;
  return (1);
}
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2005/04/03 13:42:07  yutakakn
 * URL文字列をダブルクリックするとブラウザが起動するしかけを追加（石崎氏パッチがベース）。
 *
 * Revision 1.4  2005/03/14 13:29:40  yutakakn
 * 2つめ以降のプロセスで、TeraTermバージョンが正しく取得されない問題への対処。
 *
 * Revision 1.3  2005/02/28 14:30:35  yutakakn
 * バージョンダイアログに表示するTeraTermのバージョンを、ttermpro.exeの
 * バージョン情報から取得するようにした。
 *
 * Revision 1.2  2004/11/28 13:14:51  yutakakn
 * スクロールバッファの入力桁数を5から8へ拡張した。
 * 
 */
