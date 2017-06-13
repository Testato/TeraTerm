/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* misc. routines  */
#include "teraterm.h"
#include <sys/stat.h>
#include <sys/utime.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include "tttypes.h"
#include <shlobj.h>
#include <ctype.h>

// for _ismbblead
#include <mbctype.h>

/* OS version with GetVersionEx(*1)

                dwMajorVersion   dwMinorVersion    dwPlatformId
Windows95       4                0                 VER_PLATFORM_WIN32_WINDOWS
Windows98       4                10                VER_PLATFORM_WIN32_WINDOWS 
WindowsMe       4                90                VER_PLATFORM_WIN32_WINDOWS
WindowsNT4.0    4                0                 VER_PLATFORM_WIN32_NT
Windows2000     5                0                 VER_PLATFORM_WIN32_NT
WindowsXP       5                1                 VER_PLATFORM_WIN32_NT
WindowsXPx64    5                2                 VER_PLATFORM_WIN32_NT
WindowsVista    6                0                 VER_PLATFORM_WIN32_NT
Windows7        6                1                 VER_PLATFORM_WIN32_NT
Windows8        6                2                 VER_PLATFORM_WIN32_NT
Windows8.1(*2)  6                2                 VER_PLATFORM_WIN32_NT
Windows8.1(*3)  6                3                 VER_PLATFORM_WIN32_NT
Windows10(*2)   6                2                 VER_PLATFORM_WIN32_NT
Windows10(*3)   10               0                 VER_PLATFORM_WIN32_NT

(*1) GetVersionEx()�� c4996 warning �ƂȂ�̂́AVS2013(_MSC_VER=1800) ����ł��B
(*2) manifest�� supportedOS Id ��ǉ����Ă��Ȃ��B
(*3) manifest�� supportedOS Id ��ǉ����Ă���B
*/

// for isInvalidFileNameChar / replaceInvalidFileNameChar
static char *invalidFileNameChars = "\\/:*?\"<>|";

// �t�@�C���Ɏg�p���邱�Ƃ��ł��Ȃ�����
// cf. Naming Files, Paths, and Namespaces
//     http://msdn.microsoft.com/en-us/library/aa365247.aspx
// (2013.3.9 yutaka)
static char *invalidFileNameStrings[] = {
	"AUX", "CLOCK$", "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
	"CON", "CONFIG$", "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9",
	"NUL", "PRN",
	".", "..", 
	NULL
};


