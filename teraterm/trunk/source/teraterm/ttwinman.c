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
	int i;
	char TempTitle[HostNameMaxLength + 50 + 1]; // �o�b�t�@�g��
	char Num[11];

	strcpy(TempTitle, ts.Title);

	if ((ts.TitleFormat & 1)!=0)
	{ // host name
		strncat(TempTitle," - ",sizeof(TempTitle)-1-strlen(TempTitle));
		i = sizeof(TempTitle)-1-strlen(TempTitle);
		if (Connecting) {
#ifndef NO_I18N
			strcpy(ts.UIMsg, "[connecting...]");
			get_lang_msg("DLG_MAIN_TITLE_CONNECTING", ts.UIMsg, ts.UILanguageFile);
			strncat(TempTitle,ts.UIMsg,i);
#else
			strncat(TempTitle,"[connecting...]",i);
#endif
		}
		else if (! cv.Ready) {
#ifndef NO_I18N
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
			// COM5 over�ɑΉ�
			char str[20]; // COMxx:xxxxxxbaud
			if (ts.TitleFormat & 32) {
				char baud[10];
				switch (ts.Baud) {
					case IdBaud110:    strncpy_s(baud, sizeof(baud), "110",    _TRUNCATE); break;
					case IdBaud300:    strncpy_s(baud, sizeof(baud), "300",    _TRUNCATE); break;
					case IdBaud600:    strncpy_s(baud, sizeof(baud), "600",    _TRUNCATE); break;
					case IdBaud1200:   strncpy_s(baud, sizeof(baud), "1200",   _TRUNCATE); break;
					case IdBaud2400:   strncpy_s(baud, sizeof(baud), "2400",   _TRUNCATE); break;
					case IdBaud4800:   strncpy_s(baud, sizeof(baud), "4800",   _TRUNCATE); break;
					case IdBaud9600:   strncpy_s(baud, sizeof(baud), "9600",   _TRUNCATE); break;
					case IdBaud14400:  strncpy_s(baud, sizeof(baud), "14400",  _TRUNCATE); break;
					case IdBaud19200:  strncpy_s(baud, sizeof(baud), "19200",  _TRUNCATE); break;
					case IdBaud38400:  strncpy_s(baud, sizeof(baud), "38400",  _TRUNCATE); break;
					case IdBaud57600:  strncpy_s(baud, sizeof(baud), "57600",  _TRUNCATE); break;
#ifdef TERATERM32
					case IdBaud115200: strncpy_s(baud, sizeof(baud), "115200", _TRUNCATE); break;
#endif
					case IdBaud230400: strncpy_s(baud, sizeof(baud), "230400", _TRUNCATE); break;
					case IdBaud460800: strncpy_s(baud, sizeof(baud), "460800", _TRUNCATE); break;
					case IdBaud921600: strncpy_s(baud, sizeof(baud), "921600", _TRUNCATE); break;
					default: strncpy_s(baud, sizeof(baud), "", _TRUNCATE); break;
				}
				_snprintf_s(str, sizeof(str), _TRUNCATE, "COM%d:%sbaud", ts.ComPort, baud);
			}
			else {
				_snprintf_s(str, sizeof(str), _TRUNCATE, "COM%d", ts.ComPort);
			}

			if (ts.TitleFormat & 8) {
				_snprintf(TempTitle, sizeof(TempTitle), "%s - %s", str, ts.Title);
			} else {
				strncat_s(TempTitle, sizeof(TempTitle), str, _TRUNCATE); 
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
				if (ts.TitleFormat & 16) {
					_snprintf_s(TempTitle, sizeof(TempTitle), _TRUNCATE, "%s:%d - %s", ts.HostName, ts.TCPPort, ts.Title);
				}
				else {
					// format ID = 13(8 + 5): <hots/port> - <title>
					_snprintf_s(TempTitle, sizeof(TempTitle), _TRUNCATE, "%s - %s", ts.HostName, ts.Title);
				}
			} else {
				if (ts.TitleFormat & 16) {
					_snprintf_s(TempTitle, sizeof(TempTitle), _TRUNCATE, "%s - %s:%d", ts.Title, ts.HostName, ts.TCPPort);
				}
				else {
					strncat_s(TempTitle, sizeof(TempTitle), ts.HostName, _TRUNCATE);
				}
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

  // �w���v�̃I�[�i�[�͏�Ƀf�X�N�g�b�v�ɂȂ� (2007.5.12 maya)
  HWin = GetDesktopWindow();
  _snprintf(HelpFN, sizeof(HelpFN), "%s\\%s", ts.HomeDir, ts.UIMsg);
  if (HtmlHelp(HWin, HelpFN, Command, Data) == NULL && Command != HH_CLOSE_ALL) {
    char buf[MAXPATHLEN];
    strcpy(ts.UIMsg, "Can't open HTML help file(%s).");
    get_lang_msg("MSG_OPENHELP_ERROR", ts.UIMsg, ts.UILanguageFile);
    _snprintf(buf, sizeof(buf), ts.UIMsg, HelpFN);
    MessageBox(HWin, buf, "Tera Term: HTML help", MB_OK | MB_ICONERROR);
  }
}

// HTML help ���J�� 
// HTML Help workshop�Ɋ܂܂�� htmlhelp.h �� htmlhelp.lib ��2�̃t�@�C�����A�r���h���ɕK�v�B
// (2006.3.11 yutaka)
#if 0
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
#endif


/*
 * $Log: not supported by cvs2svn $
 * Revision 1.12  2007/06/06 14:02:53  maya
 * �v���v���Z�b�T�ɂ��\���̂��ς���Ă��܂��̂ŁAINET6 �� I18N �� #define ���t�]�������B
 *
 * Revision 1.11  2007/05/11 18:47:01  maya
 * �w���v�̃I�[�i�[����Ƀf�X�N�g�b�v�ɂȂ�悤�ɕύX�����B
 *
 * Revision 1.10  2007/04/09 15:42:57  maya
 * hlp �̑ւ��� chm ���J���悤�ɕύX�����B
 *
 * Revision 1.9  2006/12/23 02:50:17  maya
 * html�w���v���v���O��������Ăяo�����߂̏����������B
 *
 * Revision 1.8  2006/11/23 02:19:12  maya
 * �\�����b�Z�[�W������t�@�C������ǂݍ��݂ރR�[�h�̍쐬���J�n�����B
 *
 * Revision 1.7  2006/06/15 16:27:59  yutakakn
 * �^�C�g���ɓ��{�����͂���ƁA������������o�O���C�������B�p�b�`�쐬�Ɋ��ӂ��܂����i�c��
 *
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
