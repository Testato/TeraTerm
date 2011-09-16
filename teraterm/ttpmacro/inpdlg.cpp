/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TTMACRO.EXE, input dialog box */

#include "stdafx.h"
#include "teraterm.h"
#include "ttlib.h"
#include "ttm_res.h"
#include "ttmlib.h"

#include "inpdlg.h"

#define MaxStrLen 256

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CInpDlg dialog
CInpDlg::CInpDlg(PCHAR Input, PCHAR Text, PCHAR Title,
                 PCHAR Default, BOOL Paswd,
                 int x, int y) : CDialog(CInpDlg::IDD)
{
	//{{AFX_DATA_INIT(CInpDlg)
	//}}AFX_DATA_INIT
	InputStr = Input;
	TextStr = Text;
	TitleStr = Title;
	DefaultStr = Default;
	PaswdFlag = Paswd;
	PosX = x;
	PosY = y;
}

BEGIN_MESSAGE_MAP(CInpDlg, CDialog)
	//{{AFX_MSG_MAP(CInpDlg)
	ON_MESSAGE(WM_EXITSIZEMOVE, OnExitSizeMove)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// CInpDlg message handler

// msgdlg �̂悤�ɁA���b�Z�[�W�������ꍇ�ɂ̓_�C�A���O���g����悤�ɂ��� (2006.7.29 maya)
BOOL CInpDlg::OnInitDialog()
{
	RECT R;
	HDC TmpDC;
	HWND HEdit, HOk;
	char uimsg[MAX_UIMSG], uimsg2[MAX_UIMSG];
	LOGFONT logfont;
	HFONT font;

	CDialog::OnInitDialog();
	font = (HFONT)SendMessage(WM_GETFONT, 0, 0);
	GetObject(font, sizeof(LOGFONT), &logfont);
	if (get_lang_font("DLG_SYSTEM_FONT", m_hWnd, &logfont, &DlgFont, UILanguageFile)) {
		SendDlgItemMessage(IDC_INPTEXT, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
		SendDlgItemMessage(IDC_INPEDIT, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
		SendDlgItemMessage(IDOK, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
	}

	GetDlgItemText(IDOK, uimsg2, sizeof(uimsg2));
	get_lang_msg("BTN_OK", uimsg, sizeof(uimsg), uimsg2, UILanguageFile);
	SetDlgItemText(IDOK, uimsg);

	SetWindowText(TitleStr);
	SetDlgItemText(IDC_INPTEXT,TextStr);
	SetDlgItemText(IDC_INPEDIT,DefaultStr);

	TmpDC = ::GetDC(GetSafeHwnd());
	CalcTextExtent(TmpDC,TextStr,&s);
	::ReleaseDC(GetSafeHwnd(),TmpDC);
	TW = s.cx + s.cx/10;
	TH = s.cy;

	HEdit = ::GetDlgItem(GetSafeHwnd(), IDC_INPEDIT);
	::GetWindowRect(HEdit,&R);
	EW = R.right-R.left;
	EH = R.bottom-R.top;

	HOk = ::GetDlgItem(GetSafeHwnd(), IDOK);
	::GetWindowRect(HOk,&R);
	BW = R.right-R.left;
	BH = R.bottom-R.top;

	GetWindowRect(&R);
	WW = R.right-R.left;
	WH = R.bottom-R.top;

	Relocation(TRUE, WW);

	SetForegroundWindow();

	return TRUE;
}

void CInpDlg::OnOK()
{
	GetDlgItemText(IDC_INPEDIT,InputStr,MaxStrLen-1);
	EndDialog(IDOK);
}

LONG CInpDlg::OnExitSizeMove(UINT wParam, LONG lParam)
{
	RECT R;

	GetWindowRect(&R);
	if (R.bottom-R.top == WH && R.right-R.left == WW) {
		// �T�C�Y���ς���Ă��Ȃ���Ή������Ȃ�
	}
	else if (R.bottom-R.top != WH || R.right-R.left < init_WW) {
		// �������ύX���ꂽ���A�ŏ���蕝�������Ȃ����ꍇ�͌��ɖ߂�
		SetWindowPos(&wndTop,R.left,R.top,WW,WH,0);
	}
	else {
		// �����łȂ���΍Ĕz�u����
		Relocation(FALSE, R.right-R.left);
	}

	return CDialog::DefWindowProc(WM_EXITSIZEMOVE,wParam,lParam);
}

void CInpDlg::Relocation(BOOL is_init, int new_WW)
{
	RECT R;
	HDC TmpDC;
	HWND HText, HOk, HEdit;
	int CW, CH;

	GetClientRect(&R);
	CW = R.right-R.left;
	CH = R.bottom-R.top;

	// ����̂�
	if (is_init) {
		// �e�L�X�g�R���g���[���T�C�Y��␳
		if (TW < CW) {
			TW = CW;
		}
		if (EW < s.cx) {
			EW = s.cx;
		}
		// �E�C���h�E�T�C�Y�̌v�Z
		WW = TW + (WW - CW);
		WH = TH + EH + BH + BH*2 + (WH - CH);
		init_WW = WW;
	}
	else {
		TW = CW;
		WW = new_WW;
	}

	HText = ::GetDlgItem(GetSafeHwnd(), IDC_INPTEXT);
	HOk = ::GetDlgItem(GetSafeHwnd(), IDOK);
	HEdit = ::GetDlgItem(GetSafeHwnd(), IDC_INPEDIT);

	::MoveWindow(HText,(TW-s.cx)/2,BH/2,TW,TH,TRUE);
	::MoveWindow(HEdit,(WW-EW)/2-4,TH+BH,EW,EH,TRUE);
	::MoveWindow(HOk,(TW-BW)/2,TH+EH+BH*3/2,BW,BH,TRUE);

	if (PaswdFlag) {
		SendDlgItemMessage(IDC_INPEDIT,EM_SETPASSWORDCHAR,(UINT)'*',0);
	}

	SendDlgItemMessage(IDC_INPEDIT, EM_LIMITTEXT, MaxStrLen, 0);

	if (PosX<=-100) {
		GetWindowRect(&R);
		TmpDC = ::GetDC(GetSafeHwnd());
		PosX = (GetDeviceCaps(TmpDC,HORZRES)-R.right+R.left) / 2;
		PosY = (GetDeviceCaps(TmpDC,VERTRES)-R.bottom+R.top) / 2;
		::ReleaseDC(GetSafeHwnd(),TmpDC);
	}
	SetWindowPos(&wndTop,PosX,PosY,WW,WH,0);

	InvalidateRect(NULL);
}