// for b64encode/b64decode
static char *b64enc_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static char b64dec_table[] = {
   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
   52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
   -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
   15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
   -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
   41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

void b64encode(PCHAR d, int dsize, PCHAR s, int len)
{
	unsigned int b = 0;
	int state = 0;
	unsigned char *src, *dst;

	src = (unsigned char *)s;
	dst = (unsigned char *)d;

	if (dsize == 0 || dst == NULL || src == NULL) {
		return;
	}
	if (dsize < 5) {
		*dst = 0;
		return;
	}

	while (len > 0) {
		b = (b << 8) | *src++;
		len--;
		state++;

		if (state == 3) {
			*dst++ = b64enc_table[(b>>18) & 0x3f];
			*dst++ = b64enc_table[(b>>12) & 0x3f];
			*dst++ = b64enc_table[(b>>6) & 0x3f];
			*dst++ = b64enc_table[b & 0x3f];
			dsize -= 4;
			state = 0;
			b = 0;
			if (dsize < 5)
				break;
		}
	}

	if (dsize >= 5) {
		if (state == 1) {
			*dst++ = b64enc_table[(b>>2) & 0x3f];
			*dst++ = b64enc_table[(b<<4) & 0x3f];
			*dst++ = '=';
			*dst++ = '=';
		}
		else if (state == 2) {
			*dst++ = b64enc_table[(b>>10) & 0x3f];
			*dst++ = b64enc_table[(b>>4) & 0x3f];
			*dst++ = b64enc_table[(b<<2) & 0x3f];
			*dst++ = '=';
		}
	}

	*dst = 0;
	return;
}

int b64decode(PCHAR dst, int dsize, PCHAR src)
{
	unsigned int b = 0;
	char c;
	int len = 0, state = 0;

	if (src == NULL || dst == NULL || dsize == 0)
		return -1;

	while (1) {
		if (isspace(*src)) {
			src++;
			continue;
		}

		if ((c = b64dec_table[*src++]) == -1)
			break;

		b = (b << 6) | c;
		state++;

		if (state == 4) {
			if (dsize > len)
				dst[len++] = (b >> 16) & 0xff;
			if (dsize > len)
				dst[len++] = (b >> 8) & 0xff;
			if (dsize > len)
				dst[len++] = b & 0xff;
			state = 0;
			if (dsize <= len)
				break;
		}
	}

	if (state == 2) {
		b <<= 4;
		if (dsize > len)
			dst[len++] = (b >> 8) & 0xff;
	}
	else if (state == 3) {
		b <<= 6;
		if (dsize > len)
			dst[len++] = (b >> 16) & 0xff;
		if (dsize > len)
			dst[len++] = (b >> 8) & 0xff;
	}
	if (len < dsize) {
		dst[len] = 0;
	}
	else {
		dst[dsize-1] = 0;
	}
	return len;
}

BOOL GetFileNamePos(PCHAR PathName, int far *DirLen, int far *FNPos)
{
	BYTE b;
	LPTSTR Ptr, DirPtr, FNPtr, PtrOld;

	*DirLen = 0;
	*FNPos = 0;
	if (PathName==NULL)
		return FALSE;

	if ((strlen(PathName)>=2) && (PathName[1]==':'))
		Ptr = &PathName[2];
	else
		Ptr = PathName;
	if (Ptr[0]=='\\')
		Ptr = CharNext(Ptr);

	DirPtr = Ptr;
	FNPtr = Ptr;
	while (Ptr[0]!=0) {
		b = Ptr[0];
		PtrOld = Ptr;
		Ptr = CharNext(Ptr);
		switch (b) {
			case ':':
				return FALSE;
			case '\\':
				DirPtr = PtrOld;
				FNPtr = Ptr;
				break;
		}
	}
	*DirLen = DirPtr-PathName;
	*FNPos = FNPtr-PathName;
	return TRUE;
}

BOOL ExtractFileName(PCHAR PathName, PCHAR FileName, int destlen)
{
	int i, j;

	if (FileName==NULL)
		return FALSE;
	if (! GetFileNamePos(PathName,&i,&j))
		return FALSE;
	strncpy_s(FileName,destlen,&PathName[j],_TRUNCATE);
	return (strlen(FileName)>0);
}

BOOL ExtractDirName(PCHAR PathName, PCHAR DirName)
{
	int i, j;

	if (DirName==NULL)
		return FALSE;
	if (! GetFileNamePos(PathName,&i,&j))
		return FALSE;
	memmove(DirName,PathName,i); // do not use memcpy
	DirName[i] = 0;
	return TRUE;
}

/* fit a filename to the windows-filename format */
/* FileName must contain filename part only. */
void FitFileName(PCHAR FileName, int destlen, PCHAR DefExt)
{
	int i, j, NumOfDots;
	char Temp[MAX_PATH];
	BYTE b;

	NumOfDots = 0;
	i = 0;
	j = 0;
	/* filename started with a dot is illeagal */
	if (FileName[0]=='.') {
		Temp[0] = '_';  /* insert an underscore char */
		j++;
	}

	do {
		b = FileName[i];
		i++;
		if (b=='.')
			NumOfDots++;
		if ((b!=0) &&
		    (j < MAX_PATH-1)) {
			Temp[j] = b;
			j++;
		}
	} while (b!=0);
	Temp[j] = 0;

	if ((NumOfDots==0) &&
	    (DefExt!=NULL)) {
		/* add the default extension */
		strncat_s(Temp,sizeof(Temp),DefExt,_TRUNCATE);
	}

	strncpy_s(FileName,destlen,Temp,_TRUNCATE);
}

// Append a slash to the end of a path name
void AppendSlash(PCHAR Path, int destlen)
{
	if (strcmp(CharPrev((LPCTSTR)Path,
	           (LPCTSTR)(&Path[strlen(Path)])),
	           "\\") != 0) {
		strncat_s(Path,destlen,"\\",_TRUNCATE);
	}
}

// Delete slashes at the end of a path name
void DeleteSlash(PCHAR Path)
{
	size_t i;
	for (i=strlen(Path)-1; i>=0; i--) {
		if (i ==0 && Path[i] == '\\' ||
		    Path[i] == '\\' && !_ismbblead(Path[i-1])) {
			Path[i] = '\0';
		}
		else {
			break;
		}
	}
}

void Str2Hex(PCHAR Str, PCHAR Hex, int Len, int MaxHexLen, BOOL ConvSP)
{
	BYTE b, low;
	int i, j;

	if (ConvSP)
		low = 0x20;
	else
		low = 0x1F;

	j = 0;
	for (i=0; i<=Len-1; i++) {
		b = Str[i];
		if ((b!='$') && (b>low) && (b<0x7f)) {
			if (j < MaxHexLen) {
				Hex[j] = b;
				j++;
			}
		}
		else {
			if (j < MaxHexLen-2) {
				Hex[j] = '$';
				j++;
				if (b<=0x9f) {
					Hex[j] = (char)((b >> 4) + 0x30);
				}
				else {
					Hex[j] = (char)((b >> 4) + 0x37);
				}
				j++;
				if ((b & 0x0f) <= 0x9) {
					Hex[j] = (char)((b & 0x0f) + 0x30);
				}
				else {
					Hex[j] = (char)((b & 0x0f) + 0x37);
				}
				j++;
			}
		}
	}
	Hex[j] = 0;
}

BYTE ConvHexChar(BYTE b)
{
	if ((b>='0') && (b<='9')) {
		return (b - 0x30);
	}
	else if ((b>='A') && (b<='F')) {
		return (b - 0x37);
	}
	else if ((b>='a') && (b<='f')) {
		return (b - 0x57);
	}
	else {
		return 0;
	}
}

int Hex2Str(PCHAR Hex, PCHAR Str, int MaxLen)
{
	BYTE b, c;
	int i, imax, j;

	j = 0;
	imax = strlen(Hex);
	i = 0;
	while ((i < imax) && (j<MaxLen)) {
		b = Hex[i];
		if (b=='$') {
			i++;
			if (i < imax) {
				c = Hex[i];
			}
			else {
				c = 0x30;
			}
			b = ConvHexChar(c) << 4;
			i++;
			if (i < imax) {
				c = Hex[i];
			}
			else {
				c = 0x30;
			}
			b = b + ConvHexChar(c);
		};

		Str[j] = b;
		j++;
		i++;
	}
	if (j<MaxLen) {
		Str[j] = 0;
	}

	return j;
}

BOOL DoesFileExist(PCHAR FName)
{
	// check if a file exists or not
	// �t�H���_�܂��̓t�@�C��������� TRUE ��Ԃ�
	struct _stat st;

	return (_stat(FName,&st)==0);
}

BOOL DoesFolderExist(PCHAR FName)
{
	// check if a folder exists or not
	// �}�N���݊����̂���
	// DoesFileExist �͏]���ʂ�t�H���_�܂��̓t�@�C��������� TRUE ��Ԃ�
	// DoesFileExist �̓t�H���_������ꍇ�̂� TRUE ��Ԃ��B
	struct _stat st;

	if (_stat(FName,&st)==0) {
		if ((st.st_mode & _S_IFDIR) > 0) {
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
	else {
		return FALSE;
	}
}

long GetFSize(PCHAR FName)
{
	struct _stat st;

	if (_stat(FName,&st)==-1) {
		return 0;
	}
	return (long)st.st_size;
}

long GetFMtime(PCHAR FName)
{
	struct _stat st;

	if (_stat(FName,&st)==-1) {
		return 0;
	}
	return (long)st.st_mtime;
}

BOOL SetFMtime(PCHAR FName, DWORD mtime)
{
	struct _utimbuf filetime;

	filetime.actime = mtime;
	filetime.modtime = mtime;
	return _utime(FName, &filetime);
}

void uint2str(UINT i, PCHAR Str, int destlen, int len)
{
	char Temp[20];

	memset(Temp, 0, sizeof(Temp));
	_snprintf_s(Temp,sizeof(Temp),_TRUNCATE,"%u",i);
	Temp[len] = 0;
	strncpy_s(Str,destlen,Temp,_TRUNCATE);
}

void QuoteFName(PCHAR FName)
{
	int i;

	if (FName[0]==0) {
		return;
	}
	if (strchr(FName,' ')==NULL) {
		return;
	}
	i = strlen(FName);
	memmove(&(FName[1]),FName,i);
	FName[0] = '\"';
	FName[i+1] = '\"';
	FName[i+2] = 0;
}

// �t�@�C�����Ɏg�p�ł��Ȃ��������܂܂�Ă��邩�m���߂� (2006.8.28 maya)
int isInvalidFileNameChar(PCHAR FName)
{
	int i, len;
	char **p, c;

	// �`�F�b�N�Ώۂ̕��������������B(2013.3.9 yutaka)
	p = invalidFileNameStrings;
	while (*p) {
		if (_strcmpi(FName, *p) == 0) {
			return 1;  // Invalid
		}
		p++;
	}

	len = strlen(FName);
	for (i=0; i<len; i++) {
		if (_ismbblead(FName[i])) {
			i++;
			continue;
		}
		if ((FName[i] >= 0 && FName[i] < ' ') || strchr(invalidFileNameChars, FName[i])) {
			return 1;
		}
	}

	// �t�@�C�����̖����Ƀs���I�h����ы󔒂�NG�B
	c = FName[len - 1];
	if (c == '.' || c == ' ')
		return 1;

	return 0;
}

// �t�@�C�����Ɏg�p�ł��Ȃ������� c �ɒu��������
// c �� 0 ���w�肵���ꍇ�͕������폜����
void replaceInvalidFileNameChar(PCHAR FName, unsigned char c)
{
	int i, j=0, len;

	if ((c >= 0 && c < ' ') || strchr(invalidFileNameChars, c)) {
		c = 0;
	}

	len = strlen(FName);
	for (i=0; i<len; i++) {
		if (_ismbblead(FName[i])) {
			FName[j++] = FName[i];
			FName[j++] = FName[++i];
			continue;
		}
		if ((FName[i] >= 0 && FName[i] < ' ') || strchr(invalidFileNameChars, FName[i])) {
			if (c) {
				FName[j++] = c;
			}
		}
		else {
			FName[j++] = FName[i];
		}
	}
	FName[j] = 0;
}

// strftime �ɓn���Ȃ��������܂܂�Ă��邩�m���߂� (2006.8.28 maya)
int isInvalidStrftimeChar(PCHAR FName)
{
	int i, len, p;

	len = strlen(FName);
	for (i=0; i<len; i++) {
		if (FName[i] == '%') {
			if (FName[i+1] != 0) {
				p = i+1;
				if (FName[i+2] != 0 && FName[i+1] == '#') {
					p = i+2;
				}
				switch (FName[p]) {
					case 'a':
					case 'A':
					case 'b':
					case 'B':
					case 'c':
					case 'd':
					case 'H':
					case 'I':
					case 'j':
					case 'm':
					case 'M':
					case 'p':
					case 'S':
					case 'U':
					case 'w':
					case 'W':
					case 'x':
					case 'X':
					case 'y':
					case 'Y':
					case 'z':
					case 'Z':
					case '%':
						i = p;
						break;
					default:
						return 1;
				}
			}
			else {
				// % �ŏI����Ă���ꍇ�̓G���[�Ƃ���
				return 1;
			}
		}
	}

	return 0;
}

// strftime �ɓn���Ȃ��������폜���� (2006.8.28 maya)
void deleteInvalidStrftimeChar(PCHAR FName)
{
	int i, j=0, len, p;

	len = strlen(FName);
	for (i=0; i<len; i++) {
		if (FName[i] == '%') {
			if (FName[i+1] != 0) {
				p = i+1;
				if (FName[i+2] != 0 && FName[i+1] == '#') {
					p = i+2;
				}
				switch (FName[p]) {
					case 'a':
					case 'A':
					case 'b':
					case 'B':
					case 'c':
					case 'd':
					case 'H':
					case 'I':
					case 'j':
					case 'm':
					case 'M':
					case 'p':
					case 'S':
					case 'U':
					case 'w':
					case 'W':
					case 'x':
					case 'X':
					case 'y':
					case 'Y':
					case 'z':
					case 'Z':
					case '%':
						FName[j] = FName[i]; // %
						j++;
						i++;
						if (p-i == 2) {
							FName[j] = FName[i]; // #
							j++;
							i++;
						}
						FName[j] = FName[i];
						j++;
						break;
					default:
						i++; // %
						if (p-i == 2) {
							i++; // #
						}
				}
			}
			// % �ŏI����Ă���ꍇ�̓R�s�[���Ȃ�
		}
		else {
			FName[j] = FName[i];
			j++;
		}
	}

	FName[j] = 0;
}

// �t���p�X����A�t�@�C���������݂̂� strftime �ŕϊ����� (2006.8.28 maya)
void ParseStrftimeFileName(PCHAR FName, int destlen)
{
	char filename[MAX_PATH];
	char dirname[MAX_PATH];
	char buf[MAX_PATH];
	char *c;
	time_t time_local;
	struct tm *tm_local;

	// �t�@�C���������݂̂� flename �Ɋi�[
	ExtractFileName(FName, filename ,sizeof(filename));

	// strftime �Ɏg�p�ł��Ȃ��������폜
	deleteInvalidStrftimeChar(filename);

	// ���ݎ������擾
	time(&time_local);
	tm_local = localtime(&time_local);

	// ����������ɕϊ�
	if (strftime(buf, sizeof(buf), filename, tm_local) == 0) {
		strncpy_s(buf, sizeof(buf), filename, _TRUNCATE);
	}

	// �t�@�C�����Ɏg�p�ł��Ȃ��������폜
	deleteInvalidFileNameChar(buf);

	c = strrchr(FName, '\\');
	if (c != NULL) {
		ExtractDirName(FName, dirname);
		strncpy_s(FName, destlen, dirname, _TRUNCATE);
		AppendSlash(FName,destlen);
		strncat_s(FName, destlen, buf, _TRUNCATE);
	}
	else { // "\"���܂܂Ȃ�(�t���p�X�łȂ�)�ꍇ�ɑΉ� (2006.11.30 maya)
		strncpy_s(FName, destlen, buf, _TRUNCATE);
	}
}

void ConvFName(PCHAR HomeDir, PCHAR Temp, int templen, PCHAR DefExt, PCHAR FName, int destlen)
{
	// destlen = sizeof FName
	int DirLen, FNPos;

	FName[0] = 0;
	if ( ! GetFileNamePos(Temp,&DirLen,&FNPos) ) {
		return;
	}
	FitFileName(&Temp[FNPos],templen - FNPos,DefExt);
	if ( DirLen==0 ) {
		strncpy_s(FName,destlen,HomeDir,_TRUNCATE);
		AppendSlash(FName,destlen);
	}
	strncat_s(FName,destlen,Temp,_TRUNCATE);
}

// "\n" �����s�ɕϊ����� (2006.7.29 maya)
// "\t" ���^�u�ɕϊ����� (2006.11.6 maya)
void RestoreNewLine(PCHAR Text)
{
	int i, j=0, size=strlen(Text);
	char *buf = (char *)_alloca(size+1);

	memset(buf, 0, size+1);
	for (i=0; i<size; i++) {
		if (Text[i] == '\\' && i<size ) {
			switch (Text[i+1]) {
				case '\\':
					buf[j] = '\\';
					i++;
					break;
				case 'n':
					buf[j] = '\n';
					i++;
					break;
				case 't':
					buf[j] = '\t';
					i++;
					break;
				case '0':
					buf[j] = '\0';
					i++;
					break;
				default:
					buf[j] = '\\';
			}
			j++;
		}
		else {
			buf[j] = Text[i];
			j++;
		}
	}
	/* use memcpy to copy with '\0' */
	memcpy(Text, buf, size);
}

BOOL GetNthString(PCHAR Source, int Nth, int Size, PCHAR Dest)
{
	int i, j, k;

	i = 1;
	j = 0;
	k = 0;

	while (i<Nth && Source[j] != 0) {
		if (Source[j++] == ',') {
			i++;
		}
	}

	if (i == Nth) {
		while (Source[j] != 0 && Source[j] != ',' && k<Size-1) {
			Dest[k++] = Source[j++];
		}
	}

	Dest[k] = 0;
	return (i>=Nth);
}

void GetNthNum(PCHAR Source, int Nth, int far *Num)
{
	char T[15];

	GetNthString(Source,Nth,sizeof(T),T);
	if (sscanf(T, "%d", Num) != 1) {
		*Num = 0;
	}
}

int GetNthNum2(PCHAR Source, int Nth, int defval)
{
	char T[15];
	int v;

	GetNthString(Source, Nth, sizeof(T), T);
	if (sscanf(T, "%d", &v) != 1) {
		v = defval;
	}
	
	return v;
}

void GetDownloadFolder(char *dest, int destlen)
{
	HMODULE hDll;
	typedef GUID KNOWNFOLDERID;
	typedef HRESULT(WINAPI *SHGETKNOWNFOLDERPATH)(KNOWNFOLDERID*, DWORD, HANDLE, PWSTR*);
	// {374DE290-123F-4565-9164-39C4925E467B}
	KNOWNFOLDERID FOLDERID_Downloads = { 0x374de290, 0x123f, 0x4565, 0x91, 0x64, 0x39, 0xc4, 0x92, 0x5e, 0x46, 0x7b };
	char download[MAX_PATH];

	memset(download, 0, sizeof(download));
	if (hDll = LoadLibrary("shell32.dll")) {
		SHGETKNOWNFOLDERPATH pSHGetKnownFolderPath = (SHGETKNOWNFOLDERPATH)GetProcAddress(hDll, "SHGetKnownFolderPath");
		if (pSHGetKnownFolderPath) {
			PWSTR pBuffer = NULL;
			pSHGetKnownFolderPath(&FOLDERID_Downloads, 0, NULL, &pBuffer);
			WideCharToMultiByte(CP_ACP, 0, pBuffer, -1, download, sizeof(download), NULL, NULL);
		}
		FreeLibrary(hDll);
	}
	if (strlen(download) == 0) {
		LPITEMIDLIST pidl;
		if (SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &pidl) == NOERROR) {
			SHGetPathFromIDList(pidl, download);
			CoTaskMemFree(pidl);
		}
	}
	if (strlen(download) > 0) {
		strncpy_s(dest, destlen, download, _TRUNCATE);
	}
}

void WINAPI GetDefaultFName(char *home, char *file, char *dest, int destlen)
{
	// My Documents �� file ������ꍇ�A
	// �����ǂݍ��ނ悤�ɂ����B(2007.2.18 maya)
	char MyDoc[MAX_PATH];
	char MyDocSetupFName[MAX_PATH];
	LPITEMIDLIST pidl;

	IMalloc *pmalloc;
	SHGetMalloc(&pmalloc);
	if (SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &pidl) == S_OK) {
		SHGetPathFromIDList(pidl, MyDoc);
		pmalloc->lpVtbl->Free(pmalloc, pidl);
		pmalloc->lpVtbl->Release(pmalloc);
	}
	else {
		pmalloc->lpVtbl->Release(pmalloc);
		goto homedir;
	}
	strncpy_s(MyDocSetupFName, sizeof(MyDocSetupFName), MyDoc, _TRUNCATE);
	AppendSlash(MyDocSetupFName,sizeof(MyDocSetupFName));
	strncat_s(MyDocSetupFName, sizeof(MyDocSetupFName), file, _TRUNCATE);
	if (GetFileAttributes(MyDocSetupFName) != -1) {
		strncpy_s(dest, destlen, MyDocSetupFName, _TRUNCATE);
		return;
	}

homedir:
	strncpy_s(dest, destlen, home, _TRUNCATE);
	AppendSlash(dest,destlen);
	strncat_s(dest, destlen, file, _TRUNCATE);
}

// �f�t�H���g�� TERATERM.INI �̃t���p�X�� ttpmacro �����
// �擾���邽�߂ɒǉ������B(2007.2.18 maya)
void GetDefaultSetupFName(char *home, char *dest, int destlen)
{
	GetDefaultFName(home, "TERATERM.INI", dest, destlen);
}

void GetUILanguageFile(char *buf, int buflen)
{
	char HomeDir[MAX_PATH];
	char Temp[MAX_PATH];
	char SetupFName[MAX_PATH];
	char CurDir[MAX_PATH];

	/* Get home directory */
	if (GetModuleFileName(NULL,Temp,sizeof(Temp)) == 0) {
		memset(buf, 0, buflen);
		return;
	}
	ExtractDirName(Temp, HomeDir);

	/* Get SetupFName */
	GetDefaultSetupFName(HomeDir, SetupFName, sizeof(SetupFName));
	
	/* Get LanguageFile name */
	GetPrivateProfileString("Tera Term", "UILanguageFile", "",
	                        Temp, sizeof(Temp), SetupFName);

	GetCurrentDirectory(sizeof(CurDir), CurDir);
	SetCurrentDirectory(HomeDir);
	_fullpath(buf, Temp, buflen);
	SetCurrentDirectory(CurDir);
}

// �w�肵���G���g���� teraterm.ini ����ǂݎ�� (2009.3.23 yutaka)
void GetOnOffEntryInifile(char *entry, char *buf, int buflen)
{
	char HomeDir[MAX_PATH];
	char Temp[MAX_PATH];
	char SetupFName[MAX_PATH];

	/* Get home directory */
	if (GetModuleFileName(NULL,Temp,sizeof(Temp)) == 0) {
		strncpy_s(buf, buflen, "off", _TRUNCATE);
		return;
	}
	ExtractDirName(Temp, HomeDir);

	/* Get SetupFName */
	GetDefaultSetupFName(HomeDir, SetupFName, sizeof(SetupFName));
	
	/* Get LanguageFile name */
	GetPrivateProfileString("Tera Term", entry, "off",
	                        Temp, sizeof(Temp), SetupFName);

	strncpy_s(buf, buflen, Temp, _TRUNCATE);
}

void get_lang_msg(PCHAR key, PCHAR buf, int buf_len, PCHAR def, PCHAR iniFile)
{
	GetI18nStr("Tera Term", key, buf, buf_len, def, iniFile);
}

int get_lang_font(PCHAR key, HWND dlg, PLOGFONT logfont, HFONT *font, PCHAR iniFile)
{
	if (GetI18nLogfont("Tera Term", key, logfont,
	                   GetDeviceCaps(GetDC(dlg),LOGPIXELSY),
	                   iniFile) == FALSE) {
		return FALSE;
	}

	if ((*font = CreateFontIndirect(logfont)) == NULL) {
		return FALSE;
	}

	return TRUE;
}

//
// cf. http://homepage2.nifty.com/DSS/VCPP/API/SHBrowseForFolder.htm
//
int CALLBACK setDefaultFolder(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if(uMsg == BFFM_INITIALIZED) {
		SendMessage(hwnd, BFFM_SETSELECTION, (WPARAM)TRUE, lpData);
	}
	return 0;
}

BOOL doSelectFolder(HWND hWnd, char *path, int pathlen, char *def, char *msg)
{
	BROWSEINFO      bi;
	LPITEMIDLIST    pidlRoot;      // �u���E�Y�̃��[�gPIDL
	LPITEMIDLIST    pidlBrowse;    // ���[�U�[���I������PIDL
	char buf[MAX_PATH];
	BOOL ret = FALSE;

	// �_�C�A���O�\�����̃��[�g�t�H���_��PIDL���擾
	// ���ȉ��̓f�X�N�g�b�v�����[�g�Ƃ��Ă���B�f�X�N�g�b�v�����[�g�Ƃ���
	//   �ꍇ�́A�P�� bi.pidlRoot �ɂO��ݒ肷�邾���ł��悢�B���̑��̓�
	//   ��t�H���_�����[�g�Ƃ��鎖���ł���B�ڍׂ�SHGetSpecialFolderLoca
	//   tion�̃w���v���Q�Ƃ̎��B
	if (!SUCCEEDED(SHGetSpecialFolderLocation(hWnd, CSIDL_DESKTOP, &pidlRoot))) {
		return FALSE;
	}

	// BROWSEINFO�\���̂̏����l�ݒ�
	// ��BROWSEINFO�\���̂̊e�����o�̏ڍא������w���v���Q��
	bi.hwndOwner = hWnd;
	bi.pidlRoot = pidlRoot;
	bi.pszDisplayName = buf;
	bi.lpszTitle = msg;
	bi.ulFlags = 0;
	bi.lpfn = setDefaultFolder;
	bi.lParam = (LPARAM)def;
	// �t�H���_�I���_�C�A���O�̕\�� 
	pidlBrowse = SHBrowseForFolder(&bi);
	if (pidlBrowse != NULL) {  
		// PIDL�`���̖߂�l�̃t�@�C���V�X�e���̃p�X�ɕϊ�
		if (SHGetPathFromIDList(pidlBrowse, buf)) {
			// �擾����
			strncpy_s(path, pathlen, buf, _TRUNCATE);
			ret = TRUE;
		}
		// SHBrowseForFolder�̖߂�lPIDL�����
		CoTaskMemFree(pidlBrowse);
	}
	// �N���[���A�b�v����
	CoTaskMemFree(pidlRoot);

	return ret;
}

void OutputDebugPrintf(char *fmt, ...) {
	char tmp[1024];
	va_list arg;
	va_start(arg, fmt);
	_vsnprintf(tmp, sizeof(tmp), fmt, arg);
	OutputDebugString(tmp);
}

#if (_MSC_VER < 1800)
BOOL vercmp(
	DWORD cond_val,
	DWORD act_val,
	DWORD dwTypeMask)
{
	switch (dwTypeMask) {
	case VER_EQUAL:
		if (act_val == cond_val) {
			return TRUE;
		}
		break;
	case VER_GREATER:
		if (act_val > cond_val) {
			return TRUE;
		}
		break;
	case VER_GREATER_EQUAL:
		if (act_val >= cond_val) {
			return TRUE;
		}
		break;
	case VER_LESS:
		if (act_val < cond_val) {
			return TRUE;
		}
		break;
	case VER_LESS_EQUAL:
		if (act_val <= cond_val) {
			return TRUE;
		}
		break;
	}
	return FALSE;
}

/*
DWORDLONG dwlConditionMask
| 000 | 000 | 000 | 000 | 000 | 000 | 000 | 000 |
   |     |     |     |     |     |     |     +- condition of dwMinorVersion
   |     |     |     |     |     |     +------- condition of dwMajorVersion
   |     |     |     |     |     +------------- condition of dwBuildNumber
   |     |     |     |     +------------------- condition of dwPlatformId
   |     |     |     +------------------------- condition of wServicePackMinor
   |     |     +------------------------------- condition of wServicePackMajor
   |     +------------------------------------- condition of wSuiteMask
   +------------------------------------------- condition of wProductType
*/
BOOL _myVerifyVersionInfo(
	LPOSVERSIONINFOEX lpVersionInformation,
	DWORD dwTypeMask,
	DWORDLONG dwlConditionMask)
{
	OSVERSIONINFO osvi;
	WORD cond;
	BOOL ret, check_next;

	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);

	if (dwTypeMask & VER_BUILDNUMBER) {
		cond = (WORD)((dwlConditionMask >> (2*3)) & 0x07);
		if (!vercmp(lpVersionInformation->dwBuildNumber, osvi.dwBuildNumber, cond)) {
			return FALSE;
		}
	}
	if (dwTypeMask & VER_PLATFORMID) {
		cond = (WORD)((dwlConditionMask >> (3*3)) & 0x07);
		if (!vercmp(lpVersionInformation->dwPlatformId, osvi.dwPlatformId, cond)) {
			return FALSE;
		}
	}
	ret = TRUE;
	if (dwTypeMask & (VER_MAJORVERSION | VER_MINORVERSION)) {
		check_next = TRUE;
		if (dwTypeMask & VER_MAJORVERSION) {
			cond = (WORD)((dwlConditionMask >> (1*3)) & 0x07);
			if (cond == VER_EQUAL) {
				if (!vercmp(lpVersionInformation->dwMajorVersion, osvi.dwMajorVersion, cond)) {
					return FALSE;
				}
			}
			else {
				ret = vercmp(lpVersionInformation->dwMajorVersion, osvi.dwMajorVersion, cond);
				// ret: result of major version
				if (!vercmp(lpVersionInformation->dwMajorVersion, osvi.dwMajorVersion, VER_EQUAL)) {
					// !vercmp(...: result of GRATOR/LESS than (not "GRATOR/LESS than or equal to") of major version
					// e.g.
					//   lpvi:5.1 actual:5.0 cond:VER_GREATER_EQUAL  ret:TRUE  !vercmp(...:FALSE  must check minor
					//   lpvi:5.1 actual:5.1 cond:VER_GREATER_EQUAL  ret:TRUE  !vercmp(...:FALSE  must check minor
					//   lpvi:5.1 actual:5.2 cond:VER_GREATER_EQUAL  ret:TRUE  !vercmp(...:FALSE  must check minor
					//   lpvi:5.1 actual:6.0 cond:VER_GREATER_EQUAL  ret:TRUE  !vercmp(...:TRUE   must not check minor
					//   lpvi:5.1 actual:6.1 cond:VER_GREATER_EQUAL  ret:TRUE  !vercmp(...:TRUE   must not check minor
					//   lpvi:5.1 actual:6.2 cond:VER_GREATER_EQUAL  ret:TRUE  !vercmp(...:TRUE   must not check minor
					//   lpvi:5.1 actual:5.0 cond:VER_GREATER        ret:FALSE !vercmp(...:FALSE  must check minor
					//   lpvi:5.1 actual:5.1 cond:VER_GREATER        ret:FALSE !vercmp(...:FALSE  must check minor
					//   lpvi:5.1 actual:5.2 cond:VER_GREATER        ret:FALSE !vercmp(...:FALSE  must check minor
					//   lpvi:5.1 actual:6.0 cond:VER_GREATER        ret:TRUE  !vercmp(...:TRUE   must not check minor
					//   lpvi:5.1 actual:6.1 cond:VER_GREATER        ret:TRUE  !vercmp(...:TRUE   must not check minor
					//   lpvi:5.1 actual:6.2 cond:VER_GREATER        ret:TRUE  !vercmp(...:TRUE   must not check minor
					check_next = FALSE;
				}
			}
		}
		if (check_next && (dwTypeMask & VER_MINORVERSION)) {
			cond = (WORD)((dwlConditionMask >> (0*3)) & 0x07);
			if (cond == VER_EQUAL) {
				if (!vercmp(lpVersionInformation->dwMinorVersion, osvi.dwMinorVersion, cond)) {
					return FALSE;
				}
			}
			else {
				ret = vercmp(lpVersionInformation->dwMinorVersion, osvi.dwMinorVersion, cond);
			}
		}
	}
	return ret;
}
#endif

