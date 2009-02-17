/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TTMACRO.EXE, main */

#include "stdafx.h"
#include "teraterm.h"
#include "ttm_res.h"
#include "ttmmain.h"
#include "ttl.h"

#include "ttmacro.h"
#include "ttmlib.h"
#include "ttlib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

char UILanguageFile[MAX_PATH];

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CCtrlApp, CWinApp)
	//{{AFX_MSG_MAP(CCtrlApp)
	//}}AFX_MSG
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CCtrlApp::CCtrlApp()
{
}

/////////////////////////////////////////////////////////////////////////////

CCtrlApp theApp;

/////////////////////////////////////////////////////////////////////////////

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

BOOL CCtrlApp::InitInstance()
{
	static HMODULE HTTSET = NULL;

	// �C���X�g�[���Ŏ��s�����o���邽�߂� mutex ���쐬���� (2006.8.12 maya)
	// 2�d�N���h�~�̂��߂ł͂Ȃ��̂ŁA���ɕԂ�l�͌��Ȃ�
	HANDLE hMutex;
	hMutex = CreateMutex(NULL, TRUE, "TeraTermProMacroAppMutex");

	GetUILanguageFile(UILanguageFile, sizeof(UILanguageFile));

	Busy = TRUE;
	m_pMainWnd = new CCtrlWindow();
	PCtrlWindow(m_pMainWnd)->Create();
	Busy = FALSE;  
	return TRUE;
}

int CCtrlApp::ExitInstance()
{
	m_pMainWnd = NULL;
	return ExitCode;
}

// TTMACRO main engine
BOOL CCtrlApp::OnIdle(LONG lCount)
{
	BOOL Continue;

	// Avoid multi entry
	if (Busy) {
		return FALSE;
	}
	Busy = TRUE;
	if (m_pMainWnd != NULL) {
		Continue = PCtrlWindow(m_pMainWnd)->OnIdle();
	}
	else {
		Continue = FALSE;
	}
	Busy = FALSE;
	return Continue;
}
