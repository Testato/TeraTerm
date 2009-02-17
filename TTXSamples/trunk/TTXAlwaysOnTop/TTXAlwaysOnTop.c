#include "teraterm.h"
#include "tttypes.h"
#include "ttplugin.h"
#include "tt_res.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <Windows.h>

// VS2005�Ńr���h���ꂽ�o�C�i���� Windows95 �ł��N���ł���悤�ɂ��邽�߂ɁA
// IsDebuggerPresent()�̃V���{����`��ǉ�����B
//
// cf.http://jet2.u-abel.net/program/tips/forceimp.htm
// �������ꂽ���O�̃A�h���X����邽�߂̉���`
// (���ꂾ���ŃC���|�[�g������肵�Ă���)
EXTERN_C int WINAPI _imp__IsDebuggerPresent()
    { return PtrToInt((void*) &_imp__IsDebuggerPresent); }
// ���ۂɉ���菈�����s���֐�
EXTERN_C BOOL WINAPI Cover_IsDebuggerPresent()
    { return FALSE; }
// �֐������ۂɌĂяo���ꂽ�Ƃ��ɔ�����
// ����菈���֐����Ăяo�����邽�߂̉�����
EXTERN_C void __stdcall DoCover_IsDebuggerPresent()
{
    DWORD dw;
    DWORD_PTR FAR* lpdw;
    // �����֐���ݒ肷��A�h���X���擾
    lpdw = (DWORD_PTR FAR*) &_imp__IsDebuggerPresent;
    // ���̃A�h���X���������߂�悤�ɐݒ�
    // (�����v���O�������Ȃ̂ŏ�Q�Ȃ��s����)
    VirtualProtect(lpdw, sizeof(DWORD_PTR), PAGE_READWRITE, &dw);
    // �����֐���ݒ�
    *lpdw = (DWORD_PTR)(FARPROC) Cover_IsDebuggerPresent;
    // �ǂݏ����̏�Ԃ����ɖ߂�
    VirtualProtect(lpdw, sizeof(DWORD_PTR), dw, NULL);
}
// �A�v���P�[�V�����������������O�ɉ��������Ăяo��
// �� ���Ȃ葁���ɏ������������Ƃ��́A���̃R�[�h��
//  �t�@�C���̖����ɏ����āu#pragma init_seg(lib)�v���A
//  ���̕ϐ��錾�̎�O�ɏ����܂��B
//  ���������}���K�v�������ꍇ�� WinMain ������
//  DoCover_IsDebuggerPresent ���Ăяo���č\���܂���B
EXTERN_C int s_DoCover_IsDebuggerPresent
    = (int) (DoCover_IsDebuggerPresent(), 0);

#define ORDER 5800
#define ID_MENU_BASE      55000
#define ID_MENU_TOP       ID_MENU_BASE + 1
#define ID_MENU_BOTTOM    ID_MENU_BASE + 2
#define ID_MENU_TOPMOST   ID_MENU_BASE + 3
#define ID_MENU_NOTOPMOST ID_MENU_BASE + 4

static HANDLE hInst; /* Instance handle of TTX*.DLL */

typedef struct {
  HMENU ControlMenu;
  BOOL ontop;
} TInstVar;

static TInstVar FAR * pvar;
static TInstVar InstVar;

#define GetFileMenu(menu)	GetSubMenuByChildID(menu, ID_FILE_NEWCONNECTION)
#define GetEditMenu(menu)	GetSubMenuByChildID(menu, ID_EDIT_COPY2)
#define GetSetupMenu(menu)	GetSubMenuByChildID(menu, ID_SETUP_TERMINAL)
#define GetControlMenu(menu)	GetSubMenuByChildID(menu, ID_CONTROL_RESETTERMINAL)
#define GetHelpMenu(menu)	GetSubMenuByChildID(menu, ID_HELP_ABOUT)

HMENU GetSubMenuByChildID(HMENU menu, UINT id) {
  int i, j, items, subitems, cur_id;
  HMENU m;

  items = GetMenuItemCount(menu);

  for (i=0; i<items; i++) {
    if (m = GetSubMenu(menu, i)) {
      subitems = GetMenuItemCount(m);
      for (j=0; j<subitems; j++) {
        cur_id = GetMenuItemID(m, j);
	if (cur_id == id) {
	  return m;
	}
      }
    }
  }
  return NULL;
}

static void PASCAL FAR TTXInit(PTTSet ts, PComVar cv) {
  pvar->ontop = FALSE;
}

static void PASCAL FAR TTXModifyMenu(HMENU menu) {
  UINT flag = MF_BYCOMMAND | MF_STRING | MF_ENABLED;

  pvar->ControlMenu = GetControlMenu(menu);
  if (pvar->ontop) {
    flag |= MF_CHECKED;
  }
  InsertMenu(pvar->ControlMenu, ID_CONTROL_MACRO,
		flag, ID_MENU_BASE, "&Always on top");
  InsertMenu(pvar->ControlMenu, ID_CONTROL_MACRO,
		MF_BYCOMMAND | MF_SEPARATOR, 0, NULL);
}

static int PASCAL FAR TTXProcessCommand(HWND hWin, WORD cmd) {
  switch (cmd) {
    case ID_MENU_BASE:
      if (pvar->ontop) {
        pvar->ontop = FALSE;
        CheckMenuItem(pvar->ControlMenu, ID_MENU_BASE, MF_BYCOMMAND | MF_UNCHECKED);
        SetWindowPos(hWin, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
      }
      else {
        pvar->ontop = TRUE;
        CheckMenuItem(pvar->ControlMenu, ID_MENU_BASE, MF_BYCOMMAND | MF_CHECKED);
        SetWindowPos(hWin, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
      }
      break;
    case ID_MENU_TOP:
      SetWindowPos(hWin, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
      break;
    case ID_MENU_BOTTOM:
      SetWindowPos(hWin, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
      break;
    case ID_MENU_TOPMOST:
      pvar->ontop = TRUE;
      CheckMenuItem(pvar->ControlMenu, ID_MENU_BASE, MF_BYCOMMAND | MF_CHECKED);
      SetWindowPos(hWin, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
      break;
    case ID_MENU_NOTOPMOST:
      pvar->ontop = FALSE;
      CheckMenuItem(pvar->ControlMenu, ID_MENU_BASE, MF_BYCOMMAND | MF_UNCHECKED);
      SetWindowPos(hWin, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
      break;
    default:
      return 0;
  }
  return 1;
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
