/* Tera Term
 Copyright(C) 2006 TeraTerm Project
 All rights reserved. */

#include "i18n.h"
#include "ttlib.h"

void FAR PASCAL GetI18nStr(PCHAR section, PCHAR key, PCHAR buf, PCHAR iniFile)
{
	static char tmp[MAX_UIMSG];

	GetPrivateProfileString(section, key, buf, tmp, MAX_UIMSG, iniFile);
	strncpy(buf, tmp, MAX_UIMSG);
	RestoreNewLine(buf);
}

int FAR PASCAL GetI18nLogfont(PCHAR section, PCHAR key, PLOGFONT logfont, int ppi, PCHAR iniFile)
{
	static char tmp[MAX_UIMSG];
	static char font[LF_FACESIZE];
	int hight, charset;
	GetPrivateProfileString(section, key, "-", tmp, MAX_UIMSG, iniFile);
	if (strcmp(tmp, "-") == 0) {
		return FALSE;
	}

	GetNthString(tmp, 1, LF_FACESIZE-1, font);
	GetNthNum(tmp, 2, &hight);
	GetNthNum(tmp, 3, &charset);

	strncpy(logfont->lfFaceName, font, LF_FACESIZE-1);
	logfont->lfHeight = MulDiv(hight, -ppi, 72);
	logfont->lfCharSet = charset;

	return TRUE;
}