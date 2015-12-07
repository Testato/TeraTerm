/* Tera Term
(C) 2008-2010 TeraTerm Project
All rights reserved. */

/* TTFILE.DLL, YMODEM protocol */
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>

#include "teraterm.h"
#include "tttypes.h"
#include "ttftypes.h"

#include "tt_res.h"
#include "ttcommon.h"
#include "ttlib.h"
#include "ftlib.h"
#include "dlglib.h"

#include "ymodem.h"

#define TimeOutInit  10
#define TimeOutC     3
#define TimeOutShort 10
#define TimeOutLong  20
#define TimeOutVeryLong 60

// �f�[�^�]���T�C�Y�BYMODEM�ł� 128 or 1024 byte ���T�|�[�g����B
#define SOH_DATALEN	128
#define STX_DATALEN	1024

int YRead1Byte(PFileVar fv, PYVar yv, PComVar cv, LPBYTE b)
{
	if (CommRead1Byte(cv,b) == 0)
		return 0;

	if (fv->LogFlag)
	{
		if (fv->LogState==0)
		{
			// �c���ASCII�\�����s��
			fv->FlushLogLineBuf = 1;
			FTLog1Byte(fv,0);
			fv->FlushLogLineBuf = 0;

			fv->LogState = 1;
			fv->LogCount = 0;
			fv->FlushLogLineBuf = 0;
			_lwrite(fv->LogFile,"\015\012<<<\015\012",7);
		}
		FTLog1Byte(fv,*b);
	}
	return 1;
}

int YWrite(PFileVar fv, PYVar yv, PComVar cv, PCHAR B, int C)
{
	int i, j;

	i = CommBinaryOut(cv,B,C);
	if (fv->LogFlag && (i>0))
	{
		if (fv->LogState != 0)
		{
			// �c���ASCII�\�����s��
			fv->FlushLogLineBuf = 1;
			FTLog1Byte(fv,0);
			fv->FlushLogLineBuf = 0;

			fv->LogState = 0;
			fv->LogCount = 0;
			_lwrite(fv->LogFile,"\015\012>>>\015\012",7);
		}
		for (j=0 ; j <= i-1 ; j++)
			FTLog1Byte(fv,B[j]);
	}
	return i;
}

void YSetOpt(PFileVar fv, PYVar yv, WORD Opt)
{
	char Tmp[21];

	yv->YOpt = Opt;

	strncpy_s(Tmp, sizeof(Tmp),"YMODEM (", _TRUNCATE);
	switch (yv->YOpt) {
	case Yopt1K: /* YMODEM */
		strncat_s(Tmp,sizeof(Tmp),"1k)",_TRUNCATE);
		yv->__DataLen = STX_DATALEN;
		yv->DataLen = STX_DATALEN;
		yv->CheckLen = 2;
		break;
	case YoptG: /* YMODEM-g */
		strncat_s(Tmp,sizeof(Tmp),"-g)",_TRUNCATE);
		yv->__DataLen = STX_DATALEN;
		yv->DataLen = STX_DATALEN;
		yv->CheckLen = 2;
		break;
	case YoptSingle: /* YMODEM(-g) single mode */
		strncat_s(Tmp,sizeof(Tmp),"single mode)",_TRUNCATE);
		yv->__DataLen = STX_DATALEN;
		yv->DataLen = STX_DATALEN;
		yv->CheckLen = 2;
		break;
	}
	SetDlgItemText(fv->HWin, IDC_PROTOPROT, Tmp);
}

