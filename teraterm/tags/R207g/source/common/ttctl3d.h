/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TERATERM.EXE, CTL3D interface */

#ifdef __cplusplus
extern "C" {
#endif

BOOL LoadCtl3d(HINSTANCE hInstApp);
void FreeCtl3d();
void SysColorChange();
void SubClassDlg(HWND hDlg);

#ifdef __cplusplus
}
#endif

