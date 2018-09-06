﻿
#include "tmfc.h"
#include "ttlib.h"

const RECT TTCFrameWnd::rectDefault =
{
	CW_USEDEFAULT, CW_USEDEFAULT,
//	2*CW_USEDEFAULT, 2*CW_USEDEFAULT
	0, 0
};

TTCFrameWnd::TTCFrameWnd()
{
}

TTCFrameWnd::~TTCFrameWnd()
{
}

BOOL TTCFrameWnd::Create(
	HINSTANCE hInstance,
	LPCTSTR lpszClassName,
	LPCTSTR lpszWindowName,
	DWORD dwStyle,
	const RECT& rect,
	HWND hParentWnd,
	LPCTSTR lpszMenuName,
	DWORD dwExStyle)
{
	pseudoPtr = this;
	HWND hWnd = ::CreateWindowExA(
		0,
		lpszClassName,
		lpszWindowName,
		dwStyle,
		rect.left, rect.top,
		rect.right - rect.left, rect.bottom - rect.top,
		hParentWnd,
		NULL,
		hInstance,
		NULL);
	pseudoPtr = NULL;
	if (hWnd == NULL) {
		OutputDebugPrintf("CreateWindow %d\n", GetLastError());
		return FALSE;
	} else {
		m_hWnd = hWnd;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);
		return TRUE;
	}
}

TTCFrameWnd *TTCFrameWnd::pseudoPtr;

LRESULT TTCFrameWnd::ProcStub(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	TTCFrameWnd *self;
	if (pseudoPtr != NULL) {
		self = pseudoPtr;
		self->m_hWnd = hWnd;
	} else {
		self = (TTCFrameWnd *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	}
	return self->Proc(msg, wp, lp);
}

LRESULT TTCFrameWnd::Proc(UINT msg, WPARAM wp, LPARAM lp)
{
	return DefWindowProc(msg, wp, lp);
}

BOOL TTCFrameWnd::OnCommand(WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}

void TTCFrameWnd::OnInitMenuPopup(TTCMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{}

void TTCFrameWnd::OnKillFocus(HWND hNewWnd)
{}

void TTCFrameWnd::OnDestroy()
{}

void TTCFrameWnd::OnSetFocus(HWND hOldWnd)
{}

void TTCFrameWnd::OnSysCommand(UINT nID, LPARAM lParam)
{}

void TTCFrameWnd::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	TTCFrameWnd::Proc(WM_SYSKEYDOWN, (WPARAM)nChar, MAKELONG(nRepCnt, nFlags));
}

void TTCFrameWnd::OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	TTCFrameWnd::Proc(WM_SYSKEYUP, (WPARAM)nChar, MAKELONG(nRepCnt, nFlags));
}

void TTCFrameWnd::OnClose()
{}


////////////////////////////////////////

void TTCWnd::DestroyWindow()
{
	::DestroyWindow(m_hWnd);
}

HDC TTCWnd::BeginPaint(LPPAINTSTRUCT lpPaint)
{
	return ::BeginPaint(m_hWnd, lpPaint);
}

BOOL TTCWnd::EndPaint(LPPAINTSTRUCT lpPaint)
{
	return ::EndPaint(m_hWnd, lpPaint);
}

LRESULT TTCWnd::DefWindowProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	return ::DefWindowProc(m_hWnd, msg, wParam, lParam);
}
