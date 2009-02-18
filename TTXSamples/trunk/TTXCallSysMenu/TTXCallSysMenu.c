#include "teraterm.h"
#include "tttypes.h"
#include "ttplugin.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// VS2005でビルドされたバイナリが Windows95 でも起動できるようにするために、
// IsDebuggerPresent()のシンボル定義を追加する。
//
// cf.http://jet2.u-abel.net/program/tips/forceimp.htm
// 装飾された名前のアドレスを作るための仮定義
// (これだけでインポートを横取りしている)
int WINAPI _imp__IsDebuggerPresent()
    { return PtrToInt((void*) &_imp__IsDebuggerPresent); }
// 実際に横取り処理を行う関数
BOOL WINAPI Cover_IsDebuggerPresent()
    { return FALSE; }
// 関数が実際に呼び出されたときに備えて
// 横取り処理関数を呼び出させるための下準備
void __stdcall DoCover_IsDebuggerPresent()
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
//int s_DoCover_IsDebuggerPresent
//    = (int) (DoCover_IsDebuggerPresent(), 0);

#define ORDER 4000
#define MENU_ID 39393

static HANDLE hInst; /* Instance handle of TTX*.DLL */

typedef struct {
  PTTSet ts;
  PComVar cv;
} TInstVar;

static TInstVar FAR * pvar;
static TInstVar InstVar;

static int PASCAL FAR TTXProcessCommand(HWND hWin, WORD cmd) {
  if (cmd==MENU_ID) {
    PostMessage(hWin, WM_SYSCOMMAND, SC_KEYMENU, MAKELPARAM(0x20, 0));
    return 1;
  }
  return 0;
}

static TTXExports Exports = {
  sizeof(TTXExports),
  ORDER,

/* Now we just list the functions that we've implemented. */
  NULL, // TTXInit,
  NULL, // TTXGetUIHooks,
  NULL, // TTXGetSetupHooks,
  NULL, // TTXOpenTCP,
  NULL, // TTXCloseTCP,
  NULL, // TTXSetWinSize,
  NULL, // TTXModifyMenu,
  NULL, // TTXModifyPopupMenu,
  TTXProcessCommand,
  NULL, // TTXEnd,
  NULL, // TTXSetCommandLine,
  NULL, // TTXOpenFile,
  NULL, // TTXCloseFile
};

BOOL __declspec(dllexport) PASCAL FAR TTXBind(WORD Version, TTXExports FAR * exports) {
  int size = sizeof(Exports) - sizeof(exports->size);
  /* do version checking if necessary */
  /* if (Version!=TTVERSION) return FALSE; */

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
      DoCover_IsDebuggerPresent();
      hInst = hInstance;
      pvar = &InstVar;
      break;
    case DLL_PROCESS_DETACH:
      /* do process cleanup */
      break;
  }
  return TRUE;
}
