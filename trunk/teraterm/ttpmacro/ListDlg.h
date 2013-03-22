#pragma once
#include "afxwin.h"


// CListDlg �_�C�A���O

class CListDlg : public CDialog
{
	DECLARE_DYNAMIC(CListDlg)

public:
	CListDlg(PCHAR Text, PCHAR Caption, CHAR **Lists);   // �W���R���X�g���N�^
	virtual ~CListDlg();

// �_�C�A���O �f�[�^
	enum { IDD = IDD_LISTDLG };

protected:
	PCHAR m_Text;
	PCHAR m_Caption;
	CHAR **m_Lists;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g
	HFONT DlgFont;

	DECLARE_MESSAGE_MAP()
public:
	CListBox m_xcList;
	int m_SelectItem;
public:
	afx_msg void OnBnClickedOk();
public:
	virtual BOOL OnInitDialog();
public:
	afx_msg void OnBnClickedCancel();
};
