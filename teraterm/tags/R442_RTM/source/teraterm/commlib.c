/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */
/* IPv6 modification is Copyright (C) 2000, 2001 Jun-ya KATO <kato@win6.jp> */

/* TERATERM.EXE, Communication routines */
#include "teraterm.h"
#include "tttypes.h"
#ifdef TERATERM32
#include "tt_res.h"
#include <process.h>
#else
#include "tt_res16.h"
#include <stdlib.h>
#include <string.h>
#endif

#include "ttcommon.h"
#include "ttwsk.h"
#include "ttlib.h"
#include "ttplug.h" /* TTPLUG */

#include "commlib.h"
#ifdef INET6
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h> /* for _snprintf() */
#include "WSAAsyncGetAddrInfo.h"
#endif /* INET6 */

static SOCKET OpenSocket(PComVar);
static void AsyncConnect(PComVar);
static int CloseSocket(SOCKET);

/* create socket */
static SOCKET OpenSocket(PComVar cv)
{
  cv->s = cv->res->ai_family;
  cv->s = Psocket(cv->res->ai_family, cv->res->ai_socktype, cv->res->ai_protocol);
  return cv->s;
}

/* connect with asynchronous mode */
static void AsyncConnect(PComVar cv)
{
  int Err;
  BOOL BBuf;
  BBuf = TRUE;
  /* set synchronous mode */
  PWSAAsyncSelect(cv->s,cv->HWin,0,0);
  Psetsockopt(cv->s,(int)SOL_SOCKET,SO_OOBINLINE,(char FAR *)&BBuf,sizeof(BBuf));
  /* set asynchronous mode */
  PWSAAsyncSelect(cv->s,cv->HWin,WM_USER_COMMOPEN, FD_CONNECT);
  /* WM_USER_COMMOPEN occurs, CommOpen is called, then CommStart is called */
  Err = Pconnect(cv->s, cv->res->ai_addr, cv->res->ai_addrlen);
  if (Err != 0) {
    Err = PWSAGetLastError();
    if (Err == WSAEWOULDBLOCK)
    {
      /* Do nothing */
    } else if (Err!=0 ) {
      PostMessage(cv->HWin, WM_USER_COMMOPEN,0,
		  MAKELONG(FD_CONNECT,Err));
    }
  }
}

/* close socket */
static int CloseSocket(SOCKET s)
{
  return Pclosesocket(s);
}

#define ErrorCaption "Tera Term: Error"
#define ErrorCantConn "Cannot connect the host"
#define CommInQueSize 8192
#define CommOutQueSize 2048
#define CommXonLim 2048
#define CommXoffLim 2048

#ifdef TERATERM32
#define READENDNAME "ReadEnd"
#define WRITENAME "Write"
#define READNAME "Read"
#define PRNWRITENAME "PrnWrite"

static HANDLE ReadEnd;
static OVERLAPPED wol, rol;
#endif

// Winsock async operation handle
static HANDLE HAsync=0;

BOOL TCPIPClosed = TRUE;

/* Printer port handle for
   direct pass-thru printing */
#ifdef TERATERM32
static HANDLE PrnID = INVALID_HANDLE_VALUE;
#else
static int PrnID = -1;
#endif
static BOOL LPTFlag;

// Initialize ComVar.
// This routine is called only once
// by the initialization procedure of Tera Term.
void CommInit(PComVar cv)
{
  cv->Open = FALSE;
  cv->Ready = FALSE;

// log-buffer variables
  cv->HLogBuf = 0;
  cv->HBinBuf = 0;
  cv->LogBuf = NULL;
  cv->BinBuf = NULL;
  cv->LogPtr = 0;
  cv->LStart = 0;
  cv->LCount = 0;
  cv->BinPtr = 0;
  cv->BStart = 0;
  cv->BCount = 0;
  cv->DStart = 0;
  cv->DCount = 0;
  cv->BinSkip = 0;
  cv->FilePause = 0;
  cv->ProtoFlag = FALSE;
/* message flag */
  cv->NoMsg = 0;
}

