// ListDlg.cpp : �����t�@�C��
//

#include "stdafx.h"
#include "teraterm.h"
#include "ttlib.h"
#include "ttm_res.h"
#include "ttmlib.h"
#include "tttypes.h"

#include "stdafx.h"
#include "ListDlg.h"


// CListDlg �_�C�A���O

IMPLEMENT_DYNAMIC(CListDlg, CDialog)

CListDlg::CListDlg(PCHAR Text, PCHAR Caption, CHAR **Lists, int x, int y)
	: CDialog(CListDlg::IDD)
{
	m_Text = Text;
	m_Caption = Caption;
	m_Lists = Lists;
	PosX = x;
	PosY = y;
	DlgFont = NULL;
}

CListDlg::~CListDlg()
{
}

void CListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTBOX, m_xcList);
}


BEGIN_MESSAGE_MAP(CListDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CListDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CListDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CListDlg ���b�Z�[�W �n���h��

void CListDlg::OnBnClickedOk()
{
	// TODO: �����ɃR���g���[���ʒm�n���h�� �R�[�h��ǉ����܂��B
	m_SelectItem = m_xcList.GetCurSel();
	OnOK();
}

BOOL CListDlg::OnInitDialog()
{
	char **p;
	char uimsg[MAX_UIMSG], uimsg2[MAX_UIMSG];
	LOGFONT logfont;
	HFONT font, tmpfont;
	int ListMaxWidth = 0;
	int ListWidth;
	CDC *pDC;
	CFont *pOldFont;
	RECT R;
	HDC TmpDC;
	HWND HList, HOk;

	CDialog::OnInitDialog();

	// TODO:  �����ɏ�������ǉ����Ă�������
	font = (HFONT)SendMessage(WM_GETFONT, 0, 0);
	GetObject(font, sizeof(LOGFONT), &logfont);
	if (get_lang_font("DLG_SYSTEM_FONT", m_hWnd, &logfont, &DlgFont, UILanguageFile)) {
		SendDlgItemMessage(IDC_LISTBOX, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
		SendDlgItemMessage(IDC_STATIC, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
		SendDlgItemMessage(IDOK, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
		SendDlgItemMessage(IDCANCEL, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
	}

	GetDlgItemText(IDOK, uimsg2, sizeof(uimsg2));
	get_lang_msg("BTN_OK", uimsg, sizeof(uimsg), uimsg2, UILanguageFile);
	SetDlgItemText(IDOK, uimsg);
	GetDlgItemText(IDCANCEL, uimsg2, sizeof(uimsg2));
	get_lang_msg("BTN_CANCEL", uimsg, sizeof(uimsg), uimsg2, UILanguageFile);
	SetDlgItemText(IDCANCEL, uimsg);

	pDC = m_xcList.GetDC(); // ���X�g�{�b�N�X�����X�N���[���ł���悤�ɍő啝���擾
	pOldFont = pDC->SelectObject(m_xcList.GetFont());

	p = m_Lists;
	while (*p) {
		m_xcList.InsertString(-1, _T(*p));
		ListWidth = pDC->GetTextExtent(*p).cx;
		if (ListWidth > ListMaxWidth) {
			ListMaxWidth = ListWidth;
		}
		p++;
	}
	UpdateData(FALSE);

	m_xcList.SetHorizontalExtent(ListMaxWidth + 5);
	pDC->SelectObject(pOldFont);
	ReleaseDC(pDC);

	// 1�߂�I����Ԃɂ���B
	m_xcList.SetCurSel(0);

	// �{���ƃ^�C�g��
	SetDlgItemText(IDC_STATIC, m_Text);
	SetWindowText(m_Caption);


	TmpDC = ::GetDC(GetDlgItem(IDC_STATIC)->GetSafeHwnd());
	if (DlgFont) {
		tmpfont = (HFONT)SelectObject(TmpDC, DlgFont);
	}
	CalcTextExtent(TmpDC,m_Text,&s);
	if (DlgFont && tmpfont != NULL) {
		SelectObject(TmpDC, tmpfont);
	}
	::ReleaseDC(GetDlgItem(IDC_STATIC)->GetSafeHwnd(),TmpDC);
	TW = s.cx + s.cx/10;
	TH = s.cy;

	HList = ::GetDlgItem(GetSafeHwnd(), IDC_LISTBOX);
	::GetWindowRect(HList,&R);
	LW = R.right-R.left;
	LH = R.bottom-R.top;

	HOk = ::GetDlgItem(GetSafeHwnd(), IDOK);
	::GetWindowRect(HOk,&R);
	BW = R.right-R.left;
	BH = R.bottom-R.top;

	GetWindowRect(&R);
	WW = R.right-R.left;
	WH = R.bottom-R.top;

	Relocation(TRUE, WW);

	SetForegroundWindow();

	return TRUE;  // return TRUE unless you set the focus to a control
	// ��O : OCX �v���p�e�B �y�[�W�͕K�� FALSE ��Ԃ��܂��B
}

void CListDlg::OnBnClickedCancel()
{
	// TODO: �����ɃR���g���[���ʒm�n���h�� �R�[�h��ǉ����܂��B
	OnCancel();
}

void CListDlg::Relocation(BOOL is_init, int new_WW)
{
	RECT R;
	HDC TmpDC;
	HWND HText, HOk, HCancel, HList;
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
		// �E�C���h�E�T�C�Y�̌v�Z
		WW = TW + (WW - CW);
		WH = TH + LH + (int)(BH*1.5) + (WH - CH);
		init_WW = WW;
		// ���X�g�{�b�N�X�T�C�Y�̌v�Z
		if (LW < WW - BW - 14*3) {
			LW = WW - BW - 14*3;
		}
	}
	else {
		TW = CW;
		WW = new_WW;
	}

	HText = ::GetDlgItem(GetSafeHwnd(), IDC_STATIC);
	HOk = ::GetDlgItem(GetSafeHwnd(), IDOK);
	HCancel = ::GetDlgItem(GetSafeHwnd(), IDCANCEL);
	HList = ::GetDlgItem(GetSafeHwnd(), IDC_LISTBOX);

	::MoveWindow(HText,(TW-s.cx)/2,LH+BH,TW,TH,TRUE);
	::MoveWindow(HList,14,BH/2,LW,LH,TRUE);
	::MoveWindow(HOk,14+14+LW,BH/2,BW,BH,TRUE);
	::MoveWindow(HCancel,14+14+LW,BH*2,BW,BH,TRUE);

	if (PosX<=GetMonitorLeftmost(PosX, PosY)-100) {
		GetWindowRect(&R);
		TmpDC = ::GetDC(GetSafeHwnd());
		PosX = (GetDeviceCaps(TmpDC,HORZRES)-R.right+R.left) / 2;
		PosY = (GetDeviceCaps(TmpDC,VERTRES)-R.bottom+R.top) / 2;
		::ReleaseDC(GetSafeHwnd(),TmpDC);
	}
	SetWindowPos(&wndTop,PosX,PosY,WW,WH,0);

	InvalidateRect(NULL);
}
