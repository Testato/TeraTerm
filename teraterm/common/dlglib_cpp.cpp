﻿/*
 * (C) 2005-2018 TeraTerm Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Routines for dialog boxes */

#include <windows.h>
#include "dlglib.h"

// ダイアログモーダル状態の時、OnIdle()を実行する
//#define ENABLE_CALL_IDLE_MODAL	1

extern BOOL CallOnIdle(LONG lCount);

typedef struct {
	DLGPROC OrigProc;	// Dialog proc
	LONG_PTR OrigUser;	// DWLP_USER
	LPARAM ParamInit;
	int DlgResult;
	bool EndDialogFlag;
} TTDialogData;

static TTDialogData *TTDialogTmpData;

#if ENABLE_CALL_IDLE_MODAL
static int TTDoModal(HWND hDlgWnd)
{
	LONG lIdleCount = 0;
	MSG Msg;
	TTDialogData *data = (TTDialogData *)GetWindowLongPtr(hDlgWnd, DWLP_USER);

	for (;;)
	{
		if (!IsWindow(hDlgWnd)) {
			// ウインドウが閉じられた
			return IDCANCEL;
		}
#if defined(_DEBUG)
		if (!IsWindowVisible(hDlgWnd)) {
			// 誤ってEndDialog()が使われた? -> TTEndDialog()を使うこと
			::ShowWindow(hDlgWnd, SW_SHOWNORMAL);
		}
#endif
		if (data->EndDialogFlag) {
			// TTEndDialog()が呼ばれた
			return data->DlgResult;
		}

		if(!::PeekMessage(&Msg, NULL, NULL, NULL, PM_NOREMOVE))
		{
			// メッセージがない
			// OnIdel() を処理する
			if (!CallOnIdle(lIdleCount++)) {
				// Idle処理がなくなった
				lIdleCount = 0;
				Sleep(10);
			}
			continue;
		}
		else
		{
			// メッセージがある

			// pump message
			BOOL quit = !::GetMessage(&Msg, NULL, NULL, NULL);
			if (quit) {
				// QM_QUIT
				PostQuitMessage(0);
				return IDCANCEL;
			}

			if (!::IsDialogMessage(hDlgWnd, &Msg)) {
				// ダイアログ以外の処理
				::TranslateMessage(&Msg);
				::DispatchMessage(&Msg);
			}
		}
	}

	// ここには来ない
	return IDOK;
}
#endif

static INT_PTR CALLBACK TTDialogProc(
	HWND hDlgWnd, UINT msg,
	WPARAM wParam, LPARAM lParam)
{
	TTDialogData *data = (TTDialogData *)GetWindowLongPtr(hDlgWnd, DWLP_USER);
	if (msg == WM_INITDIALOG) {
		TTDialogData *data = (TTDialogData *)lParam;
		SetWindowLongPtr(hDlgWnd, DWLP_USER, (LONG_PTR)lParam);
		lParam = data->ParamInit;
	}

	if (data == NULL) {
		// WM_INITDIALOGよりも前は設定されていない
		data = TTDialogTmpData;
	} else {
		// TTEndDialog()が呼ばれたとき、DWLP_USER が参照できない
		TTDialogTmpData = data;
	}

	SetWindowLongPtr(hDlgWnd, DWLP_DLGPROC, (LONG_PTR)data->OrigProc);
	SetWindowLongPtr(hDlgWnd, DWLP_USER, (LONG_PTR)data->OrigUser);
	LRESULT Result = data->OrigProc(hDlgWnd, msg, wParam, lParam);
	data->OrigProc = (DLGPROC)GetWindowLongPtr(hDlgWnd, DWLP_DLGPROC);
	data->OrigUser = GetWindowLongPtr(hDlgWnd, DWLP_USER);
	SetWindowLongPtr(hDlgWnd, DWLP_DLGPROC, (LONG_PTR)TTDialogProc);
	SetWindowLongPtr(hDlgWnd, DWLP_USER, (LONG_PTR)data);

	if (msg == WM_NCDESTROY) {
		SetWindowLongPtr(hDlgWnd, DWLP_USER, 0);
		free(data);
	}

	return Result;
}