void YSendNAK(PFileVar fv, PYVar yv, PComVar cv)
{
	BYTE b;
	int t;

	/* flush comm buffer */
	cv->InBuffCount = 0;
	cv->InPtr = 0;

	yv->NAKCount--;
	if (yv->NAKCount<0)
	{
		if (yv->NAKMode==YnakC)
		{
			YSetOpt(fv,yv,XoptCheck);
			yv->NAKMode = YnakC;
			yv->NAKCount = 9;
		}
		else {
			YCancel(fv,yv,cv);
			return;
		}
	}

	if (yv->NAKMode!=YnakC)
	{
		b = NAK;
		if ((yv->PktNum==0) && (yv->PktNumOffset==0))
			t = TimeOutInit;
		else
			t = yv->TOutLong;
	}
	else {
		b = 'C';
		t = TimeOutC;
	}
	YWrite(fv,yv,cv,&b,1);
	yv->PktReadMode = XpktSOH;
	FTSetTimeOut(fv,t);
}

void YSendNAKTimeout(PFileVar fv, PYVar yv, PComVar cv)
{
	BYTE b;
	int t;

	/* flush comm buffer */
	cv->InBuffCount = 0;
	cv->InPtr = 0;

	yv->NAKCount--;
	if (yv->NAKCount<0)
	{
		if (yv->NAKMode==YnakC)
		{
			YSetOpt(fv,yv,XoptCheck);
			yv->NAKMode = YnakC;
			yv->NAKCount = 9;
		}
		else {
			YCancel(fv,yv,cv);
			return;
		}
	}

	//if (yv->NAKMode!=YnakC)
	if (1)
	{
		b = NAK;
		if ((yv->PktNum==0) && (yv->PktNumOffset==0))
			t = TimeOutInit;
		else
			t = yv->TOutLong;
	}
	else {
		b = 'C';
		t = TimeOutC;
	}
	YWrite(fv,yv,cv,&b,1);
	yv->PktReadMode = XpktSOH;
	FTSetTimeOut(fv,t);
}

WORD YCalcCheck(PYVar yv, const PCHAR PktBuf, const WORD len)
{
	int i;
	WORD Check;

	// CheckSum.
	if (1 == yv->CheckLen)
	{
		// Calc sum.
		Check = 0;
		for (i = 0 ; i <= len - 1 ; i++)
			Check = Check + (BYTE)(PktBuf[3 + i]);
		return (Check & 0xff);
	}
	else
	{
		// CRC.
		Check = 0;
		for (i = 0 ; i <= len - 1 ; i++)
			Check = UpdateCRC(PktBuf[3 + i],Check);
		return (Check);
	}
}

BOOL YCheckPacket(PYVar yv, const WORD len)
{
	WORD Check;

	Check = YCalcCheck(yv, yv->PktIn, len);
	// Checksum.
	if (1 == yv->CheckLen)
		return ((BYTE)Check == yv->PktIn[len + 3]);
	else
		return ((HIBYTE(Check) == yv->PktIn[len + 3]) &&
		        (LOBYTE(Check) == yv->PktIn[len + 4]));
}

static void initialize_file_info(PFileVar fv, PYVar yv)
{
	if (yv->YMode == IdYSend) {
		if (fv->FileOpen) {
			_lclose(fv->FileHandle);

			if (fv->FileMtime > 0) {
				SetFMtime(fv->FullName, fv->FileMtime);
			}
		}
		fv->FileHandle = _lopen(fv->FullName,OF_READ);
		fv->FileSize = GetFSize(fv->FullName);
	} else {
		fv->FileHandle = -1;
		fv->FileSize = 0;
		fv->FileMtime = 0;
		yv->RecvFilesize = FALSE;
	}
	fv->FileOpen = fv->FileHandle>0;

	if (yv->YMode == IdYSend) {
		InitDlgProgress(fv->HWin, IDC_PROTOPROGRESS, &fv->ProgStat);
	} else {
		fv->ProgStat = -1;
	}
	fv->StartTime = GetTickCount();
	SetDlgItemText(fv->HWin, IDC_PROTOFNAME, &(fv->FullName[fv->DirLen]));

	yv->PktNumOffset = 0;
	yv->PktNum = 0;
	yv->PktNumSent = 0;
	yv->PktBufCount = 0;
	yv->CRRecv = FALSE;
	fv->ByteCount = 0;
	yv->SendFileInfo = 0;
	yv->SendEot = 0;
	yv->LastSendEot = 0;
	yv->LastMessage = 0;
}

