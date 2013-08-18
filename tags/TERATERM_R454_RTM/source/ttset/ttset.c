/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */
/* IPv6 modification is Copyright(C) 2000 Jun-ya kato <kato@win6.jp> */

/* TTSET.DLL, setup file routines*/
#ifndef NO_INET6
#include <winsock2.h>
#include <ws2tcpip.h>
#endif							/* NO_INET6 */
#include "teraterm.h"
#include "tttypes.h"
#include <stdio.h>
#include <string.h>
#include <direct.h>
#include "ttlib.h"

#define Section "Tera Term"

static PCHAR far TermList[] =
	{ "VT100", "VT100J", "VT101", "VT102", "VT102J", "VT220J", "VT282",
	"VT320", "VT382", NULL };
// expansion (2005.11.30 yutaka)
static PCHAR BaudList[] =
	{ "110", "300", "600", "1200", "2400", "4800", "9600",
	"14400", "19200", "38400", "57600", "115200",
	"230400", "460800", "921600", NULL
};

static PCHAR far RussList[] =
	{ "Windows", "KOI8-R", "CP-866", "ISO-8859-5", NULL };
static PCHAR far RussList2[] = { "Windows", "KOI8-R", NULL };

WORD str2id(PCHAR far * List, PCHAR str, WORD DefId)
{
	WORD i;
	i = 0;
	while ((List[i] != NULL) && (_stricmp(List[i], str) != 0))
		i++;
	if (List[i] == NULL)
		i = DefId;
	else
		i++;

	return i;
}

void id2str(PCHAR far * List, WORD Id, WORD DefId, PCHAR str, int destlen)
{
	int i;

	if (Id == 0)
		i = DefId - 1;
	else {
		i = 0;
		while ((List[i] != NULL) && (i < Id - 1))
			i++;
		if (List[i] == NULL)
			i = DefId - 1;
	}
	strncpy_s(str, destlen, List[i], _TRUNCATE);
}

WORD GetOnOff(PCHAR Sect, PCHAR Key, PCHAR FName, BOOL Default)
{
	char Temp[4];
	GetPrivateProfileString(Sect, Key, "", Temp, sizeof(Temp), FName);
	if (Default) {
		if (_stricmp(Temp, "off") == 0)
			return 0;
		else
			return 1;
	}
	else {
		if (_stricmp(Temp, "on") == 0)
			return 1;
		else
			return 0;
	}
}

void WriteOnOff(PCHAR Sect, PCHAR Key, PCHAR FName, WORD Flag)
{
	char Temp[4];

	if (Flag != 0)
		strncpy_s(Temp, sizeof(Temp), "on", _TRUNCATE);
	else
		strncpy_s(Temp, sizeof(Temp), "off", _TRUNCATE);
	WritePrivateProfileString(Sect, Key, Temp, FName);
}

void WriteInt(PCHAR Sect, PCHAR Key, PCHAR FName, int i)
{
	char Temp[15];
	_snprintf_s(Temp, sizeof(Temp), _TRUNCATE, "%d", i);
	WritePrivateProfileString(Sect, Key, Temp, FName);
}

void WriteUint(PCHAR Sect, PCHAR Key, PCHAR FName, UINT i)
{
	char Temp[15];
	_snprintf_s(Temp, sizeof(Temp), _TRUNCATE, "%u", i);
	WritePrivateProfileString(Sect, Key, Temp, FName);
}

void WriteInt2(PCHAR Sect, PCHAR Key, PCHAR FName, int i1, int i2)
{
	char Temp[32];
	_snprintf_s(Temp, sizeof(Temp), _TRUNCATE, "%d,%d", i1, i2);
	WritePrivateProfileString(Sect, Key, Temp, FName);
}

void WriteInt4(PCHAR Sect, PCHAR Key, PCHAR FName,
			   int i1, int i2, int i3, int i4)
{
	char Temp[64];
	_snprintf_s(Temp, sizeof(Temp), _TRUNCATE, "%d,%d,%d,%d",
	            i1, i2, i3, i4);
	WritePrivateProfileString(Sect, Key, Temp, FName);
}

void WriteInt6(PCHAR Sect, PCHAR Key, PCHAR FName,
			   int i1, int i2, int i3, int i4, int i5, int i6)
{
	char Temp[96];
	_snprintf_s(Temp, sizeof(Temp), _TRUNCATE, "%d,%d,%d,%d,%d,%d",
	            i1, i2,i3, i4, i5, i6);
	WritePrivateProfileString(Sect, Key, Temp, FName);
}

void WriteFont(PCHAR Sect, PCHAR Key, PCHAR FName,
			   PCHAR Name, int x, int y, int charset)
{
	char Temp[80];
	if (Name[0] != 0)
		_snprintf_s(Temp, sizeof(Temp), _TRUNCATE, "%s,%d,%d,%d",
		            Name, x, y, charset);
	else
		Temp[0] = 0;
	WritePrivateProfileString(Sect, Key, Temp, FName);
}