/* reset a serial port which is already open */
void CommResetSerial(PTTSet ts, PComVar cv)
{
  DCB dcb;
#ifdef TERATERM32
  DWORD DErr;
  COMMTIMEOUTS ctmo;
#else
  COMSTAT Stat;
  BYTE b;
#endif

  if (! cv->Open ||
      (cv->PortType != IdSerial)) return;

#ifdef TERATERM32
  ClearCommError(cv->ComID,&DErr,NULL);
  SetupComm(cv->ComID,CommInQueSize,CommOutQueSize);
  /* flush input and output buffers */
  PurgeComm(cv->ComID, PURGE_TXABORT | PURGE_RXABORT |
    PURGE_TXCLEAR | PURGE_RXCLEAR);

  memset(&ctmo,0,sizeof(ctmo));
  ctmo.ReadIntervalTimeout = MAXDWORD;
  ctmo.WriteTotalTimeoutConstant = 500;
  SetCommTimeouts(cv->ComID,&ctmo);
#else
  while (GetCommError(cv->ComID, &Stat)!=0) {};
  /* flush input and output buffers */
  FlushComm(cv->ComID,0);
  FlushComm(cv->ComID,1);
#endif
  cv->InBuffCount = 0;
  cv->InPtr = 0;
  cv->OutBuffCount = 0;
  cv->OutPtr = 0;

  cv->DelayPerChar = ts->DelayPerChar;
  cv->DelayPerLine = ts->DelayPerLine;

#ifdef TERATERM32
  memset(&dcb,0,sizeof(DCB));
  dcb.DCBlength = sizeof(DCB);
#else
  GetCommState(cv->ComID,&dcb);
  b = dcb.Id;
  memset(&dcb,0,sizeof(DCB));
  dcb.Id = b;
#endif
  switch (ts->Baud) {
    case IdBaud110: dcb.BaudRate = 110; break;
    case IdBaud300: dcb.BaudRate = 300; break;
    case IdBaud600: dcb.BaudRate = 600; break;
    case IdBaud1200: dcb.BaudRate = 1200; break;
    case IdBaud2400: dcb.BaudRate = 2400; break;
    case IdBaud4800: dcb.BaudRate = 4800; break;
    case IdBaud9600: dcb.BaudRate = 9600; break;
    case IdBaud14400: dcb.BaudRate = 14400; break;
    case IdBaud19200: dcb.BaudRate = 19200; break;
    case IdBaud38400: dcb.BaudRate = 38400; break;
    case IdBaud57600: dcb.BaudRate = 57600; break;
#ifdef TERATERM32
    case IdBaud115200: dcb.BaudRate = 115200; break;
#endif
	// add (2005.11.30 yutaka)
    case IdBaud230400: dcb.BaudRate = 230400; break;
    case IdBaud460800: dcb.BaudRate = 460800; break;
    case IdBaud921600: dcb.BaudRate = 921600; break;
  }
  dcb.fBinary = TRUE;
  switch (ts->Parity) {
    case IdParityEven:
      dcb.fParity = TRUE;
      dcb.Parity = EVENPARITY;
      break;
    case IdParityOdd:
      dcb.fParity = TRUE;
      dcb.Parity = ODDPARITY;
      break;
    case IdParityNone:
      dcb.Parity = NOPARITY;
      break;
  }

#ifdef TERATERM32
  dcb.fDtrControl = DTR_CONTROL_ENABLE;
  dcb.fRtsControl = RTS_CONTROL_ENABLE;
#endif
  switch (ts->Flow) {
    case IdFlowX:
      dcb.fOutX = TRUE;
      dcb.fInX = TRUE;
      dcb.XonLim = CommXonLim;
      dcb.XoffLim = CommXoffLim;
      dcb.XonChar = XON;
      dcb.XoffChar = XOFF;
      break;
    case IdFlowHard:
      dcb.fOutxCtsFlow = TRUE;
#ifdef TERATERM32
      dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
#else
      dcb.CtsTimeout = 30;
      dcb.fRtsflow = TRUE;
#endif
      break;
  }

  switch (ts->DataBit) {
    case IdDataBit7: dcb.ByteSize = 7; break;
    case IdDataBit8: dcb.ByteSize = 8; break;
  }
  switch (ts->StopBit) {
    case IdStopBit1: dcb.StopBits = ONESTOPBIT; break;
    case IdStopBit2: dcb.StopBits = TWOSTOPBITS; break;
  }

#ifdef TERATERM32
  SetCommState(cv->ComID, &dcb);

  /* enable receive request */
  SetCommMask(cv->ComID,0);
  SetCommMask(cv->ComID,EV_RXCHAR);
#else
  SetCommState(&dcb);

  /* enable receive request */
  SetCommEventMask(cv->ComID,0);
  SetCommEventMask(cv->ComID,EV_RXCHAR);
#endif
}

