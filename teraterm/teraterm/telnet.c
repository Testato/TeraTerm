/*
 * Copyright (C) 1994-1998 T. Teranishi
 * (C) 2007-2017 TeraTerm Project
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

/* TERATERM.EXE, TELNET routines */

#include "teraterm.h"
#include "tttypes.h"
#include <stdio.h>
#include <string.h>
#include "ttcommon.h"
#include "ttwinman.h"
#include "commlib.h"
#include <time.h>
#include <process.h>

#include "telnet.h"
#include "tt_res.h"

int TelStatus;

enum OptStatus {No, Yes, WantNo, WantYes};
enum OptQue {Empty, Opposite};

typedef struct {
	BOOL Accept;
	enum OptStatus Status;
	enum OptQue Que;
} TelOpt;
typedef TelOpt *PTelOpt;

typedef struct {
	TelOpt MyOpt[MaxTelOpt+1];
	TelOpt HisOpt[MaxTelOpt+1];
	BYTE SubOptBuff[51];
	int SubOptCount;
	BOOL SubOptIAC;
	BOOL ChangeWinSize;
	POINT WinSize;
	int LogFile;
} TelRec;
typedef TelRec *PTelRec;

static TelRec tr;

static HANDLE keepalive_thread = (HANDLE)-1L;
static HWND keepalive_dialog = NULL;
int nop_interval = 0;

void DefaultTelRec()
{
	int i;

	for (i=0 ; i <= MaxTelOpt ; i++) {
		tr.MyOpt[i].Accept = FALSE;
		tr.MyOpt[i].Status = No;
		tr.MyOpt[i].Que = Empty;
		tr.HisOpt[i].Accept = FALSE;
		tr.HisOpt[i].Status = No;
		tr.HisOpt[i].Que = Empty;
	}

	tr.SubOptCount = 0;
	tr.SubOptIAC = FALSE;
	tr.ChangeWinSize = FALSE;
}

void InitTelnet()
{
	TelStatus = TelIdle;

	DefaultTelRec();
	tr.MyOpt[BINARY].Accept = TRUE;
	tr.HisOpt[BINARY].Accept = TRUE;
	tr.MyOpt[SGA].Accept = TRUE;
	tr.HisOpt[SGA].Accept = TRUE;
	tr.HisOpt[ECHO].Accept = TRUE;
	tr.MyOpt[TERMTYPE].Accept = TRUE;
	tr.MyOpt[TERMSPEED].Accept = TRUE;
	tr.MyOpt[NAWS].Accept = TRUE;
	tr.HisOpt[NAWS].Accept = TRUE;
	tr.WinSize.x = ts.TerminalWidth;
	tr.WinSize.y = ts.TerminalHeight;

	if ((ts.LogFlag & LOG_TEL) != 0)
		tr.LogFile = _lcreat("TELNET.LOG", 0);
	else
		tr.LogFile = 0;
}

void EndTelnet()
{
	if (tr.LogFile) {
		tr.LogFile = 0;
		_lclose(tr.LogFile);
	}

	TelStopKeepAliveThread();
}

void TelWriteLog1(BYTE b)
{
	BYTE Temp[3];
	BYTE Ch;

	Temp[0] = 0x20;
	Ch = b / 16;
	if (Ch <= 9)
		Ch = Ch + 0x30;
	else
		Ch = Ch + 0x37;
	Temp[1] = Ch;

	Ch = b & 15;
	if (Ch <= 9)
		Ch = Ch + 0x30;
	else
		Ch = Ch + 0x37;
	Temp[2] = Ch;
	_lwrite(tr.LogFile, Temp, 3);
}

void TelWriteLog(PCHAR Buf, int C)
{
	int i;

	_lwrite(tr.LogFile, "\015\012>", 3);
	for (i = 0 ; i<= C-1 ; i++)
		TelWriteLog1(Buf[i]);
}