BOOL myVerifyVersionInfo(
	LPOSVERSIONINFOEX lpVersionInformation,
	DWORD dwTypeMask,
	DWORDLONG dwlConditionMask)
{
#if (_MSC_VER >= 1800)
	return VerifyVersionInfo(lpVersionInformation, dwTypeMask, dwlConditionMask);
#else
	return _myVerifyVersionInfo(lpVersionInformation, dwTypeMask, dwlConditionMask);
#endif
}

ULONGLONG _myVerSetConditionMask(ULONGLONG dwlConditionMask, DWORD dwTypeBitMask, BYTE dwConditionMask)
{
	ULONGLONG result, mask;
	BYTE op = dwConditionMask & 0x07;

	switch (dwTypeBitMask) {
		case VER_MINORVERSION:
			mask = 0x07 << (0 * 3);
			result = dwlConditionMask & ~mask;
			result |= op << (0 * 3);
			break;
		case VER_MAJORVERSION:
			mask = 0x07 << (1 * 3);
			result = dwlConditionMask & ~mask;
			result |= op << (1 * 3);
			break;
		case VER_BUILDNUMBER:
			mask = 0x07 << (2 * 3);
			result = dwlConditionMask & ~mask;
			result |= op << (2 * 3);
			break;
		case VER_PLATFORMID:
			mask = 0x07 << (3 * 3);
			result = dwlConditionMask & ~mask;
			result |= op << (3 * 3);
			break;
		case VER_SERVICEPACKMINOR:
			mask = 0x07 << (4 * 3);
			result = dwlConditionMask & ~mask;
			result |= op << (4 * 3);
			break;
		case VER_SERVICEPACKMAJOR:
			mask = 0x07 << (5 * 3);
			result = dwlConditionMask & ~mask;
			result |= op << (5 * 3);
			break;
		case VER_SUITENAME:
			mask = 0x07 << (6 * 3);
			result = dwlConditionMask & ~mask;
			result |= op << (6 * 3);
			break;
		case VER_PRODUCT_TYPE:
			mask = 0x07 << (7 * 3);
			result = dwlConditionMask & ~mask;
			result |= op << (7 * 3);
			break;
	}

	return result;
}