void YInit
(PFileVar fv, PYVar yv, PComVar cv, PTTSet ts)
{
	char inistr[MAX_PATH + 10];

	if (yv->YMode == IdYSend) {
		if (!GetNextFname(fv)) {
			return;
		}
	} 

	fv->LogFlag = ((ts->LogFlag & LOG_Y)!=0);
	if (fv->LogFlag)
		fv->LogFile = _lcreat("YMODEM.LOG",0);
	fv->LogState = 0;
	fv->LogCount = 0;

	SetWindowText(fv->HWin, fv->DlgCaption);

	initialize_file_info(fv, yv);

	if (cv->PortType==IdTCPIP)
	{
		yv->TOutShort = TimeOutVeryLong;
		yv->TOutLong  = TimeOutVeryLong;
	}
	else {
		yv->TOutShort = TimeOutShort;
		yv->TOutLong  = TimeOutLong;
	}  

	YSetOpt(fv,yv,yv->YOpt);

	if (yv->YOpt == Yopt1K)  
	{
		yv->NAKMode = YnakC;
		yv->NAKCount = 10;
	}
	else {
		yv->NAKMode = YnakG;
		yv->NAKCount = 10;
	}

	if (fv->LogFlag) {
		char buf[128];
		time_t tm = time(NULL);

		_snprintf_s(buf, sizeof(buf), _TRUNCATE, "YMODEM %s start: %s\n", 
		            yv->YMode == IdYSend ? "Send" : "Recv",
		            ctime(&tm));
		_lwrite(fv->LogFile, buf, strlen(buf));
	}

	switch (yv->YMode) {
	case IdYSend:
		yv->TextFlag = 0;

		// �t�@�C�����M�J�n�O�ɁA"rb �t�@�C����"�������I�ɌĂяo���B(2007.12.20 yutaka)
		//strcpy(ts->YModemRcvCommand, "rb");
		if (ts->YModemRcvCommand[0] != '\0') {
			_snprintf_s(inistr, sizeof(inistr), _TRUNCATE, "%s\015", 
			            ts->YModemRcvCommand);
			YWrite(fv,yv,cv, inistr , strlen(inistr));
		}

		FTSetTimeOut(fv,TimeOutVeryLong);
		break;

	case IdYReceive:
#if 0   // for debug
		strcpy(inistr, "sb -b svnrev.exe lrzsz-0.12.20.tar.gz\r\n");
//		strcpy(inistr, "sb url3.txt url4.txt url5.txt\r\n");
		YWrite(fv,yv,cv, inistr , strlen(inistr));
#endif
		yv->TextFlag = 0;

		YSendNAK(fv,yv,cv);

		break;
	}
}

void YCancel(PFileVar fv, PYVar yv, PComVar cv)
{
	// five cancels & five backspaces per spec
	BYTE cancel[] = { CAN, CAN, CAN, CAN, CAN, BS, BS, BS, BS, BS };

	YWrite(fv,yv,cv, (PCHAR)&cancel, sizeof(cancel));
	yv->YMode = 0; // quit
}

void YTimeOutProc(PFileVar fv, PYVar yv, PComVar cv)
{
	switch (yv->YMode) {
	case IdXSend:
		yv->YMode = 0; // quit
		break;
	case IdXReceive:
		if ((yv->PktNum == 0) && yv->PktNumOffset == 0) 
			YSendNAK(fv,yv,cv);
		else
			YSendNAKTimeout(fv,yv,cv);
		break;
	}
}