void CommOpen(HWND HW, PTTSet ts, PComVar cv)
{
  WORD COMFlag;
#ifndef INET6
  int Err;
#endif /* INET6 */
  char ErrMsg[21];
  char P[50];

  MSG Msg;
#ifdef INET6
  ADDRINFO hints;
  char pname[NI_MAXSERV];
#else
  char HEntBuff[MAXGETHOSTSTRUCT];
  u_long addr;
  SOCKADDR_IN saddr;
#endif /* INET6 */

  BOOL InvalidHost;
#ifndef INET6
  BOOL BBuf;
#endif /* INET6 */

  /* initialize ComVar */
  cv->InBuffCount = 0;
  cv->InPtr = 0;
  cv->OutBuffCount = 0;
  cv->OutPtr = 0;
  cv->HWin = HW;
  cv->Ready = FALSE;
  cv->Open = FALSE;
  cv->PortType = ts->PortType;
  cv->ComPort = 0;
  cv->RetryCount = 0;
#ifdef INET6
  cv->RetryWithOtherProtocol = TRUE;
#endif /* INET6 */
  cv->s = INVALID_SOCKET;
#ifdef TERATERM32
  cv->ComID = INVALID_HANDLE_VALUE;
#else
  cv->ComID = -1;
#endif
  cv->CanSend = TRUE;
  cv->RRQ = FALSE;
  cv->SendKanjiFlag = FALSE;
  cv->SendCode = IdASCII;
  cv->EchoKanjiFlag = FALSE;
  cv->EchoCode = IdASCII;
  cv->Language = ts->Language;
  cv->CRSend = ts->CRSend;
  cv->KanjiCodeEcho = ts->KanjiCode;
  cv->JIS7KatakanaEcho = ts->JIS7Katakana;
  cv->KanjiCodeSend = ts->KanjiCodeSend;
  cv->JIS7KatakanaSend = ts->JIS7KatakanaSend;
  cv->KanjiIn = ts->KanjiIn;
  cv->KanjiOut = ts->KanjiOut;
  cv->RussHost = ts->RussHost;
  cv->RussClient = ts->RussClient;
  cv->DelayFlag = TRUE;
  cv->DelayPerChar = ts->DelayPerChar;
  cv->DelayPerLine = ts->DelayPerLine;
  cv->TelBinRecv = FALSE;
  cv->TelBinSend = FALSE;
  cv->TelFlag = FALSE;
  cv->TelMode = FALSE;
  cv->IACFlag = FALSE;
  cv->TelCRFlag = FALSE;
  cv->TelCRSend = FALSE;
  cv->TelCRSendEcho = FALSE;
  cv->TelAutoDetect = TRUE; /* TTPLUG */
  cv->Locale = ts->Locale;
  cv->CodePage = &ts->CodePage;

  if ((ts->PortType!=IdSerial) && (strlen(ts->HostName)==0))
  {
    PostMessage(cv->HWin, WM_USER_COMMNOTIFY, 0, FD_CLOSE);
    return;
  }

  switch (ts->PortType) {
    case IdTCPIP:
      cv->TelFlag = (ts->Telnet > 0);
      if (! LoadWinsock())
      {
	if (cv->NoMsg==0)
	  MessageBox(cv->HWin,"Cannot use winsock",ErrorCaption,
	    MB_TASKMODAL | MB_ICONEXCLAMATION);
	InvalidHost = TRUE;
      }
      else {
        TTXOpenTCP(); /* TTPLUG */
	cv->Open = TRUE;
#ifdef INET6
	/* resolving address */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = ts->ProtocolFamily;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	_snprintf(pname, sizeof(pname), "%d", ts->TCPPort);

	HAsync = WSAAsyncGetAddrInfo(HW, WM_USER_GETHOST,
				     ts->HostName, pname, &hints, &cv->res0);
	if (HAsync == 0)
	  InvalidHost = TRUE;
	else {
	    cv->ComPort = 1; // set "getting host" flag
			     //  (see CVTWindow::OnSysCommand())
	    do {
	      if (GetMessage(&Msg,0,0,0))
	      {
		if ((Msg.hwnd==HW) &&
		    ((Msg.message == WM_SYSCOMMAND) &&
		     ((Msg.wParam & 0xfff0) == SC_CLOSE) ||
		     (Msg.message == WM_COMMAND) &&
		     (LOWORD(Msg.wParam) == ID_FILE_EXIT) ||
		     (Msg.message == WM_CLOSE)))
		{ /* Exit when the user closes Tera Term */
		  PWSACancelAsyncRequest(HAsync);
		  CloseHandle(HAsync);
		  HAsync = 0;
		  cv->ComPort = 0; // clear "getting host" flag
		  PostMessage(HW,Msg.message,Msg.wParam,Msg.lParam);
		  return;
		}
		if (Msg.message != WM_USER_GETHOST)
		{ /* Prosess messages */
		  TranslateMessage(&Msg);
		  DispatchMessage(&Msg);
		}
	      }
	      else {
		return;
	      }
	    } while (Msg.message!=WM_USER_GETHOST);
	    cv->ComPort = 0; // clear "getting host" flag
    	CloseHandle(HAsync);
	    HAsync = 0;
	    InvalidHost = WSAGETASYNCERROR(Msg.lParam) != 0;
	}
      } /* if (!LoadWinsock()) */

      if (InvalidHost)
      {
	if (cv->NoMsg==0)
	  MessageBox(cv->HWin,"Invalid host",ErrorCaption,
	      MB_TASKMODAL | MB_ICONEXCLAMATION);
	goto BreakSC;
      }
      for (cv->res = cv->res0; cv->res; cv->res = cv->res->ai_next) {
	cv->s =  OpenSocket(cv);
	if (cv->s == INVALID_SOCKET) {
	  CloseSocket(cv->s);
	  continue;
	}
	/* start asynchronous connect */
	AsyncConnect(cv);
	break; /* break for-loop immediately */
      }
      break;
#else
	if ((ts->HostName[0] >= 0x30) && (ts->HostName[0] <= 0x39))
	{
	  addr = Pinet_addr(ts->HostName);
	  InvalidHost = (addr == 0xffffffff);
	}
	else {
	  HAsync = PWSAAsyncGetHostByName(HW,WM_USER_GETHOST,
	  ts->HostName,HEntBuff,sizeof(HEntBuff));
	  if (HAsync == 0)
	    InvalidHost = TRUE;
	  else {
	    cv->ComPort = 1; // set "getting host" flag
			     //  (see CVTWindow::OnSysCommand())
	    do {
	      if (GetMessage(&Msg,0,0,0))
	      {
		if ((Msg.hwnd==HW) &&
		    ((Msg.message == WM_SYSCOMMAND) &&
		     ((Msg.wParam & 0xfff0) == SC_CLOSE) ||
		     (Msg.message == WM_COMMAND) &&
		     (LOWORD(Msg.wParam) == ID_FILE_EXIT) ||
		     (Msg.message == WM_CLOSE)))
		{ /* Exit when the user closes Tera Term */
		  PWSACancelAsyncRequest(HAsync);
		  HAsync = 0;
		  cv->ComPort = 0; // clear "getting host" flag
		  PostMessage(HW,Msg.message,Msg.wParam,Msg.lParam);
		  return;
		}
		if (Msg.message != WM_USER_GETHOST)
		{ /* Prosess messages */
		  TranslateMessage(&Msg);
		  DispatchMessage(&Msg);
		}
	      }
	      else {
		return;
	      }
	    } while (Msg.message!=WM_USER_GETHOST);
	    cv->ComPort = 0; // clear "getting host" flag
	    HAsync = 0;
	    InvalidHost = WSAGETASYNCERROR(Msg.lParam) != 0;
	    if (! InvalidHost)
	    {
	      if (((PHOSTENT)HEntBuff)->h_addr_list != NULL)
		memcpy(&addr,
		  ((PHOSTENT)HEntBuff)->h_addr_list[0],sizeof(addr));
	      else
		InvalidHost = TRUE;
	    }
	  }

	}

	if (InvalidHost)
	{
	  if (cv->NoMsg==0)
	    MessageBox(cv->HWin,"Invalid host",ErrorCaption,
	      MB_TASKMODAL | MB_ICONEXCLAMATION);
	}
	else {
	  cv->s= Psocket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	  if (cv->s==INVALID_SOCKET)
	  {
	    InvalidHost = TRUE;
	    if (cv->NoMsg==0)
	      MessageBox(cv->HWin,ErrorCantConn,ErrorCaption,
		MB_TASKMODAL | MB_ICONEXCLAMATION);
	  }
	  else {
	    BBuf = TRUE;
	    Psetsockopt(cv->s,(int)SOL_SOCKET,SO_OOBINLINE,(char FAR *)&BBuf,sizeof(BBuf));

	    PWSAAsyncSelect(cv->s,cv->HWin,WM_USER_COMMOPEN, FD_CONNECT);
	    saddr.sin_family = AF_INET;
	    saddr.sin_port = Phtons(ts->TCPPort);
	    saddr.sin_addr.s_addr = addr;
	    memset(saddr.sin_zero,0,8);

	    Err = Pconnect(cv->s,(LPSOCKADDR)&saddr,sizeof(saddr));
	    if (Err!=0 ) Err = PWSAGetLastError();
	    if (Err==WSAEWOULDBLOCK )
	    {
	      /* Do nothing */
	    }
	    else if (Err!=0 )
	      PostMessage(cv->HWin, WM_USER_COMMOPEN,0,
			  MAKELONG(FD_CONNECT,Err));
	  }
	}
      }
      break;
#endif /* INET6 */

    case IdSerial:
      strcpy(P,"COM");
      uint2str(ts->ComPort,&P[3],2);
#ifdef TERATERM32
      strcpy(ErrMsg,P);
      strcpy(P,"\\\\.\\");
      strcat(P,ErrMsg);
      cv->ComID =
	CreateFile(P,GENERIC_READ | GENERIC_WRITE,
		   0,NULL,OPEN_EXISTING,
		   FILE_FLAG_OVERLAPPED,NULL);
      if (cv->ComID == INVALID_HANDLE_VALUE )
      {
#else
      cv->ComID = OpenComm(P,CommInQueSize,CommOutQueSize);
      if (cv->ComID <0)
      {
#endif
	strcpy(ErrMsg,"Cannot open ");
#ifdef TERATERM32
	strcat(ErrMsg,&P[4]);
#else
	strcat(ErrMsg,P);
#endif
	if (cv->NoMsg==0)
	  MessageBox(cv->HWin,ErrMsg,ErrorCaption,
	    MB_TASKMODAL | MB_ICONEXCLAMATION);
	InvalidHost = TRUE;
      }
      else {
	cv->Open = TRUE;
	cv->ComPort = ts->ComPort;
	CommResetSerial(ts,cv);

	/* notify to VT window that Comm Port is open */
	PostMessage(cv->HWin, WM_USER_COMMOPEN, 0, 0);
#ifndef TERATERM32
	// disable comm notification
	EnableCommNotification(cv->ComID,0,-1,-1);
#endif
	InvalidHost = FALSE;

	COMFlag = GetCOMFlag();
	COMFlag = COMFlag | (1 << (ts->ComPort-1));
	SetCOMFlag(COMFlag);
      }
      break; /* end of "case IdSerial:" */

    case IdFile:
#ifdef TERATERM32
      cv->ComID = CreateFile(ts->HostName,GENERIC_READ,0,NULL,
	OPEN_EXISTING,0,NULL);
      InvalidHost = (cv->ComID == INVALID_HANDLE_VALUE);
#else
      cv->ComID = _lopen(ts->HostName,0);
      InvalidHost = (cv->ComID<=0);
#endif
      if (InvalidHost)
      {
	if (cv->NoMsg==0)
	  MessageBox(cv->HWin,"Cannot open file",ErrorCaption,
	    MB_TASKMODAL | MB_ICONEXCLAMATION);
      }
      else {
	cv->Open = TRUE;
	PostMessage(cv->HWin, WM_USER_COMMOPEN, 0, 0);
      }
      break;
  } /* end of "switch" */

#ifdef INET6
BreakSC:
#endif /* INET6 */
  if (InvalidHost)
  {
    PostMessage(cv->HWin, WM_USER_COMMNOTIFY, 0, FD_CLOSE);
    if ( (ts->PortType==IdTCPIP) && cv->Open )
    {
      if ( cv->s!=INVALID_SOCKET ) Pclosesocket(cv->s);
      FreeWinsock();
    }
    return;
  }
}