ULONGLONG myVerSetConditionMask(ULONGLONG dwlConditionMask, DWORD dwTypeBitMask, BYTE dwConditionMask)
{
	typedef ULONGLONG(WINAPI *func)(ULONGLONG, DWORD, BYTE);
	static HMODULE hmodKernel32 = NULL;
	static func pVerSetConditionMask = NULL;
	char kernel32_dll[MAX_PATH];

	GetSystemDirectory(kernel32_dll, sizeof(kernel32_dll));
	strncat_s(kernel32_dll, sizeof(kernel32_dll), "\\kernel32.dll", _TRUNCATE);
	if (hmodKernel32 == NULL) {
		hmodKernel32 = LoadLibrary(kernel32_dll);
		if (hmodKernel32 != NULL) {
			pVerSetConditionMask = (func)GetProcAddress(hmodKernel32, "VerSetConditionMask");
		}
	}

	if (pVerSetConditionMask == NULL) {
		return _myVerSetConditionMask(dwlConditionMask, dwTypeBitMask, dwConditionMask);
	}

	return pVerSetConditionMask(dwlConditionMask, dwTypeBitMask, dwConditionMask);
}

// OS�� �w�肳�ꂽ�o�[�W�����Ɠ����� ���ǂ����𔻕ʂ���B
BOOL IsWindowsVer(DWORD dwPlatformId, DWORD dwMajorVersion, DWORD dwMinorVersion)
{
	OSVERSIONINFOEX osvi;
	DWORDLONG dwlConditionMask = 0;
	int op = VER_EQUAL;
	BOOL ret;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	osvi.dwPlatformId = dwPlatformId;
	osvi.dwMajorVersion = dwMajorVersion;
	osvi.dwMinorVersion = dwMinorVersion;
	dwlConditionMask = myVerSetConditionMask(dwlConditionMask, VER_PLATFORMID, op);
	dwlConditionMask = myVerSetConditionMask(dwlConditionMask, VER_MAJORVERSION, op);
	dwlConditionMask = myVerSetConditionMask(dwlConditionMask, VER_MINORVERSION, op);
	ret = myVerifyVersionInfo(&osvi, VER_PLATFORMID | VER_MAJORVERSION | VER_MINORVERSION, dwlConditionMask);
	return (ret);
}

