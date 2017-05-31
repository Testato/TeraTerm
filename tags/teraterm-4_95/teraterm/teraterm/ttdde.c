/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TERATERM.EXE, DDE routines */
#include "teraterm.h"
#include "tttypes.h"
#include <stdio.h>
#include <string.h>
#include <ddeml.h>
#include "ttwinman.h"
#include "ttftypes.h"
#include "filesys.h"
#include "clipboar.h"
#include "ttsetup.h"
#include "telnet.h"
#include "ttlib.h"
#include "keyboard.h"

#include "ttdde.h"
#include "commlib.h"

#include "vtwin.h"

#define ServiceName "TERATERM"
#define ItemName "DATA"
#define ItemName2 "PARAM"

#define MaxStrLen (LONG)512

char TopicName[21] = "";
HCONV ConvH = 0;
BOOL AdvFlag = FALSE;
BOOL CloseTT = FALSE;

static BOOL DdeCmnd = FALSE;

static DWORD Inst = 0;
static HSZ Service = 0;
static HSZ Topic = 0;
static HSZ Item = 0;
static HSZ Item2 = 0;
static HWND HWndDdeCli = NULL;

static StartupFlag = FALSE;

// for sync mode
static BOOL SyncMode = FALSE;
static BOOL SyncRecv;
static LONG SyncFreeSpace;

static char ParamFileName[MaxStrLen];
static WORD ParamBinaryFlag;
static WORD ParamAppendFlag;
static WORD ParamXmodemOpt;
static char ParamSecondFileName[MaxStrLen];

static BOOL AutoLogClose = FALSE;

static void BringupMacroWindow(BOOL flash_flag)
{
	HWND hwnd;
	DWORD pid_macro, pid;
	DWORD targetid;
	DWORD currentActiveThreadId;

	currentActiveThreadId = GetWindowThreadProcessId(GetForegroundWindow(), &pid);

	GetWindowThreadProcessId(HWndDdeCli, &pid_macro);
	hwnd = GetTopWindow(NULL);
	while (hwnd) {
		targetid = GetWindowThreadProcessId(hwnd, &pid);
		if (pid == pid_macro) {
			if (flash_flag)
				FlashWindow(hwnd, TRUE);

			SendMessage(hwnd, WM_USER_MSTATBRINGUP, (WPARAM)hwnd, 0);
		}
		hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
	}

	// �}�N���E�B���h�E�{��
	ShowWindow(HWndDdeCli, SW_NORMAL);
	SetForegroundWindow(HWndDdeCli);
	BringWindowToTop(HWndDdeCli);
}


void GetClientHWnd(PCHAR HWndStr)
{
	int i;
	BYTE b;
	LONG HCli;

	HCli = 0;
	i = 0;
	b = HWndStr[0];
	while (b > 0)
	{
		if (b <= 0x39)
			HCli = (HCli << 4) + (b-0x30);
		else
			HCli = (HCli << 4) + (b-0x37);
		i++;
		b = HWndStr[i];
	}
	HWndDdeCli = (HWND)HCli;
}

void Byte2HexStr(BYTE b, LPSTR HexStr)
{
	if (b<0xa0)
		HexStr[0] = 0x30 + (b >> 4);
	else
		HexStr[0] = 0x37 + (b >> 4);
	if ((b & 0x0f) < 0x0a)
		HexStr[1] = 0x30 + (b & 0x0f);
	else
		HexStr[1] = 0x37 + (b & 0x0f);
	HexStr[2] = 0;
}

void SetTopic()
{
	WORD w;

	w = HIWORD(HVTWin);
	Byte2HexStr(HIBYTE(w),&(TopicName[0]));
	Byte2HexStr(LOBYTE(w),&(TopicName[2]));
	w = LOWORD(HVTWin);
	Byte2HexStr(HIBYTE(w),&(TopicName[4]));
	Byte2HexStr(LOBYTE(w),&(TopicName[6]));
}

HDDEDATA WildConnect(HSZ ServiceHsz, HSZ TopicHsz, UINT ClipFmt)
{
	HSZPAIR Pairs[2];
	BOOL Ok;

	Pairs[0].hszSvc  = Service;
	Pairs[0].hszTopic = Topic;
	Pairs[1].hszSvc = NULL;
	Pairs[1].hszTopic = NULL;

	Ok = FALSE;

	if ((ServiceHsz == 0) && (TopicHsz == 0))
		Ok = TRUE;
	else
		if ((TopicHsz == 0) &&
		    (DdeCmpStringHandles(Service, ServiceHsz) == 0))
			Ok = TRUE;
		else
			if ((DdeCmpStringHandles(Topic, TopicHsz) == 0) &&
			    (ServiceHsz == 0))
				Ok = TRUE;

	if (Ok)
		return DdeCreateDataHandle(Inst, (LPBYTE)(&Pairs), sizeof(Pairs),
		                           0, NULL, ClipFmt, 0);
	else
		return 0;
}

BOOL DDEGet1(LPBYTE b)
{
	if (cv.DCount <= 0) return FALSE;
	*b = ((LPSTR)cv.LogBuf)[cv.DStart];
	cv.DStart++;
	if (cv.DStart>=InBuffSize)
		cv.DStart = cv.DStart-InBuffSize;
	cv.DCount--;
	return TRUE;
}

LONG DDEGetDataLen()
{
	BYTE b;
	LONG Len;
	int Start, Count;

	Len = cv.DCount;
	Start = cv.DStart;
	Count = cv.DCount;
	while (Count>0)
	{
		b = ((LPSTR)cv.LogBuf)[Start];
		if ((b==0x00) || (b==0x01)) Len++;
		Start++;
		if (Start>=InBuffSize) Start = Start-InBuffSize;
		Count--;
	}

	return Len;
}