void FAR PASCAL ReadIniFile(PCHAR FName, PTTSet ts)
{
	int i;
	HDC TmpDC;
	char Temp[MAXPATHLEN];

	ts->Minimize = 0;
	ts->HideWindow = 0;
	ts->LogFlag = 0;			// Log flags
	ts->FTFlag = 0;				// File transfer flags
	ts->MenuFlag = 0;			// Menu flags
	ts->TermFlag = 0;			// Terminal flag
	ts->ColorFlag = 0;			// ANSI color flags
	ts->PortFlag = 0;			// Port flags
	ts->TelPort = 23;

	/* Version number */
/*  GetPrivateProfileString(Section,"Version","",
			  Temp,sizeof(Temp),FName); */

	/* Language */
	GetPrivateProfileString(Section, "Language", "",
	                        Temp, sizeof(Temp), FName);
	if (_stricmp(Temp, "Japanese") == 0)
		ts->Language = IdJapanese;
	else if (_stricmp(Temp, "Russian") == 0)
		ts->Language = IdRussian;
	else if (_stricmp(Temp, "English") == 0)
		ts->Language = IdEnglish;
	else {
		switch (PRIMARYLANGID(GetSystemDefaultLangID())) {
		case LANG_JAPANESE:
			ts->Language = IdJapanese;
			break;
		case LANG_RUSSIAN:
			ts->Language = IdRussian;
			break;
		default:
			ts->Language = IdEnglish;
		}
	}

	/* Port type */
	GetPrivateProfileString(Section, "Port", "",
	                        Temp, sizeof(Temp), FName);
	if (_stricmp(Temp, "tcpip") == 0)
		ts->PortType = IdTCPIP;
	else if (_stricmp(Temp, "serial") == 0)
		ts->PortType = IdSerial;
	else {
		ts->PortType = IdTCPIP;
	}

	/* VT win position */
	GetPrivateProfileString(Section, "VTPos", "-2147483648,-2147483648", Temp, sizeof(Temp), FName);	/* default: random position */
	GetNthNum(Temp, 1, (int far *) (&ts->VTPos.x));
	GetNthNum(Temp, 2, (int far *) (&ts->VTPos.y));

	/* TEK win position */
	GetPrivateProfileString(Section, "TEKPos", "-2147483648,-2147483648", Temp, sizeof(Temp), FName);	/* default: random position */
	GetNthNum(Temp, 1, (int far *) &(ts->TEKPos.x));
	GetNthNum(Temp, 2, (int far *) &(ts->TEKPos.y));

	/* VT terminal size  */
	GetPrivateProfileString(Section, "TerminalSize", "80,24",
	                        Temp, sizeof(Temp), FName);
	GetNthNum(Temp, 1, &ts->TerminalWidth);
	GetNthNum(Temp, 2, &ts->TerminalHeight);
	if (ts->TerminalWidth < 0)
		ts->TerminalWidth = 1;
	if (ts->TerminalHeight < 0)
		ts->TerminalHeight = 1;

	/* Terminal size = Window size */
	ts->TermIsWin = GetOnOff(Section, "TermIsWin", FName, FALSE);

	/* Auto window resize flag */
	ts->AutoWinResize = GetOnOff(Section, "AutoWinResize", FName, FALSE);

	/* CR Receive */
	GetPrivateProfileString(Section, "CRReceive", "",
	                        Temp, sizeof(Temp), FName);
	if (_stricmp(Temp, "CRLF") == 0) {
		ts->CRReceive = IdCRLF;
	}
	else if (_stricmp(Temp, "LF") == 0) {
		ts->CRReceive = IdLF;
	}
	else {
		ts->CRReceive = IdCR;
	}
	/* CR Send */
	GetPrivateProfileString(Section, "CRSend", "",
	                        Temp, sizeof(Temp), FName);
	if (_stricmp(Temp, "CRLF") == 0)
		ts->CRSend = IdCRLF;
	else
		ts->CRSend = IdCR;

	/* Local echo */
	ts->LocalEcho = GetOnOff(Section, "LocalEcho", FName, FALSE);

	/* Answerback */
	GetPrivateProfileString(Section, "Answerback", "", Temp,
	                        sizeof(Temp), FName);
	ts->AnswerbackLen =
		Hex2Str(Temp, ts->Answerback, sizeof(ts->Answerback));

	/* Kanji Code (receive) */
	GetPrivateProfileString(Section, "KanjiReceive", "",
	                        Temp, sizeof(Temp), FName);
	if (_stricmp(Temp, "EUC") == 0)
		ts->KanjiCode = IdEUC;
	else if (_stricmp(Temp, "JIS") == 0)
		ts->KanjiCode = IdJIS;
	else if (_stricmp(Temp, "UTF-8") == 0)
		ts->KanjiCode = IdUTF8;
	else if (_stricmp(Temp, "UTF-8m") == 0)
		ts->KanjiCode = IdUTF8m;
	else
		ts->KanjiCode = IdSJIS;

	/* Katakana (receive) */
	GetPrivateProfileString(Section, "KatakanaReceive", "",
	                        Temp, sizeof(Temp), FName);
	if (_stricmp(Temp, "7") == 0)
		ts->JIS7Katakana = 1;
	else
		ts->JIS7Katakana = 0;

	/* Kanji Code (transmit) */
	GetPrivateProfileString(Section, "KanjiSend", "",
	                        Temp, sizeof(Temp), FName);
	if (_stricmp(Temp, "EUC") == 0)
		ts->KanjiCodeSend = IdEUC;
	else if (_stricmp(Temp, "JIS") == 0)
		ts->KanjiCodeSend = IdJIS;
	else if (_stricmp(Temp, "UTF-8") == 0)
		ts->KanjiCodeSend = IdUTF8;
	else
		ts->KanjiCodeSend = IdSJIS;

	/* Katakana (receive) */
	GetPrivateProfileString(Section, "KatakanaSend", "",
	                        Temp, sizeof(Temp), FName);
	if (_stricmp(Temp, "7") == 0)
		ts->JIS7KatakanaSend = 1;
	else
		ts->JIS7KatakanaSend = 0;

	/* KanjiIn */
	GetPrivateProfileString(Section, "KanjiIn", "",
	                        Temp, sizeof(Temp), FName);
	if (_stricmp(Temp, "@") == 0)
		ts->KanjiIn = IdKanjiInA;
	else
		ts->KanjiIn = IdKanjiInB;

	/* KanjiOut */
	GetPrivateProfileString(Section, "KanjiOut", "",
	                        Temp, sizeof(Temp), FName);
	if (_stricmp(Temp, "B") == 0)
		ts->KanjiOut = IdKanjiOutB;
	else if (_stricmp(Temp, "H") == 0)
		ts->KanjiOut = IdKanjiOutH;
	else
		ts->KanjiOut = IdKanjiOutJ;

	/* Auto Win Switch VT<->TEK */
	ts->AutoWinSwitch = GetOnOff(Section, "AutoWinSwitch", FName, FALSE);

	/* Terminal ID */
	GetPrivateProfileString(Section, "TerminalID", "",
	                        Temp, sizeof(Temp), FName);
	ts->TerminalID = str2id(TermList, Temp, IdVT100);

	/* Russian character set (host) */
	GetPrivateProfileString(Section, "RussHost", "",
	                        Temp, sizeof(Temp), FName);
	ts->RussHost = str2id(RussList, Temp, IdKOI8);

	/* Russian character set (client) */
	GetPrivateProfileString(Section, "RussClient", "",
	                        Temp, sizeof(Temp), FName);
	ts->RussClient = str2id(RussList, Temp, IdWindows);

	/* Title String */
	GetPrivateProfileString(Section, "Title", "Tera Term",
	                        ts->Title, sizeof(ts->Title), FName);

	/* Cursor shape */
	GetPrivateProfileString(Section, "CursorShape", "",
	                        Temp, sizeof(Temp), FName);
	if (_stricmp(Temp, "vertical") == 0)
		ts->CursorShape = IdVCur;
	else if (_stricmp(Temp, "horizontal") == 0)
		ts->CursorShape = IdHCur;
	else
		ts->CursorShape = IdBlkCur;

	/* Hide title */
	ts->HideTitle = GetOnOff(Section, "HideTitle", FName, FALSE);

	/* Popup menu */
	ts->PopupMenu = GetOnOff(Section, "PopupMenu", FName, FALSE);

	/* Full color */
	ts->ColorFlag |=
		CF_FULLCOLOR * GetOnOff(Section, "FullColor", FName, FALSE);

	/* Enable scroll buffer */
	ts->EnableScrollBuff =
		GetOnOff(Section, "EnableScrollBuff", FName, TRUE);

	/* Scroll buffer size */
	ts->ScrollBuffSize =
		GetPrivateProfileInt(Section, "ScrollBuffSize", 100, FName);

	/* VT Color */
	GetPrivateProfileString(Section, "VTColor", "0,0,0,255,255,255",
	                        Temp, sizeof(Temp), FName);
	for (i = 0; i <= 5; i++)
		GetNthNum(Temp, i + 1, (int far *) &(ts->TmpColor[0][i]));
	for (i = 0; i <= 1; i++)
		ts->VTColor[i] = RGB((BYTE) ts->TmpColor[0][i * 3],
		                     (BYTE) ts->TmpColor[0][i * 3 + 1],
		                     (BYTE) ts->TmpColor[0][i * 3 + 2]);

	/* VT Bold Color */
	GetPrivateProfileString(Section, "VTBoldColor", "0,0,255,255,255,255",
	                        Temp, sizeof(Temp), FName);
	for (i = 0; i <= 5; i++)
		GetNthNum(Temp, i + 1, (int far *) &(ts->TmpColor[0][i]));
	for (i = 0; i <= 1; i++)
		ts->VTBoldColor[i] = RGB((BYTE) ts->TmpColor[0][i * 3],
		                         (BYTE) ts->TmpColor[0][i * 3 + 1],
		                         (BYTE) ts->TmpColor[0][i * 3 + 2]);

	/* VT Blink Color */
	GetPrivateProfileString(Section, "VTBlinkColor", "255,0,0,255,255,255",
	                        Temp, sizeof(Temp), FName);
	for (i = 0; i <= 5; i++)
		GetNthNum(Temp, i + 1, (int far *) &(ts->TmpColor[0][i]));
	for (i = 0; i <= 1; i++)
		ts->VTBlinkColor[i] = RGB((BYTE) ts->TmpColor[0][i * 3],
		                          (BYTE) ts->TmpColor[0][i * 3 + 1],
		                          (BYTE) ts->TmpColor[0][i * 3 + 2]);

	/* begin - ishizaki */
	ts->EnableClickableUrl =
		GetOnOff(Section, "EnableClickableUrl", FName, FALSE);

	/* URL Color */
	GetPrivateProfileString(Section, "URLColor", "0,255,0,255,255,255",
	                        Temp, sizeof(Temp), FName);
	for (i = 0; i <= 5; i++)
		GetNthNum(Temp, i + 1, (int far *) &(ts->TmpColor[0][i]));
	for (i = 0; i <= 1; i++)
		ts->URLColor[i] = RGB((BYTE) ts->TmpColor[0][i * 3],
		                      (BYTE) ts->TmpColor[0][i * 3 + 1],
		                      (BYTE) ts->TmpColor[0][i * 3 + 2]);
	/* end - ishizaki */

	/* TEK Color */
	GetPrivateProfileString(Section, "TEKColor", "0,0,0,255,255,255",
	                        Temp, sizeof(Temp), FName);
	for (i = 0; i <= 5; i++)
		GetNthNum(Temp, i + 1, (int far *) &(ts->TmpColor[0][i]));
	for (i = 0; i <= 1; i++)
		ts->TEKColor[i] = RGB((BYTE) ts->TmpColor[0][i * 3],
		                      (BYTE) ts->TmpColor[0][i * 3 + 1],
		                      (BYTE) ts->TmpColor[0][i * 3 + 2]);
#ifndef NO_ANSI_COLOR_EXTENSION
	/* ANSI color definition (in the case FullColor=on)  -- special option
	   o UseTextColor should be off, or the background and foreground color of
	   VTColor are assigned to color-number 0 and 7 respectively, even if
	   they are specified in ANSIColor.
	   o ANSIColor is a set of 4 values that are color-number(0--15),
	   red-value(0--255), green-value(0--255) and blue-value(0--255). */
	GetPrivateProfileString(Section, "ANSIColor",
	                        " 0,  0,  0,  0,"
	                        " 1,255,  0,  0,"
	                        " 2,  0,255,  0,"
	                        " 3,255,255,  0,"
	                        " 4,  0,  0,255,"
	                        " 5,255,  0,255,"
	                        " 6,  0,255,255,"
	                        " 7,255,255,255,"
	                        " 8,128,128,128,"
	                        " 9,128,  0,  0,"
	                        "10,  0,128,  0,"
	                        "11,128,128,  0,"
	                        "12,  0,  0,128,"
	                        "13,128,  0,128,"
	                        "14,  0,128,128,"
	                        "15,192,192,192,", Temp, sizeof(Temp), FName);
	{
		char *t;
		int n = 1;
		for (t = Temp; *t; t++)
			if (*t == ',')
				n++;
		n /= 4;
		for (i = 0; i <= n; i++) {
			int colorid, r, g, b;
			GetNthNum(Temp, i * 4 + 1, (int far *) &colorid);
			GetNthNum(Temp, i * 4 + 2, (int far *) &r);
			GetNthNum(Temp, i * 4 + 3, (int far *) &g);
			GetNthNum(Temp, i * 4 + 4, (int far *) &b);
			ts->ANSIColor[colorid & 15] =
				RGB((BYTE) r, (BYTE) g, (BYTE) b);
		}
	}
#endif							/* NO_ANSI_COLOR_EXTENSION */

#ifndef NO_ANSI_COLOR_EXTENSION
	/* ANSI color definition (in the case FullColor=on)  -- special option
	   o UseTextColor should be off, or the background and foreground color of
	   VTColor are assigned to color-number 0 and 7 respectively, even if
	   they are specified in ANSIColor.
	   o ANSIColor is a set of 4 values that are color-number(0--15),
	   red-value(0--255), green-value(0--255) and blue-value(0--255). */
	GetPrivateProfileString(Section, "ANSIColor",
	                        " 0,  0,  0,  0,"
	                        " 1,255,  0,  0,"
	                        " 2,  0,255,  0,"
	                        " 3,255,255,  0,"
	                        " 4,  0,  0,255,"
	                        " 5,255,  0,255,"
	                        " 6,  0,255,255,"
	                        " 7,255,255,255,"
	                        " 8,128,128,128,"
	                        " 9,128,  0,  0,"
	                        "10,  0,128,  0,"
	                        "11,128,128,  0,"
	                        "12,  0,  0,128,"
	                        "13,128,  0,128,"
	                        "14,  0,128,128,"
	                        "15,192,192,192", Temp, sizeof(Temp), FName);
	{
		char *t;
		int n = 1;
		for (t = Temp; *t; t++)
			if (*t == ',')
				n++;
		n /= 4;
		for (i = 0; i <= n; i++) {
			int colorid, r, g, b;
			GetNthNum(Temp, i * 4 + 1, (int far *) &colorid);
			GetNthNum(Temp, i * 4 + 2, (int far *) &r);
			GetNthNum(Temp, i * 4 + 3, (int far *) &g);
			GetNthNum(Temp, i * 4 + 4, (int far *) &b);
			ts->ANSIColor[colorid & 15] =
				RGB((BYTE) r, (BYTE) g, (BYTE) b);
		}
	}
#endif							/* NO_ANSI_COLOR_EXTENSION */

	TmpDC = GetDC(0);			/* Get screen device context */
	for (i = 0; i <= 1; i++)
		ts->VTColor[i] = GetNearestColor(TmpDC, ts->VTColor[i]);
	for (i = 0; i <= 1; i++)
		ts->VTBoldColor[i] = GetNearestColor(TmpDC, ts->VTBoldColor[i]);
	for (i = 0; i <= 1; i++)
		ts->VTBlinkColor[i] = GetNearestColor(TmpDC, ts->VTBlinkColor[i]);
	for (i = 0; i <= 1; i++)
		ts->TEKColor[i] = GetNearestColor(TmpDC, ts->TEKColor[i]);
	/* begin - ishizaki */
	for (i = 0; i <= 1; i++)
		ts->URLColor[i] = GetNearestColor(TmpDC, ts->URLColor[i]);
	/* end - ishizaki */
#ifndef NO_ANSI_COLOR_EXTENSION
	for (i = 0; i < 16; i++)
		ts->ANSIColor[i] = GetNearestColor(TmpDC, ts->ANSIColor[i]);
#endif							/* NO_ANSI_COLOR_EXTENSION */
	ReleaseDC(0, TmpDC);

	/* TEK color emulation */
	ts->TEKColorEmu = GetOnOff(Section, "TEKColorEmulation", FName, FALSE);

	/* VT Font */
	GetPrivateProfileString(Section, "VTFont", "Terminal,0,-13,1",
	                        Temp, sizeof(Temp), FName);
	GetNthString(Temp, 1, sizeof(ts->VTFont), ts->VTFont);
	GetNthNum(Temp, 2, (int far *) &(ts->VTFontSize.x));
	GetNthNum(Temp, 3, (int far *) &(ts->VTFontSize.y));
	GetNthNum(Temp, 4, &(ts->VTFontCharSet));

	/* Bold font flag */
	ts->EnableBold = GetOnOff(Section, "EnableBold", FName, FALSE);

	/* Russian character set (font) */
	GetPrivateProfileString(Section, "RussFont", "",
	                        Temp, sizeof(Temp), FName);
	ts->RussFont = str2id(RussList, Temp, IdWindows);

	/* TEK Font */
	GetPrivateProfileString(Section, "TEKFont", "Courier,0,-13,0",
	                        Temp, sizeof(Temp), FName);
	GetNthString(Temp, 1, sizeof(ts->TEKFont), ts->TEKFont);
	GetNthNum(Temp, 2, (int far *) &(ts->TEKFontSize.x));
	GetNthNum(Temp, 3, (int far *) &(ts->TEKFontSize.y));
	GetNthNum(Temp, 4, &(ts->TEKFontCharSet));

	/* BS key */
	GetPrivateProfileString(Section, "BSKey", "",
	                        Temp, sizeof(Temp), FName);
	if (_stricmp(Temp, "DEL") == 0)
		ts->BSKey = IdDEL;
	else
		ts->BSKey = IdBS;
	/* Delete key */
	ts->DelKey = GetOnOff(Section, "DeleteKey", FName, FALSE);

	/* Meta Key */
	ts->MetaKey = GetOnOff(Section, "MetaKey", FName, FALSE);

	/* Russian keyboard type */
	GetPrivateProfileString(Section, "RussKeyb", "",
	                        Temp, sizeof(Temp), FName);
	ts->RussKeyb = str2id(RussList2, Temp, IdWindows);

	/* Serial port ID */
	ts->ComPort = GetPrivateProfileInt(Section, "ComPort", 1, FName);

	/* Baud rate */
	GetPrivateProfileString(Section, "BaudRate", "9600",
	                        Temp, sizeof(Temp), FName);
	ts->Baud = str2id(BaudList, Temp, IdBaud9600);

	/* Parity */
	GetPrivateProfileString(Section, "Parity", "",
	                        Temp, sizeof(Temp), FName);
	if (_stricmp(Temp, "even") == 0)
		ts->Parity = IdParityEven;
	else if (_stricmp(Temp, "odd") == 0)
		ts->Parity = IdParityOdd;
	else
		ts->Parity = IdParityNone;

	/* Data bit */
	GetPrivateProfileString(Section, "DataBit", "",
	                        Temp, sizeof(Temp), FName);
	if (_stricmp(Temp, "7") == 0)
		ts->DataBit = IdDataBit7;
	else
		ts->DataBit = IdDataBit8;

	/* Stop bit */
	GetPrivateProfileString(Section, "StopBit", "",
	                        Temp, sizeof(Temp), FName);
	if (_stricmp(Temp, "2") == 0)
		ts->StopBit = IdStopBit2;
	else
		ts->StopBit = IdStopBit1;

	/* Flow control */
	GetPrivateProfileString(Section, "FlowCtrl", "",
	                        Temp, sizeof(Temp), FName);
	if (_stricmp(Temp, "x") == 0)
		ts->Flow = IdFlowX;
	else if (_stricmp(Temp, "hard") == 0)
		ts->Flow = IdFlowHard;
	else
		ts->Flow = IdFlowNone;

	/* Delay per character */
	ts->DelayPerChar =
		GetPrivateProfileInt(Section, "DelayPerChar", 0, FName);

	/* Delay per line */
	ts->DelayPerLine =
		GetPrivateProfileInt(Section, "DelayPerLine", 0, FName);

	/* Telnet flag */
	ts->Telnet = GetOnOff(Section, "Telnet", FName, TRUE);

	/* Telnet terminal type */
	GetPrivateProfileString(Section, "TermType", "vt100", ts->TermType,
							sizeof(ts->TermType), FName);

	/* TCP port num */
	ts->TCPPort =
		GetPrivateProfileInt(Section, "TCPPort", ts->TelPort, FName);

	/* Auto window close flag */
	ts->AutoWinClose = GetOnOff(Section, "AutoWinClose", FName, TRUE);

	/* History list */
	ts->HistoryList = GetOnOff(Section, "HistoryList", FName, FALSE);

	/* File transfer binary flag */
	ts->TransBin = GetOnOff(Section, "TransBin", FName, FALSE);

	/* Log append */
	ts->Append = GetOnOff(Section, "LogAppend", FName, FALSE);

	/* Log plain text (2005.5.7 yutaka) */
	ts->LogTypePlainText =
		GetOnOff(Section, "LogTypePlainText", FName, FALSE);

	/* Log with timestamp (2006.7.23 maya) */
	ts->LogTimestamp = GetOnOff(Section, "LogTimestamp", FName, FALSE);

	/* Default Log file name (2006.8.28 maya) */
	GetPrivateProfileString(Section, "LogDefaultName", "teraterm.log",
	                        ts->LogDefaultName, sizeof(ts->LogDefaultName),
	                        FName);

	/* Default Log file path (2007.5.30 maya) */
	GetPrivateProfileString(Section, "LogDefaultPath", "",
	                        ts->LogDefaultPath, sizeof(ts->LogDefaultPath),
	                        FName);

	/* Auto start logging (2007.5.31 maya) */
	ts->LogAutoStart = GetOnOff(Section, "LogAutoStart", FName, FALSE);

	/* XMODEM option */
	GetPrivateProfileString(Section, "XmodemOpt", "",
	                        Temp, sizeof(Temp), FName);
	if (_stricmp(Temp, "crc") == 0)
		ts->XmodemOpt = XoptCRC;
	else if (_stricmp(Temp, "1k") == 0)
		ts->XmodemOpt = Xopt1K;
	else
		ts->XmodemOpt = XoptCheck;

	/* XMODEM binary file */
	ts->XmodemBin = GetOnOff(Section, "XmodemBin", FName, TRUE);

	/* Default directory for file transfer */
	GetPrivateProfileString(Section, "FileDir", "",
	                        ts->FileDir, sizeof(ts->FileDir), FName);
	if (strlen(ts->FileDir) == 0)
		strncpy_s(ts->FileDir, sizeof(ts->FileDir), ts->HomeDir, _TRUNCATE);
	else {
		_getcwd(Temp, sizeof(Temp));
		if (_chdir(ts->FileDir) != 0)
			strncpy_s(ts->FileDir, sizeof(ts->FileDir), ts->HomeDir, _TRUNCATE);
		_chdir(Temp);
	}

	/* filter on file send (2007.6.5 maya) */
	GetPrivateProfileString(Section, "FileSendFilter", "",
	                        ts->FileSendFilter, sizeof(ts->FileSendFilter),
	                        FName);

/*--------------------------------------------------*/
	/* 8 bit control code flag  -- special option */
	ts->TermFlag |=
		TF_ACCEPT8BITCTRL *
		GetOnOff(Section, "Accept8bitCtrl", FName, TRUE);

	/* Wrong sequence flag  -- special option */
	ts->TermFlag |=
		TF_ALLOWWRONGSEQUENCE *
		GetOnOff(Section, "AllowWrongSequence", FName, FALSE);

	if (((ts->TermFlag & TF_ALLOWWRONGSEQUENCE) == 0) &&
	    (ts->KanjiOut == IdKanjiOutH))
		ts->KanjiOut = IdKanjiOutJ;

	// Auto file renaming --- special option
	ts->FTFlag |=
		FT_RENAME * GetOnOff(Section, "AutoFileRename", FName, FALSE);

	// Auto invoking (character set->G0->GL) --- special option
	ts->TermFlag |=
		TF_AUTOINVOKE * GetOnOff(Section, "AutoInvoke", FName, FALSE);

	// Auto text copy --- special option
	ts->AutoTextCopy = GetOnOff(Section, "AutoTextCopy", FName, TRUE);

	/* Back wrap -- special option */
	ts->TermFlag |=
		TF_BACKWRAP * GetOnOff(Section, "BackWrap", FName, FALSE);

	/* Beep type -- special option */
	ts->Beep = GetOnOff(Section, "Beep", FName, TRUE);

	/* Beep on connection & disconnection -- special option */
	ts->PortFlag |=
		PF_BEEPONCONNECT * GetOnOff(Section, "BeepOnConnect", FName,
									FALSE);

	/* Auto B-Plus activation -- special option */
	ts->FTFlag |= FT_BPAUTO * GetOnOff(Section, "BPAuto", FName, FALSE);
	if ((ts->FTFlag & FT_BPAUTO) != 0) {	/* Answerback */
		strncpy_s(ts->Answerback, sizeof(ts->Answerback), "\020++\0200",
		          _TRUNCATE);
		ts->AnswerbackLen = 5;
	}

	/* B-Plus ESCCTL flag  -- special option */
	ts->FTFlag |=
		FT_BPESCCTL * GetOnOff(Section, "BPEscCtl", FName, FALSE);

	/* B-Plus log  -- special option */
	ts->LogFlag |= LOG_BP * GetOnOff(Section, "BPLog", FName, FALSE);

	/* Confirm disconnection -- special option */
	ts->PortFlag |=
		PF_CONFIRMDISCONN * GetOnOff(Section, "ConfirmDisconnect", FName,
		                             TRUE);

	/* Ctrl code in Kanji -- special option */
	ts->TermFlag |=
		TF_CTRLINKANJI * GetOnOff(Section, "CtrlInKanji", FName, TRUE);

	/* Debug flag  -- special option */
	ts->Debug = GetOnOff(Section, "Debug", FName, FALSE);

	/* Delimiter list -- special option */
	GetPrivateProfileString(Section, "DelimList",
	                        "$20!\"#$24%&\'()*+,-./:;<=>?@[\\]^`{|}~",
	                        Temp, sizeof(Temp), FName);
	Hex2Str(Temp, ts->DelimList, sizeof(ts->DelimList));

	/* regard DBCS characters as delimiters -- special option */
	ts->DelimDBCS = GetOnOff(Section, "DelimDBCS", FName, TRUE);

	// Enable popup menu -- special option
	if (GetOnOff(Section, "EnablePopupMenu", FName, TRUE) == 0)
		ts->MenuFlag |= MF_NOPOPUP;

	// Enable "Show menu" -- special option
	if (GetOnOff(Section, "EnableShowMenu", FName, TRUE) == 0)
		ts->MenuFlag |= MF_NOSHOWMENU;

	// Enable the status line -- special option
	ts->TermFlag |=
		TF_ENABLESLINE * GetOnOff(Section, "EnableStatusLine", FName, TRUE);

	// fixed JIS --- special
	ts->TermFlag |=
		TF_FIXEDJIS * GetOnOff(Section, "FixedJIS", FName, FALSE);

	/* IME Flag  -- special option */
	ts->UseIME = GetOnOff(Section, "IME", FName, TRUE);

	/* IME-inline Flag  -- special option */
	ts->IMEInline = GetOnOff(Section, "IMEInline", FName, TRUE);

	/* Kermit log  -- special option */
	ts->LogFlag |= LOG_KMT * GetOnOff(Section, "KmtLog", FName, FALSE);

	// Enable language selection -- special option
	if (GetOnOff(Section, "LanguageSelection", FName, TRUE) == 0)
		ts->MenuFlag |= MF_NOLANGUAGE;

	/* Maximum scroll buffer size  -- special option */
	ts->ScrollBuffMax =
		GetPrivateProfileInt(Section, "MaxBuffSize", 10000, FName);
	if (ts->ScrollBuffMax < 24)
		ts->ScrollBuffMax = 10000;

	/* Max com port number -- special option */
	ts->MaxComPort = GetPrivateProfileInt(Section, "MaxComPort", 4, FName);
	if (ts->MaxComPort < 4)
		ts->MaxComPort = 4;
	// COM16����99�֊g�� (2005.11.30 yutaka)
	// 99����200�֊g�� (2007.7.23 maya)
	if (ts->MaxComPort > MAXCOMPORT)
		ts->MaxComPort = MAXCOMPORT;
	if ((ts->ComPort < 1) || (ts->ComPort > ts->MaxComPort))
		ts->ComPort = 1;

	/* Non-blinking cursor -- special option */
	ts->NonblinkingCursor =
		GetOnOff(Section, "NonblinkingCursor", FName, FALSE);

	/* Delay for pass-thru printing activation */
	/*   -- special option */
	ts->PassThruDelay =
		GetPrivateProfileInt(Section, "PassThruDelay", 3, FName);

	/* Printer port for pass-thru printing */
	/*   -- special option */
	GetPrivateProfileString(Section, "PassThruPort", "",
							ts->PrnDev, sizeof(ts->PrnDev), FName);

	/* Printer Font --- special option */
	GetPrivateProfileString(Section, "PrnFont", "",
	                        Temp, sizeof(Temp), FName);
	if (strlen(Temp) == 0) {
		ts->PrnFont[0] = 0;
		ts->PrnFontSize.x = 0;
		ts->PrnFontSize.y = 0;
		ts->PrnFontCharSet = 0;
	}
	else {
		GetNthString(Temp, 1, sizeof(ts->PrnFont), ts->PrnFont);
		GetNthNum(Temp, 2, (int far *) &(ts->PrnFontSize.x));
		GetNthNum(Temp, 3, (int far *) &(ts->PrnFontSize.y));
		GetNthNum(Temp, 4, &(ts->PrnFontCharSet));
	}

	// Page margins (left, right, top, bottom) for printing
	//    -- special option
	GetPrivateProfileString(Section, "PrnMargin", "50,50,50,50",
	                        Temp, sizeof(Temp), FName);
	for (i = 0; i <= 3; i++)
		GetNthNum(Temp, 1 + i, &ts->PrnMargin[i]);

	/* Quick-VAN log  -- special option */
	ts->LogFlag |= LOG_QV * GetOnOff(Section, "QVLog", FName, FALSE);

	/* Quick-VAN window size -- special */
	ts->QVWinSize = GetPrivateProfileInt(Section, "QVWinSize", 8, FName);

	/* Russian character set (print) -- special option */
	GetPrivateProfileString(Section, "RussPrint", "",
	                        Temp, sizeof(Temp), FName);
	ts->RussPrint = str2id(RussList, Temp, IdWindows);

	/* Scroll threshold -- special option */
	ts->ScrollThreshold =
		GetPrivateProfileInt(Section, "ScrollThreshold", 12, FName);

	// Select on activate -- special option
	ts->SelOnActive = GetOnOff(Section, "SelectOnActivate", FName, TRUE);

	/* Startup macro -- special option */
	GetPrivateProfileString(Section, "StartupMacro", "",
	                        ts->MacroFN, sizeof(ts->MacroFN), FName);

	/* TEK GIN Mouse keycode -- special option */
	ts->GINMouseCode =
		GetPrivateProfileInt(Section, "TEKGINMouseCode", 32, FName);

	/* Telnet binary flag -- special option */
	ts->TelBin = GetOnOff(Section, "TelBin", FName, FALSE);

	/* Telnet Echo flag -- special option */
	ts->TelEcho = GetOnOff(Section, "TelEcho", FName, FALSE);

	/* Telnet log  -- special option */
	ts->LogFlag |= LOG_TEL * GetOnOff(Section, "TelLog", FName, FALSE);

	/* TCP port num for telnet -- special option */
	ts->TelPort = GetPrivateProfileInt(Section, "TelPort", 23, FName);

	/* Telnet keep-alive packet(NOP command) interval -- special option */
	ts->TelKeepAliveInterval =
		GetPrivateProfileInt(Section, "TelKeepAliveInterval", 300, FName);

	/* Max number of broadcast commad history */
	ts->MaxBroadcatHistory =
		GetPrivateProfileInt(Section, "MaxBroadcatHistory", 99, FName);

	/* Local echo for non-telnet */
	ts->TCPLocalEcho = GetOnOff(Section, "TCPLocalEcho", FName, FALSE);

	/* "new-line (transmit)" option for non-telnet -- special option */
	GetPrivateProfileString(Section, "TCPCRSend", "",
	                        Temp, sizeof(Temp), FName);
	if (_stricmp(Temp, "CR") == 0)
		ts->TCPCRSend = IdCR;
	else if (_stricmp(Temp, "CRLF") == 0)
		ts->TCPCRSend = IdCRLF;
	else
		ts->TCPCRSend = 0;		// disabled

	/* Use text (background) color for "white (black)"
	   --- special option */
	ts->ColorFlag |=
		CF_USETEXTCOLOR * GetOnOff(Section, "UseTextColor", FName, FALSE);

	/* Title format -- special option */
	ts->TitleFormat =
		GetPrivateProfileInt(Section, "TitleFormat", 5, FName);

	/* VT Compatible Tab -- special option */
	ts->VTCompatTab = GetOnOff(Section, "VTCompatTab", FName, FALSE);

	/* VT Font space --- special option */
	GetPrivateProfileString(Section, "VTFontSpace", "0,0,0,0",
	                        Temp, sizeof(Temp), FName);
	GetNthNum(Temp, 1, &ts->FontDX);
	GetNthNum(Temp, 2, &ts->FontDW);
	GetNthNum(Temp, 3, &ts->FontDY);
	GetNthNum(Temp, 4, &ts->FontDH);
	if (ts->FontDX < 0)
		ts->FontDX = 0;
	if (ts->FontDW < 0)
		ts->FontDW = 0;
	ts->FontDW = ts->FontDW + ts->FontDX;
	if (ts->FontDY < 0)
		ts->FontDY = 0;
	if (ts->FontDH < 0)
		ts->FontDH = 0;
	ts->FontDH = ts->FontDH + ts->FontDY;

	// VT-print scaling factors (pixels per inch) --- special option
	GetPrivateProfileString(Section, "VTPPI", "0,0",
	                        Temp, sizeof(Temp), FName);
	GetNthNum(Temp, 1, (int far *) &ts->VTPPI.x);
	GetNthNum(Temp, 2, (int far *) &ts->VTPPI.y);

	// TEK-print scaling factors (pixels per inch) --- special option
	GetPrivateProfileString(Section, "TEKPPI", "0,0",
	                        Temp, sizeof(Temp), FName);
	GetNthNum(Temp, 1, (int far *) &ts->TEKPPI.x);
	GetNthNum(Temp, 2, (int far *) &ts->TEKPPI.y);

	// Show "Window" menu -- special option
	ts->MenuFlag |=
		MF_SHOWWINMENU * GetOnOff(Section, "WindowMenu", FName, TRUE);

	/* XMODEM log  -- special option */
	ts->LogFlag |= LOG_X * GetOnOff(Section, "XmodemLog", FName, FALSE);

	/* Auto ZMODEM activation -- special option */
	ts->FTFlag |= FT_ZAUTO * GetOnOff(Section, "ZmodemAuto", FName, FALSE);

	/* ZMODEM data subpacket length for sending -- special */
	ts->ZmodemDataLen =
		GetPrivateProfileInt(Section, "ZmodemDataLen", 1024, FName);
	/* ZMODEM window size for sending -- special */
	ts->ZmodemWinSize =
		GetPrivateProfileInt(Section, "ZmodemWinSize", 32767, FName);

	/* ZMODEM ESCCTL flag  -- special option */
	ts->FTFlag |=
		FT_ZESCCTL * GetOnOff(Section, "ZmodemEscCtl", FName, FALSE);

	/* ZMODEM log  -- special option */
	ts->LogFlag |= LOG_Z * GetOnOff(Section, "ZmodemLog", FName, FALSE);
#ifndef NO_COPYLINE_FIX

	/* Enable continued-line copy  -- special option */
	ts->EnableContinuedLineCopy =
		GetOnOff(Section, "EnableContinuedLineCopy", FName, FALSE);
#endif							/* NO_COPYLINE_FIX */

	ts->DisablePasteMouseRButton =
		GetOnOff(Section, "DisablePasteMouseRButton", FName, FALSE);

	// added ConfirmPasteMouseRButton (2007.3.17 maya)
	ts->ConfirmPasteMouseRButton =
		GetOnOff(Section, "ConfirmPasteMouseRButton", FName, FALSE);

	// added DisableAcceleratorSendBreak (2007.3.17 maya)
	ts->DisableAcceleratorSendBreak =
		GetOnOff(Section, "DisableAcceleratorSendBreak", FName, FALSE);

	// WinSock connecting timeout value (2007.1.11 yutaka)
	ts->ConnectingTimeout =
		GetPrivateProfileInt(Section, "ConnectingTimeout", 0, FName);

	// mouse cursor 
	GetPrivateProfileString(Section, "MouseCursor", "IBEAM",
	                        Temp, sizeof(Temp), FName);
	strncpy_s(ts->MouseCursorName, sizeof(ts->MouseCursorName), Temp,
	          _TRUNCATE);

	// Translucent window
	ts->AlphaBlend =
		GetPrivateProfileInt(Section, "AlphaBlend ", 255, FName);
	ts->AlphaBlend = max(0, ts->AlphaBlend);
	ts->AlphaBlend = min(255, ts->AlphaBlend);

	// Cygwin install path
	GetPrivateProfileString(Section, "CygwinDirectory ", "c:\\cygwin",
	                        Temp, sizeof(Temp), FName);
	strncpy_s(ts->CygwinDirectory, sizeof(ts->CygwinDirectory), Temp,
			  _TRUNCATE);

	// Viewlog Editor path
	GetPrivateProfileString(Section, "ViewlogEditor ", "notepad.exe",
	                        Temp, sizeof(Temp), FName);
	strncpy_s(ts->ViewlogEditor, sizeof(ts->ViewlogEditor), Temp,
			  _TRUNCATE);

	// Locale for UTF-8
	GetPrivateProfileString(Section, "Locale ", DEFAULT_LOCALE,
	                        Temp, sizeof(Temp), FName);
	strncpy_s(ts->Locale, sizeof(ts->Locale), Temp, _TRUNCATE);

	// CodePage
	ts->CodePage =
		GetPrivateProfileInt(Section, "CodePage ", DEFAULT_CODEPAGE,
		                     FName);

	// UI language message file
	GetPrivateProfileString(Section, "UILanguageFile", "lang\\Default.lng",
	                        Temp, sizeof(Temp), FName);
	{
		char CurDir[MAX_PATH];
		GetCurrentDirectory(sizeof(CurDir), CurDir);
		SetCurrentDirectory(ts->HomeDir);
		_fullpath(ts->UILanguageFile, Temp, sizeof(ts->UILanguageFile));
		SetCurrentDirectory(CurDir);
	}

	// Broadcast Command History (2007.3.3 maya)
	ts->BroadcastCommandHistory =
		GetOnOff(Section, "BroadcastCommandHistory", FName, FALSE);

	// 337: 2007/03/20 Accept Broadcast
	ts->AcceptBroadcast =
		GetOnOff(Section, "AcceptBroadcast", FName, TRUE);

#ifdef USE_NORMAL_BGCOLOR
	// UseNormalBGColor
	ts->UseNormalBGColor =
		GetOnOff(Section, "UseNormalBGColor", FName, FALSE);
	// 2006/03/11 by 337
	if (ts->UseNormalBGColor) {
		ts->VTBoldColor[1] =
			ts->VTBlinkColor[1] = ts->URLColor[1] = ts->VTColor[1];
	}
#endif
}