// OS�� �w�肳�ꂽ�o�[�W�����ȍ~ ���ǂ����𔻕ʂ���B
//   dwPlatformId �����Ă��Ȃ��̂� NT �J�[�l�����ł�����r�ł��Ȃ�
//   5.0 �ȏ�Ŕ�r���邱��
BOOL IsWindowsVerOrLater(DWORD dwMajorVersion, DWORD dwMinorVersion)
{
	OSVERSIONINFOEX osvi;
	DWORDLONG dwlConditionMask = 0;
	int op = VER_GREATER_EQUAL;
	BOOL ret;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	osvi.dwMajorVersion = dwMajorVersion;
	osvi.dwMinorVersion = dwMinorVersion;
	dwlConditionMask = myVerSetConditionMask(dwlConditionMask, VER_MAJORVERSION, op);
	dwlConditionMask = myVerSetConditionMask(dwlConditionMask, VER_MINORVERSION, op);
	ret = myVerifyVersionInfo(&osvi, VER_MAJORVERSION | VER_MINORVERSION, dwlConditionMask);
	return (ret);
}

// OS�� WindowsNT �J�[�l�����ǂ����𔻕ʂ���B
//
// return TRUE:  NT kernel
//        FALSE: Not NT kernel
BOOL IsWindowsNTKernel()
{
	OSVERSIONINFOEX osvi;
	DWORDLONG dwlConditionMask = 0;
	int op = VER_EQUAL;
	BOOL ret;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	osvi.dwPlatformId = VER_PLATFORM_WIN32_NT;
	dwlConditionMask = myVerSetConditionMask(dwlConditionMask, VER_PLATFORMID, op);
	ret = myVerifyVersionInfo(&osvi, VER_PLATFORMID, dwlConditionMask);
	return (ret);
}

