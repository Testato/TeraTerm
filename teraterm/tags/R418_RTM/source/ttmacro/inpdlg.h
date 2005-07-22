/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TTMACRO.EXE, input dialog box */

class CInpDlg : public CDialog
{
public:
	CInpDlg(PCHAR Input, PCHAR Text, PCHAR Title,
		BOOL Paswd, int x, int y);

	//{{AFX_DATA(CInpDlg)
	enum { IDD = IDD_INPDLG };
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CInpDlg)
	//}}AFX_VIRTUAL

protected:
	PCHAR InputStr, TextStr, TitleStr;
	BOOL PaswdFlag;
	int PosX, PosY;

	//{{AFX_MSG(CInpDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
typedef CInpDlg *PInpDlg;
