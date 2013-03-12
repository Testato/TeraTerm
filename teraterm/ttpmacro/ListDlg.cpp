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

	CDialog::OnInitDialog();

	// TODO:  �����ɏ�������ǉ����Ă�������
	p = m_Lists;
	while (*p) {
		m_xcList.InsertString(-1, _T(*p));
		p++;
	}
    UpdateData(FALSE);

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