HDDEDATA AcceptRequest(HSZ ItemHSz)
{
	BYTE b;
	BOOL Unlock;
	HDDEDATA DH;
	LPSTR DP;
	int i;
	LONG Len;

	if ((! DDELog) || (ConvH==0)) return 0;

	if (DdeCmpStringHandles(ItemHSz, Item2) == 0) // item "PARAM"
		DH = DdeCreateDataHandle(Inst,ParamFileName,sizeof(ParamFileName),0,
		                         Item2,CF_OEMTEXT,0);
	else if (DdeCmpStringHandles(ItemHSz, Item) == 0) // item "DATA"
	{
		if (cv.HLogBuf==0) return 0;

		if (cv.LogBuf==NULL)
		{
			Unlock = TRUE;
			cv.LogBuf = GlobalLock(cv.HLogBuf);
			if (cv.LogBuf == NULL) return 0;
		}
		else Unlock = FALSE;

		Len = DDEGetDataLen();
		if ((SyncMode) &&
		    (SyncFreeSpace<Len))
			Len = SyncFreeSpace;

		DH = DdeCreateDataHandle(Inst,NULL,Len+2,0,
		                         Item,CF_OEMTEXT,0);
		DP = DdeAccessData(DH,NULL);
		if (DP != NULL)
		{
			i = 0;
			while (i < Len)
			{
				if (DDEGet1(&b)) {
					if ((b==0x00) || (b==0x01))
					{
						DP[i] = 0x01;
						DP[i+1] = b + 1;
						i = i + 2;
					}
					else {
						DP[i] = b;
						i++;
					}
				}
				else
					Len = 0;
			}
			DP[i] = 0;
			DdeUnaccessData(DH);
		}

		if (Unlock)
		{
			GlobalUnlock(cv.HLogBuf);
			cv.LogBuf = NULL;
		}
	}
	else
		return 0;

	return DH;
}

HDDEDATA AcceptPoke(HSZ ItemHSz, UINT ClipFmt,
                    HDDEDATA Data)
{
	LPSTR DataPtr;
	DWORD DataSize;

	// �A������XTYP_POKE���N���C�A���g�i�}�N���j���瑗���Ă���ƁA�T�[�o�i�{�́j�����܂�
	// �R�}���h�̓\��t�����s���Ă��Ȃ��ꍇ�ATalkStatus�� IdTalkCB �ɂȂ̂ŁADDE_FNOTPROCESSED��
	// �Ԃ����Ƃ�����BDDE_FBUSY�ɕύX�B
	// (2006.11.6 yutaka)
	if (TalkStatus != IdTalkKeyb)
		return (HDDEDATA)DDE_FBUSY;

	if (ConvH==0) return DDE_FNOTPROCESSED;

	if ((ClipFmt!=CF_TEXT) && (ClipFmt!=CF_OEMTEXT)) return DDE_FNOTPROCESSED;

	if (DdeCmpStringHandles(ItemHSz, Item) != 0) return DDE_FNOTPROCESSED;

	DataPtr = DdeAccessData(Data,&DataSize);
	if (DataPtr==NULL) return DDE_FNOTPROCESSED;
	CBStartSend(DataPtr, DataSize, FALSE);
	DdeUnaccessData(Data);
	if (TalkStatus==IdTalkCB)
		return (HDDEDATA)DDE_FACK;
	else
		return DDE_FNOTPROCESSED;
}

WORD HexStr2Word(PCHAR Str)
{
	int i;
	BYTE b;
	WORD w;

	for (i=0; i<=3; i++)
	{
		b = Str[i];
		if (b <= 0x39)
			w = (w << 4) + (b-0x30);
		else
		w = (w << 4) + (b-0x37);
	}
	return w;
}

#define CmdSetHWnd      ' '
#define CmdSetFile      '!'
#define CmdSetBinary    '"'
#define CmdSetAppend    '#'
#define CmdSetXmodemOpt '$'
#define CmdSetSync      '%'

#define CmdBPlusRecv    '&'
#define CmdBPlusSend    '\''
#define CmdChangeDir    '('
#define CmdClearScreen  ')'
#define CmdCloseWin     '*'
#define CmdConnect      '+'
#define CmdDisconnect   ','
#define CmdEnableKeyb   '-'
#define CmdGetTitle     '.'
#define CmdInit         '/'
#define CmdKmtFinish    '0'
#define CmdKmtGet       '1'
#define CmdKmtRecv      '2'
#define CmdKmtSend      '3'
#define CmdLoadKeyMap   '4'
#define CmdLogClose     '5'
#define CmdLogOpen      '6'
#define CmdLogPause     '7'
#define CmdLogStart     '8'
#define CmdLogWrite     '9'
#define CmdQVRecv       ':'
#define CmdQVSend       ';'
#define CmdRestoreSetup '<'
#define CmdSendBreak    '='
#define CmdSendFile     '>'
#define CmdSendKCode    '?'
#define CmdSetEcho      '@'
#define CmdSetTitle     'A'
#define CmdShowTT       'B'
#define CmdXmodemSend   'C'
#define CmdXmodemRecv   'D'
#define CmdZmodemSend   'E'
#define CmdZmodemRecv   'F'
#define CmdCallMenu     'G'
#define CmdScpSend      'H'
#define CmdScpRcv       'I'
#define CmdSetSecondFile 'J'
#define CmdSetBaud      'K'
#define CmdSetRts       'L'
#define CmdSetDtr       'M'
#define CmdGetHostname  'N'
#define CmdSendBroadcast  'O'
#define CmdSendMulticast  'P'
#define CmdSetMulticastName  'Q'
#define CmdSetDebug     'R'
#define CmdYmodemSend   'S'
#define CmdYmodemRecv   'T'
#define CmdDispStr      'U'
#define CmdLogInfo      'V'
#define CmdLogRotate    'W'
#define CmdLogAutoClose 'X'
#define CmdGetModemStatus 'Y'
#define CmdSetFlowCtrl  'Z'

