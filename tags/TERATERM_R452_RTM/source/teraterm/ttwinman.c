/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TERATERM.EXE, variables, flags related to VT win and TEK win */

#include "teraterm.h"
#include "tttypes.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "ttlib.h"
#include "helpid.h"
#include "htmlhelp.h"
#include "i18n.h"

/* help file names */
#define HTML_HELP "teraterm.chm"

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


// タイトルバーのCP932への変換を行う
// 現在、SJIS、EUCのみに対応。
// (2005.3.13 yutaka)
void ConvertToCP932(char *str, int len)
{
#define IS_SJIS(n) (ts.KanjiCode == IdSJIS && IsDBCSLeadByte(n))
#define IS_EUC(n) (ts.KanjiCode == IdEUC && (n & 0x80))
	extern WORD FAR PASCAL JIS2SJIS(WORD KCode);
	char *cc = _alloca(len + 1);
	char *c = cc;
	int i;
	unsigned char b;
	WORD word;

	if (strcmp(ts.Locale, DEFAULT_LOCALE) == 0) {
		for (i = 0 ; i < len ; i++) {
			b = str[i];
			if (IS_SJIS(b) || IS_EUC(b)) {
				word = b<<8;

				if (i == len - 1) {
					*c++ = b;
					continue;
				}

				b = str[i + 1];
				word |= b;
				i++;

				if (ts.KanjiCode == IdSJIS) {
					// SJISはそのままCP932として出力する

				} else if (ts.KanjiCode == IdEUC) {
					// EUC -> SJIS
					word &= ~0x8080;
					word = JIS2SJIS(word);

				} else if (ts.KanjiCode == IdJIS) {

				} else if (ts.KanjiCode == IdUTF8) {

				} else if (ts.KanjiCode == IdUTF8m) {

				} else {

				}

				*c++ = word >> 8;
				*c++ = word & 0xff;

			} else {
				*c++ = b;
			}
		}

		*c = '\0';
		strcpy(str, cc);
	}
}

// キャプションの変更
//
// (2005.2.19 yutaka) format ID=13の新規追加、COM5以上の表示に対応
// (2005.3.13 yutaka) タイトルのSJISへの変換（日本語）を追加
// (2006.6.15 maya)   ts.KanjiCodeがEUCだと、SJISでもEUCとして
//                    変換してしまうので、ここでは変換しない
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
		if (Connecting) {
#ifdef I18N
			strcpy(ts.UIMsg, "[connecting...]");
			get_lang_msg("DLG_MAIN_TITLE_CONNECTING", ts.UIMsg, ts.UILanguageFile);
			strncat(TempTitle,ts.UIMsg,i);
#else
			strncat(TempTitle,"[connecting...]",i);
#endif
		}
		else if (! cv.Ready) {
#ifdef I18N
			strcpy(ts.UIMsg, "[disconnected]");
			get_lang_msg("DLG_MAIN_TITLE_DISCONNECTED", ts.UIMsg, ts.UILanguageFile);
			strncat(TempTitle,ts.UIMsg,i);
#else
			strncat(TempTitle,"[disconnected]",i);
#endif
		}
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

  strcpy(ts.UIMsg, HTML_HELP);
  get_lang_msg("HELPFILE", ts.UIMsg, ts.UILanguageFile);

  if (HWin == NULL) {
    HWin = GetDesktopWindow();
  }
  _snprintf(HelpFN, sizeof(HelpFN), "%s\\%s", ts.HomeDir, ts.UIMsg);
  if (HtmlHelp(HWin, HelpFN, Command, Data) == NULL && Command != HH_CLOSE_ALL) {
    char buf[MAXPATHLEN];
    strcpy(ts.UIMsg, "Can't open HTML help file(%s).");
    get_lang_msg("MSG_OPENHELP_ERROR", ts.UIMsg, ts.UILanguageFile);
    _snprintf(buf, sizeof(buf), ts.UIMsg, HelpFN);
    MessageBox(HWin, buf, "Tera Term: HTML help", MB_OK | MB_ICONERROR);
  }
}

// HTML help を開く 
// HTML Help workshopに含まれる htmlhelp.h と htmlhelp.lib の2つのファイルが、ビルド時に必要。
// (2006.3.11 yutaka)
#if 0
void OpenHtmlHelp(HWND HWin, char *filename)
{
	char HelpFN[MAXPATHLEN];

	_snprintf(HelpFN, sizeof(HelpFN), "%s\\%s", ts.HomeDir, filename);
	// HTMLヘルプのオーナーをTeraTermからデスクトップへ変更 (2006.4.7 yutaka)
	if (HtmlHelp(GetDesktopWindow(), HelpFN, HH_DISPLAY_TOPIC, 0) == NULL) {
		char buf[MAXPATHLEN];
		_snprintf(buf, sizeof(buf), "Can't open HTML help file(%s).", HelpFN);
		MessageBox(HWin, buf, "Tera Term: HTML help", MB_OK | MB_ICONERROR);
	}
}
#endif


/*
 * $Log: not supported by cvs2svn $
 * Revision 1.9  2006/12/23 02:50:17  maya
 * htmlヘルプをプログラムから呼び出すための準備をした。
 *
 * Revision 1.8  2006/11/23 02:19:12  maya
 * 表示メッセージを言語ファイルから読み込みむコードの作成を開始した。
 *
 * Revision 1.7  2006/06/15 16:27:59  yutakakn
 * タイトルに日本語を入力すると、文字化けするバグを修正した。パッチ作成に感謝します＞永田氏
 *
 * Revision 1.6  2006/04/07 13:16:39  yutakakn
 * HTMLヘルプファイルのオーナーをデスクトップへ変更した
 *
 * Revision 1.5  2006/03/10 15:44:29  yutakakn
 * ヘルプファイルを .hlp から .chm へ変更した
 *
 * Revision 1.4  2005/03/13 04:05:11  yutakakn
 * タイトルバーに日本語を設定する場合、Shift_JIS(CP932)へ変換するようにした（EUC-JPのみに対応）。
 *
 * Revision 1.3  2005/02/21 14:52:11  yutakakn
 * TitleFormat=13において、COMの場合も入れ替えるようにした。
 *
 * Revision 1.2  2005/02/19 07:02:59  yutakakn
 * TitleFormatに 13 を追加。
 * COM5以上の表示に対応した。
 * キャプションバッファの拡張。
 *
 */