void SendBack(BYTE a, BYTE b)
{
	BYTE Str3[3];

	Str3[0] = IAC;
	Str3[1] = a;
	Str3[2] = b;
	CommRawOut(&cv, Str3, 3);
	if (tr.LogFile)
		TelWriteLog(Str3, 3);
}

void SendWinSize()
{
	int i;
	BYTE TmpBuff[21];

	i = 0;

	TmpBuff[i++] = IAC;
	TmpBuff[i++] = SB;
	TmpBuff[i++] = NAWS;

	if (HIBYTE(tr.WinSize.x) == IAC) {
		TmpBuff[i++] = IAC;
	}
	TmpBuff[i++] = HIBYTE(tr.WinSize.x);

	if (LOBYTE(tr.WinSize.x) == IAC) {
		TmpBuff[i++] = IAC;
	}
	TmpBuff[i++] = LOBYTE(tr.WinSize.x);

	if (HIBYTE(tr.WinSize.y) == IAC) {
		TmpBuff[i++] = IAC;
	}
	TmpBuff[i++] = HIBYTE(tr.WinSize.y);

	if (LOBYTE(tr.WinSize.y) == IAC) {
		TmpBuff[i++] = IAC;
	}
	TmpBuff[i++] = LOBYTE(tr.WinSize.y);

	TmpBuff[i++] = IAC;
	TmpBuff[i++] = SE;

	CommRawOut(&cv, TmpBuff, i);
	if (tr.LogFile)
		TelWriteLog(TmpBuff, i);
}

void ParseTelIAC(BYTE b)
{
	switch (b) {
	case SE: break;
	case NOP:
	case DM:
	case BREAK:
	case IP:
	case AO:
	case AYT:
	case EC:
	case EL:
	case GOAHEAD:
		TelStatus = TelIdle;
		break;
	case SB:
		TelStatus = TelSB;
		tr.SubOptCount = 0;
		break;
	case WILLTEL:
		TelStatus = TelWill;
		break;
	case WONTTEL:
		TelStatus = TelWont;
		break;
	case DOTEL:
		TelStatus = TelDo;
		break;
	case DONTTEL:
		TelStatus = TelDont;
		break;
	case IAC:
		TelStatus = TelIdle;
		break;
	default:
		TelStatus = TelIdle;
	}
}