// YMODEM�T�[�o����t�@�C������M����ہAProtoParse()����Ăяo�����֐��B
//
// +-------+-------+--------+---------+-----+
// |Header |Block# |1-Block#| Payload | CRC |
// +-------+-------+--------+---------+-----+
//    1       1        1      128/1024   2      byte
//
// return TRUE: �t�@�C����M��
//        FALSE: ��M����
BOOL YReadPacket(PFileVar fv, PYVar yv, PComVar cv)
{
	BYTE b, d;
	int i, c, nak;
	BOOL GetPkt;

	c = YRead1Byte(fv,yv,cv,&b);

	GetPkt = FALSE;

	while ((c>0) && (! GetPkt))
	{
		switch (yv->PktReadMode) {
		case XpktSOH:
			// SOH �� STX ���Ńu���b�N�������܂�B
			if (b==SOH)
			{
				yv->PktIn[0] = b;
				yv->PktReadMode = XpktBLK;
				yv->__DataLen = SOH_DATALEN;
				FTSetTimeOut(fv,yv->TOutShort);
			}
			else if (b==STX)
			{
				yv->PktIn[0] = b;
				yv->PktReadMode = XpktBLK;
				yv->__DataLen = STX_DATALEN;
				FTSetTimeOut(fv,yv->TOutShort);
			}
			else if (b==EOT)
			{
				// EOT��������A1�̃t�@�C����M�������������Ƃ������B
				if (fv->FileOpen) {
					fv->FileOpen = 0;
					_lclose(fv->FileHandle);
					fv->FileHandle = -1;

					if (fv->FileMtime > 0) {
						SetFMtime(fv->FullName, fv->FileMtime);
					}

					// 1��ڂ�EOT�ɑ΂���NAK��Ԃ�
					b = NAK;
					YWrite(fv,yv,cv,&b, 1);
					return TRUE;
				}

				initialize_file_info(fv, yv);

				// EOT�ɑ΂���ACK��Ԃ�
				b = ACK;
				YWrite(fv,yv,cv,&b, 1);

				// ���̃t�@�C�����M�𑣂����߁A'C'�𑗂�B
				YSendNAK(fv,yv,cv);

				return TRUE;
			}
			else {
				/* flush comm buffer */
				cv->InBuffCount = 0;
				cv->InPtr = 0;
				return TRUE;
			}
			break;
		case XpktBLK:
			yv->PktIn[1] = b;
			yv->PktReadMode = XpktBLK2;
			FTSetTimeOut(fv,yv->TOutShort);
			break;
		case XpktBLK2:
			nak = 1;
			yv->PktIn[2] = b;
			if ((b ^ yv->PktIn[1]) == 0xff) {
				nak = 0;
				if (yv->SendFileInfo) {
					if (yv->PktIn[1] == (BYTE)(yv->PktNum + 1))  // ���̃u���b�N�ԍ���
						nak = 0;
				}
			}

			if (nak == 0)
			{
				yv->PktBufPtr = 3;
				yv->PktBufCount = yv->__DataLen + yv->CheckLen;
				yv->PktReadMode = XpktDATA;
				FTSetTimeOut(fv,yv->TOutShort);
			}
			else
				YSendNAK(fv,yv,cv);
			break;
		case XpktDATA:
			yv->PktIn[yv->PktBufPtr] = b;
			yv->PktBufPtr++;
			yv->PktBufCount--;
			GetPkt = yv->PktBufCount==0;
			if (GetPkt)
			{
				FTSetTimeOut(fv,yv->TOutLong);
				yv->PktReadMode = XpktSOH;
			}
			else
				FTSetTimeOut(fv,yv->TOutShort);
			break;
		}

		if (! GetPkt) c = YRead1Byte(fv,yv,cv,&b);
	}

	if (! GetPkt) return TRUE;

	GetPkt = YCheckPacket(yv, yv->__DataLen);
	if (! GetPkt)
	{
		YSendNAK(fv,yv,cv);
		return TRUE;
	}

	// �I�[���[���Ȃ�΁A�S�t�@�C����M�̊����������B
	if (yv->PktIn[1] == 0x00 && yv->PktIn[2] == 0xFF &&
		yv->SendFileInfo == 0
		) {
		c = yv->__DataLen;
		while ((c>0) && (yv->PktIn[2+c]==0x00))
			c--;
		if (c == 0) {
		  b = ACK;
		  YWrite(fv,yv,cv,&b, 1);
		  fv->Success = TRUE;
		  return FALSE;
		}
	}

	d = yv->PktIn[1] - yv->PktNum;
	if (d>1)
	{
		YCancel(fv,yv,cv);
		return FALSE;
	}

	/* send ACK */
	b = ACK;
	YWrite(fv,yv,cv,&b, 1);
	yv->NAKMode = YnakC;
	yv->NAKCount = 10;

	// �d�����Ă���ꍇ�́A�������Ȃ��B
	if (yv->SendFileInfo &&
		yv->PktIn[1] == (BYTE)(yv->PktNum)) { 
		return TRUE;
	}

	yv->PktNum = yv->PktIn[1];

	// YMODEM�̏ꍇ�Ablock#0���u�t�@�C�����v�ƂȂ�B
	if (d == 0 &&
		yv->SendFileInfo == 0) {
		long modtime;
		long bytes_total;
		int mode;
		int ret;
		BYTE *p;
		char *name, *nameend;

		p = (BYTE *)malloc(yv->__DataLen + 1);
		memset(p, 0, yv->__DataLen + 1);
		memcpy(p, &(yv->PktIn[3]), yv->__DataLen);
		name = p;
		strncpy_s(&(fv->FullName[fv->DirLen]),
		          sizeof(fv->FullName) - fv->DirLen, name,
		          _TRUNCATE);
		if (!FTCreateFile(fv)) {
			free(p);
			return FALSE;
		}
		nameend = name + 1 + strlen(name);
		if (*nameend) {
			ret = sscanf(nameend, "%ld%lo%o", &bytes_total, &modtime, &mode);
			if (ret >= 1) {
				fv->FileSize = bytes_total;
				yv->RecvFilesize = TRUE;
			}
			if (ret >= 2) {
				fv->FileMtime = modtime;
			}
		}

		SetDlgItemText(fv->HWin, IDC_PROTOFNAME, name);

		yv->SendFileInfo = 1;

		// ���̃t�@�C�����M�𑣂����߁A'C'�𑗂�B
		YSendNAK(fv,yv,cv);

		free(p);
		return TRUE;
	}

	if (yv->PktNum==0)
		yv->PktNumOffset = yv->PktNumOffset + 256;

	c = yv->__DataLen;
	if (yv->TextFlag>0)
		while ((c>0) && (yv->PktIn[2+c]==0x1A))
			c--;

	// �ŏI�u���b�N�̗]���ȃf�[�^����������
	if (yv->RecvFilesize && fv->ByteCount + c > fv->FileSize) {
		c = fv->FileSize - fv->ByteCount;
	}

	if (yv->TextFlag>0)
		for (i = 0 ; i <= c-1 ; i++)
		{
			b = yv->PktIn[3+i];
			if ((b==LF) && (! yv->CRRecv))
				_lwrite(fv->FileHandle,"\015",1);
			if (yv->CRRecv && (b!=LF))
				_lwrite(fv->FileHandle,"\012",1);
			yv->CRRecv = b==CR;
			_lwrite(fv->FileHandle,&b,1);
		}
	else
		_lwrite(fv->FileHandle, &(yv->PktIn[3]), c);

	fv->ByteCount = fv->ByteCount + c;

	SetDlgNum(fv->HWin, IDC_PROTOPKTNUM, yv->PktNumOffset+yv->PktNum);
	SetDlgNum(fv->HWin, IDC_PROTOBYTECOUNT, fv->ByteCount);
	SetDlgTime(fv->HWin, IDC_PROTOELAPSEDTIME, fv->StartTime, fv->ByteCount);

	FTSetTimeOut(fv,yv->TOutLong);

	return TRUE;
}