// OS�� Windows95 ���ǂ����𔻕ʂ���B
BOOL IsWindows95()
{
	return IsWindowsVer(VER_PLATFORM_WIN32_WINDOWS, 4, 0);
}

// OS�� WindowsMe ���ǂ����𔻕ʂ���B
BOOL IsWindowsMe()
{
	return IsWindowsVer(VER_PLATFORM_WIN32_WINDOWS, 4, 90);
}

// OS�� WindowsNT4.0 ���ǂ����𔻕ʂ���B
BOOL IsWindowsNT4()
{
	return IsWindowsVer(VER_PLATFORM_WIN32_NT, 4, 0);
}

BOOL is_NT4()
{
	// VS2013�ȏゾ�� GetVersionEx() ���x���ƂȂ邽�߁AVerifyVersionInfo() ���g���B
	// �������AVS2013�Ńr���h�����v���O�����́A�������� NT4.0 �ł͓��삵�Ȃ����߁A
	// �������� FALSE ��Ԃ��Ă��悢��������Ȃ��B
	return IsWindowsVer(VER_PLATFORM_WIN32_NT, 4, 0);
}

// OS�� Windows2000 ���ǂ����𔻕ʂ���B
BOOL IsWindows2000()
{
	return IsWindowsVer(VER_PLATFORM_WIN32_NT, 5, 0);
}