void ParseTelSB(BYTE b)
{
	BYTE TmpStr[51];
	int i;

	if (tr.SubOptIAC) {
		tr.SubOptIAC = FALSE;
		switch (b) {
		case SE:
			if (tr.SubOptCount <= 1) {
				// �p�����[�^�Ȃ��� Sub Option �͖����Ǝv����̂ł����ł͂���
				tr.SubOptCount = 0;
				TelStatus = TelIdle;
				return ;
			}

			switch (tr.SubOptBuff[0]) {
			case TERMTYPE:
				if ((tr.MyOpt[TERMTYPE].Status == Yes) && (tr.SubOptBuff[1] == 1)) {
					_snprintf_s(TmpStr, sizeof(TmpStr), _TRUNCATE, "%c%c%c%c%s%c%c",
						IAC, SB, TERMTYPE, 0, ts.TermType, IAC, SE);
					// 4 �o�C�g�ڂ� 0 ������̂ŁA���炵�Ē������Ƃ�
					i = strlen(TmpStr + 4) + 4;
					CommRawOut(&cv, TmpStr, i);

					if (tr.LogFile)
						TelWriteLog(TmpStr, i);
				}
				break;

			case NAWS:
				if ( /* (tr.HisOpt[NAWS].Status == Yes) && */ (tr.SubOptCount >= 5)) {
					tr.WinSize.x = tr.SubOptBuff[1]*256 + tr.SubOptBuff[2];
					tr.WinSize.y = tr.SubOptBuff[3]*256 + tr.SubOptBuff[4];
					tr.ChangeWinSize = TRUE;
				}
				break;

			case TERMSPEED:
				if ((tr.MyOpt[TERMSPEED].Status == Yes) && (tr.SubOptBuff[1] == 1)) {
					_snprintf_s(TmpStr, sizeof(TmpStr), _TRUNCATE,
						"%c%c%c%c%d,%d%c%c", IAC, SB, TERMSPEED, 0,
						ts.TerminalInputSpeed, ts.TerminalOutputSpeed, IAC, SE);
					// 4 �o�C�g�ڂ� 0 ������̂ŁA���炵�Ē������Ƃ�
					i = strlen(TmpStr + 4) + 4;
					CommRawOut(&cv, TmpStr, i);

					if (tr.LogFile)
						TelWriteLog(TmpStr, i);
				}
				break;
			}

			tr.SubOptCount = 0;
			TelStatus = TelIdle;
			return ;

		/* case IAC: braek; */
		default:
			if (tr.SubOptCount >= sizeof(tr.SubOptBuff)-1) {
				tr.SubOptCount = 0;
				TelStatus = TelIdle;
				return;
			}
			else {
				tr.SubOptBuff[tr.SubOptCount] = IAC;
				tr.SubOptCount++;
				if (b==IAC) {
					tr.SubOptIAC = TRUE;
					return;
				}
			}
		}
	}
	else if (b==IAC) {
		tr.SubOptIAC = TRUE;
		return;
	}

	if (tr.SubOptCount >= sizeof(tr.SubOptBuff)-1) {
		tr.SubOptCount = 0;
		tr.SubOptIAC = FALSE;
		TelStatus = TelIdle;
	}
	else {
		tr.SubOptBuff[tr.SubOptCount] = b;
		tr.SubOptCount++;
	}
}

void ParseTelWill(BYTE b)
{
	if (b <= MaxTelOpt) {
		switch (tr.HisOpt[b].Status) {
		case No:
			if (tr.HisOpt[b].Accept) {
				SendBack(DOTEL, b);
				tr.HisOpt[b].Status = Yes;
			}
			else {
				SendBack(DONTTEL, b);
			}
			break;

		case WantNo:
			switch (tr.HisOpt[b].Que) {
			case Empty:
				tr.HisOpt[b].Status = No;
				break;
			case Opposite:
				tr.HisOpt[b].Status = Yes;
				break;
			}
			break;

		case WantYes:
			switch (tr.HisOpt[b].Que) {
			case Empty:
				tr.HisOpt[b].Status = Yes;
				break;
			case Opposite:
				tr.HisOpt[b].Status = WantNo;
				tr.HisOpt[b].Que = Empty;
				SendBack(DONTTEL, b);
				break;
			}
			break;
		}
	}
	else {
		SendBack(DONTTEL, b);
	}

	switch (b) {
	case ECHO:
		if (ts.TelEcho) {
			switch (tr.HisOpt[ECHO].Status) {
			case Yes:
				ts.LocalEcho = 0;
				break;
			case No:
				ts.LocalEcho = 1;
				break;
			}
		}
		if (tr.HisOpt[ECHO].Status == Yes) {
			cv.TelLineMode = FALSE;
		}
		break;

	case SGA:
		if (tr.HisOpt[SGA].Status == Yes) {
			cv.TelLineMode = FALSE;
		}
		break;

	case BINARY:
		switch (tr.HisOpt[BINARY].Status) {
		case Yes:
			cv.TelBinRecv = TRUE;
			break;
		case No:
			cv.TelBinRecv = FALSE;
			break;
		}
		break;
	}
	TelStatus = TelIdle;
}

