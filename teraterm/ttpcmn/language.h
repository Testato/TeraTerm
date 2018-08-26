/*
 * Copyright (C) 1994-1998 T. Teranishi
 * (C) 2010-2017 TeraTerm Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* TTCMN.DLL, character code conversion */

#pragma once

#include <windows.h>	// for WINAPI
#include "codemap.h"

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(DllExport)
#define DllExport __declspec(dllimport)
#endif

/* proto types */
DllExport unsigned int WINAPI SJIS2UTF8(WORD KCode, int *byte, char *locale);
DllExport WORD WINAPI SJIS2JIS(WORD KCode);
DllExport WORD WINAPI SJIS2EUC(WORD KCode);
DllExport WORD WINAPI JIS2SJIS(WORD KCode);
DllExport BYTE WINAPI RussConv(int cin, int cout, BYTE b);
DllExport void WINAPI RussConvStr(int cin, int cout, PCHAR Str, int count);
DllExport unsigned short WINAPI ConvertUnicode(unsigned short code, const codemap_t *table, int tmax);

#ifdef __cplusplus
}
#endif