// �t�@�C�����M(local-to-remote)���ɁAYMODEM�T�[�o����f�[�^�������Ă����Ƃ��ɌĂяo�����B
BOOL YSendPacket(PFileVar fv, PYVar yv, PComVar cv)
{
	// If current buffer is empty.
	if (0 == yv->PktBufCount)
	{
		// Main read loop.
		BOOL continue_read = TRUE;
		while (continue_read)
		{
			BYTE isym = 0;
			int is_success = YRead1Byte(fv, yv, cv, &isym);
			if (0 == is_success) return TRUE;

			// Analyze responce.
			switch (isym)
			{
			case ACK:
				// 1��ڂ�EOT���M���ACK��M�ŁA�u1�t�@�C�����M�v�̏I���Ƃ���B
				// If we already send EOT, ACK means that client confirms it.
				if (yv->SendEot)
				{
					// Reset the flag.
					yv->SendEot = 0;

					// ���M�t�@�C�����c���Ă��Ȃ��ꍇ�́A�u�S�Ẵt�@�C����]���I���v��ʒm����B
					if (!GetNextFname(fv))
					{
						// If it is the last file.
						yv->LastSendEot = 1;
						break;
					}
					else
					{
						// Process with next file.
						initialize_file_info(fv, yv);
					}
				}

				// If client confirms that last (empty) packed was received.
				// �������M����t�@�C�����Ȃ��ꍇ�́A����I���B
				if (!fv->FileOpen)
				{
					fv->Success = TRUE;
					yv->LastMessage = isym;
					return FALSE;
				}
				// ���̃u���b�N�𑗂�
				else if (yv->PktNumSent == (BYTE)(yv->PktNum + 1))
				{
					// �u���b�N0�i�t�@�C�����j���M��́AACK �� 'C' ��A�����Ď�M���邱�Ƃ�
					// �Ȃ��Ă��邽�߁A����'C'��҂B(2010.6.20 yutaka)
					if ((yv->PktNum==0) && (yv->PktNumOffset==0))
					{
						// It is an ACK for file info, wait for 'C' by some reason (?).
						// ���M�ς݃t���Oon
						yv->SendFileInfo = 1;
						continue_read = TRUE;
						break;
					}
					yv->PktNum = yv->PktNumSent;
					if (0 == yv->PktNum)
						yv->PktNumOffset = yv->PktNumOffset + 256;
					continue_read = FALSE;
				}
				break;

			case NAK:
				// 1��ڂ�EOT���M���NAK��M�ŁA�Ō�"EOT"�𑗂�B
				if (yv->SendEot)
				{
					yv->PktNum = yv->PktNumSent;
					if (0 == yv->PktNum)
						yv->PktNumOffset = yv->PktNumOffset + 256;
				}

				continue_read = FALSE;
				break;

			case CAN:
				// ���O�� CAN �̏ꍇ�̓L�����Z��
				if (yv->LastMessage == CAN) {
					fv->Success = FALSE;       // failure
					return FALSE;
				}
				break;

			case 'C':
			case 'G':
				// 'C'���󂯎��ƁA�u���b�N�̑��M���J�n����B
				if ((0 == yv->PktNum) && (0 == yv->PktNumOffset))
				{
					// �t�@�C����񑗐M��AACK -> 'C' �Ǝ�M�����̂ŁA���̃u���b�N�𑗐M����B
					if (yv->SendFileInfo)
					{
						yv->PktNum = yv->PktNumSent;
						if (0 == yv->PktNum)
							yv->PktNumOffset = yv->PktNumOffset + 256;
					}

					continue_read = FALSE;
				}
				else if (yv->LastSendEot)
				{
					continue_read = FALSE;
				}
				else
				{
					// TODO: analyze else branch.
				}
				break;

			default:
				assert(0);
				break;
			}
			yv->LastMessage = isym;
		}

		// reset timeout timer
		FTSetTimeOut(fv, TimeOutVeryLong);
#if 0
		// �㑱�̃T�[�o����̃f�[�^��ǂݎ̂Ă�B
		do
		{
			lastrx = firstch;
			i = YRead1Byte(fv,yv,cv,&b);
			if (i != 0) {
				firstch = b;
				if (firstch == CAN && lastrx == CAN) {
					// CAN(0x18)���A�����Ă���ƁA�t�@�C�����M�̎��s�ƌ��Ȃ��B
					// ���Ƃ��΁A�T�[�o�ɓ����̃t�@�C�������݂���ꍇ�ȂǁB
					// (2010.3.23 yutaka)
					fv->Success = FALSE;       // failure
					return FALSE;
				}
			}
		} while (i != 0);
#endif

		//================================
		// Last packet case.
		//================================
		// �I�[���[���̃u���b�N�𑗐M���āA�����t�@�C�����Ȃ����Ƃ�m�点��B
		if (yv->LastSendEot)
		{
			WORD Check;
			// Always 128 bytes for the last packet.
			WORD last_packet_size = SOH_DATALEN;
			int i;

			// Clear the flag.
			yv->LastSendEot = 0;

			yv->__DataLen = last_packet_size;
			yv->PktOut[0] = SOH;
			yv->PktOut[1] = 0;
			yv->PktOut[2] = ~0;

			i = 0;
			while (i < last_packet_size)
			{
				yv->PktOut[i+3] = 0x00;
				i++;
			}

			Check = YCalcCheck(yv, yv->PktOut, last_packet_size);
			// TODO: move checksum calculation to a function.
			if (1 == yv->CheckLen)
			{
				yv->PktOut[last_packet_size + 3] = (BYTE)Check;
			}
			else
			{
				yv->PktOut[last_packet_size + 3] = HIBYTE(Check);
				yv->PktOut[last_packet_size + 4] = LOBYTE(Check);
			}

			// TODO: remove magic number.
			yv->PktBufCount = 3 + last_packet_size + yv->CheckLen;
		}

		//================================
		// First or 256th packet case.
		//================================

		// Start a new sequence.
		else if (yv->PktNumSent==yv->PktNum)
		{
			 /* make a new packet */
			BYTE *dataptr = &yv->PktOut[3];
			int eot = 0;  // End Of Transfer
			WORD current_packet_size = yv->DataLen;

			if (SOH_DATALEN == current_packet_size)
				yv->PktOut[0] = SOH;
			else
				yv->PktOut[0] = STX;
			yv->PktOut[1] = yv->PktNumSent;
			yv->PktOut[2] = ~yv->PktNumSent;

			// �u���b�N�ԍ��̃J�E���g�A�b�v�BYMODEM�ł�"0"����J�n����B
			yv->PktNumSent++;

			//================================
			// First packet case.
			//================================
			// �u���b�N0
			// �t�@�C�����̑��M
			if (yv->SendFileInfo == 0)
			{
				int ret, total;
				size_t idx;
				// TODO: remove magic number.
				BYTE buf[1024 + 10];

				// 128 bytes for the first packet.
				current_packet_size = SOH_DATALEN;
				yv->__DataLen = current_packet_size;
				yv->PktOut[0] = SOH;

				// Timestamp.
				fv->FileMtime = GetFMtime(fv->FullName);

				ret = _snprintf_s(buf, sizeof(buf), _TRUNCATE, "%s",
				                  &(fv->FullName[fv->DirLen]));
				// NULL-terminated string.
				buf[ret] = 0x00;
				total = ret + 1;

				ret = _snprintf_s(&(buf[total]), sizeof(buf) - total, _TRUNCATE, "%lu %lo %o",
				                  fv->FileSize, fv->FileMtime, 0644|_S_IFREG);
				total += ret;

				// if bloack0 is long, expand to 1024 bytes.
				if (total > SOH_DATALEN) {
					current_packet_size = STX_DATALEN;
					yv->__DataLen = current_packet_size;
					yv->PktOut[0] = STX;
				}

				// Padding.
				idx = total;
				while (idx <= current_packet_size)
				{
					buf[idx] = 0x00;
					++idx;
				}

				// �f�[�^�R�s�[
				memcpy(dataptr, buf, current_packet_size);
			}

			//================================
			// 256th packet case.
			//================================

			else
			{
				BYTE fsym = 0;
				size_t idx = 1;

				yv->__DataLen = current_packet_size;

				while ((idx <= current_packet_size) && fv->FileOpen &&
				       (1 == _lread(fv->FileHandle, &fsym, 1)))
				{
					// TODO: remove magic number.
					yv->PktOut[2 + idx] = fsym;
					++idx;
					fv->ByteCount++;
				}

				// No bytes were read.
				if (1 == idx)
				{
					// Close file handle.
					if (fv->FileOpen)
					{
						_lclose(fv->FileHandle);
						fv->FileHandle = 0;
						fv->FileOpen = FALSE;
					}

					// Send EOT.
					eot = 1;
				}
				else
				{
					// Padding.
					while (idx <= current_packet_size)
					{
						// TODO: remove magic number.
						yv->PktOut[2 + idx] = 0x1A;
						++idx;
					}
				}
			}

			// �f�[�^�u���b�N
			if (0 == eot)
			{
				// Add CRC if not End-of-Tranfer.
				WORD Check = YCalcCheck(yv, yv->PktOut, current_packet_size);
				// Checksum.
				if (1 == yv->CheckLen)
					yv->PktOut[current_packet_size + 3] = (BYTE)Check;
				else {
					yv->PktOut[current_packet_size + 3] = HIBYTE(Check);
					yv->PktOut[current_packet_size + 4] = LOBYTE(Check);
				}
				// TODO: remove magic number.
				yv->PktBufCount = 3 + current_packet_size + yv->CheckLen;

			}
			else
			{
				// EOT.
				yv->PktOut[0] = EOT;
				yv->PktBufCount = 1;

				// EOT�t���Oon�B����NAK�����҂���B
				yv->SendEot = 1;
				yv->LastSendEot = 0;
			}
		}

		//================================
		// TODO: Analyze resend case.
		//================================

		else
		{
			// Resend packet.
			yv->PktBufCount = 3 + yv->__DataLen + yv->CheckLen;
		}

		// Reset counter.
		yv->PktBufPtr = 0;
	}
#if 0
	/* a NAK or C could have arrived while we were buffering.  Consume it. */
	// �㑱�̃T�[�o����̃f�[�^��ǂݎ̂Ă�B
	do {
		lastrx = firstch;
		i = YRead1Byte(fv,yv,cv,&b);
		if (i != 0) {
			firstch = b;
			if (firstch == CAN && lastrx == CAN) {
				// CAN(0x18)���A�����Ă���ƁA�t�@�C�����M�̎��s�ƌ��Ȃ��B
				// ���Ƃ��΁A�T�[�o�ɓ����̃t�@�C�������݂���ꍇ�ȂǁB
				// (2010.3.23 yutaka)
				fv->Success = FALSE;       // failure
				return FALSE;
			}
		}
	} while (i != 0);
#endif

	// Write bytes to COM.
	while (yv->PktBufCount > 0)
	{
		BYTE osym = yv->PktOut[yv->PktBufPtr];
		int is_success = YWrite(fv, yv, cv, &osym, 1);
		if (is_success > 0)
		{
			--yv->PktBufCount;
			++yv->PktBufPtr;
		}
		else
			break;
	}

	// Update dialog window.
	if (0 == yv->PktBufCount)
	{
		if (0 == yv->PktNumSent)
		{
			SetDlgNum(fv->HWin, IDC_PROTOPKTNUM, yv->PktNumOffset + 256);
		}
		else
		{
			SetDlgNum(fv->HWin, IDC_PROTOPKTNUM, yv->PktNumOffset + yv->PktNumSent);
		}

		SetDlgNum(fv->HWin, IDC_PROTOBYTECOUNT, fv->ByteCount);
		SetDlgPercent(fv->HWin, IDC_PROTOPERCENT, IDC_PROTOPROGRESS,
		              fv->ByteCount, fv->FileSize, &fv->ProgStat);
		SetDlgTime(fv->HWin, IDC_PROTOELAPSEDTIME, fv->StartTime, fv->ByteCount);
	}

	return TRUE;
}