void ParseTelWont(BYTE b)
{
	if (b <= MaxTelOpt) {
		switch (tr.HisOpt[b].Status) {
		case Yes:
			tr.HisOpt[b].Status = No;
			SendBack(DONTTEL, b);
			break;

		case WantNo:
			switch (tr.HisOpt[b].Que) {
			case Empty:
				tr.HisOpt[b].Status = No;
				break;
			case Opposite:
				tr.HisOpt[b].Status = WantYes;
				tr.HisOpt[b].Que = Empty;
				SendBack(DOTEL, b);
				break;
			}
			break;

		case WantYes:
			switch (tr.HisOpt[b].Que) {
			case Empty:
				tr.HisOpt[b].Status = No;
				break;
			case Opposite:
				tr.HisOpt[b].Status = No;
				tr.HisOpt[b].Que = Empty;
				break;
			}
			break;
		}
	}
	else {
		SendBack(DONTTEL, b);
	}

	switch (b) {
	case ECHO:
		if (ts.TelEcho) {
			switch (tr.HisOpt[ECHO].Status) {
			case Yes:
				ts.LocalEcho = 0;
				break;
			case No:
				ts.LocalEcho = 1;
				break;
			}
		}
		if (tr.HisOpt[ECHO].Status == Yes) {
			cv.TelLineMode = FALSE;
		}
		break;

	case BINARY:
		switch (tr.HisOpt[BINARY].Status) {
		case Yes:
			cv.TelBinRecv = TRUE;
			break;
		case No:
			cv.TelBinRecv = FALSE;
			break;
		}
		break;
	}
	TelStatus = TelIdle;
}

void ParseTelDo(BYTE b)
{
	if (b <= MaxTelOpt) {
		switch (tr.MyOpt[b].Status) {
		case No:
			if (tr.MyOpt[b].Accept) {
				tr.MyOpt[b].Status = Yes;
				SendBack(WILLTEL, b);
			}
			else {
				SendBack(WONTTEL, b);
			}
			break;

		case WantNo:
			switch (tr.MyOpt[b].Que) {
			case Empty:
				tr.MyOpt[b].Status = No;
				break;
			case Opposite:
				tr.MyOpt[b].Status = Yes;
				break;
			}
			break;

		case WantYes:
			switch (tr.MyOpt[b].Que) {
			case Empty:
				tr.MyOpt[b].Status = Yes;
				break;
			case Opposite:
				tr.MyOpt[b].Status = WantNo;
				tr.MyOpt[b].Que = Empty;
				SendBack(WONTTEL, b);
				break;
			}
			break;
		}
	}
	else {
		SendBack(WONTTEL, b);
	}

	switch (b) {
	case BINARY:
		switch (tr.MyOpt[BINARY].Status) {
		case Yes:
			cv.TelBinSend = TRUE;
			break;
		case No:
			cv.TelBinSend = FALSE;
			break;
		}
		break;

	case NAWS:
		if (tr.MyOpt[NAWS].Status==Yes)
			SendWinSize();
		break;

	case SGA:
		if (tr.MyOpt[SGA].Status==Yes)
			cv.TelLineMode = FALSE;
		break;
	}
	TelStatus = TelIdle;
}

void ParseTelDont(BYTE b)
{
	if (b <= MaxTelOpt) {
		switch (tr.MyOpt[b].Status) {
		case Yes:
			tr.MyOpt[b].Status = No;
			SendBack(WONTTEL, b);
			break;

		case WantNo:
			switch (tr.MyOpt[b].Que) {
			case Empty:
				tr.MyOpt[b].Status = No;
				break;
			case Opposite:
				tr.MyOpt[b].Status = WantYes;
				tr.MyOpt[b].Que = Empty;
				SendBack(WILLTEL, b);
				break;
			}
			break;

		case WantYes:
			switch (tr.MyOpt[b].Que) {
			case Empty:
				tr.MyOpt[b].Status = No;
				break;
			case Opposite:
				tr.MyOpt[b].Status = No;
				tr.MyOpt[b].Que = Empty;
				break;
			}
			break;
		}
	}
	else {
		SendBack(WONTTEL, b);
	}

	switch (b) {
	case BINARY:
		switch (tr.MyOpt[BINARY].Status) {
		case Yes:
			cv.TelBinSend = TRUE;
			break;
		case No:
			cv.TelBinSend = FALSE;
			break;
		}
		break;
	}
	TelStatus = TelIdle;
}