/**
 *	EndDialog() 互換関数
 */
BOOL TTEndDialog(HWND hDlgWnd, INT_PTR nResult)
{
#if ENABLE_CALL_IDLE_MODAL
	TTDialogData *data = TTDialogTmpData;
	data->DlgResult = nResult;
	data->EndDialogFlag = true;
	return TRUE;
#else
	return EndDialog(hDlgWnd, nResult);
#endif
}

/**
 *	CreateDialogIndirectParam() 互換関数
 */
HWND TTCreateDialogIndirectParam(
	HINSTANCE hInstance,
	LPCTSTR lpTemplateName,
	HWND hWndParent,			// オーナーウィンドウのハンドル
	DLGPROC lpDialogFunc,		// ダイアログボックスプロシージャへのポインタ
	LPARAM lParamInit)			// 初期化値
{
	TTDialogData *data = (TTDialogData *)malloc(sizeof(TTDialogData));
	data->OrigProc = lpDialogFunc;
	data->OrigUser = 0;
	data->ParamInit = lParamInit;
	data->EndDialogFlag = false;
	data->DlgResult = 0;
	DLGTEMPLATE *lpTemplate = TTGetDlgTemplate(hInstance, lpTemplateName);
	TTDialogTmpData = data;
	HWND hDlgWnd = CreateDialogIndirectParam(
		hInstance, lpTemplate, hWndParent, TTDialogProc, (LPARAM)data);
	TTDialogTmpData = NULL;
	ShowWindow(hDlgWnd, SW_SHOW);
    UpdateWindow(hDlgWnd);
	free(lpTemplate);
	return hDlgWnd;
}

/**
 *	CreateDialog() 互換関数
 */
HWND TTCreateDialog(
	HINSTANCE hInstance,
	LPCTSTR lpTemplateName,
	HWND hWndParent,
	DLGPROC lpDialogFunc)
{
	return TTCreateDialogIndirectParam(hInstance, lpTemplateName,
									   hWndParent, lpDialogFunc, NULL);
}

/**
 *	DialogBoxParam() 互換関数
 *		EndDialog()ではなく、TTEndDialog()を使用すること
 */
int TTDialogBoxParam(
	HINSTANCE hInstance,
	LPCTSTR lpTemplateName,
	HWND hWndParent,			// オーナーウィンドウのハンドル
	DLGPROC lpDialogFunc,		// ダイアログボックスプロシージャへのポインタ
	LPARAM lParamInit)			// 初期化値
{
#if ENABLE_CALL_IDLE_MODAL
	HWND hDlgWnd = TTCreateDialogIndirectParam(
		hInstance, lpTemplateName,
		hWndParent, lpDialogFunc, lParamInit);
	EnableWindow(hWndParent, FALSE);
	int DlgResult = TTDoModal(hDlgWnd);
	::DestroyWindow(hDlgWnd);
	EnableWindow(hWndParent, TRUE);
	return DlgResult;
#else
	DLGTEMPLATE *lpTemplate = TTGetDlgTemplate(hInstance, lpTemplateName);
	int DlgResult = DialogBoxIndirectParam(
		hInstance, lpTemplate, hWndParent,
		lpDialogFunc, lParamInit);
	free(lpTemplate);
	return DlgResult;
#endif
}

/**
 *	DialogBoxParam() 互換関数
 *		EndDialog()ではなく、TTEndDialog()を使用すること
 */
int TTDialogBox(
	HINSTANCE hInstance,
	LPCTSTR lpTemplateName,
	HWND hWndParent,
	DLGPROC lpDialogFunc)
{
	return TTDialogBoxParam(
		hInstance, lpTemplateName,
		hWndParent, lpDialogFunc, (LPARAM)NULL);
}
