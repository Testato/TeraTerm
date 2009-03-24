/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TERATERM.EXE, Communication routines */
#ifdef __cplusplus
extern "C" {
#endif

void CommInit(PComVar cv);
void CommOpen(HWND HW, PTTSet ts, PComVar cv);
void CommStart(PComVar cv, LONG lParam);
BOOL CommCanClose(PComVar cv);
void CommClose(PComVar cv);
void CommProcRRQ(PComVar cv);
void CommReceive(PComVar cv);
void CommSend(PComVar cv);
void CommSendBreak(PComVar cv);
void CommResetSerial(PTTSet ts, PComVar cv);
void CommLock(PTTSet ts, PComVar cv, BOOL Lock);
BOOL PrnOpen(PCHAR DevName);
int PrnWrite(PCHAR b, int c);
void PrnCancel();
void PrnClose();

extern BOOL TCPIPClosed;

#ifdef __cplusplus
}
#endif