void FAR PASCAL WriteIniFile(PCHAR FName, PTTSet ts)
{
	int i;
	char Temp[MAXPATHLEN];
	char buf[20];

	/* version */
	WritePrivateProfileString(Section, "Version", "2.3", FName);

	/* Language */
	if (ts->Language == IdJapanese)
		strncpy_s(Temp, sizeof(Temp), "Japanese", _TRUNCATE);
	else if (ts->Language == IdRussian)
		strncpy_s(Temp, sizeof(Temp), "Russian", _TRUNCATE);
	else
		strncpy_s(Temp, sizeof(Temp), "English", _TRUNCATE);
	WritePrivateProfileString(Section, "Language", Temp, FName);

	/* Port type */
	if (ts->PortType == IdSerial)
		strncpy_s(Temp, sizeof(Temp), "serial", _TRUNCATE);
	else						/* IdFile -> IdTCPIP */
		strncpy_s(Temp, sizeof(Temp), "tcpip", _TRUNCATE);

	WritePrivateProfileString(Section, "Port", Temp, FName);

	/* VT terminal size  */
	WriteInt2(Section, "TerminalSize", FName,
			  ts->TerminalWidth, ts->TerminalHeight);

	/* Terminal size = Window size */
	WriteOnOff(Section, "TermIsWin", FName, ts->TermIsWin);

	/* Auto window resize flag */
	WriteOnOff(Section, "AutoWinResize", FName, ts->AutoWinResize);

	/* CR Receive */
	if (ts->CRReceive == IdCRLF) {
		strncpy_s(Temp, sizeof(Temp), "CRLF", _TRUNCATE);
	}
	else if (ts->CRReceive == IdLF) {
		strncpy_s(Temp, sizeof(Temp), "LF", _TRUNCATE);
	}
	else {
		strncpy_s(Temp, sizeof(Temp), "CR", _TRUNCATE);
	}
	WritePrivateProfileString(Section, "CRReceive", Temp, FName);

	/* CR Send */
	if (ts->CRSend == IdCRLF)
		strncpy_s(Temp, sizeof(Temp), "CRLF", _TRUNCATE);
	else
		strncpy_s(Temp, sizeof(Temp), "CR", _TRUNCATE);
	WritePrivateProfileString(Section, "CRSend", Temp, FName);

	/* Local echo */
	WriteOnOff(Section, "LocalEcho", FName, ts->LocalEcho);

	/* Answerback */
	if ((ts->FTFlag & FT_BPAUTO) == 0) {
		Str2Hex(ts->Answerback, Temp, ts->AnswerbackLen,
		        sizeof(Temp) - 1, TRUE);
		WritePrivateProfileString(Section, "Answerback", Temp, FName);
	}

	/* Kanji Code (receive)  */
	switch (ts->KanjiCode) {
	case IdEUC:
		strncpy_s(Temp, sizeof(Temp), "EUC", _TRUNCATE);
		break;
	case IdJIS:
		strncpy_s(Temp, sizeof(Temp), "JIS", _TRUNCATE);
		break;
	case IdUTF8:
		strncpy_s(Temp, sizeof(Temp), "UTF-8", _TRUNCATE);
		break;
	case IdUTF8m:
		strncpy_s(Temp, sizeof(Temp), "UTF-8m", _TRUNCATE);
		break;
	default:
		strncpy_s(Temp, sizeof(Temp), "SJIS", _TRUNCATE);
	}
	WritePrivateProfileString(Section, "KanjiReceive", Temp, FName);

	/* Katakana (receive)  */
	if (ts->JIS7Katakana == 1)
		strncpy_s(Temp, sizeof(Temp), "7", _TRUNCATE);
	else
		strncpy_s(Temp, sizeof(Temp), "8", _TRUNCATE);

	WritePrivateProfileString(Section, "KatakanaReceive", Temp, FName);

	/* Kanji Code (transmit)  */
	switch (ts->KanjiCodeSend) {
	case IdEUC:
		strncpy_s(Temp, sizeof(Temp), "EUC", _TRUNCATE);
		break;
	case IdJIS:
		strncpy_s(Temp, sizeof(Temp), "JIS", _TRUNCATE);
		break;
	case IdUTF8:
		strncpy_s(Temp, sizeof(Temp), "UTF-8", _TRUNCATE);
		break;
	default:
		strncpy_s(Temp, sizeof(Temp), "SJIS", _TRUNCATE);
	}
	WritePrivateProfileString(Section, "KanjiSend", Temp, FName);

	/* Katakana (transmit)  */
	if (ts->JIS7KatakanaSend == 1)
		strncpy_s(Temp, sizeof(Temp), "7", _TRUNCATE);
	else
		strncpy_s(Temp, sizeof(Temp), "8", _TRUNCATE);

	WritePrivateProfileString(Section, "KatakanaSend", Temp, FName);

	/* KanjiIn */
	if (ts->KanjiIn == IdKanjiInA)
		strncpy_s(Temp, sizeof(Temp), "@", _TRUNCATE);
	else
		strncpy_s(Temp, sizeof(Temp), "B", _TRUNCATE);

	WritePrivateProfileString(Section, "KanjiIn", Temp, FName);

	/* KanjiOut */
	switch (ts->KanjiOut) {
	case IdKanjiOutB:
		strncpy_s(Temp, sizeof(Temp), "B", _TRUNCATE);
		break;
	case IdKanjiOutH:
		strncpy_s(Temp, sizeof(Temp), "H", _TRUNCATE);
		break;
	default:
		strncpy_s(Temp, sizeof(Temp), "J", _TRUNCATE);
	}
	WritePrivateProfileString(Section, "KanjiOut", Temp, FName);

	// new configuration
	WriteInt(Section, "ConnectingTimeout", FName, ts->ConnectingTimeout);
	WriteOnOff(Section, "DisablePasteMouseRButton", FName,
	           ts->DisablePasteMouseRButton);
	// added ConfirmPasteMouseRButton (2007.3.17 maya)
	WriteOnOff(Section, "ConfirmPasteMouseRButton", FName,
	           ts->ConfirmPasteMouseRButton);
	// added DisableAcceleratorSendBreak (2007.3.17 maya)
	WriteOnOff(Section, "DisableAcceleratorSendBreak", FName,
	           ts->DisableAcceleratorSendBreak);
	WriteOnOff(Section, "EnableContinuedLineCopy", FName,
	           ts->EnableContinuedLineCopy);
	WritePrivateProfileString(Section, "MouseCursor", ts->MouseCursorName,
	                          FName);
	_snprintf_s(Temp, sizeof(Temp), _TRUNCATE, "%d", ts->AlphaBlend);
	WritePrivateProfileString(Section, "AlphaBlend", Temp, FName);
	WritePrivateProfileString(Section, "CygwinDirectory",
	                          ts->CygwinDirectory, FName);
	WritePrivateProfileString(Section, "ViewlogEditor", ts->ViewlogEditor,
	                          FName);
	WritePrivateProfileString(Section, "Locale", ts->Locale, FName);
	_snprintf_s(Temp, sizeof(Temp), _TRUNCATE, "%d", ts->CodePage);
	WritePrivateProfileString(Section, "CodePage", Temp, FName);

	// ANSI color(2004.9.5 yutaka)
	Temp[0] = '\0';
	for (i = 0; i < 15; i++) {
		_snprintf_s(buf, sizeof(buf), _TRUNCATE, "%d,%d,%d,%d, ",
		            i,
		            GetRValue(ts->ANSIColor[i]),
		            GetGValue(ts->ANSIColor[i]),
		            GetBValue(ts->ANSIColor[i])
		);
		strncat_s(Temp, sizeof(Temp), buf, _TRUNCATE);
	}
	i = 15;
	_snprintf_s(buf, sizeof(buf), _TRUNCATE, "%d,%d,%d,%d",
	            i,
	            GetRValue(ts->ANSIColor[i]),
	            GetGValue(ts->ANSIColor[i]),
	            GetBValue(ts->ANSIColor[i])
	);
	strncat_s(Temp, sizeof(Temp), buf, _TRUNCATE);
	WritePrivateProfileString(Section, "ANSIColor", Temp, FName);

	/* AutoWinChange VT<->TEK */
	WriteOnOff(Section, "AutoWinSwitch", FName, ts->AutoWinSwitch);

	/* Terminal ID */
	id2str(TermList, ts->TerminalID, IdVT100, Temp, sizeof(Temp));
	WritePrivateProfileString(Section, "TerminalID", Temp, FName);

	/* Russian character set (host)  */
	id2str(RussList, ts->RussHost, IdKOI8, Temp, sizeof(Temp));
	WritePrivateProfileString(Section, "RussHost", Temp, FName);

	/* Russian character set (client)  */
	id2str(RussList, ts->RussClient, IdWindows, Temp, sizeof(Temp));
	WritePrivateProfileString(Section, "RussClient", Temp, FName);

	/* Title text */
	WritePrivateProfileString(Section, "Title", ts->Title, FName);

	/* Cursor shape */
	switch (ts->CursorShape) {
	case IdVCur:
		strncpy_s(Temp, sizeof(Temp), "vertical", _TRUNCATE);
		break;
	case IdHCur:
		strncpy_s(Temp, sizeof(Temp), "horizontal", _TRUNCATE);
		break;
	default:
		strncpy_s(Temp, sizeof(Temp), "block", _TRUNCATE);
	}
	WritePrivateProfileString(Section, "CursorShape", Temp, FName);

	/* Hide title */
	WriteOnOff(Section, "HideTitle", FName, ts->HideTitle);

	/* Popup menu */
	WriteOnOff(Section, "PopupMenu", FName, ts->PopupMenu);

	/* ANSI full color */
	WriteOnOff(Section, "FullColor", FName,
	           (WORD) (ts->ColorFlag & CF_FULLCOLOR));

	/* Enable scroll buffer */
	WriteOnOff(Section, "EnableScrollBuff", FName, ts->EnableScrollBuff);

	/* Scroll buffer size */
	WriteInt(Section, "ScrollBuffSize", FName, ts->ScrollBuffSize);

	/* VT Color */
	for (i = 0; i <= 1; i++) {
		ts->TmpColor[0][i * 3] = GetRValue(ts->VTColor[i]);
		ts->TmpColor[0][i * 3 + 1] = GetGValue(ts->VTColor[i]);
		ts->TmpColor[0][i * 3 + 2] = GetBValue(ts->VTColor[i]);
	}
	WriteInt6(Section, "VTColor", FName,
			  ts->TmpColor[0][0], ts->TmpColor[0][1], ts->TmpColor[0][2],
			  ts->TmpColor[0][3], ts->TmpColor[0][4], ts->TmpColor[0][5]);

	/* VT bold color */
	for (i = 0; i <= 1; i++) {
		ts->TmpColor[0][i * 3] = GetRValue(ts->VTBoldColor[i]);
		ts->TmpColor[0][i * 3 + 1] = GetGValue(ts->VTBoldColor[i]);
		ts->TmpColor[0][i * 3 + 2] = GetBValue(ts->VTBoldColor[i]);
	}
	WriteInt6(Section, "VTBoldColor", FName,
			  ts->TmpColor[0][0], ts->TmpColor[0][1], ts->TmpColor[0][2],
			  ts->TmpColor[0][3], ts->TmpColor[0][4], ts->TmpColor[0][5]);

	/* VT blink color */
	for (i = 0; i <= 1; i++) {
		ts->TmpColor[0][i * 3] = GetRValue(ts->VTBlinkColor[i]);
		ts->TmpColor[0][i * 3 + 1] = GetGValue(ts->VTBlinkColor[i]);
		ts->TmpColor[0][i * 3 + 2] = GetBValue(ts->VTBlinkColor[i]);
	}
	WriteInt6(Section, "VTBlinkColor", FName,
			  ts->TmpColor[0][0], ts->TmpColor[0][1], ts->TmpColor[0][2],
			  ts->TmpColor[0][3], ts->TmpColor[0][4], ts->TmpColor[0][5]);

	/* start - ishizaki */
	WriteOnOff(Section, "EnableClickableUrl", FName,
			   ts->EnableClickableUrl);

	/* URL color */
	for (i = 0; i <= 1; i++) {
		ts->TmpColor[0][i * 3] = GetRValue(ts->URLColor[i]);
		ts->TmpColor[0][i * 3 + 1] = GetGValue(ts->URLColor[i]);
		ts->TmpColor[0][i * 3 + 2] = GetBValue(ts->URLColor[i]);
	}
	WriteInt6(Section, "URLColor", FName,
			  ts->TmpColor[0][0], ts->TmpColor[0][1], ts->TmpColor[0][2],
			  ts->TmpColor[0][3], ts->TmpColor[0][4], ts->TmpColor[0][5]);
	/* end - ishizaki */

	/* TEK Color */
	for (i = 0; i <= 1; i++) {
		ts->TmpColor[0][i * 3] = GetRValue(ts->TEKColor[i]);
		ts->TmpColor[0][i * 3 + 1] = GetGValue(ts->TEKColor[i]);
		ts->TmpColor[0][i * 3 + 2] = GetBValue(ts->TEKColor[i]);
	}
	WriteInt6(Section, "TEKColor", FName,
	          ts->TmpColor[0][0], ts->TmpColor[0][1], ts->TmpColor[0][2],
              ts->TmpColor[0][3], ts->TmpColor[0][4], ts->TmpColor[0][5]);

	/* TEK color emulation */
	WriteOnOff(Section, "TEKColorEmulation", FName, ts->TEKColorEmu);

	/* VT Font */
	WriteFont(Section, "VTFont", FName,
	          ts->VTFont, ts->VTFontSize.x, ts->VTFontSize.y,
	          ts->VTFontCharSet);

	/* Enable bold font flag */
	WriteOnOff(Section, "EnableBold", FName, ts->EnableBold);

	/* Russian character set (font) */
	id2str(RussList, ts->RussFont, IdWindows, Temp, sizeof(Temp));
	WritePrivateProfileString(Section, "RussFont", Temp, FName);

	/* TEK Font */
	WriteFont(Section, "TEKFont", FName,
	          ts->TEKFont, ts->TEKFontSize.x, ts->TEKFontSize.y,
	          ts->TEKFontCharSet);

	/* BS key */
	if (ts->BSKey == IdDEL)
		strncpy_s(Temp, sizeof(Temp), "DEL", _TRUNCATE);
	else
		strncpy_s(Temp, sizeof(Temp), "BS", _TRUNCATE);
	WritePrivateProfileString(Section, "BSKey", Temp, FName);

	/* Delete key */
	WriteOnOff(Section, "DeleteKey", FName, ts->DelKey);

	/* Meta key */
	WriteOnOff(Section, "MetaKey", FName, ts->MetaKey);

	/* Russian keyboard type */
	id2str(RussList2, ts->RussKeyb, IdWindows, Temp, sizeof(Temp));
	WritePrivateProfileString(Section, "RussKeyb", Temp, FName);

	/* Serial port ID */
	uint2str(ts->ComPort, Temp, sizeof(Temp), 3);
	WritePrivateProfileString(Section, "ComPort", Temp, FName);

	/* Baud rate */
	id2str(BaudList, ts->Baud, IdBaud9600, Temp, sizeof(Temp));
	WritePrivateProfileString(Section, "BaudRate", Temp, FName);

	/* Parity */
	switch (ts->Parity) {
	case IdParityEven:
		strncpy_s(Temp, sizeof(Temp), "even", _TRUNCATE);
		break;
	case IdParityOdd:
		strncpy_s(Temp, sizeof(Temp), "odd", _TRUNCATE);
		break;
	default:
		strncpy_s(Temp, sizeof(Temp), "none", _TRUNCATE);
	}
	WritePrivateProfileString(Section, "Parity", Temp, FName);

	/* Data bit */
	if (ts->DataBit == IdDataBit7)
		strncpy_s(Temp, sizeof(Temp), "7", _TRUNCATE);
	else
		strncpy_s(Temp, sizeof(Temp), "8", _TRUNCATE);

	WritePrivateProfileString(Section, "DataBit", Temp, FName);

	/* Stop bit */
	if (ts->StopBit == IdStopBit2)
		strncpy_s(Temp, sizeof(Temp), "2", _TRUNCATE);
	else
		strncpy_s(Temp, sizeof(Temp), "1", _TRUNCATE);

	WritePrivateProfileString(Section, "StopBit", Temp, FName);

	/* Flow control */
	switch (ts->Flow) {
	case IdFlowX:
		strncpy_s(Temp, sizeof(Temp), "x", _TRUNCATE);
		break;
	case IdFlowHard:
		strncpy_s(Temp, sizeof(Temp), "hard", _TRUNCATE);
		break;
	default:
		strncpy_s(Temp, sizeof(Temp), "none", _TRUNCATE);
	}
	WritePrivateProfileString(Section, "FlowCtrl", Temp, FName);

	/* Delay per character */
	WriteInt(Section, "DelayPerChar", FName, ts->DelayPerChar);

	/* Delay per line */
	WriteInt(Section, "DelayPerLine", FName, ts->DelayPerLine);

	/* Telnet flag */
	WriteOnOff(Section, "Telnet", FName, ts->Telnet);

	/* Telnet terminal type */
	WritePrivateProfileString(Section, "TermType", ts->TermType, FName);

	/* TCP port num for non-telnet */
	WriteUint(Section, "TCPPort", FName, ts->TCPPort);

	/* Auto close flag */
	WriteOnOff(Section, "AutoWinClose", FName, ts->AutoWinClose);

	/* History list */
	WriteOnOff(Section, "Historylist", FName, ts->HistoryList);

	/* File transfer binary flag */
	WriteOnOff(Section, "TransBin", FName, ts->TransBin);

	/* Log append */
	WriteOnOff(Section, "LogAppend", FName, ts->Append);

	/* File transfer binary flag */
	WriteOnOff(Section, "LogTypePlainText", FName, ts->LogTypePlainText);

	/* Log with timestamp (2006.7.23 maya) */
	WriteOnOff(Section, "LogTimestamp", FName, ts->LogTimestamp);

	/* Default Log file name (2006.8.28 maya) */
	WritePrivateProfileString(Section, "LogDefaultName",
							  ts->LogDefaultName, FName);

	/* Default Log file path (2007.5.30 maya) */
	WritePrivateProfileString(Section, "LogDefaultPath",
							  ts->LogDefaultPath, FName);

	/* Auto start logging (2007.5.31 maya) */
	WriteOnOff(Section, "LogAutoStart", FName, ts->LogAutoStart);

	/* XMODEM option */
	switch (ts->XmodemOpt) {
	case XoptCRC:
		strncpy_s(Temp, sizeof(Temp), "crc", _TRUNCATE);
		break;
	case Xopt1K:
		strncpy_s(Temp, sizeof(Temp), "1k", _TRUNCATE);
		break;
	default:
		strncpy_s(Temp, sizeof(Temp), "checksum", _TRUNCATE);
	}
	WritePrivateProfileString(Section, "XmodemOpt", Temp, FName);

	/* XMODEM binary flag */
	WriteOnOff(Section, "XmodemBin", FName, ts->XmodemBin);

	/* Default directory for file transfer */
	WritePrivateProfileString(Section, "FileDir", ts->FileDir, FName);

	/* filter on file send (2007.6.5 maya) */
	WritePrivateProfileString(Section, "FileSendFilter",
							  ts->FileSendFilter, FName);

/*------------------------------------------------------------------*/
	/* 8 bit control code flag  -- special option */
	WriteOnOff(Section, "Accept8bitCtrl", FName,
			   (WORD) (ts->TermFlag & TF_ACCEPT8BITCTRL));

	/* Wrong sequence flag  -- special option */
	WriteOnOff(Section, "AllowWrongSequence", FName,
			   (WORD) (ts->TermFlag & TF_ALLOWWRONGSEQUENCE));

	/* Auto file renaming --- special option */
	WriteOnOff(Section, "AutoFileRename", FName,
			   (WORD) (ts->FTFlag & FT_RENAME));

	/* Auto text copy --- special option */
	WriteOnOff(Section, "AutoTextCopy", FName, ts->AutoTextCopy);

	/* Back wrap -- special option */
	WriteOnOff(Section, "BackWrap", FName,
	           (WORD) (ts->TermFlag & TF_BACKWRAP));

	/* Beep type -- special option */
	WriteOnOff(Section, "Beep", FName, ts->Beep);

	/* Beep on connection & disconnection -- special option */
	WriteOnOff(Section, "BeepOnConnect", FName,
	           (WORD) (ts->PortFlag & PF_BEEPONCONNECT));

	/* Auto B-Plus activation -- special option */
	WriteOnOff(Section, "BPAuto", FName, (WORD) (ts->FTFlag & FT_BPAUTO));

	/* B-Plus ESCCTL flag  -- special option */
	WriteOnOff(Section, "BPEscCtl", FName,
	           (WORD) (ts->FTFlag & FT_BPESCCTL));

	/* B-Plus log  -- special option */
	WriteOnOff(Section, "BPLog", FName, (WORD) (ts->LogFlag & LOG_BP));

	/* Confirm disconnection -- special option */
	WriteOnOff(Section, "ConfirmDisconnect", FName,
	           (WORD) (ts->PortFlag & PF_CONFIRMDISCONN));

	/* Ctrl code in Kanji -- special option */
	WriteOnOff(Section, "CtrlInKanji", FName,
	           (WORD) (ts->TermFlag & TF_CTRLINKANJI));

	/* Debug flag  -- special option */
	WriteOnOff(Section, "Debug", FName, ts->Debug);

	/* Delimiter list -- special option */
	Str2Hex(ts->DelimList, Temp, strlen(ts->DelimList),
	        sizeof(Temp) - 1, TRUE);
	WritePrivateProfileString(Section, "DelimList", Temp, FName);

	/* regard DBCS characters as delimiters -- special option */
	WriteOnOff(Section, "DelimDBCS", FName, ts->DelimDBCS);

	// Enable popup menu -- special option
	if ((ts->MenuFlag & MF_NOPOPUP) == 0)
		WriteOnOff(Section, "EnablePopupMenu", FName, 1);
	else
		WriteOnOff(Section, "EnablePopupMenu", FName, 0);

	// Enable "Show menu" -- special option
	if ((ts->MenuFlag & MF_NOSHOWMENU) == 0)
		WriteOnOff(Section, "EnableShowMenu", FName, 1);
	else
		WriteOnOff(Section, "EnableShowMenu", FName, 0);

	/* Enable the status line -- special option */
	WriteOnOff(Section, "EnableStatusLine", FName,
	           (WORD) (ts->TermFlag & TF_ENABLESLINE));

	/* IME Flag  -- special option */
	WriteOnOff(Section, "IME", FName, ts->UseIME);

	/* IME-inline Flag  -- special option */
	WriteOnOff(Section, "IMEInline", FName, ts->IMEInline);

	/* Kermit log  -- special option */
	WriteOnOff(Section, "KmtLog", FName, (WORD) (ts->LogFlag & LOG_KMT));

	// Enable language selection -- special option
	if ((ts->MenuFlag & MF_NOLANGUAGE) == 0)
		WriteOnOff(Section, "LanguageSelection", FName, 1);
	else
		WriteOnOff(Section, "LanguageSelection", FName, 0);

	/* Maximum scroll buffer size  -- special option */
	WriteInt(Section, "MaxBuffSize", FName, ts->ScrollBuffMax);

	/* Max com port number -- special option */
	WriteInt(Section, "MaxComPort", FName, ts->MaxComPort);

	/* Non-blinking cursor -- special option */
	WriteOnOff(Section, "NonblinkingCursor", FName, ts->NonblinkingCursor);

	/* Delay for pass-thru printing activation */
	/*   -- special option */
	WriteUint(Section, "PassThruDelay", FName, ts->PassThruDelay);

	/* Printer port for pass-thru printing */
	/*   -- special option */
	WritePrivateProfileString(Section, "PassThruPort", ts->PrnDev, FName);

	/* Printer Font --- special option */
	WriteFont(Section, "PrnFont", FName,
	          ts->PrnFont, ts->PrnFontSize.x, ts->PrnFontSize.y,
	          ts->PrnFontCharSet);

	// Page margins (left, right, top, bottom) for printing
	//    -- special option
	WriteInt4(Section, "PrnMargin", FName,
	          ts->PrnMargin[0], ts->PrnMargin[1],
	          ts->PrnMargin[2], ts->PrnMargin[3]);

	/* Quick-VAN log  -- special option */
	WriteOnOff(Section, "QVLog", FName, (WORD) (ts->LogFlag & LOG_QV));

	/* Quick-VAN window size -- special */
	WriteInt(Section, "QVWinSize", FName, ts->QVWinSize);

	/* Russian character set (print) -- special option */
	id2str(RussList, ts->RussPrint, IdWindows, Temp, sizeof(Temp));
	WritePrivateProfileString(Section, "RussPrint", Temp, FName);

	/* Scroll threshold -- special option */
	WriteInt(Section, "ScrollThreshold", FName, ts->ScrollThreshold);

	// Select on activate -- special option
	WriteOnOff(Section, "SelectOnActivate", FName, ts->SelOnActive);

	/* Startup macro -- special option */
	WritePrivateProfileString(Section, "StartupMacro", ts->MacroFN, FName);

	/* TEK GIN Mouse keycode -- special option */
	WriteInt(Section, "TEKGINMouseCode", FName, ts->GINMouseCode);

	/* Telnet binary flag -- special option */
	WriteOnOff(Section, "TelBin", FName, ts->TelBin);

	/* Telnet Echo flag -- special option */
	WriteOnOff(Section, "TelEcho", FName, ts->TelEcho);

	/* Telnet log  -- special option */
	WriteOnOff(Section, "TelLog", FName, (WORD) (ts->LogFlag & LOG_TEL));

	/* TCP port num for telnet -- special option */
	WriteUint(Section, "TelPort", FName, ts->TelPort);

	/* Telnet keep-alive packet(NOP command) interval -- special option */
	WriteUint(Section, "TelKeepAliveInterval", FName,
	          ts->TelKeepAliveInterval);

	/* Max number of broadcast commad history */
	WriteUint(Section, "MaxBroadcatHistory", FName,
	          ts->MaxBroadcatHistory);

	/* Local echo for non-telnet */
	WriteOnOff(Section, "TCPLocalEcho", FName, ts->TCPLocalEcho);

	/* "new-line (transmit)" option for non-telnet -- special option */
	if (ts->TCPCRSend == IdCRLF)
		strncpy_s(Temp, sizeof(Temp), "CRLF", _TRUNCATE);
	else if (ts->TCPCRSend == IdCR)
		strncpy_s(Temp, sizeof(Temp), "CR", _TRUNCATE);
	else
		Temp[0] = 0;
	WritePrivateProfileString(Section, "TCPCRSend", Temp, FName);

	/* Use text (background) color for "white (black)"
	   --- special option */
	WriteOnOff(Section, "UseTextColor", FName,
	           (WORD) (ts->ColorFlag & CF_USETEXTCOLOR));

	/* Title format -- special option */
	WriteUint(Section, "TitleFormat", FName, ts->TitleFormat);

	/* VT Compatible Tab -- special option */
	WriteOnOff(Section, "VTCompatTab", FName, ts->VTCompatTab);

	/* VT Font space --- special option */
	WriteInt4(Section, "VTFontSpace", FName,
	          ts->FontDX, ts->FontDW - ts->FontDX,
	          ts->FontDY, ts->FontDH - ts->FontDY);

	// VT-print scaling factors (pixels per inch) --- special option
	WriteInt2(Section, "VTPPI", FName, ts->VTPPI.x, ts->VTPPI.y);

	// TEK-print scaling factors (pixels per inch) --- special option
	WriteInt2(Section, "TEKPPI", FName, ts->TEKPPI.x, ts->TEKPPI.y);

	// Show "Window" menu -- special option
	WriteOnOff(Section, "WindowMenu", FName,
	           (WORD) (ts->MenuFlag & MF_SHOWWINMENU));

	/* XMODEM log  -- special option */
	WriteOnOff(Section, "XmodemLog", FName, (WORD) (ts->LogFlag & LOG_X));

	/* Auto ZMODEM activation -- special option */
	WriteOnOff(Section, "ZmodemAuto", FName,
	           (WORD) (ts->FTFlag & FT_ZAUTO));

	/* ZMODEM data subpacket length for sending -- special */
	WriteInt(Section, "ZmodemDataLen", FName, ts->ZmodemDataLen);
	/* ZMODEM window size for sending -- special */
	WriteInt(Section, "ZmodemWinSize", FName, ts->ZmodemWinSize);

	/* ZMODEM ESCCTL flag  -- special option */
	WriteOnOff(Section, "ZmodemEscCtl", FName,
	           (WORD) (ts->FTFlag & FT_ZESCCTL));

	/* ZMODEM log  -- special option */
	WriteOnOff(Section, "ZmodemLog", FName, (WORD) (ts->LogFlag & LOG_Z));

	/* update file */
	WritePrivateProfileString(NULL, NULL, NULL, FName);

	// Eterm lookfeel alphablend (2005.4.24 yutaka)
#define ETERM_SECTION "BG"
	WriteOnOff(ETERM_SECTION, "BGEnable", FName,
	           ts->EtermLookfeel.BGEnable);
	WriteOnOff(ETERM_SECTION, "BGUseAlphaBlendAPI", FName,
	           ts->EtermLookfeel.BGUseAlphaBlendAPI);
	WritePrivateProfileString(ETERM_SECTION, "BGSPIPath",
	                          ts->EtermLookfeel.BGSPIPath, FName);
	WriteOnOff(ETERM_SECTION, "BGFastSizeMove", FName,
	           ts->EtermLookfeel.BGFastSizeMove);
	WriteOnOff(ETERM_SECTION, "BGFlickerlessMove", FName,
	           ts->EtermLookfeel.BGNoCopyBits);
	WriteOnOff(ETERM_SECTION, "BGNoFrame", FName,
	           ts->EtermLookfeel.BGNoFrame);
	WritePrivateProfileString(ETERM_SECTION, "BGThemeFile",
	                          ts->EtermLookfeel.BGThemeFile, FName);

#ifdef USE_NORMAL_BGCOLOR
	// UseNormalBGColor
	WriteOnOff(Section, "UseNormalBGColor", FName, ts->UseNormalBGColor);
#endif

	// Broadcast Command History (2007.3.3 maya)
	WriteOnOff(Section, "BroadcastCommandHistory", FName,
	           ts->BroadcastCommandHistory);

	// 337: 2007/03/20 Accept Broadcast
	WriteOnOff(Section, "AcceptBroadcast", FName, ts->AcceptBroadcast);
}