#ifdef TERATERM32
void CommThread(void *arg)
{
  DWORD Evt;
  PComVar cv = (PComVar)arg;
  DWORD DErr;
  HANDLE REnd;
  char Temp[20];

  strcpy(Temp,READENDNAME);
  uint2str(cv->ComPort,&Temp[strlen(Temp)],2);	
  REnd = OpenEvent(EVENT_ALL_ACCESS,FALSE, Temp);
  while (TRUE) {
    if (WaitCommEvent(cv->ComID,&Evt,NULL))
    {
      if (! cv->Ready) _endthread();
      if (! cv->RRQ)
	PostMessage(cv->HWin, WM_USER_COMMNOTIFY, 0, FD_READ);
      WaitForSingleObject(REnd,INFINITE);
    }
    else {
      if (! cv->Ready) _endthread();
      ClearCommError(cv->ComID,&DErr,NULL);
    }
  }
}
#endif

void CommStart(PComVar cv, LONG lParam)
{
  char ErrMsg[31];
#ifdef TERATERM32
  char Temp[20];
  char Temp2[3];
#else
  COMSTAT Stat;
#endif

  if (! cv->Open ) return;
  if ( cv->Ready ) return;
  switch (cv->PortType) {
    case IdTCPIP:
      ErrMsg[0] = 0;
      switch (HIWORD(lParam)) {
	case WSAECONNREFUSED:
	  strcpy(ErrMsg,"Connection refused"); break;
	case WSAENETUNREACH:
	  strcpy(ErrMsg,"Network cannot be reached"); break;
	case WSAETIMEDOUT:
	  strcpy(ErrMsg,"Connection timed out"); break;
	default:
	  strcpy(ErrMsg,ErrorCantConn);
      }
      if (HIWORD(lParam)>0)
      {
#ifdef INET6
	/* connect() failed */
	if (cv->res->ai_next != NULL) {
	  /* try to connect with other protocol */
	  CloseSocket(cv->s);
	  for (cv->res = cv->res->ai_next; cv->res; cv->res = cv->res->ai_next) {
	    cv->s = OpenSocket(cv);
	    if (cv->s == INVALID_SOCKET) {
	      CloseSocket(cv->s);
	      continue;
	    }
	    AsyncConnect(cv);
	    cv->Ready = FALSE;
	    cv->RetryWithOtherProtocol = TRUE; /* retry with other procotol */
	    return;
	  }
	} else {
	  /* trying with all protocol family are failed */
	  if (cv->NoMsg==0)
	    MessageBox(cv->HWin,ErrMsg,ErrorCaption,
		       MB_TASKMODAL | MB_ICONEXCLAMATION);
	  PostMessage(cv->HWin, WM_USER_COMMNOTIFY, 0, FD_CLOSE);
	  cv->RetryWithOtherProtocol = FALSE;
	  return;
	}
#else
	if (cv->NoMsg==0)
	  MessageBox(cv->HWin,ErrMsg,ErrorCaption,
	    MB_TASKMODAL | MB_ICONEXCLAMATION);
	PostMessage(cv->HWin, WM_USER_COMMNOTIFY, 0, FD_CLOSE);
	return;
#endif /* INET6 */
      }

#ifdef INET6
      /* here is connection established */
      cv->RetryWithOtherProtocol = FALSE;
#endif /* INET6 */
      PWSAAsyncSelect(cv->s,cv->HWin,WM_USER_COMMNOTIFY, FD_READ | FD_OOB | FD_CLOSE);
      TCPIPClosed = FALSE;
      break;

    case IdSerial:
#ifdef TERATERM32
      uint2str(cv->ComPort,Temp2,2);
      strcpy(Temp,READENDNAME);
      strcat(Temp,Temp2);
      ReadEnd = CreateEvent(NULL,FALSE,FALSE,Temp);
      strcpy(Temp,WRITENAME);
      strcat(Temp,Temp2);
      memset(&wol,0,sizeof(OVERLAPPED));
      wol.hEvent = CreateEvent(NULL,TRUE,TRUE,Temp);
      strcpy(Temp,READNAME);
      strcat(Temp,Temp2);
      memset(&rol,0,sizeof(OVERLAPPED));
      rol.hEvent = CreateEvent(NULL,TRUE,FALSE,Temp);

      /* create the receiver thread */
#ifdef __WATCOMC__
      if (_beginthread(CommThread,8192,cv) == -1)
#else
      if (_beginthread(CommThread,0,cv) == -1)
#endif
	MessageBox(cv->HWin,"Can't create thread",ErrorCaption,
		   MB_TASKMODAL | MB_ICONEXCLAMATION);
#else
      // flush input que
      while (GetCommError(cv->ComID, &Stat)!=0) {};
      FlushComm(cv->ComID,1);
      // enable receive request
      SetCommEventMask(cv->ComID,EV_RXCHAR);
      EnableCommNotification(cv->ComID,cv->HWin,-1,-1);
      GetCommEventMask(cv->ComID,EV_RXCHAR);
#endif
      break;
    case IdFile: cv->RRQ = TRUE; break;
  }
  cv->Ready = TRUE;
}

