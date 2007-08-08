/* Tera Term
 Copyright(C) 2006 TeraTerm Project
 All rights reserved. */

#include "i18n.h"
#include "ttlib.h"

#if 1
void FAR PASCAL GetI18nStr(PCHAR section, PCHAR key, PCHAR buf, PCHAR iniFile)
{
	static char tmp[MAX_UIMSG];

	GetPrivateProfileString(section, key, buf, tmp, MAX_UIMSG, iniFile);
	// buf が ts.UIMsg のポインタであることを期待してサイズを固定
	// (2007.6.23 maya)
	strncpy_s(buf, MAX_UIMSG, tmp, _TRUNCATE);
	RestoreNewLine(buf);
}
#else
void FAR PASCAL GetI18nStr(PCHAR section, PCHAR key, PCHAR buf, int buf_len, PCHAR def, PCHAR iniFile)
{
	GetPrivateProfileString(section, key, def, buf, buflen, iniFile);
	RestoreNewLine(buf);
}
#endif

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

	strncpy_s(logfont->lfFaceName, sizeof(logfont->lfFaceName), font, _TRUNCATE);
	logfont->lfCharSet = charset;
	logfont->lfHeight = MulDiv(hight, -ppi, 72);
	logfont->lfWidth = 0;

	return TRUE;
}