#define VTEditor "VT editor keypad"
#define VTNumeric "VT numeric keypad"
#define VTFunction "VT function keys"
#define XFunction "X function keys"
#define ShortCut "Shortcut keys"

void GetInt(PKeyMap KeyMap, int KeyId, PCHAR Sect, PCHAR Key, PCHAR FName)
{
	char Temp[11];
	WORD Num;

	GetPrivateProfileString(Sect, Key, "", Temp, sizeof(Temp), FName);
	if (Temp[0] == 0)
		Num = 0xFFFF;
	else if (_stricmp(Temp, "off") == 0)
		Num = 0xFFFF;
	else if (sscanf(Temp, "%d", &Num) != 1)
		Num = 0xFFFF;

	KeyMap->Map[KeyId - 1] = Num;
}

void FAR PASCAL ReadKeyboardCnf
	(PCHAR FName, PKeyMap KeyMap, BOOL ShowWarning) {
	int i, j, Ptr;
	char EntName[7];
	char TempStr[221];
	char KStr[201];

	// clear key map
	for (i = 0; i <= IdKeyMax - 1; i++)
		KeyMap->Map[i] = 0xFFFF;
	for (i = 0; i <= NumOfUserKey - 1; i++) {
		KeyMap->UserKeyPtr[i] = 0;
		KeyMap->UserKeyLen[i] = 0;
	}

	// VT editor keypad
	GetInt(KeyMap, IdUp, VTEditor, "Up", FName);

	GetInt(KeyMap, IdDown, VTEditor, "Down", FName);

	GetInt(KeyMap, IdRight, VTEditor, "Right", FName);

	GetInt(KeyMap, IdLeft, VTEditor, "Left", FName);

	GetInt(KeyMap, IdFind, VTEditor, "Find", FName);

	GetInt(KeyMap, IdInsert, VTEditor, "Insert", FName);

	GetInt(KeyMap, IdRemove, VTEditor, "Remove", FName);

	GetInt(KeyMap, IdSelect, VTEditor, "Select", FName);

	GetInt(KeyMap, IdPrev, VTEditor, "Prev", FName);

	GetInt(KeyMap, IdNext, VTEditor, "Next", FName);

	// VT numeric keypad
	GetInt(KeyMap, Id0, VTNumeric, "Num0", FName);

	GetInt(KeyMap, Id1, VTNumeric, "Num1", FName);

	GetInt(KeyMap, Id2, VTNumeric, "Num2", FName);

	GetInt(KeyMap, Id3, VTNumeric, "Num3", FName);

	GetInt(KeyMap, Id4, VTNumeric, "Num4", FName);

	GetInt(KeyMap, Id5, VTNumeric, "Num5", FName);

	GetInt(KeyMap, Id6, VTNumeric, "Num6", FName);

	GetInt(KeyMap, Id7, VTNumeric, "Num7", FName);

	GetInt(KeyMap, Id8, VTNumeric, "Num8", FName);

	GetInt(KeyMap, Id9, VTNumeric, "Num9", FName);

	GetInt(KeyMap, IdMinus, VTNumeric, "NumMinus", FName);

	GetInt(KeyMap, IdComma, VTNumeric, "NumComma", FName);

	GetInt(KeyMap, IdPeriod, VTNumeric, "NumPeriod", FName);

	GetInt(KeyMap, IdEnter, VTNumeric, "NumEnter", FName);

	GetInt(KeyMap, IdPF1, VTNumeric, "PF1", FName);

	GetInt(KeyMap, IdPF2, VTNumeric, "PF2", FName);

	GetInt(KeyMap, IdPF3, VTNumeric, "PF3", FName);

	GetInt(KeyMap, IdPF4, VTNumeric, "PF4", FName);

	// VT function keys
	GetInt(KeyMap, IdHold, VTFunction, "Hold", FName);

	GetInt(KeyMap, IdPrint, VTFunction, "Print", FName);

	GetInt(KeyMap, IdBreak, VTFunction, "Break", FName);

	GetInt(KeyMap, IdF6, VTFunction, "F6", FName);

	GetInt(KeyMap, IdF7, VTFunction, "F7", FName);

	GetInt(KeyMap, IdF8, VTFunction, "F8", FName);

	GetInt(KeyMap, IdF9, VTFunction, "F9", FName);

	GetInt(KeyMap, IdF10, VTFunction, "F10", FName);

	GetInt(KeyMap, IdF11, VTFunction, "F11", FName);

	GetInt(KeyMap, IdF12, VTFunction, "F12", FName);

	GetInt(KeyMap, IdF13, VTFunction, "F13", FName);

	GetInt(KeyMap, IdF14, VTFunction, "F14", FName);

	GetInt(KeyMap, IdHelp, VTFunction, "Help", FName);

	GetInt(KeyMap, IdDo, VTFunction, "Do", FName);

	GetInt(KeyMap, IdF17, VTFunction, "F17", FName);

	GetInt(KeyMap, IdF18, VTFunction, "F18", FName);

	GetInt(KeyMap, IdF19, VTFunction, "F19", FName);

	GetInt(KeyMap, IdF20, VTFunction, "F20", FName);

	// UDK
	GetInt(KeyMap, IdUDK6, VTFunction, "UDK6", FName);

	GetInt(KeyMap, IdUDK7, VTFunction, "UDK7", FName);

	GetInt(KeyMap, IdUDK8, VTFunction, "UDK8", FName);

	GetInt(KeyMap, IdUDK9, VTFunction, "UDK9", FName);

	GetInt(KeyMap, IdUDK10, VTFunction, "UDK10", FName);

	GetInt(KeyMap, IdUDK11, VTFunction, "UDK11", FName);

	GetInt(KeyMap, IdUDK12, VTFunction, "UDK12", FName);

	GetInt(KeyMap, IdUDK13, VTFunction, "UDK13", FName);

	GetInt(KeyMap, IdUDK14, VTFunction, "UDK14", FName);

	GetInt(KeyMap, IdUDK15, VTFunction, "UDK15", FName);

	GetInt(KeyMap, IdUDK16, VTFunction, "UDK16", FName);

	GetInt(KeyMap, IdUDK17, VTFunction, "UDK17", FName);

	GetInt(KeyMap, IdUDK18, VTFunction, "UDK18", FName);

	GetInt(KeyMap, IdUDK19, VTFunction, "UDK19", FName);

	GetInt(KeyMap, IdUDK20, VTFunction, "UDK20", FName);

	// XTERM function keys
	GetInt(KeyMap, IdXF1, XFunction, "XF1", FName);

	GetInt(KeyMap, IdXF2, XFunction, "XF2", FName);

	GetInt(KeyMap, IdXF3, XFunction, "XF3", FName);

	GetInt(KeyMap, IdXF4, XFunction, "XF4", FName);

	GetInt(KeyMap, IdXF5, XFunction, "XF5", FName);

	// accelerator keys
	GetInt(KeyMap, IdCmdEditCopy, ShortCut, "EditCopy", FName);

	GetInt(KeyMap, IdCmdEditPaste, ShortCut, "EditPaste", FName);

	GetInt(KeyMap, IdCmdEditPasteCR, ShortCut, "EditPasteCR", FName);

	GetInt(KeyMap, IdCmdEditCLS, ShortCut, "EditCLS", FName);

	GetInt(KeyMap, IdCmdEditCLB, ShortCut, "EditCLB", FName);

	GetInt(KeyMap, IdCmdCtrlOpenTEK, ShortCut, "ControlOpenTEK", FName);

	GetInt(KeyMap, IdCmdCtrlCloseTEK, ShortCut, "ControlCloseTEK", FName);

	GetInt(KeyMap, IdCmdLineUp, ShortCut, "LineUp", FName);

	GetInt(KeyMap, IdCmdLineDown, ShortCut, "LineDown", FName);

	GetInt(KeyMap, IdCmdPageUp, ShortCut, "PageUp", FName);

	GetInt(KeyMap, IdCmdPageDown, ShortCut, "PageDown", FName);

	GetInt(KeyMap, IdCmdBuffTop, ShortCut, "BuffTop", FName);

	GetInt(KeyMap, IdCmdBuffBottom, ShortCut, "BuffBottom", FName);

	GetInt(KeyMap, IdCmdNextWin, ShortCut, "NextWin", FName);

	GetInt(KeyMap, IdCmdPrevWin, ShortCut, "PrevWin", FName);

	GetInt(KeyMap, IdCmdLocalEcho, ShortCut, "LocalEcho", FName);

	/* user keys */

	Ptr = 0;

	strncpy_s(EntName, sizeof(EntName), "User", _TRUNCATE);
	i = IdUser1;
	do {
		uint2str(i - IdUser1 + 1, &EntName[4], sizeof(EntName) - 4, 2);
		GetPrivateProfileString("User keys", EntName, "",
								TempStr, sizeof(TempStr), FName);
		if (strlen(TempStr) > 0) {
			/* scan code */
			GetNthString(TempStr, 1, sizeof(KStr), KStr);
			if (_stricmp(KStr, "off") == 0)
				KeyMap->Map[i - 1] = 0xFFFF;
			else {
				GetNthNum(TempStr, 1, &j);
				KeyMap->Map[i - 1] = (WORD) j;
			}
			/* conversion flag */
			GetNthNum(TempStr, 2, &j);
			KeyMap->UserKeyType[i - IdUser1] = (BYTE) j;
			/* key string */
/*	GetNthString(TempStr,3,sizeof(KStr),KStr); */
			KeyMap->UserKeyPtr[i - IdUser1] = Ptr;
/*	KeyMap->UserKeyLen[i-IdUser1] =
	Hex2Str(KStr,&(KeyMap->UserKeyStr[Ptr]),KeyStrMax-Ptr+1);
*/
			GetNthString(TempStr, 3, KeyStrMax - Ptr + 1,
			             &(KeyMap->UserKeyStr[Ptr]));
			KeyMap->UserKeyLen[i - IdUser1] =
				strlen(&(KeyMap->UserKeyStr[Ptr]));
			Ptr = Ptr + KeyMap->UserKeyLen[i - IdUser1];
		}

		i++;
	}
	while ((i <= IdKeyMax) && (strlen(TempStr) > 0) && (Ptr <= KeyStrMax));

	for (j = 1; j <= IdKeyMax - 1; j++)
		if (KeyMap->Map[j] != 0xFFFF)
			for (i = 0; i <= j - 1; i++)
				if (KeyMap->Map[i] == KeyMap->Map[j]) {
					if (ShowWarning) {
						_snprintf_s(TempStr, sizeof(TempStr), _TRUNCATE,
						            "Keycode %d is used more than once",
						            KeyMap->Map[j]);
						MessageBox(0, TempStr,
						           "Tera Term: Error in keyboard setup file",
						           MB_ICONEXCLAMATION);
					}
					KeyMap->Map[i] = 0xFFFF;
				}
}

