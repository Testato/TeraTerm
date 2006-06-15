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
		strcpy(str, cc);
	}
}

// �L���v�V�����̕ύX
//
// (2005.2.19 yutaka) format ID=13�̐V�K�ǉ��ACOM5�ȏ�̕\���ɑΉ�
// (2005.3.13 yutaka) �^�C�g����SJIS�ւ̕ϊ��i���{��j��ǉ�
// (2006.6.15 maya)   ts.KanjiCode��EUC���ƁASJIS�ł�EUC�Ƃ���
//                    �ϊ����Ă��܂��̂ŁA�����ł͕ϊ����Ȃ�
void ChangeTitle()
{
	int i;
	char TempTitle[HostNameMaxLength + 50 + 1]; // �o�b�t�@�g��
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
			// COM5 over�ɑΉ�
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

// HTML help ���J�� 
// HTML Help workshop�Ɋ܂܂�� htmlhelp.h �� htmlhelp.lib ��2�̃t�@�C�����A�r���h���ɕK�v�B
// (2006.3.11 yutaka)
void OpenHtmlHelp(HWND HWin, char *filename)
{
	char HelpFN[MAXPATHLEN];

	_snprintf(HelpFN, sizeof(HelpFN), "%s\\%s", ts.HomeDir, filename);
	// HTML�w���v�̃I�[�i�[��TeraTerm����f�X�N�g�b�v�֕ύX (2006.4.7 yutaka)
	if (HtmlHelp(GetDesktopWindow(), HelpFN, HH_DISPLAY_TOPIC, 0) == NULL) {
		char buf[MAXPATHLEN];
		_snprintf(buf, sizeof(buf), "Can't open HTML help file(%s).", HelpFN);
		MessageBox(HWin, buf, "Tera Term: HTML help", MB_OK | MB_ICONERROR);
	}
}


/*
 * $Log: not supported by cvs2svn $
 * Revision 1.6  2006/04/07 13:16:39  yutakakn
 * HTML�w���v�t�@�C���̃I�[�i�[���f�X�N�g�b�v�֕ύX����
 *
 * Revision 1.5  2006/03/10 15:44:29  yutakakn
 * �w���v�t�@�C���� .hlp ���� .chm �֕ύX����
 *
 * Revision 1.4  2005/03/13 04:05:11  yutakakn
 * �^�C�g���o�[�ɓ��{���ݒ肷��ꍇ�AShift_JIS(CP932)�֕ϊ�����悤�ɂ����iEUC-JP�݂̂ɑΉ��j�B
 *
 * Revision 1.3  2005/02/21 14:52:11  yutakakn
 * TitleFormat=13�ɂ����āACOM�̏ꍇ������ւ���悤�ɂ����B
 *
 * Revision 1.2  2005/02/19 07:02:59  yutakakn
 * TitleFormat�� 13 ��ǉ��B
 * COM5�ȏ�̕\���ɑΉ������B
 * �L���v�V�����o�b�t�@�̊g���B
 *
 */
