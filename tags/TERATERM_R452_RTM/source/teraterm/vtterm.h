/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TERATERM.EXE, VT terminal emulation */
#ifdef __cplusplus
extern "C" {
#endif

/* prototypes */
void ResetTerminal();
void ResetCharSet();
void HideStatusLine();
void ChangeTerminalSize(int Nx, int Ny);
int VTParse();

#ifdef __cplusplus
}
#endif