void FAR PASCAL CopySerialList(PCHAR IniSrc, PCHAR IniDest, PCHAR section,
                               PCHAR key)
{
	int i;
	char EntName[10];
	char TempHost[HostNameMaxLength + 1];

	if (_stricmp(IniSrc, IniDest) == 0)
		return;

	WritePrivateProfileString(section, NULL, NULL, IniDest);
	strncpy_s(EntName, sizeof(EntName) - 2, key, _TRUNCATE);

	i = 1;
	do {
		uint2str(i, &EntName[strlen(key)],
		         sizeof(EntName) - strlen(key), 2);

		/* Get one hostname from file IniSrc */
		GetPrivateProfileString(section, EntName, "",
		                        TempHost, sizeof(TempHost), IniSrc);
		/* Copy it to the file IniDest */
		if (strlen(TempHost) > 0)
			WritePrivateProfileString(section, EntName, TempHost, IniDest);
		i++;
	}
	while ((i <= 99) && (strlen(TempHost) > 0));

	/* update file */
	WritePrivateProfileString(NULL, NULL, NULL, IniDest);
}

void FAR PASCAL AddValueToList(PCHAR FName, PCHAR Host, PCHAR section,
                               PCHAR key)
{
	HANDLE MemH;
	PCHAR MemP;
	char EntName[10];
	int i, j, Len;
	BOOL Update;

	if ((FName[0] == 0) || (Host[0] == 0))
		return;
	MemH = GlobalAlloc(GHND, (HostNameMaxLength + 1) * 99);
	if (MemH == NULL)
		return;
	MemP = GlobalLock(MemH);
	if (MemP != NULL) {
		strncpy_s(MemP, (HostNameMaxLength + 1) * 99, Host, _TRUNCATE);
		j = strlen(Host) + 1;
		strncpy_s(EntName, sizeof(EntName), key, _TRUNCATE);
		i = 1;
		Update = TRUE;
		do {
			uint2str(i, &EntName[strlen(key)],
			         sizeof(EntName) - strlen(key), 2);

			/* Get a hostname */
			GetPrivateProfileString(section, EntName, "",
			                        &MemP[j], HostNameMaxLength + 1,
			                        FName);
			Len = strlen(&MemP[j]);
			if (_stricmp(&MemP[j], Host) == 0) {
				if (i == 1)
					Update = FALSE;
			}
			else
				j = j + Len + 1;
			i++;
		} while ((i <= 99) && (Len != 0) && Update);

		if (Update) {
			WritePrivateProfileString(section, NULL, NULL, FName);

			j = 0;
			i = 1;
			do {
				uint2str(i, &EntName[strlen(key)],
				         sizeof(EntName) - strlen(key), 2);

				if (MemP[j] != 0)
					WritePrivateProfileString(section, EntName, &MemP[j],
					                          FName);
				j = j + strlen(&MemP[j]) + 1;
				i++;
			} while ((i <= 99) && (MemP[j] != 0));
			/* update file */
			WritePrivateProfileString(NULL, NULL, NULL, FName);
		}
		GlobalUnlock(MemH);
	}
	GlobalFree(MemH);
}

 /* copy hostlist from source IniFile to dest IniFile */