// OS�� Windows2000 �ȍ~ ���ǂ����𔻕ʂ���B
//
// return TRUE:  2000 or later
//        FALSE: NT4 or earlier
BOOL IsWindows2000OrLater(void)
{
	return IsWindowsVerOrLater(5, 0);
}

// OS�� WindowsVista �ȍ~ ���ǂ����𔻕ʂ���B
//
// return TRUE:  Vista or later
//        FALSE: XP or earlier
BOOL IsWindowsVistaOrLater(void)
{
	return IsWindowsVerOrLater(6, 0);
}

// OS�� Windows7 �ȍ~ ���ǂ����𔻕ʂ���B
//
// return TRUE:  7 or later
//        FALSE: Vista or earlier
BOOL IsWindows7OrLater(void)
{
	return IsWindowsVerOrLater(6, 1);
}

// OS ���}���`���j�^ API ���T�|�[�g���Ă��邩�ǂ����𔻕ʂ���B
//   98 �ȍ~/2000 �ȍ~�� TRUE ��Ԃ�
BOOL HasMultiMonitorSupport()
{
	HMODULE mod;

	if (((mod = GetModuleHandle("user32.dll")) != NULL) &&
	    (GetProcAddress(mod, "MonitorFromPoint") != NULL)) {
		return TRUE;
	}
	return FALSE;
}

// OS �� GetAdaptersAddresses ���T�|�[�g���Ă��邩�ǂ����𔻕ʂ���B
//   XP �ȍ~�� TRUE ��Ԃ�
//   2000 �ȍ~�� IPv6 �ɑΉ����Ă��邪 GetAdaptersAddresses ���Ȃ�
BOOL HasGetAdaptersAddresses()
{
	HMODULE mod;

	if (((mod = GetModuleHandle("iphlpapi.dll")) != NULL) &&
		(GetProcAddress(mod, "GetAdaptersAddresses") != NULL)) {
		return TRUE;
	}
	return FALSE;
}

// OS �� DnsQuery ���T�|�[�g���Ă��邩�ǂ����𔻕ʂ���B
//   2000 �ȍ~�� TRUE ��Ԃ�
BOOL HasDnsQuery()
{
	HMODULE mod;

	if (((mod = GetModuleHandle("Dnsapi.dll")) != NULL) &&
		(GetProcAddress(mod, "DnsQuery") != NULL)) {
		return TRUE;
	}
	return FALSE;
}

// �ʒm�A�C�R���ł̃o���[���`�b�v�ɑΉ����Ă��邩���ʂ���B
// Me/2000 �ȍ~�� TRUE ��Ԃ�
BOOL HasBalloonTipSupport()
{
	return IsWindows2000OrLater() || IsWindowsMe();
}

int get_OPENFILENAME_SIZE()
{
	if (IsWindows2000OrLater()) {
		return sizeof(OPENFILENAME);
	}
	//return OPENFILENAME_SIZE_VERSION_400;
	return 76;
}

// convert table for KanjiCodeID and ListID
// cf. KanjiList,KanjiListSend
//     KoreanList,KoreanListSend
//     Utf8List,Utf8ListSend
//     IdSJIS, IdEUC, IdJIS, IdUTF8, IdUTF8m
//     IdEnglish, IdJapanese, IdRussian, IdKorean, IdUtf8
/* KanjiCode2List(Language,KanjiCodeID) returns ListID */
int KanjiCode2List(int lang, int kcode)
{
	int Table[5][5] = {
		{1, 2, 3, 4, 5}, /* English (dummy) */
		{1, 2, 3, 4, 5}, /* Japanese(dummy) */
		{1, 2, 3, 4, 5}, /* Russian (dummy) */
		{1, 1, 1, 2, 3}, /* Korean */
		{1, 1, 1, 1, 2}, /* Utf8 */
	};
	lang--;
	kcode--;
	return Table[lang][kcode];
}
/* List2KanjiCode(Language,ListID) returns KanjiCodeID */
int List2KanjiCode(int lang, int list)
{
	int Table[5][5] = {
		{1, 2, 3, 4, 5}, /* English (dummy) */
		{1, 2, 3, 4, 5}, /* Japanese(dummy) */
		{1, 2, 3, 4, 5}, /* Russian (dummy) */
		{1, 4, 5, 1, 1}, /* Korean */
		{4, 5, 4, 4, 4}, /* Utf8 */
	};
	lang--;
	list--;
	if (list < 0) {
		list = 0;
	}
	return Table[lang][list];
}
/* KanjiCodeTranslate(Language(dest), KanjiCodeID(source)) returns KanjiCodeID */
int KanjiCodeTranslate(int lang, int kcode)
{
	int Table[5][5] = {
		{1, 2, 3, 4, 5}, /* to English (dummy) */
		{1, 2, 3, 4, 5}, /* to Japanese(dummy) */
		{1, 2, 3, 4, 5}, /* to Russian (dummy) */
		{1, 1, 1, 4, 5}, /* to Korean */
		{4, 4, 4, 4, 5}, /* to Utf8 */
	};
	lang--;
	kcode--;
	return Table[lang][kcode];
}

