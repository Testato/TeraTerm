/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TERATERM.EXE, scroll buffer routines */

#ifdef __cplusplus
extern "C" {
#endif

void InitBuffer();
void LockBuffer();
void UnlockBuffer();
void FreeBuffer();
void BuffReset();
void BuffAllSelect();
void ChangeSelectRegion();
void BuffScroll(int Count, int Bottom);
void BuffInsertSpace(int Count);
void BuffEraseCurToEnd();
void BuffEraseHomeToCur();
void BuffInsertLines(int Count, int YEnd);
void BuffEraseCharsInLine(int XStart, int Count);
void BuffDeleteLines(int Count, int YEnd);
void BuffDeleteChars(int Count);
void BuffEraseChars(int Count);
void BuffFillWithE();
void BuffDrawLine(BYTE Attr, BYTE Attr2, int Direction, int C);
void BuffEraseBox(int XStart, int YStart, int XEnd, int YEnd);
void BuffCBCopy(BOOL Table);
void BuffPrint(BOOL ScrollRegion);
void BuffDumpCurrentLine(BYTE TERM);
void BuffPutChar(BYTE b, BYTE Attr, BYTE Attr2, BOOL Insert);
void BuffPutKanji(WORD w, BYTE Attr, BYTE Attr2, BOOL Insert);
void BuffUpdateRect(int XStart, int YStart, int XEnd, int YEnd);
void UpdateStr();
void UpdateStrUnicode(void);
void MoveCursor(int Xnew, int Ynew);
void MoveRight();
void BuffSetCaretWidth();
void BuffScrollNLines(int n);
void BuffClearScreen();
void BuffUpdateScroll();
void CursorUpWithScroll();
void BuffDblClk(int Xw, int Yw);
void BuffTplClk(int Yw);
void BuffStartSelect(int Xw, int Yw, BOOL Box);
void BuffChangeSelect(int Xw, int Yw, int NClick);
void BuffEndSelect();
void BuffChangeWinSize(int Nx, int Ny);
void BuffChangeTerminalSize(int Nx, int Ny);
void ChangeWin();
void ClearBuffer();
void SetTabStop();
void MoveToNextTab();
void ClearTabStop(int Ps);
void ShowStatusLine(int Show);

extern int StatusLine;
extern int CursorTop, CursorBottom;
extern BOOL Selected;
extern BOOL Wrap;

#ifdef __cplusplus
}
#endif
