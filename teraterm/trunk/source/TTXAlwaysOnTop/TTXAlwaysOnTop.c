#include "teraterm.h"
#include "tttypes.h"
#include "ttplugin.h"
#include "tt_res.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <Windows.h>

#define ORDER 5800
#define ID_MENUITEM 35000

static HANDLE hInst; /* Instance handle of TTX*.DLL */

typedef struct {
  HMENU ControlMenu;
  BOOL ontop;
} TInstVar;

static TInstVar FAR * pvar;
static TInstVar InstVar;

static void PASCAL FAR TTXInit(PTTSet ts, PComVar cv) {
  pvar->ontop = FALSE;
}

static void PASCAL FAR TTXModifyMenu(HMENU menu) {
  UINT flag = MF_BYCOMMAND | MF_STRING | MF_ENABLED;

  pvar->ControlMenu = GetSubMenu(menu, 3);
  if (pvar->ontop) {
    flag |= MF_CHECKED;
  }
  InsertMenu(pvar->ControlMenu, ID_CONTROL_MACRO,
		flag, ID_MENUITEM, "&Always on top");
  InsertMenu(pvar->ControlMenu, ID_CONTROL_MACRO,
		MF_BYCOMMAND | MF_SEPARATOR, 0, NULL);
}

static int PASCAL FAR TTXProcessCommand(HWND hWin, WORD cmd) {
  if (cmd == ID_MENUITEM) {
    if (pvar->ontop) {
      pvar->ontop = FALSE;
      CheckMenuItem(pvar->ControlMenu, ID_MENUITEM, MF_BYCOMMAND | MF_UNCHECKED);
      SetWindowPos(hWin, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
    else {
      pvar->ontop = TRUE;
      CheckMenuItem(pvar->ControlMenu, ID_MENUITEM, MF_BYCOMMAND | MF_CHECKED);
      SetWindowPos(hWin, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
    return 1;
  }
  return 0;
}

static TTXExports Exports = {
  sizeof(TTXExports),
  ORDER,

  TTXInit,
  NULL, // TTXGetUIHooks,
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

BOOL WINAPI DllMain(HANDLE hInstance,
		    ULONG ul_reason_for_call,
		    LPVOID lpReserved)
{
  switch( ul_reason_for_call ) {
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
