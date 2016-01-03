/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */
/* IPv6 modification is Copyright(C) 2000, 2001 Jun-ya kato <kato@win6.jp> */

#pragma once

#ifndef NO_INET6
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_ /* Prevent inclusion of winsock.h in windows.h */
#endif /* _WINSOCKAPI_ */
#endif /* NO_INET6 */
#include <windows.h>

/* _MSC_VER �̒l
Visual C++ 8.0 (Visual Studio 2005)  1400
Visual C++ 9.0 (Visual Studio 2008)  1500
Visual C++ 10.0 (Visual Studio 2010) 1600
Visual C++ 11.0 (Visual Studio 2012) 1700
Visual C++ 12.0 (Visual Studio 2013) 1800
Visual C++ 14.0 (Visual Studio 2015) 1900
*/

/* VS2015(VC14.0)���ƁAWSASocketA(), inet_ntoa() �Ȃǂ�API��deprecated�ł����
 * �x�����邽�߂ɁA�x����}�~����B��֊֐��ɒu������ƁAVS2005(VC8.0)�Ńr���h
 * �ł��Ȃ��Ȃ邽�߁A�x����}�~���邾���Ƃ���B
 */
#if _MSC_VER >= 1800  // VSC2013(VC12.0) or later
	#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
		#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#endif
#endif

// AKASI���ɂ��Eterm�����߃E�B���h�E
#define ALPHABLEND_TYPE2

// Eterm look-feel
#define BG_SECTION "BG"
#define BG_DESTFILE "BGDestFile"
#define BG_THEME_IMAGEFILE "theme\\ImageFile.INI"
#define BG_THEME_IMAGEFILE_DEFAULT "theme\\*.INI"
#define BG_THEME_IMAGE_BRIGHTNESS_DEFAULT 64
#define BG_THEME_IMAGE_BRIGHTNESS1 "BGSrc1Alpha"
#define BG_THEME_IMAGE_BRIGHTNESS2 "BGSrc2Alpha"

// Added by 337 2006/03/01
#define USE_NORMAL_BGCOLOR

#include "i18n.h"

#define MAXPATHLEN 256
/* version 2.3 */
#define TTVERSION (WORD)23

#define DEBUG_PRINT(val) { \
	FILE *fp; \
	fp = fopen("debugmsg.txt", "a+"); \
	if (fp != NULL) { \
		fprintf(fp, "%s = %d\n", #val, val); \
		fclose(fp); \
	} \
}

typedef struct cygterm {
	BOOL update_flag;
	char term[128];
	char term_type[80];
	char port_start[80];
	char port_range[80];
	char shell[80];
	char env1[128];
	char env2[128];
	BOOL login_shell;
	BOOL home_chdir;
	BOOL agent_proxy;
} cygterm_t;
