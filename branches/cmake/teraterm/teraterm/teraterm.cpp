/*
 * Copyright (C) 1994-1998 T. Teranishi
 * (C) 2006-2017 TeraTerm Project
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

/* TERATERM.EXE, main */

//#include "stdafx.h"
#include "teraterm.h"
#include "tttypes.h"
#include "commlib.h"
#include "ttwinman.h"
#include "buffer.h"
#include "vtterm.h"
#include "vtwin.h"
#include "clipboar.h"
#include "ttftypes.h"
#include "filesys.h"
#include "telnet.h"
#include "tektypes.h"
#include "tekwin.h"
#include "ttdde.h"
#include "keyboard.h"
#include "compat_win.h"

#include "compat_w95.h"

static void init()
{
	typedef BOOL (WINAPI *pSetDllDir)(LPCSTR);
	typedef BOOL (WINAPI *pSetDefDllDir)(DWORD);

	HMODULE module;
	pSetDllDir setDllDir;
	pSetDefDllDir setDefDllDir;

	if ((module = GetModuleHandle("kernel32.dll")) != NULL) {
		if ((setDefDllDir = (pSetDefDllDir)GetProcAddress(module, "SetDefaultDllDirectories")) != NULL) {
			// SetDefaultDllDirectories() ���g����ꍇ�́A�����p�X�� %WINDOWS%\system32 �݂̂ɐݒ肷��
			(*setDefDllDir)((DWORD)0x00000800); // LOAD_LIBRARY_SEARCH_SYSTEM32
		}
		else if ((setDllDir = (pSetDllDir)GetProcAddress(module, "SetDllDirectoryA")) != NULL) {
			// SetDefaultDllDirectories() ���g���Ȃ��Ă��ASetDllDirectory() ���g����ꍇ��
			// �J�����g�f�B���N�g�������ł������p�X����͂����Ă����B
			(*setDllDir)("");
		}
	}

	WinCompatInit();
	if (PSetThreadDpiAwarenessContext) {
		PSetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	}
}

// Tera Term main engine
static BOOL OnIdle(LONG lCount)
{
	static int Busy = 2;
	int Change, nx, ny;
	BOOL Size;

	if (lCount==0) Busy = 2;

	if (cv.Ready)
	{
		/* Sender */
		CommSend(&cv);

		/* Parser */
		if ((cv.HLogBuf!=NULL) && (cv.LogBuf==NULL))
			cv.LogBuf = (PCHAR)GlobalLock(cv.HLogBuf);

		if ((cv.HBinBuf!=NULL) && (cv.BinBuf==NULL))
			cv.BinBuf = (PCHAR)GlobalLock(cv.HBinBuf);

		if ((TelStatus==TelIdle) && cv.TelMode)
			TelStatus = TelIAC;

		if (TelStatus != TelIdle)
		{
			ParseTel(&Size,&nx,&ny);
			if (Size) {
				LockBuffer();
				ChangeTerminalSize(nx,ny);
				UnlockBuffer();
			}
		}
		else {
			if (cv.ProtoFlag) Change = ProtoDlgParse();
			else {
				switch (ActiveWin) {
				case IdVT:
					Change =  ((CVTWindow*)pVTWin)->Parse();
					// TEK window�̃A�N�e�B�u���� pause ���g���ƁACPU�g�p��100%�ƂȂ�
					// ���ۂւ̎b��Ώ��B(2006.2.6 yutaka)
					// �҂����Ԃ��Ȃ����A�R���e�L�X�g�X�C�b�`�����ɂ���B(2006.3.20 yutaka)
					Sleep(0);
					break;

				case IdTEK:
					if (pTEKWin != NULL) {
						Change = ((CTEKWindow*)pTEKWin)->Parse();
						// TEK window�̃A�N�e�B�u���� pause ���g���ƁACPU�g�p��100%�ƂȂ�
						// ���ۂւ̎b��Ώ��B(2006.2.6 yutaka)
						Sleep(1);
					}
					else {
						Change = IdVT;
					}
					break;

				default:
					Change = 0;
				}

				switch (Change) {
					case IdVT:
						VTActivate();
						break;
					case IdTEK:
						((CVTWindow*)pVTWin)->OpenTEK();
						break;
				}
			}
		}

		if (cv.LogBuf!=NULL)
		{
			if (FileLog) {
				LogToFile();
			}
			if (DDELog && AdvFlag) {
				DDEAdv();
			}
			GlobalUnlock(cv.HLogBuf);
			cv.LogBuf = NULL;
		}

		if (cv.BinBuf!=NULL)
		{
			if (BinLog) {
				LogToFile();
			}
			GlobalUnlock(cv.HBinBuf);
			cv.BinBuf = NULL;
		}

		/* Talker */
		switch (TalkStatus) {
		case IdTalkCB:
			CBSend();
			break; /* clip board */
		case IdTalkFile:
			FileSend();
			break; /* file */
		}

		/* Receiver */
		if (DDELog && cv.DCount >0) {
			// ���O�o�b�t�@���܂�DDE�N���C�A���g�֑����Ă��Ȃ��ꍇ�́A
			// TCP�p�P�b�g�̎�M���s��Ȃ��B
			// �A�����Ď�M���s���ƁA���O�o�b�t�@�����E���h���r���ɂ�薢���M�̃f�[�^��
			// �㏑�����Ă��܂��\��������B(2007.6.14 yutaka)

		} else {
			CommReceive(&cv);
		}

	}

	if (cv.Ready &&
	    (cv.RRQ || (cv.OutBuffCount>0) || (cv.InBuffCount>0) || (cv.FlushLen>0) || (cv.LCount>0) || (cv.BCount>0) || (cv.DCount>0)) ) {
		Busy = 2;
	}
	else {
		Busy--;
	}

	return (Busy>0);
}

BOOL CallOnIdle(LONG lCount)
{
	return OnIdle(lCount);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst,
                   LPSTR lpszCmdLine, int nCmdShow)
{
	init();
	hInst = hInstance;
	CVTWindow *m_pMainWnd = new CVTWindow();
	pVTWin = m_pMainWnd->m_hWnd;
	ShowWindow(m_pMainWnd->m_hWnd, nCmdShow);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		if (MetaKey(ts.MetaKey)) {
			continue;
		}
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (msg.wParam);
}

#if 0

#define COMPILE_NEWAPIS_STUBS 
//#define WANT_GETDISKFREESPACEEX_WRAPPER // wrap for GetDiskFreeSpaceEx
//#define WANT_GETLONGPATHNAME_WRAPPER // wrap for GetLongPathName
//#define WANT_GETFILEATTRIBUTESEX_WRAPPER // wrap for GetFileAttributesEx
#define WANT_ISDEBUGGERPRESENT_WRAPPER // wrap for wrap for IsDebuggerPresent
#include <NewAPIs.h>

// https://bearwindows.zcm.com.au/msvc.htm

#endif