BOOL CommCanClose(PComVar cv)
{ // check if data remains in buffer
  if (! cv->Open) return TRUE;
  if (cv->InBuffCount>0) return FALSE;
  if ((cv->HLogBuf!=NULL) &&
      ((cv->LCount>0) ||
       (cv->DCount>0))) return FALSE;
  if ((cv->HBinBuf!=NULL) &&
      (cv->BCount>0)) return FALSE;
  return TRUE;
}

void CommClose(PComVar cv)
{
  WORD COMFlag;

  if ( ! cv->Open ) return;
  cv->Open = FALSE;

  /* disable event message posting & flush buffer */
  cv->RRQ = FALSE;
  cv->Ready = FALSE;
  cv->InPtr = 0;
  cv->InBuffCount = 0;
  cv->OutPtr = 0;
  cv->OutBuffCount = 0;

  /* close port & release resources */
  switch (cv->PortType) {
    case IdTCPIP:
      if (HAsync!=0)
	PWSACancelAsyncRequest(HAsync);
      HAsync = 0;
#ifdef INET6
      freeaddrinfo(cv->res0);
#endif /* INET6 */
      if ( cv->s!=INVALID_SOCKET )
	Pclosesocket(cv->s);
      cv->s = INVALID_SOCKET;
      TTXCloseTCP(); /* TTPLUG */
      FreeWinsock();
      break;
    case IdSerial:
#ifdef TERATERM32
      if ( cv->ComID != INVALID_HANDLE_VALUE )
      {
	CloseHandle(ReadEnd);
	CloseHandle(wol.hEvent);
	CloseHandle(rol.hEvent);
	PurgeComm(cv->ComID,
		  PURGE_TXABORT | PURGE_RXABORT |
		  PURGE_TXCLEAR | PURGE_RXCLEAR);
	EscapeCommFunction(cv->ComID,CLRDTR);
	SetCommMask(cv->ComID,0);
	CloseHandle(cv->ComID);
#else
      if (cv->ComID >= 0)
      {
	FlushComm(cv->ComID,0);
	FlushComm(cv->ComID,1);
	EscapeCommFunction(cv->ComID,CLRDTR);
	EnableCommNotification(cv->ComID,0,-1,-1) ;
	CloseComm(cv->ComID);
#endif
	COMFlag = GetCOMFlag();
	COMFlag = COMFlag & ~(1 << (cv->ComPort-1));
	SetCOMFlag(COMFlag); 
      }
      break;
    case IdFile:
#ifdef TERATERM32
      if (cv->ComID != INVALID_HANDLE_VALUE)
	CloseHandle(cv->ComID);
      break;
  }
  cv->ComID = INVALID_HANDLE_VALUE;
#else
      if (cv->ComID >= 0)
	_lclose(cv->ComID);
      break;
  }
  cv->ComID = -1;
#endif
  cv->PortType = 0;
}