void FAR PASCAL CopyHostList(PCHAR IniSrc, PCHAR IniDest)
{
	CopySerialList(IniSrc, IniDest, "Hosts", "Host");
}

void FAR PASCAL AddHostToList(PCHAR FName, PCHAR Host)
{
	AddValueToList(FName, Host, "Hosts", "Host");
}

BOOL NextParam(PCHAR Param, int *i, PCHAR Temp, int Size)
{
	int j;
	char c;
	BOOL Quoted;

	if ((unsigned int) (*i) >= strlen(Param))
		return FALSE;
	j = 0;

	while (Param[*i] == ' ')
		(*i)++;

	Quoted = FALSE;
	c = Param[*i];
	(*i)++;
	while ((c != 0) && (Quoted || (c != ' ')) &&
	       (Quoted || (c != ';')) && (j < Size - 1)) {
		if (c == '"')
			Quoted = !Quoted;
		Temp[j] = c;
		j++;
		c = Param[*i];
		(*i)++;
	}
	if (!Quoted && (c == ';'))
		(*i)--;

	Temp[j] = 0;
	return (strlen(Temp) > 0);
}

void Dequote(PCHAR Source, PCHAR Dest)
{
	int i, j;
	char q, c;

	Dest[0] = 0;
	if (Source[0] == 0)
		return;
	i = 0;
	/* quoting char */
	q = Source[i];
	/* only '"' is used as quoting char */
	if (q != '"')
		q = 0;
	else
		i++;

	c = Source[i];
	i++;
	j = 0;
	while ((c != 0) && (c != q)) {
		Dest[j] = c;
		j++;
		c = Source[i];
		i++;
	}

	Dest[j] = 0;
}