HDDEDATA AcceptExecute(HSZ TopicHSz, HDDEDATA Data)
{
	char Command[MaxStrLen + 1];
	int i;
	WORD w, c;

	memset(Command, 0, sizeof(Command));

	if ((ConvH==0) ||
		(DdeCmpStringHandles(TopicHSz, Topic) != 0) ||
		(DdeGetData(Data,Command,sizeof(Command),0) == 0))
		return DDE_FNOTPROCESSED;

	switch (Command[0]) {
	case CmdSetHWnd:
		GetClientHWnd(&Command[1]);
		if (cv.Ready)
			SetDdeComReady(1);
		break;
	case CmdSetFile:
		strncpy_s(ParamFileName, sizeof(ParamFileName),&(Command[1]), _TRUNCATE);
		break;
	case CmdSetBinary:
		ParamBinaryFlag = Command[1] & 1;
		break;
	case CmdSetAppend:
		ParamAppendFlag     = Command[1] & 1;
		ts.LogTypePlainText = Command[13] & 1;
		ts.LogTimestamp     = Command[14] & 1;
		ts.LogHideDialog    = Command[15] & 1;
		ts.LogAllBuffIncludedInFirst    = Command[16] & 1;
		break;
	case CmdSetXmodemOpt:
		ParamXmodemOpt = Command[1] & 3;
		if (ParamXmodemOpt==0) ParamXmodemOpt = 1;
		break;
	case CmdSetSync:
		if (sscanf(&(Command[1]),"%u",&SyncFreeSpace)!=1)
			SyncFreeSpace = 0;
		SyncMode = (SyncFreeSpace>0);
		SyncRecv = TRUE;
		break;
	case CmdBPlusRecv:
		if ((FileVar==NULL) && NewFileVar(&FileVar))
		{
			FileVar->NoMsg = TRUE;
			DdeCmnd = TRUE;
			BPStart(IdBPReceive);
		}
		else
			return DDE_FNOTPROCESSED;
		break;
	case CmdBPlusSend:
		if ((FileVar==NULL) && NewFileVar(&FileVar))
		{
			FileVar->DirLen = 0;
			strncpy_s(FileVar->FullName, sizeof(FileVar->FullName),ParamFileName, _TRUNCATE);
			FileVar->NumFname = 1;
			FileVar->NoMsg = TRUE;
			DdeCmnd = TRUE;
			BPStart(IdBPSend);
		}
		else
			return DDE_FNOTPROCESSED;
		break;
	case CmdChangeDir:
		strncpy_s(ts.FileDir, sizeof(ts.FileDir),ParamFileName, _TRUNCATE);
		break;
	case CmdClearScreen:
		switch (ParamFileName[0]) {
		case '0':
			PostMessage(HVTWin,WM_USER_ACCELCOMMAND,IdCmdEditCLS,0);
			break;
		case '1':
			PostMessage(HVTWin,WM_USER_ACCELCOMMAND,IdCmdEditCLB,0);
			break;
		case '2':
			PostMessage(HTEKWin,WM_USER_ACCELCOMMAND,IdCmdEditCLS,0);
			break;
		}
		break;
	case CmdCloseWin:
		CloseTT = TRUE;
		break;
	case CmdConnect:
		if (cv.Open) {
			if (cv.Ready)
				SetDdeComReady(1);
			break;
		}
		{
		char Temp[MaxStrLen + 2];
		strncpy_s(Temp, sizeof(Temp),"a ", _TRUNCATE); // dummy exe name
		strncat_s(Temp,sizeof(Temp),ParamFileName,_TRUNCATE);
		if (LoadTTSET())
			(*ParseParam)(Temp, &ts, NULL);
		FreeTTSET();
		cv.NoMsg = 1; /* suppress error messages */
		PostMessage(HVTWin,WM_USER_COMMSTART,0,0);
		}
		break;
	case CmdDisconnect:
		if (ParamFileName[0] == '0') {
			PostMessage(HVTWin,WM_USER_ACCELCOMMAND,IdCmdDisconnect,0);
		}
		else {
			PostMessage(HVTWin,WM_USER_ACCELCOMMAND,IdCmdDisconnect,1);
		}
		break;
	case CmdEnableKeyb:
		KeybEnabled = (ParamBinaryFlag!=0);
		break;
	case CmdGetTitle:
		// title is transferred later by XTYP_REQUEST
		strncpy_s(ParamFileName, sizeof(ParamFileName),ts.Title, _TRUNCATE);
		break;
	case CmdInit: // initialization signal from TTMACRO
		if (StartupFlag) // in case of startup macro
		{ // TTMACRO is waiting for connecting to the host
			// �V���A���ڑ��Ŏ����ڑ��������̏ꍇ�́A�ڑ��_�C�A���O���o���Ȃ� (2006.9.15 maya)
			if (!((ts.PortType==IdSerial) && (ts.ComAutoConnect == FALSE)) &&
				((ts.PortType==IdSerial) || (ts.HostName[0]!=0)))
			{
				cv.NoMsg = 1;
				// start connecting
				PostMessage(HVTWin,WM_USER_COMMSTART,0,0);
			}
			else  // notify TTMACRO that I can not connect
				SetDdeComReady(0);
			StartupFlag = FALSE;
		}
		break;
	case CmdKmtFinish:
	case CmdKmtRecv:
		if ((FileVar==NULL) && NewFileVar(&FileVar))
		{
			FileVar->NoMsg = TRUE;
			DdeCmnd = TRUE;
			if (Command[0]==CmdKmtFinish)
				i = IdKmtFinish;
			else
				i = IdKmtReceive;
			KermitStart(i);
		}
		else
			return DDE_FNOTPROCESSED;
		break;
	case CmdKmtGet:
	case CmdKmtSend:
		if ((FileVar==NULL) && NewFileVar(&FileVar))
		{
			FileVar->DirLen = 0;
			strncpy_s(FileVar->FullName, sizeof(FileVar->FullName),ParamFileName, _TRUNCATE);
			FileVar->NumFname = 1;
			FileVar->NoMsg = TRUE;
			DdeCmnd = TRUE;
			if (Command[0]==CmdKmtGet)
				i = IdKmtGet;
			else
				i = IdKmtSend;
			KermitStart(i);
		}
		else
			return DDE_FNOTPROCESSED;
		break;
	case CmdLoadKeyMap:
		strncpy_s(ts.KeyCnfFN, sizeof(ts.KeyCnfFN),ParamFileName, _TRUNCATE);
		PostMessage(HVTWin,WM_USER_ACCELCOMMAND,IdCmdLoadKeyMap,0);
		break;

	case CmdLogRotate:
		if (LogVar != NULL) {
			char *p = ParamFileName;
			int s;

			if (strncmp(p, "size", 4) == 0) {
				s = atoi(&p[5]);
				LogVar->RotateMode = ROTATE_SIZE;
				LogVar->RotateSize = s;

			} else if (strncmp(p, "rotate", 6) == 0) {
				s = atoi(&p[7]);
				LogVar->RotateStep = s;

			} else if (strncmp(p, "halt", 4) == 0) {
				LogVar->RotateMode = ROTATE_NONE;
				LogVar->RotateSize = 0;
				LogVar->RotateStep = 0;
			}
		}
		break;

	case CmdLogAutoClose:
		AutoLogClose = (ParamBinaryFlag!=0);
		break;

	case CmdLogClose:
		if (LogVar != NULL) FileTransEnd(OpLog);
		break;
	case CmdLogOpen:
		if ((LogVar==NULL) && NewFileVar(&LogVar))
		{
			BOOL ret;
			LogVar->DirLen = 0;
			LogVar->NoMsg = TRUE;
			strncpy_s(LogVar->FullName, sizeof(LogVar->FullName),ParamFileName, _TRUNCATE);
			ParseStrftimeFileName(LogVar->FullName, sizeof(LogVar->FullName));
			ts.LogBinary = ParamBinaryFlag;
			ts.Append = ParamAppendFlag;
			ret = LogStart();
			if (ret) {
				strncpy_s(ParamFileName, sizeof(ParamFileName),"1", _TRUNCATE);
			}
			else {
				strncpy_s(ParamFileName, sizeof(ParamFileName),"0", _TRUNCATE);
			}
		}
		else
			return DDE_FNOTPROCESSED;
		break;
	case CmdLogPause:
		FLogChangeButton(TRUE);
		break;
	case CmdLogStart:
		FLogChangeButton(FALSE);
		break;
	case CmdLogWrite:
		if (LogVar != NULL)
		{
			DWORD wrote;
#ifdef FileVarWin16
			_lwrite(LogVar->FileHandle,
				ParamFileName,strlen(ParamFileName));
#else
			WriteFile((HANDLE)LogVar->FileHandle, ParamFileName, strlen(ParamFileName), &wrote, NULL);
#endif
			LogVar->ByteCount =
				LogVar->ByteCount + strlen(ParamFileName);
			FLogRefreshNum();
		}
		break;
	case CmdQVRecv:
		if ((FileVar==NULL) && NewFileVar(&FileVar))
		{
			FileVar->NoMsg = TRUE;
			DdeCmnd = TRUE;
			QVStart(IdQVReceive);
		}
		else
			return DDE_FNOTPROCESSED;
		break;
	case CmdQVSend:
		if ((FileVar==NULL) && NewFileVar(&FileVar))
		{
			FileVar->DirLen = 0;
			strncpy_s(FileVar->FullName, sizeof(FileVar->FullName),ParamFileName, _TRUNCATE);
			FileVar->NumFname = 1;
			FileVar->NoMsg = TRUE;
			DdeCmnd = TRUE;
			QVStart(IdQVSend);
		}
		else
			return DDE_FNOTPROCESSED;
		break;
	case CmdRestoreSetup:
		strncpy_s(ts.SetupFName, sizeof(ts.SetupFName),ParamFileName, _TRUNCATE);
		PostMessage(HVTWin,WM_USER_ACCELCOMMAND,IdCmdRestoreSetup,0);
		break;
	case CmdSendBreak:
		PostMessage(HVTWin,WM_USER_ACCELCOMMAND,IdBreak,0);
		break;
	case CmdSendFile:
		if ((SendVar==NULL) && NewFileVar(&SendVar))
		{
			SendVar->DirLen = 0;
			strncpy_s(SendVar->FullName, sizeof(SendVar->FullName),ParamFileName, _TRUNCATE);
			ts.TransBin = ParamBinaryFlag;
			SendVar->NoMsg = TRUE;
			DdeCmnd = TRUE;
			FileSendStart();
		}
		else
			return DDE_FNOTPROCESSED;
		break;
	case CmdSendKCode:
		w = HexStr2Word(ParamFileName);
		c = HexStr2Word(&ParamFileName[4]);
		PostMessage(HVTWin,WM_USER_KEYCODE,w,(LPARAM)c);
		break;
	case CmdSetEcho:
		ts.LocalEcho = ParamBinaryFlag;
		if (cv.Ready && cv.TelFlag && (ts.TelEcho>0))
			TelChangeEcho();
		break;
	case CmdSetDebug:
		DebugFlag = (Command[1]-'0')%DEBUG_FLAG_MAXD;
		break;
	case CmdSetTitle:
		strncpy_s(ts.Title, sizeof(ts.Title),ParamFileName, _TRUNCATE);
		if (ts.AcceptTitleChangeRequest == IdTitleChangeRequestOverwrite) {
			cv.TitleRemote[0] = '\0';
		}
		ChangeTitle();
		break;
	case CmdShowTT:
		switch (ParamFileName[0]) {
		case '-': ShowWindow(HVTWin,SW_HIDE); break;
		case '0': ShowWindow(HVTWin,SW_MINIMIZE); break;
		case '1': ShowWindow(HVTWin,SW_RESTORE); break;
		case '2': ShowWindow(HTEKWin,SW_HIDE); break;
		case '3': ShowWindow(HTEKWin,SW_MINIMIZE); break;
		case '4':
			PostMessage(HVTWin,WM_USER_ACCELCOMMAND,IdCmdCtrlOpenTEK,0);
			break;
		case '5':
			PostMessage(HVTWin,WM_USER_ACCELCOMMAND,IdCmdCtrlCloseTEK,0);
			break;
		case '6': //steven add
			if (HWndLog == NULL)
				break;
			else
				ShowWindow(HWndLog, SW_HIDE);
			break;
		case '7': //steven add
			if (HWndLog == NULL)
				break;
			else
				ShowWindow(HWndLog, SW_MINIMIZE);
			break;
		case '8': //steven add
			if (HWndLog == NULL)
				break;
			else {
				ShowWindow(HWndLog, SW_RESTORE);
				// �g���X�^�C�� WS_EX_NOACTIVATE ��Ԃ���������
				SetForegroundWindow(HWndLog);
			}
			break;
		}
		break;
	case CmdXmodemRecv:
		if ((FileVar==NULL) && NewFileVar(&FileVar))
		{
			FileVar->DirLen = 0;
			strncpy_s(FileVar->FullName, sizeof(FileVar->FullName),ParamFileName, _TRUNCATE);
			if (IsXopt1k(ts.XmodemOpt)) {
				if (IsXoptCRC(ParamXmodemOpt)) {
					// CRC
					ts.XmodemOpt = Xopt1kCRC;
				}
				else {	// Checksum
					ts.XmodemOpt = Xopt1kCksum;
				}
			}
			else {
				if (IsXoptCRC(ParamXmodemOpt)) {
					ts.XmodemOpt = XoptCRC;
				}
				else {
					ts.XmodemOpt = XoptCheck;
				}
			}
			ts.XmodemBin = ParamBinaryFlag;
			FileVar->NoMsg = TRUE;
			DdeCmnd = TRUE;
			XMODEMStart(IdXReceive);
		}
		else
			return DDE_FNOTPROCESSED;
		break;
	case CmdXmodemSend:
		if ((FileVar==NULL) && NewFileVar(&FileVar))
		{
			FileVar->DirLen = 0;
			strncpy_s(FileVar->FullName, sizeof(FileVar->FullName),ParamFileName, _TRUNCATE);
			if (IsXoptCRC(ts.XmodemOpt)) {
				if (IsXopt1k(ParamXmodemOpt)) {
					ts.XmodemOpt = Xopt1kCRC;
				}
				else {
					ts.XmodemOpt = XoptCRC;
				}
			}
			else {
				if (IsXopt1k(ParamXmodemOpt)) {
					ts.XmodemOpt = Xopt1kCksum;
				}
				else {
					ts.XmodemOpt = XoptCheck;
				}
			}
			FileVar->NoMsg = TRUE;
			DdeCmnd = TRUE;
			XMODEMStart(IdXSend);
		}
		else
			return DDE_FNOTPROCESSED;
		break;
	case CmdZmodemRecv:
		if ((FileVar==NULL) && NewFileVar(&FileVar))
		{
			FileVar->NoMsg = TRUE;
			DdeCmnd = TRUE;
			ZMODEMStart(IdZReceive);
		}
		else
			return DDE_FNOTPROCESSED;
		break;
	case CmdZmodemSend:
		if ((FileVar==NULL) && NewFileVar(&FileVar))
		{
			FileVar->DirLen = 0;
			strncpy_s(FileVar->FullName, sizeof(FileVar->FullName),ParamFileName, _TRUNCATE);
			FileVar->NumFname = 1;
			ts.XmodemBin = ParamBinaryFlag;
			FileVar->NoMsg = TRUE;
			DdeCmnd = TRUE;
			ZMODEMStart(IdZSend);
		}
		else
			return DDE_FNOTPROCESSED;
		break;

	case CmdYmodemRecv:
		if ((FileVar==NULL) && NewFileVar(&FileVar))
		{
			FileVar->NoMsg = TRUE;
			DdeCmnd = TRUE;
			YMODEMStart(IdYReceive);
		}
		else
			return DDE_FNOTPROCESSED;
		break;
	case CmdYmodemSend:
		if ((FileVar==NULL) && NewFileVar(&FileVar))
		{
			FileVar->DirLen = 0;
			strncpy_s(FileVar->FullName, sizeof(FileVar->FullName),ParamFileName, _TRUNCATE);
			FileVar->NumFname = 1;
			//ts.XmodemBin = ParamBinaryFlag;
			FileVar->NoMsg = TRUE;
			DdeCmnd = TRUE;
			YMODEMStart(IdYSend);
		}
		else
			return DDE_FNOTPROCESSED;
		break;

		// add 'callmenu' (2007.11.18 maya)
	case CmdCallMenu:
		i = atoi(ParamFileName);
		if (i >= 51110 && i <= 51990) {
			PostMessage(HTEKWin,WM_COMMAND,MAKELONG(i,0),0);
		}
		else {
			PostMessage(HVTWin,WM_COMMAND,MAKELONG(i,0),0);
		}
		break;

	case CmdSetSecondFile:  // add 'scpsend' (2008.1.3 yutaka)
		memset(ParamSecondFileName, 0, sizeof(ParamSecondFileName));
		strncpy_s(ParamSecondFileName, sizeof(ParamSecondFileName),&(Command[1]), _TRUNCATE);
		break;

	case CmdScpSend:  // add 'scpsend' (2008.1.1 yutaka)
		{
		typedef int (CALLBACK *PSSH_start_scp)(char *, char *);
		static PSSH_start_scp func = NULL;
		static HMODULE h = NULL;
		char msg[128];

		//MessageBox(NULL, "hoge", "foo", MB_OK);

		if (func == NULL) {
			if ( ((h = GetModuleHandle("ttxssh.dll")) == NULL) ) {
				_snprintf_s(msg, sizeof(msg), _TRUNCATE, "GetModuleHandle(\"ttxssh.dll\")) %d", GetLastError());
				goto scp_send_error;
			}
			func = (PSSH_start_scp)GetProcAddress(h, "TTXScpSendfile");
			if (func == NULL) {
				_snprintf_s(msg, sizeof(msg), _TRUNCATE, "GetProcAddress(\"TTXScpSendfile\")) %d", GetLastError());
				goto scp_send_error;
			}
		}

		if (func != NULL) {
			//MessageBox(NULL, ParamFileName, "foo2", MB_OK);
			//MessageBox(NULL, ParamSecondFileName, "foo3", MB_OK);
			DdeCmnd = TRUE;
			func(ParamFileName, ParamSecondFileName);
			EndDdeCmnd(1);     // �}�N�����s���I��������B�{���Ȃ�ASCP�]�����������Ă���Ăяo�����ق����]�܂����B
			break;
		}

scp_send_error:
		MessageBox(NULL, msg, "Tera Term: scpsend command error", MB_OK | MB_ICONERROR);
		return DDE_FNOTPROCESSED;
		}
		break;

	case CmdScpRcv:
		{
		typedef int (CALLBACK *PSSH_start_scp)(char *, char *);
		static PSSH_start_scp func = NULL;
		static HMODULE h = NULL;
		char msg[128];

		//MessageBox(NULL, "hoge", "foo", MB_OK);

		if (func == NULL) {
			if ( ((h = GetModuleHandle("ttxssh.dll")) == NULL) ) {
				_snprintf_s(msg, sizeof(msg), _TRUNCATE, "GetModuleHandle(\"ttxssh.dll\")) %d", GetLastError());
				goto scp_rcv_error;
			}
			func = (PSSH_start_scp)GetProcAddress(h, "TTXScpReceivefile");
			if (func == NULL) {
				_snprintf_s(msg, sizeof(msg), _TRUNCATE, "GetProcAddress(\"TTXScpReceivefile\")) %d", GetLastError());
				goto scp_rcv_error;
			}
		}

		if (func != NULL) {
			//MessageBox(NULL, ParamFileName, "foo2", MB_OK);
			//MessageBox(NULL, ParamSecondFileName, "foo3", MB_OK);
			DdeCmnd = TRUE;
			func(ParamFileName, ParamSecondFileName);
			EndDdeCmnd(1);     // �}�N�����s���I��������B�{���Ȃ�ASCP�]�����������Ă���Ăяo�����ق����]�܂����B
			break;
		}

scp_rcv_error:
		MessageBox(NULL, msg, "Tera Term: scpsend command error", MB_OK | MB_ICONERROR);
		return DDE_FNOTPROCESSED;
		}
		break;

	case CmdSetBaud:  // add 'setbaud' (2008.2.13 steven patch)
		{
		int val;

		//OutputDebugPrintf("CmdSetBaud entered\n");

		if (!cv.Open || cv.PortType != IdSerial)
			return DDE_FNOTPROCESSED;

		val = atoi(ParamFileName);
		//OutputDebugPrintf("CmdSetBaud: %d (%d)\n", val, ts.Baud);
		if (val > 0) {
			ts.Baud = val;
			CommResetSerial(&ts,&cv,FALSE);   // reset serial port
			PostMessage(HVTWin,WM_USER_CHANGETITLE,0,0); // refresh title bar
		}
		}
		break;

	case CmdSetFlowCtrl:
		{
		int val;

		if (!cv.Open || cv.PortType != IdSerial)
			return DDE_FNOTPROCESSED;

		val = atoi(ParamFileName);
		switch(val) {
			case 1:
			case 2:
			case 3:
				ts.Flow = val;
				break;
		}
		break;
		}

	case CmdSetRts:  // add 'setrts' (2008.3.12 maya)
		{
		int val, ret;

		if (!cv.Open || cv.PortType != IdSerial || ts.Flow != IdFlowNone)
			return DDE_FNOTPROCESSED;

		val = atoi(ParamFileName);
		if (val == 0) {
			ret = EscapeCommFunction(cv.ComID, CLRRTS);
		}
		else {
			ret = EscapeCommFunction(cv.ComID, SETRTS);
		}
		}
		break;

	case CmdSetDtr:  // add 'setdtr' (2008.3.12 maya)
		{
		int val, ret;

		if (!cv.Open || cv.PortType != IdSerial || ts.Flow != IdFlowNone)
			return DDE_FNOTPROCESSED;

		val = atoi(ParamFileName);
		if (val == 0) {
			ret = EscapeCommFunction(cv.ComID, CLRDTR);
		}
		else {
			ret = EscapeCommFunction(cv.ComID, SETDTR);
		}
		}
		break;

	case CmdGetModemStatus: // add 'getmodemstatus' (2015.1.8 yutaka)
		{
		DWORD val, n;

		if (!cv.Open || cv.PortType != IdSerial)
			return DDE_FNOTPROCESSED;

		if (GetCommModemStatus(cv.ComID, &val) == 0) // error
			return DDE_FNOTPROCESSED;
		//val = MS_CTS_ON | MS_DSR_ON | MS_RING_ON | MS_RLSD_ON;
		n = 0;
		if (val & MS_CTS_ON)
			n |= 0x01;
		if (val & MS_DSR_ON)
			n |= 0x02;
		if (val & MS_RING_ON)
			n |= 0x04;
		if (val & MS_RLSD_ON)
			n |= 0x08;

		_snprintf_s(ParamFileName, sizeof(ParamFileName), _TRUNCATE, "%d", n);
		}
		break;

	case CmdGetHostname:  // add 'gethostname' (2008.12.15 maya)
		if (cv.Open) {
			if (cv.PortType == IdTCPIP) {
				strncpy_s(ParamFileName, sizeof(ParamFileName),ts.HostName, _TRUNCATE);
			}
			else if (cv.PortType == IdSerial) {
				_snprintf_s(ParamFileName, sizeof(ParamFileName), _TRUNCATE, "COM%d", ts.ComPort);
			}
		}
		break;

	case CmdSendBroadcast: // 'sendbroadcast'
		SendAllBroadcastMessage(HVTWin, HVTWin, 0, ParamFileName, strlen(ParamFileName));
		DdeCmnd = TRUE;
		EndDdeCmnd(0);     // �}�N�����s���I��������B
		break;

	case CmdSendMulticast: // 'sendmulticast'
		SendMulticastMessage(HVTWin, HVTWin, ParamFileName, ParamSecondFileName, strlen(ParamSecondFileName));
		DdeCmnd = TRUE;
		EndDdeCmnd(0);     // �}�N�����s���I��������B
		break;

	case CmdSetMulticastName: // 'setmulticastname'
		SetMulticastName(ParamFileName);
		DdeCmnd = TRUE;
		EndDdeCmnd(0);     // �}�N�����s���I��������B
		break;

	case CmdDispStr:
		CBStartSend(ParamFileName, sizeof(ParamFileName), TRUE);
		break;

	case CmdLogInfo:
		if (LogVar) {
			ParamFileName[0] = '0'
				+ (ts.LogBinary != 0)
				+ ((ts.Append != 0) << 1)
				+ ((ts.LogTypePlainText != 0) << 2)
				+ ((ts.LogTimestamp != 0) << 3)
				+ ((ts.LogHideDialog != 0) << 4);
			strncpy_s(ParamFileName+1, sizeof(ParamFileName)-1, LogVar->FullName, _TRUNCATE);
		}
		else {
			ParamFileName[0] = '0' - 1;
			ParamFileName[1] = 0;
		}
		break;

	default:
		return DDE_FNOTPROCESSED;
	}
	return (HDDEDATA)DDE_FACK;
}

