/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TERATERM.EXE, Clipboard routines */

#ifdef __cplusplus
extern "C" {
#endif

/* prototypes */
PCHAR CBOpen(LONG MemSize);
void CBClose();
void CBStartPaste(HWND HWin, BOOL AddCR, BOOL Bracketed,
		  int BuffSize, PCHAR DataPtr, int DataSize);
void CBStartPasteB64(HWND HWin, PCHAR header, PCHAR footer);
void CBStartEcho(PCHAR DataPtr, int DataSize);
void CBSend();
void CBEcho();
void CBEndPaste();
int CBStartPasteConfirmChange(HWND HWin, BOOL AddCR);

#ifdef __cplusplus
}
#endif