#ifndef NO_INET6
static void ParseHostName(char *HostStr, WORD * port)
{
	/*
	 * hostname.domain.com
	 * hostname.domain.com:23
	 * [3ffe:1234:1234::1]     IPv6 raw address
	 * [3ffe:1234:1234::1]:23  IPv6 raw address and port#
	 * telnet://hostname.domain.com/
	 * telnet://hostname.domain.com:23/
	 * telnet://[3ffe:1234:1234::1]/
	 * telnet://[3ffe:1234:1234::1]:23/
	 * tn3270:// .... /
	 */

	int i, is_handler = 0, is_port = 0;
	char *s;
	char b;

	/* strlen("telnet://") == 9 */
	if ((_strnicmp(HostStr, "telnet://", 9) == 0) ||
		(_strnicmp(HostStr, "tn3270://", 9) == 0)) {
		/* trim "telnet://" and tail "/" */
		memmove(HostStr, &(HostStr[9]), strlen(HostStr) - 8);
		i = strlen(HostStr);
		if (i > 0 && (HostStr[i - 1] == '/'))
			HostStr[i - 1] = '\0';
		is_handler = 1;
	}

	/* parsing string enclosed by [ ] */
	s = HostStr;
	if (*s == '[') {
		BOOL inet6found = FALSE;
		s++;
		while (*s) {
			if (*s == ']') {
				/* found IPv6 raw address */
				/* triming [ ] */
				int len = strlen(HostStr);
				char *lastptr = &HostStr[len - 1];
				memmove(HostStr, HostStr + 1, len - 1);
				s = s - 1;
				lastptr = lastptr - 1;
				memmove(s, s + 1, lastptr - s);
				/* because of triming 2 characters */
				HostStr[len - 2] = '\0';


				inet6found = TRUE;
				break;
			}
			s++;
		}
		if (inet6found == FALSE)
			s = HostStr;
	}

	/* parsing port# */
	/*
	 * s points:
	 *   [3ffe:1234:1234::1]:XXX....
	 *   3ffe:1234:1234::1:XXX....
	 *                    |
	 *                    s
	 *
	 *   hostname.domain.com
	 *   |
	 *   s
	 */
	i = 0;
	do {
		b = s[i];
		i++;
	} while (b != '\0' && b != ':');
	if (b == ':') {
		s[i - 1] = '\0';
		if (sscanf(&(s[i]), "%d", port) != 1)
			*port = 65535;
		is_port = 1;
	}
	if (is_handler == 1 && is_port == 0) {
		*port = 23;
	}
}
#endif							/* NO_INET6 */


