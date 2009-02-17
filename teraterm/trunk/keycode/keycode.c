/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* keycode.exe for Tera Term Pro */

#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "kc_res.h"
#define ClassName "KeyCodeWin32"

// Prototypes
LRESULT WINAPI MainWndProc( HWND, UINT, WPARAM, LPARAM );

// Global variables;
static HANDLE ghInstance;
static BOOL KeyDown = FALSE;
static BOOL Short;
static WORD Scan;

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

int PASCAL WinMain(HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPSTR lpszCmdLine,
  int nCmdShow)
{
  WNDCLASS wc;
  MSG msg;
  HWND hWnd;

  // �C���X�g�[���Ŏ��s�����o���邽�߂� mutex ���쐬���� (2006.8.12 maya)
  // 2�d�N���h�~�̂��߂ł͂Ȃ��̂ŁA���ɕԂ�l�͌��Ȃ�
  HANDLE hMutex;
  hMutex = CreateMutex(NULL, TRUE, "TeraTermProKeycodeAppMutex");

  if(!hPrevInstance)
  {
    wc.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = ClassName;
    RegisterClass(&wc);
  }

  ghInstance = hInstance;

  hWnd = CreateWindow(ClassName,
    "Key code for Tera Term",
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    200,
    100,
    NULL,
    NULL,
    hInstance,
    NULL);

  ShowWindow(hWnd, nCmdShow);

  PostMessage(hWnd,WM_SETICON,ICON_SMALL,
              (LPARAM)LoadImage(hInstance,
                                MAKEINTRESOURCE(IDI_KEYCODE),
                                IMAGE_ICON,16,16,0));
  PostMessage(hWnd,WM_SETICON,ICON_BIG,
              (LPARAM)LoadImage(hInstance,
                                MAKEINTRESOURCE(IDI_KEYCODE),
                                IMAGE_ICON,0,0,0));

  while(GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return msg.wParam;
}

void KeyDownProc(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
  if ((wParam==VK_SHIFT) ||
      (wParam==VK_CONTROL) ||
      (wParam==VK_MENU)) return;

  Scan = HIWORD(lParam) & 0x1ff;
  if ((GetKeyState(VK_SHIFT) & 0x80) != 0)
    Scan = Scan | 0x200;
  if ((GetKeyState(VK_CONTROL) & 0x80) != 0)
    Scan = Scan | 0x400;
  if ((GetKeyState(VK_MENU) & 0x80) != 0)
    Scan = Scan | 0x800;

  if (! KeyDown)
  {
    KeyDown = TRUE;
    Short = TRUE;
    SetTimer(hWnd,1,10,NULL);
    InvalidateRect(hWnd,NULL,TRUE);
  }
}

void KeyUpProc(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
  if (! KeyDown) return;
  if (Short)
    SetTimer(hWnd,2,500,NULL);
  else {
    KeyDown = FALSE;
    InvalidateRect(hWnd,NULL,TRUE);
  }
}

void PaintProc(HWND hWnd)
{
  PAINTSTRUCT ps;
  HDC hDC;
  char OutStr[30];

  hDC = BeginPaint(hWnd, &ps);

  if (KeyDown)
  {
    _snprintf_s(OutStr,sizeof(OutStr),_TRUNCATE,"Key code is %u.",Scan);
    TextOut(hDC,10,10,OutStr,strlen(OutStr));
  }
  else
    TextOut(hDC,10,10,"Push any key.",13);

  EndPaint(hWnd, &ps);
}

void TimerProc(HWND hWnd, WPARAM wParam)
{
  KillTimer(hWnd,wParam);
  if (wParam==1)
    Short = FALSE;
  else if (wParam==2)
  {
    KeyDown = FALSE;
    InvalidateRect(hWnd,NULL,TRUE);
  }
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam,
  LPARAM lParam)
{
  switch( msg ) {
    case WM_KEYDOWN:
      KeyDownProc(hWnd, wParam, lParam);
      break;
    case WM_KEYUP:
      KeyUpProc(hWnd, wParam, lParam);
      break;
    case WM_SYSKEYDOWN:
      if (wParam==VK_F10)
	KeyDownProc(hWnd, wParam, lParam);
      else
	return (DefWindowProc(hWnd, msg, wParam, lParam));
      break;
    case WM_SYSKEYUP:
      if (wParam==VK_F10)
	KeyUpProc(hWnd, wParam, lParam);
      else
	return (DefWindowProc(hWnd, msg, wParam, lParam));
      break;
    case WM_PAINT:
      PaintProc(hWnd);
      break;
    case WM_TIMER:
      TimerProc(hWnd, wParam);
      break;
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    default:
      return (DefWindowProc(hWnd, msg, wParam, lParam));
  }

  return 0;
}
