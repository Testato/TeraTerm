/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TERATERM.EXE, CTL3D interface */

#include "teraterm.h"
#include <string.h>
#include "ttlib.h"

static HANDLE HCtl3d = NULL;
static hInst;

typedef BOOL (PASCAL FAR *PCtl3dColorChange)();
typedef BOOL (PASCAL FAR *PCtl3dAutoSubclass)(HANDLE hinstApp);
typedef BOOL (PASCAL FAR *PCtl3dRegister)(HANDLE hinstApp);
typedef BOOL (PASCAL FAR *PCtl3dUnregister)(HANDLE hinstApp);
typedef BOOL (PASCAL FAR *PCtl3dSubclassDlgEx)(HWND hwndDlg, DWORD grbit);

static PCtl3dColorChange Ctl3dColorChange;
static PCtl3dAutoSubclass Ctl3dAutoSubclass;
static PCtl3dRegister Ctl3dRegister;
static PCtl3dUnregister Ctl3dUnregister;
static PCtl3dSubclassDlgEx Ctl3dSubclassDlgEx;

BOOL LoadCtl3d(HINSTANCE hInstApp)
{
  char FN[MAXPATHLEN];
  BOOL Err;

  if (HCtl3d >= HINSTANCE_ERROR) return TRUE;

  GetSystemDirectory(FN,sizeof(FN));
  strcat(FN,"\\CTL3DV2.DLL");
  if (! DoesFileExist(FN))
  {
    GetSystemDirectory(FN,sizeof(FN));
    strcat(FN,"\\CTL3D.DLL");
    if (! DoesFileExist(FN)) return FALSE;
  }

  HCtl3d = LoadLibrary(FN);
 if (HCtl3d < HINSTANCE_ERROR) return FALSE;

  Err = FALSE;

  Ctl3dColorChange = (PCtl3dColorChange)GetProcAddress(HCtl3d,
	"CTL3DCOLORCHANGE");
  if (Ctl3dColorChange==NULL) Err = TRUE;

  Ctl3dAutoSubclass = (PCtl3dAutoSubclass)GetProcAddress(HCtl3d,
	"CTL3DAUTOSUBCLASS");
  if (Ctl3dAutoSubclass==NULL) Err = TRUE;

  Ctl3dRegister = (PCtl3dRegister)GetProcAddress(HCtl3d,
	"CTL3DREGISTER");
  if (Ctl3dRegister==NULL) Err = TRUE;

  Ctl3dUnregister = (PCtl3dUnregister)GetProcAddress(HCtl3d,
	"CTL3DUNREGISTER");
  if (Ctl3dUnregister==NULL) Err = TRUE;

  /* This may not exist in an old version of CTL3D */
  Ctl3dSubclassDlgEx = (PCtl3dSubclassDlgEx)GetProcAddress(HCtl3d,
	"CTL3DSUBCLASSDLGEX");

  if (Err)
  {
    FreeLibrary(HCtl3d);
    HCtl3d = NULL;
  }
  else {
    hInst = hInstApp;
    Ctl3dRegister(hInst);
    Ctl3dAutoSubclass(hInst);
  }
  return (! Err);
}

void FreeCtl3d()
{
  if (HCtl3d >= HINSTANCE_ERROR)
  {
    Ctl3dUnregister(hInst);
    FreeLibrary(HCtl3d);
    HCtl3d = NULL;
  }
}

void SysColorChange()
{
  if (HCtl3d<HINSTANCE_ERROR) return;
  Ctl3dColorChange();
}

void SubClassDlg(HWND hDlg)
{
  if ((HCtl3d>=HINSTANCE_ERROR) &&
      (Ctl3dSubclassDlgEx!=NULL))
    Ctl3dSubclassDlgEx(hDlg,0x0000ffff);
}
