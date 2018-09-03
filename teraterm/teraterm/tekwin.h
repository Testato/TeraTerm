/*
 * Copyright (C) 1994-1998 T. Teranishi
 * (C) 2006-2017 TeraTerm Project
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

/* TERATERM.EXE, TEK window */

/////////////////////////////////////////////////////////////////////////////
// CTEKWindow
#include "tekwin_sub.h"
class CTEKWindow : public TTCFrameWnd
{
private:
  TTEKVar tk;
#ifndef NO_I18N
  HMENU MainMenu, EditMenu, WinMenu,
    FileMenu, SetupMenu, HelpMenu;
#else
  HMENU MainMenu, EditMenu, WinMenu;
#endif

public:
	CTEKWindow();
	int Parse();
	void RestoreSetup();
	void InitMenu(HMENU *Menu);
	void InitMenuPopup(HMENU SubMenu);

protected:

public:
	//{{AFX_VIRTUAL(CTEKWindow)
	protected:
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CTEKWindow)
#define afx_msg
	afx_msg void OnActivate(UINT nState, TTCWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnDestroy();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO *lpMMI);
	afx_msg void OnInitMenuPopup(TTCMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKillFocus(HWND hNewWnd);
	afx_msg void OnLButtonDown(UINT nFlags, TTCPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, TTCPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, TTCPoint point);
	afx_msg int OnMouseActivate(TTCWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, TTCPoint point);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnPaint();
	afx_msg void OnRButtonUp(UINT nFlags, TTCPoint point);
	afx_msg void OnSetFocus(HWND hOldWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnAccelCommand(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnChangeMenu(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnChangeTBar(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDlgHelp(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGetSerialNo(WPARAM wParam, LPARAM lParam);
	afx_msg void OnFilePrint();
	afx_msg void OnFileExit();
	afx_msg void OnEditCopy();
	afx_msg void OnEditCopyScreen();
	afx_msg void OnEditPaste();
	afx_msg void OnEditPasteCR();
	afx_msg void OnEditClearScreen();
	afx_msg void OnSetupWindow();
	afx_msg void OnSetupFont();
	afx_msg void OnVTWin();
	afx_msg void OnWindowWindow();
	afx_msg void OnHelpIndex();
	afx_msg void OnHelpUsing();
	afx_msg void OnHelpAbout();
	//}}AFX_MSG
//	DECLARE_MESSAGE_MAP()
#undef afx_msg
};