void ParseTel(BOOL *Size, int *nx, int *ny)
{
	BYTE b;
	int c;

	c = CommReadRawByte(&cv, &b);

	while ((c>0) && (cv.TelMode)) {
		if (tr.LogFile) {
			if (TelStatus==TelIAC) {
				_lwrite(tr.LogFile, "\015\012<", 3);
				TelWriteLog1(0xff);
			}
			TelWriteLog1(b);
		}

		tr.ChangeWinSize = FALSE;

		switch (TelStatus) {
			case TelIAC: ParseTelIAC(b); break;
			case TelSB: ParseTelSB(b); break;
			case TelWill: ParseTelWill(b); break;
			case TelWont: ParseTelWont(b); break;
			case TelDo: ParseTelDo(b); break;
			case TelDont: ParseTelDont(b); break;
			case TelNop: TelStatus = TelIdle; break;
		}
		if (TelStatus == TelIdle) cv.TelMode = FALSE;

		if (cv.TelMode) c = CommReadRawByte(&cv, &b);
	}

	*Size = tr.ChangeWinSize;
	*nx = tr.WinSize.x;
	*ny = tr.WinSize.x;
}

void TelEnableHisOpt(BYTE b)
{
	if (b <= MaxTelOpt) {
		switch (tr.HisOpt[b].Status) {
		case No:
			tr.HisOpt[b].Status = WantYes;
			SendBack(DOTEL, b);
			break;

		case WantNo:
			if (tr.HisOpt[b].Que==Empty)
				tr.HisOpt[b].Que = Opposite;
			break;

		case WantYes:
			if (tr.HisOpt[b].Que==Opposite)
				tr.HisOpt[b].Que = Empty;
			break;
		}
	}
}

void TelDisableHisOpt(BYTE b)
{
	if (b <= MaxTelOpt) {
		switch (tr.HisOpt[b].Status) {
		case Yes:
			tr.HisOpt[b].Status = WantNo;
			SendBack(DONTTEL, b);
			break;

		case WantNo:
			if (tr.HisOpt[b].Que==Opposite)
				tr.HisOpt[b].Que = Empty;
			break;

		case WantYes:
			if (tr.HisOpt[b].Que==Empty)
				tr.HisOpt[b].Que = Opposite;
			break;
		}
	}
}

void TelEnableMyOpt(BYTE b)
{
	if (b <= MaxTelOpt) {
		switch (tr.MyOpt[b].Status) {
		case No:
			tr.MyOpt[b].Status = WantYes;
			SendBack(WILLTEL, b);
			break;

		case WantNo:
			if (tr.MyOpt[b].Que==Empty)
				tr.MyOpt[b].Que = Opposite;
			break;

		case WantYes:
			if (tr.MyOpt[b].Que==Opposite)
				tr.MyOpt[b].Que = Empty;
			break;
		}
	}
}

void TelDisableMyOpt(BYTE b)
{
	if (b <= MaxTelOpt) {
		switch (tr.MyOpt[b].Status) {
		case Yes:
			tr.MyOpt[b].Status = WantNo;
			SendBack(WONTTEL, b);
			break;

		case WantNo:
			if (tr.MyOpt[b].Que==Opposite)
				tr.MyOpt[b].Que = Empty;
			break;

		case WantYes:
			if (tr.MyOpt[b].Que==Empty)
				tr.MyOpt[b].Que = Opposite;
			break;
		}
	}
}

void TelInformWinSize(int nx, int ny)
{
	if ((tr.MyOpt[NAWS].Status==Yes) &&
	    (nx != tr.WinSize.x || ny != tr.WinSize.y))
	{
		tr.WinSize.x = nx;
		tr.WinSize.y = ny;
		SendWinSize();
	}
}