HDDEDATA CALLBACK DdeCallbackProc(UINT CallType, UINT Fmt, HCONV Conv,
                                  HSZ HSz1, HSZ HSz2, HDDEDATA Data,
                                  DWORD Data1, DWORD Data2)
{
	HDDEDATA Result;

	if (Inst==0) return 0;
	Result = 0;

	switch (CallType) {
		case XTYP_WILDCONNECT:
			Result = WildConnect(HSz2, HSz1, Fmt);
			break;
		case XTYP_CONNECT:
			if (Conv == 0)
			{
				if ((DdeCmpStringHandles(Topic, HSz1) == 0) &&
				    (DdeCmpStringHandles(Service, HSz2) == 0))
				{
					if (cv.Ready)
						SetDdeComReady(1);
					Result = (HDDEDATA)TRUE;
				}
			}
			break;
		case XTYP_CONNECT_CONFIRM:
			ConvH = Conv;
			break;
		case XTYP_ADVREQ:
		case XTYP_REQUEST:
			Result = AcceptRequest(HSz2);
			break;

	// �N���C�A���g(ttpmacro.exe)����T�[�o(ttermpro.exe)�փf�[�^�������Ă���ƁA
	// ���̃��b�Z�[�W�n���h���֔��ł���B
	// ���������� DDE_FACK�A�r�W�[�̏ꍇ�� DDE_FBUSY �A��������ꍇ�� DDE_FNOTPROCESSED ��
	// �N���C�A���g�֕Ԃ��K�v������Abreak���������Ă����̂Œǉ������B
	// (2006.11.6 yutaka)
		case XTYP_POKE:
			Result = AcceptPoke(HSz2, Fmt, Data);
			break;

		case XTYP_ADVSTART:
			if ((ConvH!=0) &&
			    (DdeCmpStringHandles(Topic, HSz1) == 0) &&
			    (DdeCmpStringHandles(Item, HSz2) == 0) &&
			    ! AdvFlag)
			{
				AdvFlag = TRUE;
				Result = (HDDEDATA)TRUE;
			}
			break;
		case XTYP_ADVSTOP:
			if ((ConvH!=0) &&
			    (DdeCmpStringHandles(Topic, HSz1) == 0) &&
			    (DdeCmpStringHandles(Item, HSz2) == 0) &&
			    AdvFlag)
			{
				AdvFlag = FALSE;
				Result = (HDDEDATA)TRUE;
			}
			break;
		case XTYP_DISCONNECT:
			// �}�N���I�����A���O�̎�������I�ɒ�~����B(2013.6.24 yutaka)
			if (AutoLogClose) {
				if (LogVar != NULL) FileTransEnd(OpLog);
				AutoLogClose = FALSE;
			}
			ConvH = 0;
			PostMessage(HVTWin,WM_USER_DDEEND,0,0);
			break;
		case XTYP_EXECUTE:
			Result = AcceptExecute(HSz1, Data);
	}  /* switch (CallType) */

	return Result;
}

