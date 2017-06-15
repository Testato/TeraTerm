/*
 * Copyright (C) 2008-2017 TeraTerm Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. The name of the author may not be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */

/* TERATERM.EXE, Serial/File interface */
#ifdef __cplusplus
extern "C" {
#endif

typedef BOOL (PASCAL *TReadFile)
  (HANDLE FHandle, LPVOID Buff, DWORD ReadSize, LPDWORD ReadBytes,
   LPOVERLAPPED ReadOverLap);
typedef BOOL (PASCAL *TWriteFile)
  (HANDLE FHandle, LPCVOID Buff, DWORD WriteSize, LPDWORD WriteBytes,
   LPOVERLAPPED WriteOverLap);
typedef HANDLE (PASCAL *TCreateFile)
  (LPCTSTR FName, DWORD AcMode, DWORD ShMode, LPSECURITY_ATTRIBUTES SecAttr,
   DWORD CreateDisposition, DWORD FileAttr, HANDLE Template);
typedef BOOL (PASCAL *TCloseFile)
  (HANDLE FHandle);

extern TReadFile PReadFile;
extern TWriteFile PWriteFile;
extern TCreateFile PCreateFile;
extern TCloseFile PCloseFile;

/* proto types */
void InitFileIO(int ConnType);

#ifdef __cplusplus
}
#endif
