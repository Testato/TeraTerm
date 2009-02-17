#include "teraterm.h"
#include "tttypes.h"
#include "ttplugin.h"
#include "tt_res.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// VS2005でビルドされたバイナリが Windows95 でも起動できるようにするために、
// IsDebuggerPresent()のシンボル定義を追加する。
//
// cf.http://jet2.u-abel.net/program/tips/forceimp.htm
// 装飾された名前のアドレスを作るための仮定義
// (これだけでインポートを横取りしている)
EXTERN_C int WINAPI _imp__IsDebuggerPresent()
    { return PtrToInt((void*) &_imp__IsDebuggerPresent); }
// 実際に横取り処理を行う関数
EXTERN_C BOOL WINAPI Cover_IsDebuggerPresent()
    { return FALSE; }
// 関数が実際に呼び出されたときに備えて
// 横取り処理関数を呼び出させるための下準備
EXTERN_C void __stdcall DoCover_IsDebuggerPresent()
{
    DWORD dw;
    DWORD_PTR FAR* lpdw;
    // 横取り関数を設定するアドレスを取得
    lpdw = (DWORD_PTR FAR*) &_imp__IsDebuggerPresent;
    // このアドレスを書き込めるように設定
    // (同じプログラム内なので障害なく行える)
    VirtualProtect(lpdw, sizeof(DWORD_PTR), PAGE_READWRITE, &dw);
    // 横取り関数を設定
    *lpdw = (DWORD_PTR)(FARPROC) Cover_IsDebuggerPresent;
    // 読み書きの状態を元に戻す
    VirtualProtect(lpdw, sizeof(DWORD_PTR), dw, NULL);
}
// アプリケーションが初期化される前に下準備を呼び出す
// ※ かなり早くに初期化したいときは、このコードを
//  ファイルの末尾に書いて「#pragma init_seg(lib)」を、
//  この変数宣言の手前に書きます。
//  初期化を急ぐ必要が無い場合は WinMain 内から
//  DoCover_IsDebuggerPresent を呼び出して構いません。
EXTERN_C int s_DoCover_IsDebuggerPresent
    = (int) (DoCover_IsDebuggerPresent(), 0);

#define ORDER 5900
#define SECTION "Resize Menu"

int def_resize_list[][2] = {
	{80, 37},
	{120, 52},
	{80, 24},
	{110, 37}
};

#define ID_MENUID_BASE 55101
#define MAX_MENU_ITEMS 20

static HANDLE hInst; /* Instance handle of TTX*.DLL */

typedef struct {
  PTTSet ts;
  PComVar cv;
  HMENU ResizeMenu;
  BOOL ReplaceTermDlg;
  PReadIniFile origReadIniFile;
  int MenuItems;
  int ResizeList[MAX_MENU_ITEMS][2];
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

  for (i=0; i < pvar->MenuItems; i++) {
    x = pvar->ResizeList[i][0];
    y = pvar->ResizeList[i][1];
    if (i < 15) {
      if (x == 0)
	_snprintf_s(tmp, sizeof(tmp), _TRUNCATE, "height: %d(&%x)", y, i+1);
      else if (y == 0)
	_snprintf_s(tmp, sizeof(tmp), _TRUNCATE, "width: %d(&%x)", x, i+1);
      else
	_snprintf_s(tmp, sizeof(tmp), _TRUNCATE, "%dx%d(&%x)", x, y, i+1);
    }
    else if (i < 35) {
      if (x == 0)
	_snprintf_s(tmp, sizeof(tmp), _TRUNCATE, "height: %d(&%c)", y, 'a' + i - 9);
      else if (y == 0)
	_snprintf_s(tmp, sizeof(tmp), _TRUNCATE, "width: %d(&%c)", x, 'a' + i - 9);
      else
	_snprintf_s(tmp, sizeof(tmp), _TRUNCATE, "%dx%d(&%c)", x, y, 'a' + i - 9);
    }
    else {
      if (x == 0)
	_snprintf_s(tmp, sizeof(tmp), _TRUNCATE, "height: %d", y);
      else if (y == 0)
	_snprintf_s(tmp, sizeof(tmp), _TRUNCATE, "width: %d", x);
      else
	_snprintf_s(tmp, sizeof(tmp), _TRUNCATE, "%dx%d", x, y);
    }
    InsertMenu(pvar->ResizeMenu, -1, MF_BYPOSITION, ID_MENUID_BASE+i, tmp);
  }
}

static void PASCAL FAR TTXInit(PTTSet ts, PComVar cv) {
  int i;

  pvar->ts = ts;
  pvar->cv = cv;
  pvar->ReplaceTermDlg = FALSE;
  pvar->ResizeMenu = NULL;

  pvar->MenuItems = sizeof(def_resize_list)/sizeof(def_resize_list[0]);

  for (i=0; i < pvar->MenuItems; i++) {
    pvar->ResizeList[i][0] = def_resize_list[i][0];
    pvar->ResizeList[i][1] = def_resize_list[i][1];
  }

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

static void PASCAL FAR ResizeMenuReadIniFile(PCHAR fn, PTTSet ts) {
  int i, x, y;
  char Key[20], Buff[100];

  (pvar->origReadIniFile)(fn, ts);

  for (i=0; i<MAX_MENU_ITEMS; i++) {
    _snprintf_s(Key, sizeof(Key), _TRUNCATE, "ResizeMenu%d", i+1);
    GetPrivateProfileString(SECTION, Key, "\n", Buff, sizeof(Buff), fn);

    if (sscanf_s(Buff, "%d , %d", &x, &y) == 2) {
      if (x < 0 ) {
        x = 0;
      }

      if (y < 0 ) {
        y = 0;
      }

      if (x > TermWidthMax ) {
        x = TermWidthMax;
      }

      if (y > TermHeightMax ) {
        y = TermHeightMax;
      }

      if (x == 0 && y == 0) {
        break;
      }

      pvar->ResizeList[i][0] = x;
      pvar->ResizeList[i][1] = y;
    }
    else {
      break;
    }
  }

  if (i == 0) {
    pvar->MenuItems = sizeof(def_resize_list)/sizeof(def_resize_list[0]);

    for (i=0; i < pvar->MenuItems; i++) {
      pvar->ResizeList[i][0] = def_resize_list[i][0];
      pvar->ResizeList[i][1] = def_resize_list[i][1];
    }
  }
  else {
    pvar->MenuItems = i;
  }
}

static void PASCAL FAR TTXGetSetupHooks(TTXSetupHooks FAR * hooks) {
  pvar->origReadIniFile = *hooks->ReadIniFile;
  *hooks->ReadIniFile = ResizeMenuReadIniFile;
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
  if (cmd >= ID_MENUID_BASE && cmd < ID_MENUID_BASE + pvar->MenuItems) {
    num = cmd - ID_MENUID_BASE;
    if (pvar->ResizeList[num][0] > 0)
      pvar->ts->TerminalWidth = pvar->ResizeList[num][0];
    if (pvar->ResizeList[num][1] > 0)
      pvar->ts->TerminalHeight = pvar->ResizeList[num][1];
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
  TTXGetSetupHooks,
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