BOOL InitDDE()
{
	BOOL Ok;

	if (ConvH != 0) return FALSE;

	Ok = TRUE;

	if (DdeInitialize(&Inst,(PFNCALLBACK)DdeCallbackProc,0,0) == DMLERR_NO_ERROR)
	{
		Service= DdeCreateStringHandle(Inst, ServiceName, CP_WINANSI);
		Topic  = DdeCreateStringHandle(Inst, TopicName, CP_WINANSI);
		Item   = DdeCreateStringHandle(Inst, ItemName, CP_WINANSI);
		Item2  = DdeCreateStringHandle(Inst, ItemName2, CP_WINANSI);

		Ok = DdeNameService(Inst, Service, 0, DNS_REGISTER) != 0;
	}
	else
		Ok = FALSE;

	SyncMode = FALSE;
	CloseTT = FALSE;
	StartupFlag = FALSE;
	DDELog = FALSE;

	if (Ok)
	{
		Ok = CreateLogBuf();
		if (Ok) DDELog = TRUE;
	}

	if (! Ok) EndDDE();
	return Ok;
}

void SendDDEReady()
{
	GetClientHWnd(TopicName);
	PostMessage(HWndDdeCli,WM_USER_DDEREADY,0,0);
}

void EndDDE()
{
	DWORD Temp;

	if (ConvH != 0)
		DdeDisconnect(ConvH);
	ConvH = 0;
	SyncMode = FALSE;
	StartupFlag = FALSE;

	Temp = Inst;
	if (Inst != 0)
	{
		Inst = 0;
		DdeNameService(Temp, Service, 0, DNS_UNREGISTER);
		if (Service != 0)
			DdeFreeStringHandle(Temp, Service);
		Service = 0;
		if (Topic != 0)
			DdeFreeStringHandle(Temp, Topic);
		Topic = 0;
		if (Item != 0)
			DdeFreeStringHandle(Temp, Item);
		Item = 0;
		if (Item2 != 0)
			DdeFreeStringHandle(Temp, Item2);
		Item2 = 0;

		DdeUninitialize(Temp);
	}
	TopicName[0] = 0;

	if (HWndDdeCli!=NULL)
		PostMessage(HWndDdeCli,WM_USER_DDECMNDEND,0,0);
	HWndDdeCli = NULL;
	AdvFlag = FALSE;

	DDELog = FALSE;
	FreeLogBuf();
	cv.NoMsg = 0;
}

