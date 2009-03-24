/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TTMACRO.EXE, message dialog box */

class CMsgDlg : public CDialog
{
public:
	CMsgDlg(PCHAR Text, PCHAR Title, BOOL YesNo, int x, int y);

	//{{AFX_DATA(CMsgDlg)
	enum { IDD = IDD_MSGDLG };
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CMsgDlg)
	//}}AFX_VIRTUAL

protected:
	PCHAR TextStr, TitleStr;
	BOOL YesNoFlag;
	int  PosX, PosY;

	//{{AFX_MSG(CMsgDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