void CommProcRRQ(PComVar cv)
{
#ifndef TERATERM32
  COMSTAT Stat;
#endif

  if ( ! cv->Ready ) return;
  /* disable receive request */
  switch (cv->PortType) {
    case IdTCPIP:
      if (! TCPIPClosed)
	PWSAAsyncSelect(cv->s,cv->HWin,WM_USER_COMMNOTIFY, FD_OOB | FD_CLOSE);
      break;
    case IdSerial:
#ifndef TERATERM32
      EnableCommNotification(cv->ComID,0,-1,-1);
      while (GetCommError(cv->ComID, &Stat)!=0) {};
#endif
      break;
  }
  cv->RRQ = TRUE;
  CommReceive(cv);
}

void CommReceive(PComVar cv)
{
#ifdef TERATERM32
  DWORD C;
  DWORD DErr;
#else
  int C;
  COMSTAT Stat;
#endif

  if (! cv->Ready || ! cv->RRQ ||
      (cv->InBuffCount>=InBuffSize)) return;

  /* Compact buffer */
  if ((cv->InBuffCount>0) && (cv->InPtr>0))
  {
    memmove(cv->InBuff,&(cv->InBuff[cv->InPtr]),cv->InBuffCount);
    cv->InPtr = 0;
  }

  if (cv->InBuffCount<InBuffSize)
  {
    switch (cv->PortType) {
      case IdTCPIP:
	C = Precv(cv->s, &(cv->InBuff[cv->InBuffCount]),
		 InBuffSize-cv->InBuffCount, 0);
	if (C == SOCKET_ERROR)
	{
	  C = 0;
	  PWSAGetLastError();
	}
	cv->InBuffCount = cv->InBuffCount + C;
	break;
      case IdSerial:
#ifdef TERATERM32
	do {
	  ClearCommError(cv->ComID,&DErr,NULL);
	  if (! ReadFile(cv->ComID,&(cv->InBuff[cv->InBuffCount]),
	    InBuffSize-cv->InBuffCount,&C,&rol))
	  {
	    if (GetLastError() == ERROR_IO_PENDING)
	    {
	      if (WaitForSingleObject(rol.hEvent, 1000) !=
		  WAIT_OBJECT_0)
		C = 0;
	      else
		GetOverlappedResult(cv->ComID,&rol,&C,FALSE);
	    }
	    else
	      C = 0;
	  }
	  cv->InBuffCount = cv->InBuffCount + C;
	} while ((C!=0) && (cv->InBuffCount<InBuffSize));
	ClearCommError(cv->ComID,&DErr,NULL);
#else
	do {
	  C = ReadComm(cv->ComID, &cv->InBuff[cv->InBuffCount], InBuffSize-cv->InBuffCount);
	  C = abs(C);
	  do {} while (GetCommError(cv->ComID, &Stat)!=0);
	  cv->InBuffCount = cv->InBuffCount + C;
	} while ((C!=0) && (cv->InBuffCount<InBuffSize));
#endif
	break;
      case IdFile:
#ifdef TERATERM32
	ReadFile(cv->ComID,&(cv->InBuff[cv->InBuffCount]),
		 InBuffSize-cv->InBuffCount,&C,NULL);
#else
	C = _lread(cv->ComID, &cv->InBuff[cv->InBuffCount],
		   InBuffSize-cv->InBuffCount);
#endif
	cv->InBuffCount = cv->InBuffCount + C;
	break;
    }
  }

  if (cv->InBuffCount==0)
  {
    switch (cv->PortType) {
      case IdTCPIP:
	if (! TCPIPClosed)
	  PWSAAsyncSelect(cv->s,cv->HWin,
	    WM_USER_COMMNOTIFY, FD_READ | FD_OOB | FD_CLOSE);
	break;
      case IdSerial:
#ifdef TERATERM32
	cv->RRQ = FALSE;
	SetEvent(ReadEnd);
	return;
#else
	while (GetCommError(cv->ComID, &Stat)!=0) {};
	EnableCommNotification(cv->ComID,cv->HWin,-1,-1);
	GetCommEventMask(cv->ComID,EV_RXCHAR);
	break;
#endif
      case IdFile:
	PostMessage(cv->HWin, WM_USER_COMMNOTIFY, 0, FD_CLOSE);
	break;
    }
    cv->RRQ = FALSE;

  }
}

