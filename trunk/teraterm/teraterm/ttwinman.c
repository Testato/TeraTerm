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
#include "commlib.h"

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


// �^�C�g���o�[��CP932�ւ̕ϊ����s��
// ���݁ASJIS�AEUC�݂̂ɑΉ��B
// (2005.3.13 yutaka)
void ConvertToCP932(char *str, int destlen)
{
#define IS_SJIS(n) (ts.KanjiCode == IdSJIS && IsDBCSLeadByte(n))
#define IS_EUC(n) (ts.KanjiCode == IdEUC && (n & 0x80))
	extern WORD FAR PASCAL JIS2SJIS(WORD KCode);
	int len = strlen(str);
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
					// SJIS�͂��̂܂�CP932�Ƃ��ďo�͂���

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
		strncpy_s(str, destlen, cc, _TRUNCATE);
	}
}

// �L���v�V�����̕ύX
//
// (2005.2.19 yutaka) format ID=13�̐V�K�ǉ��ACOM5�ȏ�̕\���ɑΉ�
// (2005.3.13 yutaka) �^�C�g����SJIS�ւ̕ϊ��i���{��j��ǉ�
// (2006.6.15 maya)   ts.KanjiCode��EUC���ƁASJIS�ł�EUC�Ƃ���
//                    �ϊ����Ă��܂��̂ŁA�����ł͕ϊ����Ȃ�
// (2007.7.19 maya)   TCP �|�[�g�ԍ� �� �V���A���|�[�g�̃{�[���[�g�̕\���ɑΉ�
/*
 *  TitleFormat
 *    0 0 0 0 0 0 (2)
 *    | | | | | +----- displays TCP host/serial port
 *    | | | | +------- displays session no
 *    | | | +--------- displays VT/TEK
 *    | | +----------- displays TCP host/serial port first
 *    | +------------- displays TCP port number
 *    +--------------- displays baud rate of serial port
 */
void ChangeTitle()
{
	char TempTitle[HostNameMaxLength + TitleBuffSize * 2 + 1]; // �o�b�t�@�g��
	char TempTitleWithRemote[TitleBuffSize * 2];

	if (Connecting || !cv.Ready || strlen(cv.TitleRemote) == 0) {
		strncpy_s(TempTitleWithRemote, sizeof(TempTitleWithRemote), ts.Title, _TRUNCATE);
		strncpy_s(TempTitle, sizeof(TempTitle), ts.Title, _TRUNCATE);
	}
	else {
		// �����[�g����̃^�C�g����ʂɈ��� (2008.11.1 maya)
		if (ts.AcceptTitleChangeRequest == IdTitleChangeRequestOff) {
			strncpy_s(TempTitleWithRemote, sizeof(TempTitleWithRemote), ts.Title, _TRUNCATE);
		}
		else if (ts.AcceptTitleChangeRequest == IdTitleChangeRequestAhead) {
			_snprintf_s(TempTitleWithRemote, sizeof(TempTitleWithRemote), _TRUNCATE,
			            "%s %s", cv.TitleRemote, ts.Title);
		}
		else if (ts.AcceptTitleChangeRequest == IdTitleChangeRequestLast) {
			_snprintf_s(TempTitleWithRemote, sizeof(TempTitleWithRemote), _TRUNCATE,
			            "%s %s", ts.Title, cv.TitleRemote);
		}
		else {
			strncpy_s(TempTitleWithRemote, sizeof(TempTitleWithRemote), cv.TitleRemote, _TRUNCATE);
		}
		strncpy_s(TempTitle, sizeof(TempTitle), TempTitleWithRemote, _TRUNCATE);
	}

	if ((ts.TitleFormat & 1)!=0)
	{ // host name
		strncat_s(TempTitle,sizeof(TempTitle), " - ",_TRUNCATE);
		if (Connecting) {
			get_lang_msg("DLG_MAIN_TITLE_CONNECTING", ts.UIMsg, sizeof(ts.UIMsg), "[connecting...]", ts.UILanguageFile);
			strncat_s(TempTitle,sizeof(TempTitle),ts.UIMsg,_TRUNCATE);
		}
		else if (! cv.Ready) {
			get_lang_msg("DLG_MAIN_TITLE_DISCONNECTED", ts.UIMsg, sizeof(ts.UIMsg), "[disconnected]", ts.UILanguageFile);
			strncat_s(TempTitle,sizeof(TempTitle),ts.UIMsg,_TRUNCATE);
		}
		else if (cv.PortType==IdSerial)
		{
			// COM5 over�ɑΉ�
			char str[24]; // COMxxxx:xxxxxxxxxxbaud
			if (ts.TitleFormat & 32) {
				_snprintf_s(str, sizeof(str), _TRUNCATE, "COM%d:%ubaud", ts.ComPort, ts.Baud);
			}
			else {
				_snprintf_s(str, sizeof(str), _TRUNCATE, "COM%d", ts.ComPort);
			}

			if (ts.TitleFormat & 8) {
				_snprintf_s(TempTitle, sizeof(TempTitle), _TRUNCATE, "%s - %s", str, TempTitleWithRemote);
			} else {
				strncat_s(TempTitle, sizeof(TempTitle), str, _TRUNCATE);
			}
		}
		else if (cv.PortType == IdNamedPipe)
		{
			char str[sizeof(TempTitle)];
			strncpy_s(str, sizeof(str), ts.HostName, _TRUNCATE);

			if (ts.TitleFormat & 8) {
				// format ID = 13(8 + 5): <hots/port> - <title>
				_snprintf_s(TempTitle, sizeof(TempTitle), _TRUNCATE, "%s - %s", str, TempTitleWithRemote);
			}
			else {
				strncat_s(TempTitle, sizeof(TempTitle), str, _TRUNCATE);
			}
		}
		else {
			char str[sizeof(TempTitle)];
			if (ts.TitleFormat & 16) {
				_snprintf_s(str, sizeof(str), _TRUNCATE, "%s:%d", ts.HostName, ts.TCPPort);
			}
			else {
				strncpy_s(str, sizeof(str), ts.HostName, _TRUNCATE);
			}

			if (ts.TitleFormat & 8) {
				// format ID = 13(8 + 5): <hots/port> - <title>
				_snprintf_s(TempTitle, sizeof(TempTitle), _TRUNCATE, "%s - %s", str, TempTitleWithRemote);
			}
			else {
				strncat_s(TempTitle, sizeof(TempTitle), str, _TRUNCATE);
			}
		}
	}

	if ((ts.TitleFormat & 2)!=0)
	{ // serial no.
		char Num[11];
		strncat_s(TempTitle,sizeof(TempTitle)," (",_TRUNCATE);
		_snprintf_s(Num,sizeof(Num),_TRUNCATE,"%u",SerialNo);
		strncat_s(TempTitle,sizeof(TempTitle),Num,_TRUNCATE);
		strncat_s(TempTitle,sizeof(TempTitle),")",_TRUNCATE);
	}

	if ((ts.TitleFormat & 4)!=0) // VT
		strncat_s(TempTitle,sizeof(TempTitle)," VT",_TRUNCATE);

	SetWindowText(HVTWin,TempTitle);

	if (HTEKWin!=0)
	{
		if ((ts.TitleFormat & 4)!=0) // TEK
		{
			strncat_s(TempTitle,sizeof(TempTitle)," TEK",_TRUNCATE);
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
