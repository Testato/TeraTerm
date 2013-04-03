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

CListDlg::CListDlg(PCHAR Text, PCHAR Caption, CHAR **Lists)
	: CDialog(CListDlg::IDD)
{
	m_Text = Text;
	m_Caption = Caption;
	m_Lists = Lists;
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
	HFONT font;
	int ListMaxWidth = 0;
	int ListWidth;
	CDC *pDC;
	CFont *pOldFont;

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

	SetForegroundWindow();

	return TRUE;  // return TRUE unless you set the focus to a control
	// ��O : OCX �v���p�e�B �y�[�W�͕K�� FALSE ��Ԃ��܂��B
}

void CListDlg::OnBnClickedCancel()
{
	// TODO: �����ɃR���g���[���ʒm�n���h�� �R�[�h��ǉ����܂��B
	OnCancel();
}
