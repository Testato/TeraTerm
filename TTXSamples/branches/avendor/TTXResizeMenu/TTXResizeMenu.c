#include "teraterm.h"
#include "tttypes.h"
#include "ttplugin.h"
#include "tt_res.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ORDER 5900

resize_list[][2] = {
	{80, 37},
	{120, 52},
	{80, 24},
	{110, 37}
};

#define ID_MENUID_BASE 36500
#define MENUID_MAX (sizeof(resize_list)/sizeof(resize_list[0]))

static HANDLE hInst; /* Instance handle of TTX*.DLL */

typedef struct {
  PTTSet ts;
  PComVar cv;
  HMENU ResizeMenu;
  BOOL ReplaceTermDlg;
} TInstVar;

static TInstVar FAR * pvar;

/* WIN32 allows multiple instances of a DLL */
static TInstVar InstVar;

void InitMenu() {
  int i, x, y;
  char tmp[20];

  if (pvar->ResizeMenu != NULL) {
    DestroyMenu(pvar->ResizeMenu);
  }

  pvar->ResizeMenu = CreateMenu();

  for (i=0; i<MENUID_MAX; i++) {
    x = resize_list[i][0];
    y = resize_list[i][1];
    if (x == 0)
      _snprintf_s(tmp, sizeof(tmp), _TRUNCATE, "height: %d(&%x)", y, i+1);
    else if (y == 0)
      _snprintf_s(tmp, sizeof(tmp), _TRUNCATE, "width: %d(&%x)", x, i+1);
    else
      _snprintf_s(tmp, sizeof(tmp), _TRUNCATE, "%dx%d(&%x)", x, y, i+1);
    InsertMenu(pvar->ResizeMenu, -1, MF_BYPOSITION, ID_MENUID_BASE+i, tmp);
  }
}

static void PASCAL FAR TTXInit(PTTSet ts, PComVar cv) {
  pvar->ts = ts;
  pvar->cv = cv;
  pvar->ReplaceTermDlg = FALSE;
  pvar->ResizeMenu = NULL;

  InitMenu();
}

static BOOL FAR PASCAL TTXSetupTerminal(HWND parent, PTTSet ts) {
  pvar->ReplaceTermDlg = FALSE;
  return (TRUE);
}

static void PASCAL FAR TTXGetUIHooks(TTXUIHooks FAR * hooks) {
  if (pvar->ReplaceTermDlg) {
    *hooks->SetupTerminal = TTXSetupTerminal;
  }
  return;
}

static void PASCAL FAR TTXModifyMenu(HMENU menu) {
  MENUITEMINFO mi;

  InitMenu();

  memset(&mi, 0, sizeof(mi));
  mi.cbSize = sizeof(mi);
  mi.fMask  = MIIM_TYPE | MIIM_SUBMENU;
  mi.fType  = MFT_STRING;
  mi.hSubMenu = pvar->ResizeMenu;
  mi.dwTypeData = "Resi&ze";
  InsertMenuItem(menu, ID_HELPMENU, FALSE, &mi);
}

static int PASCAL FAR TTXProcessCommand(HWND hWin, WORD cmd) {
  int num;
  if (cmd >= ID_MENUID_BASE && cmd < ID_MENUID_BASE + MENUID_MAX) {
    num = cmd - ID_MENUID_BASE;
    if (resize_list[num][0] > 0)
      pvar->ts->TerminalWidth = resize_list[num][0];
    if (resize_list[num][1] > 0)
      pvar->ts->TerminalHeight = resize_list[num][1];
    pvar->ReplaceTermDlg = TRUE;

    // Call Setup-Terminal dialog
    SendMessage(hWin, WM_COMMAND, MAKELONG(ID_SETUP_TERMINAL, 0), 0);
    return 1;
  }
  return 0;
}

static TTXExports Exports = {
  sizeof(TTXExports),
  ORDER,

  TTXInit,
  TTXGetUIHooks,
  NULL, // TTXGetSetupHooks,
  NULL, // TTXOpenTCP,
  NULL, // TTXCloseTCP,
  NULL, // TTXSetWinSize,
  TTXModifyMenu,
  NULL, // TTXModifyPopupMenu,
  TTXProcessCommand,
  NULL, // TTXEnd
};

BOOL __declspec(dllexport) PASCAL FAR TTXBind(WORD Version, TTXExports FAR * exports) {
  int size = sizeof(Exports) - sizeof(exports->size);

  if (size > exports->size) {
    size = exports->size;
  }
  memcpy((char FAR *)exports + sizeof(exports->size),
         (char FAR *)&Exports + sizeof(exports->size),
         size);
  return TRUE;
}

BOOL WINAPI DllMain(HANDLE hInstance, ULONG ul_reason, LPVOID lpReserved)
{
  switch (ul_reason) {
    case DLL_THREAD_ATTACH:
      /* do thread initialization */
      break;
    case DLL_THREAD_DETACH:
      /* do thread cleanup */
      break;
    case DLL_PROCESS_ATTACH:
      /* do process initialization */
      hInst = hInstance;
      pvar = &InstVar;
      break;
    case DLL_PROCESS_DETACH:
      /* do process cleanup */
      break;
  }
  return TRUE;
}