void DDEAdv()
{
	if ((ConvH==0) ||
	    (! AdvFlag) ||
	    (cv.DCount==0))
		return;

	if ((! SyncMode) ||
	    SyncMode && SyncRecv)
	{
		if (SyncFreeSpace<10)
			SyncFreeSpace=0;
		else
			SyncFreeSpace=SyncFreeSpace-10;
		DdePostAdvise(Inst,Topic,Item);
		SyncRecv = FALSE;
	}
}

void EndDdeCmnd(int Result)
{
	if ((ConvH==0) || (HWndDdeCli==NULL) || (! DdeCmnd)) return;
	PostMessage(HWndDdeCli,WM_USER_DDECMNDEND,(WPARAM)Result,0);
	DdeCmnd = FALSE;
}

void SetDdeComReady(WORD Ready)
{
	if (HWndDdeCli==NULL) return;
	PostMessage(HWndDdeCli,WM_USER_DDECOMREADY,Ready,0);
}

void RunMacro(PCHAR FName, BOOL Startup)
//  FName: macro filename
//  Startup: TRUE in case of startup macro execution.
//		  In this case, the connection to the host will
//		  made after the link to TT(P)MACRO is established.
{
	PROCESS_INFORMATION pi;
	int i;
	char Cmnd[MAX_PATH+36]; // "TTPMACRO /D="(12) + TopicName(20) + " "(1) + MAX_PATH + " /S"(3)
	STARTUPINFO si;
	DWORD pri = NORMAL_PRIORITY_CLASS;

	// Control menu����̃}�N���Ăяo���ŁA���łɃ}�N���N�����̏ꍇ�A
	// �Y������"ttpmacro"���t���b�V������B
	// (2010.4.2 yutaka, maya)
	if ((FName == NULL && Startup == FALSE) && ConvH != 0) {
		BringupMacroWindow(TRUE);
		return;
	}

	SetTopic();
	if (! InitDDE()) return;
	strncpy_s(Cmnd, sizeof(Cmnd),"TTPMACRO /D=", _TRUNCATE);
	strncat_s(Cmnd,sizeof(Cmnd),TopicName,_TRUNCATE);
	if (FName!=NULL)
	{
		strncat_s(Cmnd,sizeof(Cmnd)," ",_TRUNCATE);
		i = strlen(Cmnd);
		strncat_s(Cmnd,sizeof(Cmnd),FName,_TRUNCATE);
		QuoteFName(&Cmnd[i]);
	}

	StartupFlag = Startup;
	if (Startup)
		strncat_s(Cmnd,sizeof(Cmnd)," /S",_TRUNCATE); // "startup" flag

#if 0
	if (WinExec(Cmnd,SW_MINIMIZE) < 32)
		EndDDE();
#else

	// ���O�̎撆�������Ȃ����Ƃɂ���B(2005.8.14 yutaka)
#if 0
	// Tera Term�{�̂Ń��O�̎撆�Ƀ}�N�������s����ƁA�}�N���̓��삪��~���邱�Ƃ�
	// ���邽�߁A�v���Z�X�̗D��x��1�����Ď��s������B(2004/9/5 yutaka)
	// ���O�̎撆�݂̂ɉ�����B(2004/11/28 yutaka)
	if (FileLog || BinLog) {
		pri = BELOW_NORMAL_PRIORITY_CLASS;
	}
	// �b�菈�u�Ƃ��āA��ɉ����邱�Ƃɂ���B(2005/5/15 yutaka)
	// �}�N���ɂ��telnet�������O�C�������s���邱�Ƃ�����̂ŁA�����Ȃ����Ƃɂ���B(2005/5/23 yutaka)
	pri = BELOW_NORMAL_PRIORITY_CLASS;
#endif

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	GetStartupInfo(&si);
	si.wShowWindow = SW_MINIMIZE;

	if (CreateProcess(
		NULL,
		Cmnd,
		NULL,
		NULL,
		FALSE,
		pri,
		NULL, NULL,
		&si, &pi) == 0) {
			EndDDE();
	}
#endif
}