void FAR PASCAL ParseParam(PCHAR Param, PTTSet ts, PCHAR DDETopic)
{
	int i, pos, c;
#ifdef NO_INET6
	BYTE b;
#endif							/* NO_INET6 */
	char Temp[MAXPATHLEN + 3];
	char Temp2[MAXPATHLEN];
	char TempDir[MAXPATHLEN];
	WORD ParamPort = 0;
	WORD ParamCom = 0;
	WORD ParamTCP = 65535;
	WORD ParamTel = 2;
	WORD ParamBin = 2;
	BOOL HostNameFlag = FALSE;
	BOOL JustAfterHost = FALSE;
	WORD DisableTCPEchoCR = FALSE;

	ts->HostName[0] = 0;
	ts->KeyCnfFN[0] = 0;
#ifndef NO_INET6
	/* user specifies the protocol connecting to the host */
	/* ts->ProtocolFamily = AF_UNSPEC; */
#endif							/* NO_INET6 */

	/* Get command line parameters */
	if (DDETopic != NULL)
		DDETopic[0] = 0;
	i = 0;
	/* the first term shuld be executable filename of Tera Term */
	NextParam(Param, &i, Temp, sizeof(Temp));
	while (NextParam(Param, &i, Temp, sizeof(Temp))) {
		if (HostNameFlag) {
			JustAfterHost = TRUE;
			HostNameFlag = FALSE;
		}

		if (_strnicmp(Temp, "/B", 2) == 0) {	/* telnet binary */
			ParamPort = IdTCPIP;
			ParamBin = 1;
		}
		else if (_strnicmp(Temp, "/C=", 3) == 0) {	/* COM port num */
			ParamPort = IdSerial;
			if (strlen(&Temp[3]) >= 1)
				ParamCom = (WORD) (Temp[3] - 0x30);
			if (strlen(&Temp[3]) >= 2)
				ParamCom = ParamCom * 10 + (WORD) (Temp[4] - 0x30);
			if ((ParamCom < 1) || (ParamCom > ts->MaxComPort))
				ParamCom = 0;
		}
		else if (_strnicmp(Temp, "/D=", 3) == 0) {
			if (DDETopic != NULL)
				strncpy_s(DDETopic, 21, &Temp[3], _TRUNCATE);	// 21 = sizeof(TopicName)
		}
		// TCPLocalEcho/TCPCRSend �𖳌��ɂ��� (maya 2007.4.25)
		else if (_strnicmp(Temp, "/E", 2) == 0) {
			DisableTCPEchoCR = TRUE;
		}
		else if (_strnicmp(Temp, "/F=", 3) == 0) {	/* setup filename */
			Dequote(&Temp[3], Temp2);
			if (strlen(Temp2) > 0) {
				ConvFName(ts->HomeDir, Temp2, sizeof(Temp2), ".INI", Temp,
				          sizeof(Temp));
				if (_stricmp(ts->SetupFName, Temp) != 0) {
					strncpy_s(ts->SetupFName, sizeof(ts->SetupFName), Temp,
					          _TRUNCATE);
					ReadIniFile(ts->SetupFName, ts);
				}
			}
		}
		else if (_strnicmp(Temp, "/FD=", 4) == 0) {	/* file transfer directory */
			Dequote(&Temp[4], Temp2);
			if (strlen(Temp2) > 0) {
				_getcwd(TempDir, sizeof(TempDir));
				if (_chdir(Temp2) == 0)
					strncpy_s(ts->FileDir, sizeof(ts->FileDir), Temp2,
					          _TRUNCATE);
				_chdir(TempDir);
			}
		}
		else if (_strnicmp(Temp, "/H", 2) == 0)	/* hide title bar */
			ts->HideTitle = 1;
		else if (_strnicmp(Temp, "/I", 2) == 0)	/* iconize */
			ts->Minimize = 1;
		else if (_strnicmp(Temp, "/K=", 3) == 0) {	/* Keyboard setup file */
			Dequote(&Temp[3], Temp2);
			ConvFName(ts->HomeDir, Temp2, sizeof(Temp2), ".CNF",
			          ts->KeyCnfFN, sizeof(ts->KeyCnfFN));
		}
		else if ((_strnicmp(Temp, "/KR=", 4) == 0) ||
		         (_strnicmp(Temp, "/KT=", 4) == 0)) {	/* kanji code */
			if (_strnicmp(&Temp[4], "UTF8m", 5) == 0)
				c = IdUTF8m;
			else if (_strnicmp(&Temp[4], "UTF8", 4) == 0)
				c = IdUTF8;
			else if (_strnicmp(&Temp[4], "SJIS", 4) == 0)
				c = IdSJIS;
			else if (_strnicmp(&Temp[4], "EUC", 3) == 0)
				c = IdEUC;
			else if (_strnicmp(&Temp[4], "JIS", 3) == 0)
				c = IdJIS;
			else
				c = -1;
			if (c != -1) {
				if (_strnicmp(Temp, "/KR=", 4) == 0)
					ts->KanjiCode = c;
				else
					ts->KanjiCodeSend = c;
			}
		}
		else if (_strnicmp(Temp, "/L=", 3) == 0) {	/* log file */
			Dequote(&Temp[3], Temp2);
			strncpy_s(ts->LogFN, sizeof(ts->LogFN), Temp2, _TRUNCATE);
		}
		else if (_strnicmp(Temp, "/LA=", 4) == 0) {	/* language */
			if (_strnicmp(&Temp[4], "E", 1) == 0)
				ts->Language = IdEnglish;
			else if (_strnicmp(&Temp[4], "J", 1) == 0)
				ts->Language = IdJapanese;
			else if (_strnicmp(&Temp[4], "R", 1) == 0)
				ts->Language = IdRussian;
		}
		else if (_strnicmp(Temp, "/M=", 3) == 0) {	/* macro filename */
			if ((Temp[3] == 0) || (Temp[3] == '*'))
				strncpy_s(ts->MacroFN, sizeof(ts->MacroFN), "*",
				          _TRUNCATE);
			else {
				Dequote(&Temp[3], Temp2);
				ConvFName(ts->HomeDir, Temp2, sizeof(Temp2), ".TTL",
				          ts->MacroFN, sizeof(ts->MacroFN));
			}
			/* Disable auto connect to serial when macro mode (2006.9.15 maya) */
			ts->ComAutoConnect = FALSE;
		}
		else if (_strnicmp(Temp, "/M", 2) == 0) {	/* macro option without file name */
			strncpy_s(ts->MacroFN, sizeof(ts->MacroFN), "*", _TRUNCATE);
			/* Disable auto connect to serial when macro mode (2006.9.15 maya) */
			ts->ComAutoConnect = FALSE;
		}
		else if (_strnicmp(Temp, "/P=", 3) == 0) {	/* TCP port num */
			ParamPort = IdTCPIP;
			if (sscanf(&Temp[3], "%d", &ParamTCP) != 1)
				ParamTCP = 65535;
		}
		else if (_strnicmp(Temp, "/R=", 3) == 0) {	/* Replay filename */
			Dequote(&Temp[3], Temp2);
			ConvFName(ts->HomeDir, Temp2, sizeof(Temp2), "", ts->HostName,
			          sizeof(ts->HostName));
			if (strlen(ts->HostName) > 0)
				ParamPort = IdFile;
		}
		else if (_strnicmp(Temp, "/T=0", 4) == 0) {	/* telnet disable */
			ParamPort = IdTCPIP;
			ParamTel = 0;
		}
		else if (_strnicmp(Temp, "/T=1", 4) == 0) {	/* telnet enable */
			ParamPort = IdTCPIP;
			ParamTel = 1;
		}
		else if (_strnicmp(Temp, "/V=", 2) == 0) {	/* invisible */
			ts->HideWindow = 1;
		}
		else if (_strnicmp(Temp, "/W=", 3) == 0) {	/* Window title */
			Dequote(&Temp[3], ts->Title);
		}
		else if (_strnicmp(Temp, "/X=", 3) == 0) {	/* Window pos (X) */
			if (sscanf(&Temp[3], "%d", &pos) == 1) {
				ts->VTPos.x = pos;
				if (ts->VTPos.y == CW_USEDEFAULT)
					ts->VTPos.y = 0;
			}
		}
		else if (_strnicmp(Temp, "/Y=", 3) == 0) {	/* Window pos (Y) */
			if (sscanf(&Temp[3], "%d", &pos) == 1) {
				ts->VTPos.y = pos;
				if (ts->VTPos.x == CW_USEDEFAULT)
					ts->VTPos.x = 0;
			}
		}
#ifndef NO_INET6
		else if (_strnicmp(Temp, "/4", 2) == 0)	/* Protocol TeraTerm speaking */
			ts->ProtocolFamily = AF_INET;
		else if (_strnicmp(Temp, "/6", 2) == 0)
			ts->ProtocolFamily = AF_INET6;
#endif
		else if (_strnicmp(Temp, "/DUPLICATE", 9) == 0) {	// duplicate session (2004.12.7. yutaka)
			ts->DuplicateSession = 1;

		}
		else if (_strnicmp(Temp, "/TIMEOUT=", 9) == 0) {	// Connecting Timeout value (2007.1.11. yutaka)
			if (sscanf(&Temp[9], "%d", &pos) == 1) {
				if (pos >= 0)
					ts->ConnectingTimeout = pos;
			}

		}
		else if ((Temp[0] != '/') && (strlen(Temp) > 0)) {
			if (JustAfterHost && (sscanf(Temp, "%d", &c) == 1))
				ParamTCP = c;
			else {
				ParamPort = IdTCPIP;
				strncpy_s(ts->HostName, sizeof(ts->HostName), Temp, _TRUNCATE);	/* host name */
				HostNameFlag = TRUE;
			}
		}
		JustAfterHost = FALSE;
	}

	if ((DDETopic != NULL) && (DDETopic[0] != 0))
		ts->MacroFN[0] = 0;

	if ((ts->HostName[0] != 0) && (ParamPort == IdTCPIP)) {
#ifndef NO_INET6
		ParseHostName(ts->HostName, &ParamTCP);
#else							/* NO_INET6 */
		if ((_strnicmp(ts->HostName, "telnet://", 9) == 0) ||
		    (_strnicmp(ts->HostName, "tn3270://", 9) == 0)) {
			memmove(ts->HostName, &(ts->HostName[9]),
			        strlen(ts->HostName) - 8);
			i = strlen(ts->HostName);
			if ((i > 0) && (ts->HostName[i - 1] == '/'))
				ts->HostName[i - 1] = 0;
		}
		i = 0;
		do {
			b = ts->HostName[i];
			i++;
		} while ((b != 0) && (b != ':'));
		if (b == ':') {
			ts->HostName[i - 1] = 0;
			if (sscanf(&(ts->HostName[i]), "%d", &ParamTCP) != 1)
				ParamTCP = 65535;
		}
#endif							/* NO_INET6 */
	}

	switch (ParamPort) {
	case IdTCPIP:
		ts->PortType = IdTCPIP;
		if (ParamTCP < 65535)
			ts->TCPPort = ParamTCP;
		if (ParamTel < 2)
			ts->Telnet = ParamTel;
		if (ParamBin < 2)
			ts->TelBin = ParamBin;
		break;
	case IdSerial:
		ts->PortType = IdSerial;
		if (ParamCom > 0) {
			ts->ComPort = ParamCom;
			/* Don't display new connection dialog if COM port is specified explicitly (2006.9.15 maya) */
			ts->ComAutoConnect = TRUE;
		}
		break;
	case IdFile:
		ts->PortType = IdFile;
	}

	// TCPLocalEcho/TCPCRSend �𖳌��ɂ��� (maya 2007.4.25)
	if (DisableTCPEchoCR == TRUE) {
		ts->DisableTCPEchoCR = TRUE;
	}
}

BOOL WINAPI DllMain(HANDLE hInst,
                    ULONG ul_reason_for_call, LPVOID lpReserved)
{
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