void CommSend(PComVar cv)
{
  int delay;
  COMSTAT Stat;
  BYTE LineEnd;
  int C, D, Max;
#ifdef TERATERM32
  DWORD DErr;
#endif

  if ((! cv->Open) || (! cv->Ready))
  {
    cv->OutBuffCount = 0;  
    return;
  }

  if ((cv->OutBuffCount == 0) || (! cv->CanSend)) return;

  /* Max num of bytes to be written */
  switch (cv->PortType) {
    case IdTCPIP:
      if (TCPIPClosed) cv->OutBuffCount = 0;
      Max = cv->OutBuffCount;
      break;
    case IdSerial:
#ifdef TERATERM32
      ClearCommError(cv->ComID,&DErr,&Stat);
#else
      GetCommError(cv->ComID,&Stat);
#endif
      Max = OutBuffSize - Stat.cbOutQue;
      break;
    case IdFile: Max = cv->OutBuffCount;
      break;
  }

  if ( Max<=0 ) return;
  if ( Max > cv->OutBuffCount ) Max = cv->OutBuffCount;

  C = Max;
  delay = 0;

  if ( cv->DelayFlag && (cv->PortType==IdSerial) )
  {
    if ( cv->DelayPerLine > 0 )
    {
      if ( cv->CRSend==IdCR ) LineEnd = 0x0d;
			 else LineEnd = 0x0a;
      C = 1;
      if ( cv->DelayPerChar==0 )
	while ((C<Max) && (cv->OutBuff[cv->OutPtr+C-1]!=LineEnd))
	  C++;
      if ( cv->OutBuff[cv->OutPtr+C-1]==LineEnd )
	delay = cv->DelayPerLine;
      else delay = cv->DelayPerChar;
    }
    else if ( cv->DelayPerChar > 0 )
    {
      C = 1;
      delay = cv->DelayPerChar;
    }
  }

  /* Write to comm driver/Winsock */
  switch (cv->PortType) {
    case IdTCPIP:
      D = Psend(cv->s, &(cv->OutBuff[cv->OutPtr]), C, 0);
      if ( D==SOCKET_ERROR ) /* if error occurs */
      {
	PWSAGetLastError(); /* Clear error */
	D = 0;
      }
      break;

    case IdSerial:
#ifdef TERATERM32
      if (! WriteFile(cv->ComID,&(cv->OutBuff[cv->OutPtr]),C,(LPDWORD)&D,&wol))
      {
	if (GetLastError() == ERROR_IO_PENDING)
	{
	  if (WaitForSingleObject(wol.hEvent,1000) !=
	      WAIT_OBJECT_0)
	    D = C;	/* Time out, ignore data */
	  else
	    GetOverlappedResult(cv->ComID,&wol,(LPDWORD)&D,FALSE);
	}
	else /* I/O error */
	  D = C; /* ignore error */
      }
      ClearCommError(cv->ComID,&DErr,&Stat);
#else
      D = WriteComm(cv->ComID, &cv->OutBuff[cv->OutPtr], C);
      D = abs(D);
      while (GetCommError(cv->ComID, &Stat)!=0) {};
#endif
      break;
    case IdFile: D = C; break;
  }

  cv->OutBuffCount = cv->OutBuffCount - D;
  if ( cv->OutBuffCount==0 )
    cv->OutPtr = 0;
  else
    cv->OutPtr = cv->OutPtr + D;

  if ( (C==D) && (delay>0) )
  {
    cv->CanSend = FALSE;
    SetTimer(cv->HWin, IdDelayTimer, delay, NULL);
  }
}

