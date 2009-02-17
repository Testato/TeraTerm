#include "teraterm.h"
#include "tttypes.h"
#include "ttplugin.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

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

#define ORDER 9999

static HANDLE hInst; /* Instance handle of TTX*.DLL */

typedef struct {
	PReadIniFile origReadIniFile;
	PWriteIniFile origWriteIniFile;
	char origIniFileName[MAXPATHLEN];
} TInstVar;

static TInstVar FAR * pvar;
static TInstVar InstVar;

static void PASCAL FAR TTXReadIniFile(PCHAR fn, PTTSet ts) {
	strcpy_s(pvar->origIniFileName, sizeof(pvar->origIniFileName), fn);
	(pvar->origReadIniFile)(fn, ts);
}

static void PASCAL FAR TTXWriteIniFile(PCHAR fn, PTTSet ts) {
	CopyFile(pvar->origIniFileName, fn, TRUE);
	(pvar->origWriteIniFile)(fn, ts);
}

static void PASCAL FAR TTXGetSetupHooks(TTXSetupHooks FAR * hooks) {
	if (pvar->origIniFileName[0] == 0) {
		pvar->origReadIniFile = *hooks->ReadIniFile;
		*hooks->ReadIniFile = TTXReadIniFile;
	}
	else {
		pvar->origWriteIniFile = *hooks->WriteIniFile;
		*hooks->WriteIniFile = TTXWriteIniFile;
	}
}

static TTXExports Exports = {
	sizeof(TTXExports),
	ORDER,

	NULL,	// TTXInit,
	NULL,	// TTXGetUIHooks,
	TTXGetSetupHooks,
	NULL,	// TTXOpenTCP,
	NULL,	// TTXCloseTCP,
	NULL,	// TTXSetWinSize,
	NULL,	// TTXModifyMenu,
	NULL,	// TTXModifyPopupMenu,
	NULL,	// TTXProcessCommand,
	NULL,	// TTXEnd,
	NULL,	// TTXSetCommandLine,
	NULL,	// TTXOpenFile,
	NULL	// TTXCloseFile
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