void TelSendAYT()
{
	BYTE Str[2];

	Str[0] = IAC;
	Str[1] = AYT;
	CommRawOut(&cv, Str, 2);
	CommSend(&cv);
	if (tr.LogFile)
		TelWriteLog(Str, 2);
}

void TelSendBreak()
{
	BYTE Str[2];

	Str[0] = IAC;
	Str[1] = BREAK;
	CommRawOut(&cv, Str, 2);
	CommSend(&cv);
	if (tr.LogFile)
		TelWriteLog(Str, 2);
}

void TelChangeEcho()
{
	if (ts.LocalEcho==0)
		TelEnableHisOpt(ECHO);
	else
		TelDisableHisOpt(ECHO);
}

void TelSendNOP()
{
	BYTE Str[2];

	Str[0] = IAC;
	Str[1] = NOP;
	CommRawOut(&cv, Str, 2);
	CommSend(&cv);
	if (tr.LogFile)
		TelWriteLog(Str, 2);
}

#define WM_SEND_HEARTBEAT (WM_USER + 1)

static LRESULT CALLBACK telnet_heartbeat_dlg_proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_INITDIALOG:
		return FALSE;

	case WM_SEND_HEARTBEAT:
		TelSendNOP();
		return TRUE;
		break;

	case WM_COMMAND:
		switch (LOWORD(wp)) {
		case IDOK:
			return TRUE;
		case IDCANCEL:
			EndDialog(hWnd, 0);
			return TRUE;
		default:
			return FALSE;
		}
		break;

	case WM_CLOSE:
		// close�{�^������������Ă� window �����Ȃ��悤�ɂ���B
		return TRUE;

	case WM_DESTROY:
		return TRUE;

	default:
		return FALSE;
	}
	return TRUE;
}


static unsigned _stdcall TelKeepAliveThread(void *dummy) {
	static int instance = 0;

	if (instance > 0)
		return 0;
	instance++;

	while (cv.Open && nop_interval > 0) {
		if (time(NULL) >= cv.LastSendTime + nop_interval) {
			SendMessage(keepalive_dialog, WM_SEND_HEARTBEAT, 0, 0);
		}

		Sleep(100);
	}
	instance--;
	return 0;
}

void TelStartKeepAliveThread() {
	unsigned tid;

	if (ts.TelKeepAliveInterval > 0) {
		nop_interval = ts.TelKeepAliveInterval;

	// ���[�h���X�_�C�A���O��ǉ� (2007.12.26 yutaka)
	keepalive_dialog = CreateDialog(hInst, MAKEINTRESOURCE(IDD_BROADCAST_DIALOG),
	                                HVTWin, (DLGPROC)telnet_heartbeat_dlg_proc);

		keepalive_thread = (HANDLE)_beginthreadex(NULL, 0, TelKeepAliveThread, NULL, 0, &tid);
		if (keepalive_thread == (HANDLE)-1) {
			nop_interval = 0;
		}
	}
}

void TelStopKeepAliveThread() {
	if (keepalive_thread != (HANDLE)-1L) {
		nop_interval = 0;
		WaitForSingleObject(keepalive_thread, INFINITE);
		CloseHandle(keepalive_thread);
		keepalive_thread = (HANDLE)-1L;

		DestroyWindow(keepalive_dialog);
	}
}

void TelUpdateKeepAliveInterval() {
	if (cv.Open && cv.TelFlag && ts.TCPPort==ts.TelPort) {
		if (ts.TelKeepAliveInterval > 0 && keepalive_thread == (HANDLE)-1)
			TelStartKeepAliveThread();
		else if (ts.TelKeepAliveInterval == 0 && keepalive_thread != (HANDLE)-1)
			TelStopKeepAliveThread();
		else
			nop_interval = ts.TelKeepAliveInterval;
	}
}
