/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TERATERM.EXE, variables, flags related to VT win and TEK win */

#include "teraterm.h"
#include "tttypes.h"
#include <stdio.h>
#include <string.h>
#include "ttlib.h"
#include "helpid.h"

HWND HVTWin = NULL;
HWND HTEKWin = NULL;
  
int ActiveWin = IdVT; /* IdVT, IdTEK */
int TalkStatus = IdTalkKeyb; /* IdTalkKeyb, IdTalkCB, IdTalkTextFile */
BOOL KeybEnabled = TRUE; /* keyboard switch */
BOOL Connecting = FALSE;

/* 'help' button on dialog box */
WORD MsgDlgHelp;
LONG HelpId;

TTTSet ts;
TComVar cv;

/* pointers to window objects */
void* pVTWin = NULL;
void* pTEKWin = NULL;
/* instance handle */
HINSTANCE hInst;

int SerialNo;

void VTActivate()
{
  ActiveWin = IdVT;
  ShowWindow(HVTWin, SW_SHOWNORMAL);
  SetFocus(HVTWin);
}

// キャプションの変更
//
// (2005.2.19 yutaka) format ID=13の新規追加、COM5以上の表示に対応
void ChangeTitle()
{
	int i;
	char TempTitle[HostNameMaxLength + 50 + 1]; // バッファ拡張
	char Num[11];

	strcpy(TempTitle, ts.Title);

	if ((ts.TitleFormat & 1)!=0)
	{ // host name
		strncat(TempTitle," - ",sizeof(TempTitle)-1-strlen(TempTitle));
		i = sizeof(TempTitle)-1-strlen(TempTitle);
		if (Connecting)
			strncat(TempTitle,"[connecting...]",i);
		else if (! cv.Ready)
			strncat(TempTitle,"[disconnected]",i);
		else if (cv.PortType==IdSerial)
		{
#if 1
			// COM5 overに対応
			char str[10];
			_snprintf(str, sizeof(str), "COM%d", ts.ComPort);

			if (ts.TitleFormat & 8) {
				_snprintf(TempTitle, sizeof(TempTitle), "%s - %s", str, ts.Title);
			} else {
				strncat(TempTitle, str, i); 
			}

#else
			switch (ts.ComPort) {
			case 1: strncat(TempTitle,"COM1",i); break;
			case 2: strncat(TempTitle,"COM2",i); break;
			case 3: strncat(TempTitle,"COM3",i); break;
			case 4: strncat(TempTitle,"COM4",i); break;
			}
#endif
		}
		else {
			if (ts.TitleFormat & 8) {
				// format ID = 13(8 + 5): <hots/port> - <title>
				_snprintf(TempTitle, sizeof(TempTitle), "%s - %s", ts.HostName, ts.Title);

			} else {
				strncat(TempTitle,ts.HostName,i);
			}
		}
	}

	if ((ts.TitleFormat & 2)!=0)
	{ // serial no.
		strncat(TempTitle," (",sizeof(TempTitle)-1-strlen(TempTitle));
		sprintf(Num,"%u",SerialNo);
		strncat(TempTitle,Num,sizeof(TempTitle)-1-strlen(TempTitle));
		strncat(TempTitle,")",sizeof(TempTitle)-1-strlen(TempTitle));
	}

	if ((ts.TitleFormat & 4)!=0) // VT
		strncat(TempTitle," VT",sizeof(TempTitle)-1-strlen(TempTitle));

	SetWindowText(HVTWin,TempTitle);

	if (HTEKWin!=0)
	{
		if ((ts.TitleFormat & 4)!=0) // TEK
		{
			TempTitle[strlen(TempTitle)-2] = 0;
			strncat(TempTitle,"TEK",
				sizeof(TempTitle)-1-strlen(TempTitle));
		}
		SetWindowText(HTEKWin,TempTitle);
	}
}

void SwitchMenu()
{
  HWND H1, H2;

  if (ActiveWin==IdVT)
  {
    H1 = HTEKWin;
    H2 = HVTWin;
  }
  else {
    H1 = HVTWin;
    H2 = HTEKWin;
  }

  if (H1!=0)
    PostMessage(H1,WM_USER_CHANGEMENU,0,0);
  if (H2!=0)
    PostMessage(H2,WM_USER_CHANGEMENU,0,0);
}

void SwitchTitleBar()
{
  HWND H1, H2;

  if (ActiveWin==IdVT)
  {
    H1 = HTEKWin;
    H2 = HVTWin;
  }
  else {
    H1 = HVTWin;
    H2 = HTEKWin;
  }

  if (H1!=0)
    PostMessage(H1,WM_USER_CHANGETBAR,0,0);
  if (H2!=0)
    PostMessage(H2,WM_USER_CHANGETBAR,0,0);
}

void OpenHelp(HWND HWin, UINT Command, DWORD Data)
{
  char HelpFN[MAXPATHLEN];

  strcpy(HelpFN,ts.HomeDir);
  AppendSlash(HelpFN);
  if (ts.Language==IdJapanese)
    strcat(HelpFN,HelpJpn);
  else
    strcat(HelpFN,HelpEng);
  WinHelp(HWin, HelpFN, Command, Data);
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2005/02/19 07:02:59  yutakakn
 * TitleFormatに 13 を追加。
 * COM5以上の表示に対応した。
 * キャプションバッファの拡張。
 *
 */
