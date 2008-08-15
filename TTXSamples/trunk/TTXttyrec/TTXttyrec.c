#include "teraterm.h"
#include "tttypes.h"
#include "ttplugin.h"
#include "tt_res.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef NO_INET6
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <winsock.h>
#endif /* NO_INET6 */

#include "gettimeofday.h"

#define ORDER 6000
#define ID_MENUITEM 37000

static HANDLE hInst; /* Instance handle of TTX*.DLL */

typedef struct {
  PTTSet ts;
  PComVar cv;
  Trecv origPrecv;
  TReadFile origPReadFile;
  HANDLE fh;
  BOOL record;
  HMENU ControlMenu;
} TInstVar;

struct recheader {
  struct timeval tv;
  int len;
};

static TInstVar FAR * pvar;
static TInstVar InstVar;

static void PASCAL FAR TTXInit(PTTSet ts, PComVar cv) {
  pvar->ts = ts;
  pvar->cv = cv;
  pvar->fh = INVALID_HANDLE_VALUE;
  pvar->record = FALSE;
}

int PASCAL FAR TTXrecv(SOCKET s, char FAR *buff, int len, int flags) {
  struct recheader h;
  int b[3], w;

  h.len = pvar->origPrecv(s, buff, len, flags);
  if (pvar->record && h.len > 0) {
    gettimeofday(&h.tv, NULL);
    b[0] = h.tv.tv_sec;
    b[1] = h.tv.tv_usec;
    b[2] = h.len;
    WriteFile(pvar->fh, b, sizeof(b), &w, NULL);
    WriteFile(pvar->fh, buff, h.len, &w, NULL);
  }
  return h.len;
}

BOOL PASCAL FAR TTXReadFile(HANDLE fh, LPVOID buff, DWORD len, LPDWORD rbytes, LPOVERLAPPED rol) {
  struct recheader h;
  int b[3], w;

  if (!pvar->origPReadFile(fh, buff, len, rbytes, rol))
    return FALSE;

  if (pvar->record && *rbytes > 0) {
    gettimeofday(&h.tv, NULL);
    b[0] = h.tv.tv_sec;
    b[1] = h.tv.tv_usec;
    b[2] = *rbytes;
    WriteFile(pvar->fh, b, sizeof(b), &w, NULL);
    WriteFile(pvar->fh, buff, *rbytes, &w, NULL);
  };

  return TRUE;
}

static void PASCAL FAR TTXOpenTCP(TTXSockHooks FAR * hooks) {
  pvar->origPrecv = *hooks->Precv;
  *hooks->Precv = TTXrecv;
}

static void PASCAL FAR TTXOpenFile(TTXFileHooks FAR * hooks) {
  pvar->origPReadFile = *hooks->PReadFile;
  *hooks->PReadFile = TTXReadFile;
}

static void PASCAL FAR TTXModifyMenu(HMENU menu) {
  UINT flag = MF_BYCOMMAND | MF_STRING | MF_ENABLED;

  pvar->ControlMenu = GetSubMenu(menu, 3);
  if (pvar->record) {
    flag |= MF_CHECKED;
  }
  InsertMenu(pvar->ControlMenu, ID_CONTROL_MACRO,
		flag, ID_MENUITEM, "TT&Y Record");
  InsertMenu(pvar->ControlMenu, ID_CONTROL_MACRO,
		MF_BYCOMMAND | MF_SEPARATOR, 0, NULL);
}

static void PASCAL FAR TTXModifyPopupMenu(HMENU menu) {
  if (menu==pvar->ControlMenu) {
    if (pvar->cv->Ready)
      EnableMenuItem(pvar->ControlMenu, ID_MENUITEM, MF_BYCOMMAND | MF_ENABLED);
    else
      EnableMenuItem(pvar->ControlMenu, ID_MENUITEM, MF_BYCOMMAND | MF_GRAYED);
  }
}

static int PASCAL FAR TTXProcessCommand(HWND hWin, WORD cmd) {
  OPENFILENAME ofn;
  char fname[MAX_PATH];
  fname[0] = '\0';

  if (cmd==ID_MENUITEM) {
    if (pvar->record) {
      if (pvar->fh != INVALID_HANDLE_VALUE) {
	CloseHandle(pvar->fh);
	pvar->fh = INVALID_HANDLE_VALUE;
      }
      pvar->record = FALSE;
      CheckMenuItem(pvar->ControlMenu, ID_MENUITEM, MF_BYCOMMAND | MF_UNCHECKED);
    }
    else {
      if (pvar->fh != INVALID_HANDLE_VALUE) {
	CloseHandle(pvar->fh);
      }

      memset(&ofn, 0, sizeof(ofn));
      ofn.lStructSize = sizeof(ofn);
      ofn.hwndOwner = hWin;
      ofn.lpstrFilter = "ttyrec(*.tty)\0*.tty\0All files(*.*)\0*.*\0\0";
      ofn.lpstrFile = fname;
      ofn.nMaxFile = sizeof(fname);
      ofn.lpstrDefExt = "tty";
      // ofn.lpstrTitle = "";
      ofn.Flags = OFN_OVERWRITEPROMPT;
      if (GetSaveFileName(&ofn)) {
	pvar->fh = CreateFile(fname, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (pvar->fh != INVALID_HANDLE_VALUE) {
	  pvar->record = TRUE;
	  CheckMenuItem(pvar->ControlMenu, ID_MENUITEM, MF_BYCOMMAND | MF_CHECKED);
	}
      }
    }
    return 1;
  }
  return 0;
}

static void PASCAL FAR TTXEnd(void) {
  if (pvar->fh != INVALID_HANDLE_VALUE) {
    CloseHandle(pvar->fh);
    pvar->fh = INVALID_HANDLE_VALUE;
  }
}

static TTXExports Exports = {
  sizeof(TTXExports),
  ORDER,

  TTXInit,
  NULL, // TTXGetUIHooks,
  NULL, // TTXGetSetupHooks,
  TTXOpenTCP,
  NULL, // TTXCloseTCP,
  NULL, // TTXSetWinSize,
  TTXModifyMenu,
  TTXModifyPopupMenu,
  TTXProcessCommand,
  TTXEnd,
  NULL, // TTXSetCommandLine,
  TTXOpenFile,
  NULL  // TTXCloseFile
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
