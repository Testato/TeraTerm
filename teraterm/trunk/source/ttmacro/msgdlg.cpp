/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TTMACRO.EXE, message dialog box */

#include "stdafx.h"
#include "teraterm.h"
#include "ttlib.h"
#include "ttm_res.h"
#include "ttmlib.h"

#include "msgdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CMsgDlg dialog

CMsgDlg::CMsgDlg(PCHAR Text, PCHAR Title,
                 BOOL YesNo, BOOL SPECIAL,
                 int x, int y) : CDialog(CMsgDlg::IDD)
{
  //{{AFX_DATA_INIT(CMsgDlg)
  //}}AFX_DATA_INIT
  if (SPECIAL) { // (2007.9.11 maya)
    RestoreNewLine(Text); // (2006.7.29 maya)
  }
  TextStr = Text;
  TitleStr = Title;
  YesNoFlag = YesNo;
  PosX = x;
  PosY = y;
}

BEGIN_MESSAGE_MAP(CMsgDlg, CDialog)
	//{{AFX_MSG_MAP(CMsgDlg)
	ON_MESSAGE(WM_EXITSIZEMOVE, OnExitSizeMove)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// CMsgDlg message handler

BOOL CMsgDlg::OnInitDialog()
{
  RECT R;
  HDC TmpDC;
  HWND HOk, HNo;
  char uimsg[MAX_UIMSG], uimsg2[MAX_UIMSG];
  LOGFONT logfont;
  HFONT font;

  CDialog::OnInitDialog();
  font = (HFONT)SendMessage(WM_GETFONT, 0, 0);
  GetObject(font, sizeof(LOGFONT), &logfont);
  if (get_lang_font("DLG_SYSTEM_FONT", m_hWnd, &logfont, &DlgFont, UILanguageFile)) {
    SendDlgItemMessage(IDC_MSGTEXT, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
    SendDlgItemMessage(IDOK, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
    SendDlgItemMessage(IDCANCEL, WM_SETFONT, (WPARAM)DlgFont, MAKELPARAM(TRUE,0));
  }

  GetDlgItemText(IDOK, uimsg2, sizeof(uimsg2));
  get_lang_msg("BTN_OK", uimsg, sizeof(uimsg), uimsg2, UILanguageFile);
  SetDlgItemText(IDOK, uimsg);

  SetWindowText(TitleStr);
  SetDlgItemText(IDC_MSGTEXT,TextStr);

  TmpDC = ::GetDC(GetSafeHwnd());
  CalcTextExtent(TmpDC,TextStr,&s);
  ::ReleaseDC(GetSafeHwnd(),TmpDC);
  TW = s.cx + s.cx/10;
  TH = s.cy;

  HOk = ::GetDlgItem(GetSafeHwnd(), IDOK);
  HNo = ::GetDlgItem(GetSafeHwnd(), IDCANCEL);
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

LONG CMsgDlg::OnExitSizeMove(UINT wParam, LONG lParam)
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

void CMsgDlg::Relocation(BOOL is_init, int new_WW)
{
  RECT R;
  HDC TmpDC;
  HWND HText, HOk, HNo;
  int CW, CH;
  char uimsg[MAX_UIMSG], uimsg2[MAX_UIMSG];

  GetClientRect(&R);
  CW = R.right-R.left;
  CH = R.bottom-R.top;

  // ����̂�
  if (is_init) {
    // �e�L�X�g�R���g���[���T�C�Y��␳
    if (TW < CW)
      TW = CW;
    if (YesNoFlag && (TW < 7*BW/2))
      TW = 7*BW/2;
	// �E�C���h�E�T�C�Y�̌v�Z
    WW = WW + TW - CW;
    WH = WH + 2*TH+3*BH/2 - CH;
	init_WW = WW;
  }
  else {
    TW = CW;
    WW = new_WW;
  }

  HText = ::GetDlgItem(GetSafeHwnd(), IDC_MSGTEXT);
  HOk = ::GetDlgItem(GetSafeHwnd(), IDOK);
  HNo = ::GetDlgItem(GetSafeHwnd(), IDCANCEL);

  ::MoveWindow(HText,(TW-s.cx)/2,TH/2,TW,TH,TRUE);
  if (YesNoFlag)
  {
    if (is_init) {
      ::SetWindowText(HOk,"&Yes");
      ::SetWindowText(HNo,"&No");
      GetDlgItemText(IDOK, uimsg2, sizeof(uimsg2));
      get_lang_msg("BTN_YES", uimsg, sizeof(uimsg), uimsg2, UILanguageFile);
      ::SetWindowText(HOk,uimsg);
      GetDlgItemText(IDCANCEL, uimsg2, sizeof(uimsg2));
      get_lang_msg("BTN_NO", uimsg, sizeof(uimsg), uimsg2, UILanguageFile);
      ::SetWindowText(HNo,uimsg);
    }
    ::MoveWindow(HOk,(2*TW-5*BW)/4,2*TH,BW,BH,TRUE);
    ::MoveWindow(HNo,(2*TW+BW)/4,2*TH,BW,BH,TRUE);
    ::ShowWindow(HNo,SW_SHOW);
  }
  else
    ::MoveWindow(HOk,(TW-BW)/2,2*TH,BW,BH,TRUE);

  if (PosX<=-100)
  {
    TmpDC = ::GetDC(GetSafeHwnd());
    PosX = (GetDeviceCaps(TmpDC,HORZRES)-WW) / 2;
    PosY = (GetDeviceCaps(TmpDC,VERTRES)-WH) / 2;
    ::ReleaseDC(GetSafeHwnd(),TmpDC);
  }
  SetWindowPos(&wndTop,PosX,PosY,WW,WH,0);
  InvalidateRect(NULL);
}