void CommSendBreak(PComVar cv)
/* for only serial ports */
{
  MSG DummyMsg;

  if ( ! cv->Ready ) return;

  switch (cv->PortType) {
    case IdSerial:
      /* Set com port into a break state */
      SetCommBreak(cv->ComID);

      /* pause for 1 sec */
      if (SetTimer(cv->HWin, IdBreakTimer, 1000, NULL) != 0)
	GetMessage(&DummyMsg,cv->HWin,WM_TIMER,WM_TIMER);

      /* Set com port into the nonbreak state */
      ClearCommBreak(cv->ComID);
      break;
  }
}

void CommLock(PTTSet ts, PComVar cv, BOOL Lock)
{
  BYTE b;
  DWORD Func;

  if (! cv->Ready) return;
  if ((cv->PortType==IdTCPIP) ||
      (cv->PortType==IdSerial) &&
      (ts->Flow!=IdFlowHard))
  {
    if (Lock)
      b = XOFF;
    else
      b = XON;
    CommBinaryOut(cv,&b,1);
  }
  else if ((cv->PortType==IdSerial) &&
	   (ts->Flow==IdFlowHard))
  {
    if (Lock)
      Func = CLRRTS;
    else
      Func = SETRTS;
    EscapeCommFunction(cv->ComID,Func);
  }
}

BOOL PrnOpen(PCHAR DevName)
{
  char Temp[MAXPATHLEN];
  DCB dcb;
#ifdef TERATERM32
  DWORD DErr;
  COMMTIMEOUTS ctmo;
#else
  COMSTAT Stat;
#endif

  strcpy(Temp,DevName);
  Temp[4] = 0; // COMn or LPTn
  LPTFlag = (Temp[0]=='L') ||
	    (Temp[0]=='l');
#ifdef TERATERM32
  PrnID =
    CreateFile(Temp,GENERIC_WRITE,
	       0,NULL,OPEN_EXISTING,
	       0,NULL);
  if (PrnID == INVALID_HANDLE_VALUE) return FALSE;

  if (GetCommState(PrnID,&dcb))
  {
    BuildCommDCB(DevName,&dcb);
    SetCommState(PrnID,&dcb);
  }
  ClearCommError(PrnID,&DErr,NULL);
  if (! LPTFlag)
    SetupComm(PrnID,0,CommOutQueSize);
  /* flush output buffer */
  PurgeComm(PrnID, PURGE_TXABORT |
    PURGE_TXCLEAR);
  memset(&ctmo,0,sizeof(ctmo));
  ctmo.WriteTotalTimeoutConstant = 1000;
  SetCommTimeouts(PrnID,&ctmo);
#else
  PrnID = OpenComm(Temp,0,CommOutQueSize);
  if (PrnID<0) return FALSE;
  if (GetCommState(PrnID,&dcb)==0)
  {
    BuildCommDCB(DevName,&dcb);
    SetCommState(&dcb);
  }
  GetCommError(PrnID, &Stat);
  /* flush output buffer */
  FlushComm(PrnID,0);
#endif
  if (! LPTFlag)
    EscapeCommFunction(PrnID,SETDTR);
  return TRUE;
}

int PrnWrite(PCHAR b, int c)
{
  int d;
#ifdef TERATERM32
  DWORD DErr;
#endif
  COMSTAT Stat;

#ifdef TERATERM32
  if (PrnID == INVALID_HANDLE_VALUE )
#else
  if (PrnID < 0)
#endif
    return c;

#ifdef TERATERM32
  ClearCommError(PrnID,&DErr,&Stat);
  if (! LPTFlag &&
      (OutBuffSize - (int)Stat.cbOutQue < c))
    c = OutBuffSize - Stat.cbOutQue;
  if (c<=0) return 0;
  if (! WriteFile(PrnID,b,c,(LPDWORD)&d,NULL))
    d = 0;
  ClearCommError(PrnID,&DErr,NULL);
#else
  GetCommError(PrnID,&Stat);
  if (OutBuffSize - Stat.cbOutQue < c)
    c = OutBuffSize - Stat.cbOutQue;
  if (c<=0) return 0;
  d = WriteComm(PrnID, b, c);
  d = abs(d);
  GetCommError(PrnID, &Stat);
#endif
  return d;
}

void PrnCancel()
{
#ifdef TERATERM32
  PurgeComm(PrnID,
    PURGE_TXABORT | PURGE_TXCLEAR);
#else
  FlushComm(PrnID,0);
#endif
  PrnClose();
}

void PrnClose()
{
#ifdef TERATERM32
  if (PrnID != INVALID_HANDLE_VALUE)
  {
    if (!LPTFlag)
      EscapeCommFunction(PrnID,CLRDTR);
    CloseHandle(PrnID);
  }
  PrnID = INVALID_HANDLE_VALUE;
#else
  if (PrnID >= 0)
  {
    if (!LPTFlag)
      EscapeCommFunction(PrnID,CLRDTR);
    CloseComm(PrnID);
  }
  PrnID = -1;
#endif
}
