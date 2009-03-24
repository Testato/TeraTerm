/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */
/* IPv6 modification is Copyright(C) 2000, 2001 Jun-ya kato <kato@win6.jp> */

#ifdef INET6
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_ /* Prevent inclusion of winsock.h in windows.h */
#endif /* _WINSOCKAPI_ */
#endif /* INET6 */
#include <windows.h>

// AKASI氏によるEterm風透過ウィンドウ
#define ALPHABLEND_TYPE2

// Added by 337 2006/03/01
#define USE_NORMAL_BGCOLOR

#ifdef I18N
#include "i18n.h"
#endif

#ifdef __WATCOMC__
  #if defined(__WINDOWS__)
    #define TERATERM16
  #elif defined(__NT__)
    #define TERATERM32
  #endif
#endif

#if ! defined(TERATERM16) && ! defined(TERATERM32)
//  #define TERATERM16
  #define TERATERM32
#endif

#ifdef TERATERM32
  #define MAXPATHLEN 256
  /* version 2.3 */
  #define TTVERSION (WORD)23
#else
  #define MAXPATHLEN 144
  typedef LPSTR PCHAR;
  typedef LPSTR LPTSTR;
  typedef LPCSTR LPCTSTR;
  typedef BOOL far *PBOOL;
  /* version 1.4 */
  #define TTVERSION (WORD)14
#endif

#define DEBUG_PRINT(val) { \
	FILE *fp; \
	fp = fopen("debugmsg.txt", "a+"); \
	if (fp != NULL) { \
		fprintf(fp, "%s = %d\n", #val, val); \
		fclose(fp); \
	} \
}