char *mctimelocal(char *format, BOOL utc_flag)
{
	SYSTEMTIME systime;
	static char strtime[29];
	char week[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	char month[][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	                   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	char tmp[5];
	unsigned int i = strlen(format);

	if (utc_flag) {
		GetSystemTime(&systime);
	}
	else {
		GetLocalTime(&systime);
	}
	memset(strtime, 0, sizeof(strtime));
	for (i=0; i<strlen(format); i++) {
		if (format[i] == '%') {
			char c = format[i + 1];
			switch (c) {
				case 'a':
					_snprintf_s(tmp, sizeof(tmp), _TRUNCATE, "%s", week[systime.wDayOfWeek]);
					strncat_s(strtime, sizeof(strtime), tmp, _TRUNCATE);
					i++;
					break;
				case 'b':
					_snprintf_s(tmp, sizeof(tmp), _TRUNCATE, "%s", month[systime.wMonth - 1]);
					strncat_s(strtime, sizeof(strtime), tmp, _TRUNCATE);
					i++;
					break;
				case 'd':
					_snprintf_s(tmp, sizeof(tmp), _TRUNCATE, "%02d", systime.wDay);
					strncat_s(strtime, sizeof(strtime), tmp, _TRUNCATE);
					i++;
					break;
				case 'e':
					_snprintf_s(tmp, sizeof(tmp), _TRUNCATE, "%2d", systime.wDay);
					strncat_s(strtime, sizeof(strtime), tmp, _TRUNCATE);
					i++;
					break;
				case 'H':
					_snprintf_s(tmp, sizeof(tmp), _TRUNCATE, "%02d", systime.wHour);
					strncat_s(strtime, sizeof(strtime), tmp, _TRUNCATE);
					i++;
					break;
				case 'N':
					_snprintf_s(tmp, sizeof(tmp), _TRUNCATE, "%03d", systime.wMilliseconds);
					strncat_s(strtime, sizeof(strtime), tmp, _TRUNCATE);
					i++;
					break;
				case 'm':
					_snprintf_s(tmp, sizeof(tmp), _TRUNCATE, "%02d", systime.wMonth);
					strncat_s(strtime, sizeof(strtime), tmp, _TRUNCATE);
					i++;
					break;
				case 'M':
					_snprintf_s(tmp, sizeof(tmp), _TRUNCATE, "%02d", systime.wMinute);
					strncat_s(strtime, sizeof(strtime), tmp, _TRUNCATE);
					i++;
					break;
				case 'S':
					_snprintf_s(tmp, sizeof(tmp), _TRUNCATE, "%02d", systime.wSecond);
					strncat_s(strtime, sizeof(strtime), tmp, _TRUNCATE);
					i++;
					break;
				case 'w':
					_snprintf_s(tmp, sizeof(tmp), _TRUNCATE, "%d", systime.wDayOfWeek);
					strncat_s(strtime, sizeof(strtime), tmp, _TRUNCATE);
					i++;
					break;
				case 'Y':
					_snprintf_s(tmp, sizeof(tmp), _TRUNCATE, "%04d", systime.wYear);
					strncat_s(strtime, sizeof(strtime), tmp, _TRUNCATE);
					i++;
					break;
				case '%':
					strncat_s(strtime, sizeof(strtime), "%", _TRUNCATE);
					i++;
					break;
				default:
					_snprintf_s(tmp, sizeof(tmp), _TRUNCATE, "%c", format[i]);
					strncat_s(strtime, sizeof(strtime), tmp, _TRUNCATE);
					break;
			}
		}
		else {
			_snprintf_s(tmp, sizeof(tmp), _TRUNCATE, "%c", format[i]);
			strncat_s(strtime, sizeof(strtime), tmp, _TRUNCATE);
		}
	}

	return strtime;
}

PCHAR PASCAL GetParam(PCHAR buff, int size, PCHAR param)
{
	int i = 0;
	BOOL quoted = FALSE;

	while (*param == ' ' || *param == '\t') {
		param++;
	}

	if (*param == '\0' || *param == ';') {
		return NULL;
	}

	while (*param != '\0' && (quoted || (*param != ';' && *param != ' ' && *param != '\t'))) {
		if (*param == '"') {
			if (*(param+1) != '"') {
				quoted = !quoted;
			}
			else {
				if (i < size - 1) {
					buff[i++] = *param;
				}
				param++;
			}
		}
		if (i < size - 1) {
			buff[i++] = *param;
		}
		param++;
	}
	if (!quoted && (buff[i-1] == ';')) {
		i--;
	}

	buff[i] = '\0';
	return (param);
}

void PASCAL DequoteParam(PCHAR dest, int dest_len, PCHAR src)
{
	BOOL quoted = FALSE;
	PCHAR dest_end = dest + dest_len - 1;

	if (src == dest) {
		while (*src != '\0' && *src != '"' && dest < dest_end) {
			src++; dest++;
		}
	}

	while (*src != '\0' && dest < dest_end) {
		if (*src != '"' || (*++src == '"' && quoted)) {
			*dest++ = *src++;
		}
		else {
			quoted = !quoted;
		}
	}

	*dest = '\0';
}

void PASCAL DeleteComment(PCHAR dest, int dest_size, PCHAR src)
{
	BOOL quoted = FALSE;
	PCHAR dest_end = dest + dest_size - 1;

	while (*src != '\0' && dest < dest_end && (quoted || *src != ';')) {
		*dest++ = *src;

		if (*src++ == '"') {
			if (*src == '"' && dest < dest_end) {
				*dest++ = *src++;
			}
			else {
				quoted = !quoted;
			}
		}
	}

	*dest = '\0';

	return;
}

void split_buffer(char *buffer, int delimiter, char **head, char **body)
{
	char *p1, *p2;

	*head = *body = NULL;

	if (!isalnum(*buffer) || (p1 = strchr(buffer, delimiter)) == NULL) {
		return;
	}

	*head = buffer;

	p2 = buffer;
	while (p2 < p1 && !isspace(*p2)) {
		p2++;
	}

	*p2 = '\0';

	p1++;
	while (*p1 && isspace(*p1)) {
		p1++;
	}

	*body = p1;
}
