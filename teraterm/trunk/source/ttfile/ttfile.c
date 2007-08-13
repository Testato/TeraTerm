/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TTFILE.DLL, file transfer, VT window printing */
#include "teraterm.h"
#include "tttypes.h"
#include "ttftypes.h"
#include <direct.h>
#include <commdlg.h>
#include <string.h>

#include "ttlib.h"
#include "ftlib.h"
#include "dlglib.h"
#include "kermit.h"
#include "xmodem.h"
#include "zmodem.h"
#include "bplus.h"
#include "quickvan.h"
// resource IDs
#ifdef TERATERM32
#include "file_res.h"
#else
#include "file_r16.h"
#endif

#include <stdlib.h>
#include <stdio.h>

static HANDLE hInst;

static HFONT DlgFoptFont;
static HFONT DlgXoptFont;
static HFONT DlgGetfnFont;

char UILanguageFile[MAX_PATH];
char FileSendFilter[128];

#ifdef TERATERM32
BOOL IS_WIN4()
{
  OSVERSIONINFO verinfo;

  verinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  if (!GetVersionEx(&verinfo)) return FALSE;
  return (verinfo.dwMajorVersion>=4);
}
#endif

BOOL FAR PASCAL GetSetupFname(HWND HWin, WORD FuncId, PTTSet ts)
{
	int i, j;
	OPENFILENAME ofn;
	char uimsg[MAX_UIMSG];

	//  char FNameFilter[HostNameMaxLength + 1]; // 81(yutaka)
	char FNameFilter[81]; // 81(yutaka)
	char TempDir[MAXPATHLEN];
	char Dir[MAXPATHLEN];
	char Name[MAXPATHLEN];
	BOOL Ok;

	/* save current dir */
	_getcwd(TempDir,sizeof(TempDir));

	/* File name filter */
	memset(FNameFilter, 0, sizeof(FNameFilter));
	if (FuncId==GSF_LOADKEY)
	{
		get_lang_msg("FILEDLG_KEYBOARD_FILTER", uimsg, sizeof(uimsg), "keyboard setup files (*.cnf)\\0*.cnf\\0\\0", UILanguageFile);
		memcpy(FNameFilter, uimsg, sizeof(FNameFilter));
	}
	else {
		get_lang_msg("FILEDLG_SETUP_FILTER", uimsg, sizeof(uimsg), "setup files (*.ini)\\0*.ini\\0\\0", UILanguageFile);
		memcpy(FNameFilter, uimsg, sizeof(FNameFilter));
	}

	/* OPENFILENAME record */
	memset(&ofn, 0, sizeof(OPENFILENAME));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner   = HWin;
	ofn.lpstrFile   = Name;
	ofn.nMaxFile	  = sizeof(Name);
	ofn.lpstrFilter = FNameFilter;
	ofn.nFilterIndex = 1;
	ofn.hInstance = hInst;

	if (FuncId==GSF_LOADKEY)
	{
		ofn.lpstrDefExt = "cnf";
		GetFileNamePos(ts->KeyCnfFN,&i,&j);
		strncpy_s(Name, sizeof(Name),&(ts->KeyCnfFN[j]), _TRUNCATE);
		memcpy(Dir,ts->KeyCnfFN,i);
		Dir[i] = 0;

		if ((strlen(Name)==0) || (_stricmp(Name,"KEYBOARD.CNF")==0))
			strncpy_s(Name, sizeof(Name),"KEYBOARD.CNF", _TRUNCATE);
	}
	else {
		ofn.lpstrDefExt = "ini";
		GetFileNamePos(ts->SetupFName,&i,&j);
		strncpy_s(Name, sizeof(Name),&(ts->SetupFName[j]), _TRUNCATE);
		memcpy(Dir,ts->SetupFName,i);
		Dir[i] = 0;

		if ((strlen(Name)==0) || (_stricmp(Name,"TERATERM.INI")==0))
			strncpy_s(Name, sizeof(Name),"TERATERM.INI", _TRUNCATE);
	}

	if (strlen(Dir)==0)
		strncpy_s(Dir, sizeof(Dir),ts->HomeDir, _TRUNCATE);

	_chdir(Dir);

	ofn.Flags = OFN_SHOWHELP | OFN_HIDEREADONLY;
	switch (FuncId) {
	case GSF_SAVE:
		// 初期ファイルディレクトリをプログラム本体がある箇所に固定する (2005.1.6 yutaka)
		// 読み込まれたteraterm.iniがあるディレクトリに固定する。
		// これにより、/F= で指定された位置に保存されるようになる。(2005.1.26 yutaka)
		// Windows Vista ではファイル名まで指定すると NULL と同じ挙動をするようなので、
		// ファイル名を含まない形でディレクトリを指定するようにした。(2006.9.16 maya)
//		ofn.lpstrInitialDir = __argv[0];
//		ofn.lpstrInitialDir = ts->SetupFName;
		ofn.lpstrInitialDir = Dir;
		get_lang_msg("FILEDLG_SAVE_SETUP_TITLE", uimsg, sizeof(uimsg), "Tera Term: Save setup", UILanguageFile);
		ofn.lpstrTitle = uimsg;
		Ok = GetSaveFileName(&ofn);
		if (Ok)
			strncpy_s(ts->SetupFName, sizeof(ts->SetupFName),Name, _TRUNCATE);
		break;
	case GSF_RESTORE:
		ofn.Flags = ofn.Flags | OFN_FILEMUSTEXIST;
		get_lang_msg("FILEDLG_RESTORE_SETUP_TITLE", uimsg, sizeof(uimsg), "Tera Term: Restore setup", UILanguageFile);
		ofn.lpstrTitle = uimsg;
		Ok = GetOpenFileName(&ofn);
		if (Ok)
			strncpy_s(ts->SetupFName, sizeof(ts->SetupFName),Name, _TRUNCATE);
		break;
	case GSF_LOADKEY:
		ofn.Flags = ofn.Flags | OFN_FILEMUSTEXIST;
		get_lang_msg("FILEDLG_LOAD_KEYMAP_TITLE", uimsg, sizeof(uimsg), "Tera Term: Load key map", UILanguageFile);
		ofn.lpstrTitle = uimsg;
		Ok = GetOpenFileName(&ofn);
		if (Ok)
			strncpy_s(ts->KeyCnfFN, sizeof(ts->KeyCnfFN),Name, _TRUNCATE);
		break;
	}

	/* restore dir */
	_chdir(TempDir);

	return Ok;
}

/* Hook function for file name dialog box */
BOOL CALLBACK TFnHook(HWND Dialog, UINT Message, WPARAM wParam, LPARAM lParam)
{
	LPOPENFILENAME ofn;
	WORD Lo, Hi;
	LPLONG pl;
#ifdef TERATERM32
	LPOFNOTIFY notify;
#endif
	char uimsg[MAX_UIMSG], uimsg2[MAX_UIMSG];
	LOGFONT logfont;
	HFONT font;

	switch (Message) {
	case WM_INITDIALOG:
		ofn = (LPOPENFILENAME)lParam;
		pl = (LPLONG)(ofn->lCustData);
		SetWindowLong(Dialog, DWL_USER, (LONG)pl);

      font = (HFONT)SendMessage(Dialog, WM_GETFONT, 0, 0);
      GetObject(font, sizeof(LOGFONT), &logfont);
      if (get_lang_font("DLG_TAHOMA_FONT", Dialog, &logfont, &DlgFoptFont, UILanguageFile)) {
        SendDlgItemMessage(Dialog, IDC_FOPT, WM_SETFONT, (WPARAM)DlgFoptFont, MAKELPARAM(TRUE,0));
        SendDlgItemMessage(Dialog, IDC_FOPTBIN, WM_SETFONT, (WPARAM)DlgFoptFont, MAKELPARAM(TRUE,0));
        SendDlgItemMessage(Dialog, IDC_FOPTAPPEND, WM_SETFONT, (WPARAM)DlgFoptFont, MAKELPARAM(TRUE,0));
        SendDlgItemMessage(Dialog, IDC_PLAINTEXT, WM_SETFONT, (WPARAM)DlgFoptFont, MAKELPARAM(TRUE,0));
        SendDlgItemMessage(Dialog, IDC_TIMESTAMP, WM_SETFONT, (WPARAM)DlgFoptFont, MAKELPARAM(TRUE,0));
      }
      else {
        DlgFoptFont = NULL;
      }

      GetDlgItemText(Dialog, IDC_FOPT, uimsg2, sizeof(uimsg2));
      get_lang_msg("DLG_FOPT", uimsg, sizeof(uimsg), uimsg2, UILanguageFile);
      SetDlgItemText(Dialog, IDC_FOPT, uimsg);
      GetDlgItemText(Dialog, IDC_FOPTBIN, uimsg2, sizeof(uimsg2));
      get_lang_msg("DLG_FOPT_BINARY", uimsg, sizeof(uimsg), uimsg2, UILanguageFile);
      SetDlgItemText(Dialog, IDC_FOPTBIN, uimsg);
      GetDlgItemText(Dialog, IDC_FOPTAPPEND, uimsg2, sizeof(uimsg2));
      get_lang_msg("DLG_FOPT_APPEND", uimsg, sizeof(uimsg), uimsg2, UILanguageFile);
      SetDlgItemText(Dialog, IDC_FOPTAPPEND, uimsg);
      GetDlgItemText(Dialog, IDC_PLAINTEXT, uimsg2, sizeof(uimsg2));
      get_lang_msg("DLG_FOPT_PLAIN", uimsg, sizeof(uimsg), uimsg2, UILanguageFile);
      SetDlgItemText(Dialog, IDC_PLAINTEXT, uimsg);
      GetDlgItemText(Dialog, IDC_TIMESTAMP, uimsg2, sizeof(uimsg2));
      get_lang_msg("DLG_FOPT_TIMESTAMP", uimsg, sizeof(uimsg), uimsg2, UILanguageFile);
      SetDlgItemText(Dialog, IDC_TIMESTAMP, uimsg);

		Lo = LOWORD(*pl) & 1;
		Hi = HIWORD(*pl);
		SetRB(Dialog,Lo,IDC_FOPTBIN,IDC_FOPTBIN);
		if (Hi!=0xFFFF)
		{
			ShowDlgItem(Dialog,IDC_FOPTAPPEND,IDC_FOPTAPPEND);
			SetRB(Dialog,Hi & 1,IDC_FOPTAPPEND,IDC_FOPTAPPEND);

			// plain textチェックボックスはデフォルトでON (2005.2.20 yutaka)
			ShowDlgItem(Dialog,IDC_PLAINTEXT,IDC_PLAINTEXT);
			if (Hi & 0x1000) {
				SetRB(Dialog,1,IDC_PLAINTEXT,IDC_PLAINTEXT);
			}

			// timestampチェックボックス (2006.7.23 maya)
			ShowDlgItem(Dialog,IDC_TIMESTAMP,IDC_TIMESTAMP);
			if (Hi & 0x2000) {
				SetRB(Dialog,1,IDC_TIMESTAMP,IDC_TIMESTAMP);
			}
		}
		return TRUE;

	case WM_COMMAND: // for old style dialog
		switch (LOWORD(wParam)) {
	case IDOK:
		pl = (LPLONG)GetWindowLong(Dialog,DWL_USER);
		if (pl!=NULL)
		{
			GetRB(Dialog,&Lo,IDC_FOPTBIN,IDC_FOPTBIN);
			Hi = HIWORD(*pl);
			if (Hi!=0xFFFF)
				GetRB(Dialog,&Hi,IDC_FOPTAPPEND,IDC_FOPTAPPEND);
			*pl = MAKELONG(Lo,Hi);
		}
		break;
	case IDCANCEL:
		break;
		}
		break;
#ifdef TERATERM32
	case WM_NOTIFY:	// for Explorer-style dialog
		notify = (LPOFNOTIFY)lParam;
		switch (notify->hdr.code) {
	case CDN_FILEOK:
		pl = (LPLONG)GetWindowLong(Dialog,DWL_USER);
		if (pl!=NULL)
		{
			WORD val = 0;

			GetRB(Dialog,&Lo,IDC_FOPTBIN,IDC_FOPTBIN);
			Hi = HIWORD(*pl);
			if (Hi!=0xFFFF)
				GetRB(Dialog,&Hi,IDC_FOPTAPPEND,IDC_FOPTAPPEND);

			// plain text check-box
			GetRB(Dialog,&val,IDC_PLAINTEXT,IDC_PLAINTEXT);
			if (val > 0) { // checked
				Hi |= 0x1000;
			}

			// timestampチェックボックス (2006.7.23 maya)
			GetRB(Dialog,&val,IDC_TIMESTAMP,IDC_TIMESTAMP);
			if (val > 0) {
				Hi |= 0x2000;
			}

			*pl = MAKELONG(Lo,Hi);
		}
		if (DlgFoptFont != NULL) {
			DeleteObject(DlgFoptFont);
		}
		break;
		}
		break;
#endif
	}
	return FALSE;
}

#ifndef TERATERM32
  typedef UINT (CALLBACK *LPOFNHOOKPROC)(HWND, UINT, WPARAM, LPARAM);
#endif

BOOL FAR PASCAL GetTransFname
  (PFileVar fv, PCHAR CurDir, WORD FuncId, LPLONG Option)
{
	char uimsg[MAX_UIMSG];
	char FNFilter[sizeof(FileSendFilter)*3], *pf;
	OPENFILENAME ofn;
	LONG opt;
	char TempDir[MAXPATHLEN];
	BOOL Ok;
	char FileName[MAX_PATH];

	/* save current dir */
	_getcwd(TempDir,sizeof(TempDir));

	memset(FNFilter, 0, sizeof(FNFilter));  /* Set up for double null at end */
	memset(&ofn, 0, sizeof(OPENFILENAME));

	strncpy_s(fv->DlgCaption, sizeof(fv->DlgCaption),"Tera Term: ", _TRUNCATE);
	pf = FNFilter;
	switch (FuncId) {
	case GTF_SEND:
		get_lang_msg("FILEDLG_TRANS_TITLE_SENDFILE", uimsg, sizeof(uimsg), TitSendFile, UILanguageFile);
		strncat_s(fv->DlgCaption, sizeof(fv->DlgCaption), uimsg, _TRUNCATE);
		if (strlen(FileSendFilter) > 0) {
			get_lang_msg("FILEDLG_USER_FILTER_NAME", uimsg, sizeof(uimsg), "User define", UILanguageFile);
			_snprintf_s(FNFilter, sizeof(FNFilter), _TRUNCATE, "%s(%s)", uimsg, FileSendFilter);
			pf = pf + strlen(FNFilter) + 1;
			strncat_s(FNFilter, sizeof(FNFilter) ,FileSendFilter, _TRUNCATE);
			pf = pf + strlen(pf) + 1;
		}
		break;
	case GTF_LOG:
		get_lang_msg("FILEDLG_TRANS_TITLE_LOG", uimsg, sizeof(uimsg), TitLog, UILanguageFile);
		strncat_s(fv->DlgCaption, sizeof(fv->DlgCaption), uimsg, _TRUNCATE);
		break;
	case GTF_BP:
		get_lang_msg("FILEDLG_TRANS_TITLE_BPSEND", uimsg, sizeof(uimsg), TitBPSend, UILanguageFile);
		strncat_s(fv->DlgCaption, sizeof(fv->DlgCaption), uimsg, _TRUNCATE);
		if (strlen(FileSendFilter) > 0) {
			get_lang_msg("FILEDLG_USER_FILTER_NAME", uimsg, sizeof(uimsg), "User define", UILanguageFile);
			_snprintf_s(FNFilter, sizeof(FNFilter), _TRUNCATE, "%s(%s)", uimsg, FileSendFilter);
			pf = pf + strlen(FNFilter) + 1;
			strncat_s(FNFilter, sizeof(FNFilter) ,FileSendFilter, _TRUNCATE);
			pf = pf + strlen(pf) + 1;
		}
		break;
	default: return FALSE;
	}

	get_lang_msg("FILEDLG_ALL_FILTER", uimsg, sizeof(uimsg), "All(*.*)\\0*.*\\0\\0", UILanguageFile);
	// \0\0 で終わる必要があるので 2 バイト
	memcpy(pf, uimsg, sizeof(FNFilter) - (pf - FNFilter + 2));

	ExtractFileName(fv->FullName, FileName ,sizeof(FileName));
	strncpy_s(fv->FullName, sizeof(fv->FullName), FileName, _TRUNCATE);
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner   = fv->HMainWin;
	ofn.lpstrFilter = FNFilter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = fv->FullName;
	ofn.nMaxFile = sizeof(fv->FullName);
	ofn.lpstrInitialDir = CurDir;
	ofn.Flags = OFN_SHOWHELP | OFN_HIDEREADONLY;
	if (FuncId!=GTF_BP)
	{
		ofn.Flags = ofn.Flags | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK;
#ifdef TERATERM32
		if (IS_WIN4())
		{
			ofn.Flags = ofn.Flags | OFN_EXPLORER;
			ofn.lpTemplateName = MAKEINTRESOURCE(IDD_FOPT);
		}
		else {
			ofn.lpTemplateName = MAKEINTRESOURCE(IDD_FOPTOLD);
		}
		ofn.lpfnHook = (LPOFNHOOKPROC)(&TFnHook);
#else
		ofn.lpTemplateName = MAKEINTRESOURCE(IDD_FOPTOLD);
		ofn.lpfnHook = (LPOFNHOOKPROC)MakeProcInstance(TFnHook, hInst);
#endif
	}
	opt = *Option;
	if (FuncId!=GTF_LOG)
	{
		ofn.Flags = ofn.Flags | OFN_FILEMUSTEXIST;
		opt = MAKELONG(LOWORD(*Option),0xFFFF);
	}
	ofn.lCustData = (DWORD)&opt;
	ofn.lpstrTitle = fv->DlgCaption;

	ofn.hInstance = hInst;

	// loggingの場合、オープンダイアログをセーブダイアログへ変更 (2005.1.6 yutaka)
	if (FuncId == GTF_LOG) {
		// ログのデフォルト値(log_YYYYMMDD_HHMMSS.txt)を設定する (2005.1.21 yutaka)
		// デフォルトファイル名を teraterm.log へ変更 (2005.2.22 yutaka)
		// デフォルトファイル名の設定場所を呼び出し元へ移動 (2006.8.28 maya)
		Ok = GetSaveFileName(&ofn);
	} else {
		Ok = GetOpenFileName(&ofn);
	}

#ifndef TERATERM32
	FreeProcInstance(ofn.lpfnHook);
#endif
	if (Ok)
	{
		if (FuncId==GTF_LOG)
			*Option = opt;
		else
			*Option = MAKELONG(LOWORD(opt),HIWORD(*Option));

		fv->DirLen = ofn.nFileOffset;

#ifdef TERATERM32
		// for Win NT 3.5: short name -> long name
		GetLongFName(fv->FullName,&fv->FullName[fv->DirLen],sizeof(&fv->FullName) - fv->DirLen);
#endif

		if (CurDir!=NULL)
		{
			memcpy(CurDir,fv->FullName,fv->DirLen-1);
			CurDir[fv->DirLen-1] = 0;
		}
	}
	/* restore dir */
	_chdir(TempDir);
	return Ok;
}

BOOL CALLBACK TFn2Hook(HWND Dialog, UINT Message, WPARAM wParam, LPARAM lParam)
{
  LPOPENFILENAME ofn;
  LPWORD pw;
#ifdef TERATERM32
  LPOFNOTIFY notify;
#endif
	char uimsg[MAX_UIMSG], uimsg2[MAX_UIMSG];
	LOGFONT logfont;
	HFONT font;

  switch (Message) {
    case WM_INITDIALOG:
      ofn = (LPOPENFILENAME)lParam;
      pw = (LPWORD)ofn->lCustData;
      SetWindowLong(Dialog, DWL_USER, (LONG)pw);

      font = (HFONT)SendMessage(Dialog, WM_GETFONT, 0, 0);
      GetObject(font, sizeof(LOGFONT), &logfont);
      if (get_lang_font("DLG_TAHOMA_FONT", Dialog, &logfont, &DlgFoptFont, UILanguageFile)) {
        SendDlgItemMessage(Dialog, IDC_FOPT, WM_SETFONT, (WPARAM)DlgFoptFont, MAKELPARAM(TRUE,0));
        SendDlgItemMessage(Dialog, IDC_FOPTBIN, WM_SETFONT, (WPARAM)DlgFoptFont, MAKELPARAM(TRUE,0));
        SendDlgItemMessage(Dialog, IDC_FOPTAPPEND, WM_SETFONT, (WPARAM)DlgFoptFont, MAKELPARAM(TRUE,0));
        SendDlgItemMessage(Dialog, IDC_PLAINTEXT, WM_SETFONT, (WPARAM)DlgFoptFont, MAKELPARAM(TRUE,0));
        SendDlgItemMessage(Dialog, IDC_TIMESTAMP, WM_SETFONT, (WPARAM)DlgFoptFont, MAKELPARAM(TRUE,0));
      }
      else {
        DlgFoptFont = NULL;
      }

      GetDlgItemText(Dialog, IDC_FOPT, uimsg2, sizeof(uimsg2));
      get_lang_msg("DLG_FOPT", uimsg, sizeof(uimsg), uimsg2, UILanguageFile);
      SetDlgItemText(Dialog, IDC_FOPT, uimsg);
      GetDlgItemText(Dialog, IDC_FOPTBIN, uimsg2, sizeof(uimsg2));
      get_lang_msg("DLG_FOPT_BINARY", uimsg, sizeof(uimsg), uimsg2, UILanguageFile);
      SetDlgItemText(Dialog, IDC_FOPTBIN, uimsg);

      SetRB(Dialog,*pw & 1,IDC_FOPTBIN,IDC_FOPTBIN);
      return TRUE;
    case WM_COMMAND: // for old style dialog
      switch (LOWORD(wParam)) {
	case IDOK:
	  pw = (LPWORD)GetWindowLong(Dialog,DWL_USER);
	  if (pw!=NULL)
	    GetRB(Dialog,pw,IDC_FOPTBIN,IDC_FOPTBIN);
	  break;
	case IDCANCEL:
	  break;
      }
      break;
#ifdef TERATERM32
    case WM_NOTIFY: // for Explorer-style dialog
      notify = (LPOFNOTIFY)lParam;
      switch (notify->hdr.code) {
	case CDN_FILEOK:
	  pw = (LPWORD)GetWindowLong(Dialog,DWL_USER);
	  if (pw!=NULL)
	    GetRB(Dialog,pw,IDC_FOPTBIN,IDC_FOPTBIN);
		if (DlgFoptFont != NULL) {
			DeleteObject(DlgFoptFont);
		}
	  break;
      }
      break;
#endif
  }
  return FALSE;
}

BOOL FAR PASCAL GetMultiFname
  (PFileVar fv, PCHAR CurDir, WORD FuncId, LPWORD Option)
{
  int i, len;
  char uimsg[MAX_UIMSG];
  char FNFilter[sizeof(FileSendFilter)*2+128], *pf;
  OPENFILENAME ofn;
  char TempDir[MAXPATHLEN];
  BOOL Ok;

  memset(FNFilter, 0, sizeof(FNFilter));  /* Set up for double null at end */

  /* save current dir */
  _getcwd(TempDir,sizeof(TempDir));

  fv->NumFname = 0;

  strncpy_s(fv->DlgCaption, sizeof(fv->DlgCaption),"Tera Term: ", _TRUNCATE);
  pf = FNFilter;
  switch (FuncId) {
    case GMF_KERMIT:
      get_lang_msg("FILEDLG_TRANS_TITLE_KMTSEND", uimsg, sizeof(uimsg), TitKmtSend, UILanguageFile);
      strncat_s(fv->DlgCaption, sizeof(fv->DlgCaption), uimsg, _TRUNCATE);
      break;
    case GMF_Z:
      get_lang_msg("FILEDLG_TRANS_TITLE_ZSEND", uimsg, sizeof(uimsg), TitZSend, UILanguageFile);
      strncat_s(fv->DlgCaption, sizeof(fv->DlgCaption), uimsg, _TRUNCATE);
      break;
    case GMF_QV:
      get_lang_msg("FILEDLG_TRANS_TITLE_QVSEND", uimsg, sizeof(uimsg), TitQVSend, UILanguageFile);
      strncat_s(fv->DlgCaption, sizeof(fv->DlgCaption), uimsg, _TRUNCATE);
      break;
    default: return FALSE;
  }
  if (strlen(FileSendFilter) > 0) {
    get_lang_msg("FILEDLG_USER_FILTER_NAME", uimsg, sizeof(uimsg), "User define", UILanguageFile);
    _snprintf_s(FNFilter, sizeof(FNFilter), _TRUNCATE, "%s(%s)", uimsg, FileSendFilter);
    pf = pf + strlen(FNFilter) + 1;
    strncat_s(FNFilter, sizeof(FNFilter) ,FileSendFilter, _TRUNCATE);
    pf = pf + strlen(pf) + 1;
  }

  /* moemory should be zero-initialized */
  fv->FnStrMemHandle = GlobalAlloc(GHND, FnStrMemSize);
  if (fv->FnStrMemHandle == NULL)
  {
    MessageBeep(0);
    return FALSE;
  }
  else {
    fv->FnStrMem = GlobalLock(fv->FnStrMemHandle);
    if (fv->FnStrMem == NULL)
    {
      GlobalFree(fv->FnStrMemHandle);
      fv->FnStrMemHandle = 0;
      MessageBeep(0);
      return FALSE;
    }
  }

  get_lang_msg("FILEDLG_ALL_FILTER", uimsg, sizeof(uimsg), "All(*.*)\\0*.*\\0\\0", UILanguageFile);
  // \0\0 で終わる必要があるので 2 バイト
  memcpy(pf, uimsg, sizeof(FNFilter) - (pf - FNFilter + 2));

  memset(&ofn, 0, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner   = fv->HMainWin;
  ofn.lpstrFilter = FNFilter;
  ofn.nFilterIndex = 1;
  ofn.lpstrFile = fv->FnStrMem;
  ofn.nMaxFile = FnStrMemSize;
  ofn.lpstrTitle= fv->DlgCaption;
  ofn.lpstrInitialDir = CurDir;
  ofn.Flags = OFN_SHOWHELP | OFN_ALLOWMULTISELECT |
	      OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
#ifdef TERATERM32
  if (IS_WIN4())
    ofn.Flags = ofn.Flags | OFN_EXPLORER;
#endif
  ofn.lCustData = 0;
  if (FuncId==GMF_Z)
  {
    ofn.Flags = ofn.Flags | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK;
    ofn.lCustData = (DWORD)Option;
#ifdef TERATERM32
    ofn.lpfnHook = (LPOFNHOOKPROC)(&TFn2Hook);
    if (IS_WIN4())
      ofn.lpTemplateName = MAKEINTRESOURCE(IDD_FOPT);
    else
      ofn.lpTemplateName = MAKEINTRESOURCE(IDD_ZOPTOLD);
#else
    ofn.lpfnHook = (LPOFNHOOKPROC)MakeProcInstance(TFn2Hook, hInst);
    ofn.lpTemplateName = MAKEINTRESOURCE(IDD_ZOPTOLD);
#endif
  }
  ofn.hInstance = hInst;

  Ok = GetOpenFileName(&ofn);
#ifndef TERATERM32
  FreeProcInstance(ofn.lpfnHook);
#endif
  if (Ok)
  {
#ifdef TERATERM32
    if (! IS_WIN4())
    {  // for old style dialog box
#endif
      i = 0;
      do { // replace space by NULL
	if (fv->FnStrMem[i]==' ')
	  fv->FnStrMem[i] = 0;
	i++;
      } while (fv->FnStrMem[i]!=0);
      fv->FnStrMem[i+1] = 0; // add one more NULL
#ifdef TERATERM32
    }
#endif
    /* count number of file names */
    len = strlen(fv->FnStrMem);
    i = 0;
    while (len>0)
    {
      i = i + len + 1;
      fv->NumFname++;
      len = strlen(&fv->FnStrMem[i]);
    }

    fv->NumFname--;

    if (fv->NumFname<1)
    { // single selection
      fv->NumFname = 1;
      fv->DirLen = ofn.nFileOffset;
      strncpy_s(fv->FullName, sizeof(fv->FullName),fv->FnStrMem, _TRUNCATE);
      fv->FnPtr = 0;
#ifdef TERATERM32
      // for Win NT 3.5: short name -> long name
      GetLongFName(fv->FullName,&fv->FullName[fv->DirLen],sizeof(&fv->FullName) - fv->DirLen);
#endif
    }
    else { // multiple selection
      strncpy_s(fv->FullName, sizeof(fv->FullName),fv->FnStrMem, _TRUNCATE);
      AppendSlash(fv->FullName,sizeof(fv->FullName));
      fv->DirLen = strlen(fv->FullName);
      fv->FnPtr = strlen(fv->FnStrMem)+1;
    }

    memcpy(CurDir,fv->FullName,fv->DirLen);
    CurDir[fv->DirLen] = 0;
    if ((fv->DirLen>3) &&
	(CurDir[fv->DirLen-1]=='\\'))
      CurDir[fv->DirLen-1] = 0;

    fv->FNCount = 0;
  }

  GlobalUnlock(fv->FnStrMemHandle);
  if (! Ok)
  {
    GlobalFree(fv->FnStrMemHandle);
    fv->FnStrMemHandle = NULL;
  }

  /* restore dir */
  _chdir(TempDir);

  return Ok;
}

BOOL CALLBACK GetFnDlg
  (HWND Dialog, UINT Message, WPARAM wParam, LPARAM lParam)
{
  PFileVar fv;
  char TempFull[MAXPATHLEN];
  int i, j;
	char uimsg[MAX_UIMSG], uimsg2[MAX_UIMSG];
	LOGFONT logfont;
	HFONT font;

  switch (Message) {
    case WM_INITDIALOG:
      fv = (PFileVar)lParam;
      SetWindowLong(Dialog, DWL_USER, lParam);
      SendDlgItemMessage(Dialog, IDC_GETFN, EM_LIMITTEXT, sizeof(TempFull)-1,0);

      font = (HFONT)SendMessage(Dialog, WM_GETFONT, 0, 0);
      GetObject(font, sizeof(LOGFONT), &logfont);
      if (get_lang_font("DLG_SYSTEM_FONT", Dialog, &logfont, &DlgGetfnFont, UILanguageFile)) {
        SendDlgItemMessage(Dialog, IDC_FILENAME, WM_SETFONT, (WPARAM)DlgGetfnFont, MAKELPARAM(TRUE,0));
        SendDlgItemMessage(Dialog, IDC_GETFN, WM_SETFONT, (WPARAM)DlgGetfnFont, MAKELPARAM(TRUE,0));
        SendDlgItemMessage(Dialog, IDOK, WM_SETFONT, (WPARAM)DlgGetfnFont, MAKELPARAM(TRUE,0));
        SendDlgItemMessage(Dialog, IDCANCEL, WM_SETFONT, (WPARAM)DlgGetfnFont, MAKELPARAM(TRUE,0));
        SendDlgItemMessage(Dialog, IDC_GETFNHELP, WM_SETFONT, (WPARAM)DlgGetfnFont, MAKELPARAM(TRUE,0));
      }
      else {
        DlgGetfnFont = NULL;
      }

	  GetWindowText(Dialog, uimsg2, sizeof(uimsg2));
	  get_lang_msg("DLG_GETFN_TITLE", uimsg, sizeof(uimsg), uimsg2, UILanguageFile);
	  SetWindowText(Dialog, uimsg);
	  GetDlgItemText(Dialog, IDC_FILENAME, uimsg2, sizeof(uimsg2));
	  get_lang_msg("DLG_GETFN_FILENAME", uimsg, sizeof(uimsg), uimsg2, UILanguageFile);
	  SetDlgItemText(Dialog, IDC_FILENAME, uimsg);
	  GetDlgItemText(Dialog, IDOK, uimsg2, sizeof(uimsg2));
	  get_lang_msg("BTN_OK", uimsg, sizeof(uimsg), uimsg2, UILanguageFile);
	  SetDlgItemText(Dialog, IDOK, uimsg);
	  GetDlgItemText(Dialog, IDCANCEL, uimsg2, sizeof(uimsg2));
	  get_lang_msg("BTN_CANCEL", uimsg, sizeof(uimsg), uimsg2, UILanguageFile);
	  SetDlgItemText(Dialog, IDCANCEL, uimsg);
	  GetDlgItemText(Dialog, IDC_GETFNHELP, uimsg2, sizeof(uimsg2));
	  get_lang_msg("BTN_HELP", uimsg, sizeof(uimsg), uimsg2, UILanguageFile);
	  SetDlgItemText(Dialog, IDC_GETFNHELP, uimsg);

      return TRUE;
    case WM_COMMAND:
      fv = (PFileVar)GetWindowLong(Dialog,DWL_USER);
      switch (LOWORD(wParam)) {
	case IDOK:
	  if (fv!=NULL)
	  {
	    GetDlgItemText(Dialog, IDC_GETFN, TempFull, sizeof(TempFull));
	    if (strlen(TempFull)==0) return TRUE;
	    GetFileNamePos(TempFull,&i,&j);
	    FitFileName(&(TempFull[j]),sizeof(TempFull) - j, NULL);
	    strncat_s(fv->FullName,sizeof(fv->FullName),&(TempFull[j]),_TRUNCATE);
	  }
	  EndDialog(Dialog, 1);
	  if (DlgGetfnFont != NULL) {
	    DeleteObject(DlgGetfnFont);
	  }
	  return TRUE;
	case IDCANCEL:
	  EndDialog(Dialog, 0);
	  if (DlgGetfnFont != NULL) {
	    DeleteObject(DlgGetfnFont);
	  }
	  return TRUE;
	case IDC_GETFNHELP:
	  if (fv!=NULL)
	    PostMessage(fv->HMainWin,WM_USER_DLGHELP2,0,0);
	  break;
      }
  }
  return FALSE;
}

BOOL FAR PASCAL GetGetFname(HWND HWin, PFileVar fv)
{
#ifndef TERATERM32
  DLGPROC GetFnProc;
  BOOL Ok;
#endif

#ifdef TERATERM32
  return
    (BOOL)DialogBoxParam(hInst,
	  MAKEINTRESOURCE(IDD_GETFNDLG),
	  HWin, GetFnDlg, (LONG)fv);
#else
  GetFnProc = MakeProcInstance(GetFnDlg, hInst);
  Ok = (BOOL)DialogBoxParam(hInst,
	  MAKEINTRESOURCE(IDD_GETFNDLG),
	  HWin, GetFnProc, (LPARAM)fv);
  FreeProcInstance(GetFnProc);
  return Ok;
#endif
}

void FAR PASCAL SetFileVar(PFileVar fv)
{
  int i;
  char uimsg[MAX_UIMSG];

  GetFileNamePos(fv->FullName,&(fv->DirLen),&i);
  if (fv->FullName[fv->DirLen]=='\\') fv->DirLen++;
  strncpy_s(fv->DlgCaption, sizeof(fv->DlgCaption),"Tera Term: ", _TRUNCATE);
  switch (fv->OpId) {
    case OpLog:
      get_lang_msg("FILEDLG_TRANS_TITLE_LOG", uimsg, sizeof(uimsg), TitLog, UILanguageFile);
      strncat_s(fv->DlgCaption, sizeof(fv->DlgCaption), uimsg, _TRUNCATE);
    break;
    case OpSendFile:
      get_lang_msg("FILEDLG_TRANS_TITLE_SENDFILE", uimsg, sizeof(uimsg), TitSendFile, UILanguageFile);
      strncat_s(fv->DlgCaption, sizeof(fv->DlgCaption), uimsg, _TRUNCATE);
      break;
    case OpKmtRcv:
      get_lang_msg("FILEDLG_TRANS_TITLE_KMTRCV", uimsg, sizeof(uimsg), TitKmtRcv, UILanguageFile);
      strncat_s(fv->DlgCaption, sizeof(fv->DlgCaption), uimsg, _TRUNCATE);
      break;
    case OpKmtGet:
      get_lang_msg("FILEDLG_TRANS_TITLE_KMTGET", uimsg, sizeof(uimsg), TitKmtGet, UILanguageFile);
      strncat_s(fv->DlgCaption, sizeof(fv->DlgCaption), uimsg, _TRUNCATE);
      break;
    case OpKmtSend:
      get_lang_msg("FILEDLG_TRANS_TITLE_KMTSEND", uimsg, sizeof(uimsg), TitKmtSend, UILanguageFile);
      strncat_s(fv->DlgCaption, sizeof(fv->DlgCaption), uimsg, _TRUNCATE);
      break;
    case OpKmtFin:
      get_lang_msg("FILEDLG_TRANS_TITLE_KMTFIN", uimsg, sizeof(uimsg), TitKmtFin, UILanguageFile);
      strncat_s(fv->DlgCaption, sizeof(fv->DlgCaption), uimsg, _TRUNCATE);
      break;
    case OpXRcv:
      get_lang_msg("FILEDLG_TRANS_TITLE_XRCV", uimsg, sizeof(uimsg), TitXRcv, UILanguageFile);
      strncat_s(fv->DlgCaption, sizeof(fv->DlgCaption), uimsg, _TRUNCATE);
      break;
    case OpXSend:
      get_lang_msg("FILEDLG_TRANS_TITLE_XSEND", uimsg, sizeof(uimsg), TitXSend, UILanguageFile);
      strncat_s(fv->DlgCaption, sizeof(fv->DlgCaption), uimsg, _TRUNCATE);
      break;
    case OpZRcv:
      get_lang_msg("FILEDLG_TRANS_TITLE_ZRCV", uimsg, sizeof(uimsg), TitZRcv, UILanguageFile);
      strncat_s(fv->DlgCaption, sizeof(fv->DlgCaption), uimsg, _TRUNCATE);
      break;
    case OpZSend:
      get_lang_msg("FILEDLG_TRANS_TITLE_ZSEND", uimsg, sizeof(uimsg), TitZSend, UILanguageFile);
      strncat_s(fv->DlgCaption, sizeof(fv->DlgCaption), uimsg, _TRUNCATE);
      break;
    case OpBPRcv:
      get_lang_msg("FILEDLG_TRANS_TITLE_BPRCV", uimsg, sizeof(uimsg), TitBPRcv, UILanguageFile);
      strncat_s(fv->DlgCaption, sizeof(fv->DlgCaption), uimsg, _TRUNCATE);
      break;
    case OpBPSend:
      get_lang_msg("FILEDLG_TRANS_TITLE_BPSEND", uimsg, sizeof(uimsg), TitBPSend, UILanguageFile);
      strncat_s(fv->DlgCaption, sizeof(fv->DlgCaption), uimsg, _TRUNCATE);
      break;
    case OpQVRcv:
      get_lang_msg("FILEDLG_TRANS_TITLE_QVRCV", uimsg, sizeof(uimsg), TitQVRcv, UILanguageFile);
      strncat_s(fv->DlgCaption, sizeof(fv->DlgCaption), uimsg, _TRUNCATE);
      break;
    case OpQVSend:
      get_lang_msg("FILEDLG_TRANS_TITLE_QVSEND", uimsg, sizeof(uimsg), TitQVSend, UILanguageFile);
      strncat_s(fv->DlgCaption, sizeof(fv->DlgCaption), uimsg, _TRUNCATE);
      break;
  }
}

/* Hook function for XMODEM file name dialog box */
BOOL CALLBACK XFnHook(HWND Dialog, UINT Message, WPARAM wParam, LPARAM lParam)
{
  LPOPENFILENAME ofn;
  WORD Hi, Lo;
  LPLONG pl;
#ifdef TERATERM32
  LPOFNOTIFY notify;
#endif
	char uimsg[MAX_UIMSG], uimsg2[MAX_UIMSG];
	LOGFONT logfont;
	HFONT font;

  switch (Message) {
    case WM_INITDIALOG:
      ofn = (LPOPENFILENAME)lParam;
      pl = (LPLONG)ofn->lCustData;
      SetWindowLong(Dialog, DWL_USER, (LONG)pl);

      font = (HFONT)SendMessage(Dialog, WM_GETFONT, 0, 0);
      GetObject(font, sizeof(LOGFONT), &logfont);
      if (get_lang_font("DLG_TAHOMA_FONT", Dialog, &logfont, &DlgFoptFont, UILanguageFile)) {
        SendDlgItemMessage(Dialog, IDC_XOPT, WM_SETFONT, (WPARAM)DlgFoptFont, MAKELPARAM(TRUE,0));
        SendDlgItemMessage(Dialog, IDC_XOPTCHECK, WM_SETFONT, (WPARAM)DlgFoptFont, MAKELPARAM(TRUE,0));
        SendDlgItemMessage(Dialog, IDC_XOPTCRC, WM_SETFONT, (WPARAM)DlgFoptFont, MAKELPARAM(TRUE,0));
        SendDlgItemMessage(Dialog, IDC_XOPT1K, WM_SETFONT, (WPARAM)DlgFoptFont, MAKELPARAM(TRUE,0));
        SendDlgItemMessage(Dialog, IDC_XOPTBIN, WM_SETFONT, (WPARAM)DlgFoptFont, MAKELPARAM(TRUE,0));
      }
      else {
        DlgFoptFont = NULL;
      }

      GetDlgItemText(Dialog, IDC_XOPT, uimsg2, sizeof(uimsg2));
      get_lang_msg("DLG_XOPT", uimsg, sizeof(uimsg), uimsg2, UILanguageFile);
      SetDlgItemText(Dialog, IDC_XOPT, uimsg);
      GetDlgItemText(Dialog, IDC_XOPTCHECK, uimsg2, sizeof(uimsg2));
      get_lang_msg("DLG_XOPT_CHECKSUM", uimsg, sizeof(uimsg), uimsg2, UILanguageFile);
      SetDlgItemText(Dialog, IDC_XOPTCHECK, uimsg);
      GetDlgItemText(Dialog, IDC_XOPTCRC, uimsg2, sizeof(uimsg2));
      get_lang_msg("DLG_XOPT_CRC", uimsg, sizeof(uimsg), uimsg2, UILanguageFile);
      SetDlgItemText(Dialog, IDC_XOPTCRC, uimsg);
      GetDlgItemText(Dialog, IDC_XOPT1K, uimsg2, sizeof(uimsg2));
      get_lang_msg("DLG_XOPT_1K", uimsg, sizeof(uimsg), uimsg2, UILanguageFile);
      SetDlgItemText(Dialog, IDC_XOPT1K, uimsg);
      GetDlgItemText(Dialog, IDC_XOPTBIN, uimsg2, sizeof(uimsg2));
      get_lang_msg("DLG_XOPT_BINARY", uimsg, sizeof(uimsg), uimsg2, UILanguageFile);
      SetDlgItemText(Dialog, IDC_XOPTBIN, uimsg);

      SetRB(Dialog,HIWORD(*pl),IDC_XOPTCHECK,IDC_XOPT1K);
      if (LOWORD(*pl)!=0xFFFF)
      {
	ShowDlgItem(Dialog,IDC_XOPTBIN,IDC_XOPTBIN);
	SetRB(Dialog,LOWORD(*pl),IDC_XOPTBIN,IDC_XOPTBIN);
      }
      return TRUE;
    case WM_COMMAND: // for old style dialog
      switch (LOWORD(wParam)) {
	case IDOK:
	  pl = (LPLONG)GetWindowLong(Dialog,DWL_USER);
	  if (pl!=NULL)
	  {
	    GetRB(Dialog,&Hi,IDC_XOPTCHECK,IDC_XOPT1K);
	    if (LOWORD(*pl)==0xFFFF)
	      Lo = 0xFFFF;
	    else
	      GetRB(Dialog,&Lo,IDC_XOPTBIN,IDC_XOPTBIN);
	    *pl = MAKELONG(Lo,Hi);
	  }
	  break;
	case IDCANCEL:
	  break;
      }
      break;
#ifdef TERATERM32
    case WM_NOTIFY:	// for Explorer-style dialog
      notify = (LPOFNOTIFY)lParam;
      switch (notify->hdr.code) {
	case CDN_FILEOK:
	  pl = (LPLONG)GetWindowLong(Dialog,DWL_USER);
	  if (pl!=NULL)
	  {
	    GetRB(Dialog,&Hi,IDC_XOPTCHECK,IDC_XOPT1K);
	    if (LOWORD(*pl)==0xFFFF)
	      Lo = 0xFFFF;
	    else
	      GetRB(Dialog,&Lo,IDC_XOPTBIN,IDC_XOPTBIN);
	    *pl = MAKELONG(Lo,Hi);
	  }
	if (DlgXoptFont != NULL) {
		DeleteObject(DlgXoptFont);
	}
	  break;
      }
      break;
#endif
  }
  return FALSE;
}

BOOL FAR PASCAL GetXFname
  (HWND HWin, BOOL Receive, LPLONG Option, PFileVar fv, PCHAR CurDir)
{
  char uimsg[MAX_UIMSG];
  char FNFilter[sizeof(FileSendFilter)*2+128], *pf;
  OPENFILENAME ofn;
  LONG opt;
  char TempDir[MAXPATHLEN];
  BOOL Ok;

  memset(FNFilter, 0, sizeof(FNFilter));  /* Set up for double null at end */

  /* save current dir */
  _getcwd(TempDir,sizeof(TempDir));

  fv->FullName[0] = 0;
  memset(FNFilter, 0, sizeof(FNFilter));  /* Set up for double null at end */
  memset(&ofn, 0, sizeof(OPENFILENAME));

  strncpy_s(fv->DlgCaption, sizeof(fv->DlgCaption),"Tera Term: ", _TRUNCATE);
  pf = FNFilter;
  if (Receive)
  {
    get_lang_msg("FILEDLG_TRANS_TITLE_XRCV", uimsg, sizeof(uimsg), TitXRcv, UILanguageFile);
    strncat_s(fv->DlgCaption, sizeof(fv->DlgCaption), uimsg, _TRUNCATE);
  }
  else
  {
    get_lang_msg("FILEDLG_TRANS_TITLE_XSEND", uimsg, sizeof(uimsg), TitXSend, UILanguageFile);
    strncat_s(fv->DlgCaption, sizeof(fv->DlgCaption), uimsg, _TRUNCATE);
    if (strlen(FileSendFilter) > 0) {
      get_lang_msg("FILEDLG_USER_FILTER_NAME", uimsg, sizeof(uimsg), "User define", UILanguageFile);
      _snprintf_s(FNFilter, sizeof(FNFilter), _TRUNCATE, "%s(%s)", uimsg, FileSendFilter);
      pf = pf + strlen(FNFilter) + 1;
      strncat_s(FNFilter, sizeof(FNFilter) ,FileSendFilter, _TRUNCATE);
      pf = pf + strlen(pf) + 1;
    }
  }

  get_lang_msg("FILEDLG_ALL_FILTER", uimsg, sizeof(uimsg), "All(*.*)\\0*.*\\0\\0", UILanguageFile);
  // \0\0 で終わる必要があるので 2 バイト
  memcpy(pf, uimsg, sizeof(FNFilter) - (pf - FNFilter + 2));

  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner   = HWin;
  ofn.lpstrFilter = FNFilter;
  ofn.nFilterIndex = 1;
  ofn.lpstrFile = fv->FullName;
  ofn.nMaxFile = sizeof(fv->FullName);
  ofn.lpstrInitialDir = CurDir;
  ofn.Flags = OFN_SHOWHELP | OFN_HIDEREADONLY |
	      OFN_ENABLETEMPLATE | OFN_ENABLEHOOK;
  opt = *Option;
  if (! Receive)
  {
    ofn.Flags = ofn.Flags | OFN_FILEMUSTEXIST;
    opt = opt | 0xFFFF;
  }
  ofn.lCustData = (DWORD)&opt;

  ofn.lpstrTitle = fv->DlgCaption;
#ifdef TERATERM32
  ofn.lpfnHook = (LPOFNHOOKPROC)(&XFnHook);
  if (IS_WIN4())
  {
    ofn.Flags = ofn.Flags | OFN_EXPLORER;
    ofn.lpTemplateName = MAKEINTRESOURCE(IDD_XOPT);
  }
  else {
    ofn.lpTemplateName = MAKEINTRESOURCE(IDD_XOPTOLD);
  }
#else
  ofn.lpfnHook = (LPOFNHOOKPROC)MakeProcInstance(XFnHook, hInst);
  ofn.lpTemplateName = MAKEINTRESOURCE(IDD_XOPTOLD);
#endif
  ofn.hInstance = hInst;

  Ok = GetOpenFileName(&ofn);
#ifndef TERATERM32
  FreeProcInstance(ofn.lpfnHook);
#endif
  if (Ok)
  {
    fv->DirLen = ofn.nFileOffset;
    fv->FnPtr = ofn.nFileOffset;
    memcpy(CurDir,fv->FullName,fv->DirLen-1);
    CurDir[fv->DirLen-1] = 0;

    if (Receive)
      *Option = opt;
    else
      *Option = MAKELONG(LOWORD(*Option),HIWORD(opt));
  }

  /* restore dir */
  _chdir(TempDir);

  return Ok;
}

void FAR PASCAL ProtoInit(int Proto, PFileVar fv, PCHAR pv, PComVar cv, PTTSet ts)
{
  switch (Proto) {
    case PROTO_KMT:
      KmtInit(fv,(PKmtVar)pv,cv,ts);
      break;
    case PROTO_XM:
      XInit(fv,(PXVar)pv,cv,ts);
      break;
    case PROTO_ZM:
      ZInit(fv,(PZVar)pv,cv,ts);
      break;
    case PROTO_BP:
      BPInit(fv,(PBPVar)pv,cv,ts);
      break;
    case PROTO_QV:
      QVInit(fv,(PQVVar)pv,cv,ts);
      break;
  }
}

BOOL FAR PASCAL ProtoParse
  (int Proto, PFileVar fv, PCHAR pv, PComVar cv)
{
  BOOL Ok;

  Ok = FALSE;
  switch (Proto) {
    case PROTO_KMT:
      Ok = KmtReadPacket(fv,(PKmtVar)pv,cv);
      break;
    case PROTO_XM:
      switch (((PXVar)pv)->XMode) {
	case IdXReceive:
	  Ok = XReadPacket(fv,(PXVar)pv,cv);
	  break;
	case IdXSend:
	  Ok = XSendPacket(fv,(PXVar)pv,cv);
	  break;
      }
      break;
    case PROTO_ZM:
      Ok = ZParse(fv,(PZVar)pv,cv);
      break;
    case PROTO_BP:
      Ok = BPParse(fv,(PBPVar)pv,cv);
      break;
    case PROTO_QV:
      switch (((PQVVar)pv)->QVMode) {
	case IdQVReceive:
	  Ok = QVReadPacket(fv,(PQVVar)pv,cv);
	  break;
	case IdQVSend:
	  Ok = QVSendPacket(fv,(PQVVar)pv,cv);
	  break;
      }
      break;
  }
  return Ok;
}

void FAR PASCAL ProtoTimeOutProc
  (int Proto, PFileVar fv, PCHAR pv, PComVar cv)
{
  switch (Proto) {
    case PROTO_KMT:
      KmtTimeOutProc(fv,(PKmtVar)pv,cv);
      break;
    case PROTO_XM:
      XTimeOutProc(fv,(PXVar)pv,cv);
      break;
    case PROTO_ZM:
      ZTimeOutProc(fv,(PZVar)pv,cv);
      break;
    case PROTO_BP:
      BPTimeOutProc(fv,(PBPVar)pv,cv);
      break;
    case PROTO_QV:
      QVTimeOutProc(fv,(PQVVar)pv,cv);
      break;
  }
}

BOOL FAR PASCAL ProtoCancel
  (int Proto, PFileVar fv, PCHAR pv, PComVar cv)
{
  switch (Proto) {
    case PROTO_KMT:
      KmtCancel(fv,(PKmtVar)pv,cv);
      break;
    case PROTO_XM:
      if (((PXVar)pv)->XMode==IdXReceive)
	XCancel(fv,(PXVar)pv,cv);
      break;
    case PROTO_ZM:
      ZCancel((PZVar)pv);
      break;
    case PROTO_BP:
      if (((PBPVar)pv)->BPState != BP_Failure)
      {
	BPCancel((PBPVar)pv);
	return FALSE;
      }
      break;
    case PROTO_QV:
      QVCancel(fv,(PQVVar)pv,cv);
      break;
  }
  return TRUE;
}

#ifdef TERATERM32
#ifdef WATCOM
  #pragma off (unreferenced);
#endif
BOOL WINAPI DllMain(HANDLE hInstance,
		    ULONG ul_reason_for_call,
		    LPVOID lpReserved)
#ifdef WATCOM
  #pragma on (unreferenced);
#endif
{
  PMap pm;
  HANDLE HMap = NULL;

  hInst = hInstance;
  switch( ul_reason_for_call ) {
  case DLL_THREAD_ATTACH:
    /* do thread initialization */
    break;
  case DLL_THREAD_DETACH:
    /* do thread cleanup */
    break;
  case DLL_PROCESS_ATTACH:
     /* do process initialization */
      HMap = CreateFileMapping(
        (HANDLE) 0xFFFFFFFF, NULL, PAGE_READONLY,
        0, sizeof(TMap), TT_FILEMAPNAME);
      if (HMap != NULL) {
        pm = (PMap)MapViewOfFile(
        HMap,FILE_MAP_READ,0,0,0);
        if (pm != NULL) {
          strncpy_s(UILanguageFile, sizeof(UILanguageFile), pm->ts.UILanguageFile, _TRUNCATE);
          strncpy_s(FileSendFilter, sizeof(FileSendFilter), pm->ts.FileSendFilter, _TRUNCATE);
        }
      }
    break;
  case DLL_PROCESS_DETACH:
    /* do process cleanup */
    break;
  }
   return TRUE;
 }
#else
#ifdef WATCOM
#pragma off (unreferenced);
#endif
int CALLBACK LibMain(HANDLE hInstance, WORD wDataSegment,
		     WORD wHeapSize, LPSTR lpszCmdLine )
#ifdef WATCOM
#pragma on (unreferenced);
#endif
{
  hInst = hInstance;
  return (1);
}
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.23  2007/08/08 15:58:28  maya
 * 安全な関数を使用するように変更した。
 *
 * Revision 1.22  2007/07/16 13:52:59  maya
 * GetOpenFileName からファイル名を取得できないバグを修正した。
 *
 * Revision 1.21  2007/07/02 10:51:06  doda
 * 旧バージョンとの間では共有メモリによる設定の共有が出来ないため、
 * ファイルマッピングオブジェクト名を変更した。
 *
 * Revision 1.20  2007/06/06 14:04:52  maya
 * プリプロセッサにより構造体が変わってしまうので、INET6 と I18N の #define を逆転させた。
 *
 * Revision 1.19  2007/06/04 15:43:55  maya
 * ファイル送信ダイアログでファイル名フィルタをかけられるようにした。
 *
 * Revision 1.18  2007/05/30 16:05:18  maya
 * 標準のログ保存先を指定できるようにした。
 *
 * Revision 1.17  2007/05/02 04:52:10  maya
 * バッファのサイズを取得する方法を修正した。
 *
 * Revision 1.16  2007/01/31 13:15:27  maya
 * 言語ファイルがないときに \0 が正しく認識されないバグを修正した。
 *
 * Revision 1.15  2007/01/22 13:50:11  maya
 * 表示メッセージの読み込み対応
 *
 * Revision 1.14  2007/01/21 16:18:38  maya
 * 表示メッセージの読み込み対応
 *
 * Revision 1.13  2007/01/04 15:11:48  maya
 * 表示メッセージの読み込み対応
 *
 * Revision 1.12  2006/09/16 07:34:45  maya
 * Windows Vista で teraterm.ini の保存先のデフォルトがデスクトップになるのを修正した。
 *
 * Revision 1.11  2006/08/28 12:27:17  maya
 * デフォルトのログファイル名を指定できるようにした。
 *   エディットコントロールを "Additional settings" ダイアログに追加した。
 *   teraterm.ini ファイルに LogDefaultName エントリを追加した。
 *   ファイル名に strftime のフォーマットを使えるようにした。
 *
 * Revision 1.10  2006/07/22 16:15:54  maya
 * ログ記録時に時刻も書き込む機能を追加した。
 *
 * Revision 1.9  2006/02/18 08:40:07  yutakakn
 *   ・コンパイラを Visual Studio 2005 Standard Edition に切り替えた。
 *   ・stricmp()を_stricmp()へ置換した
 *   ・strnicmp()を_strnicmp()へ置換した
 *   ・strdup()を_strdup()へ置換した
 *   ・chdir()を_chdir()へ置換した
 *   ・getcwd()を_getcwd()へ置換した
 *   ・strupr()を_strupr()へ置換した
 *   ・time_tの64ビット化にともなう警告メッセージを抑止した
 *   ・TeraTerm Menuがビルドエラーとなる現象に対処した
 *   ・Oniguruma 4.0.1へ差し替えた
 *
 * Revision 1.8  2005/05/07 09:49:24  yutakakn
 * teraterm.iniに LogTypePlainText を追加した。
 *
 * Revision 1.7  2005/02/22 11:55:39  yutakakn
 * 警告の抑止（未使用のローカル変数を削除）
 *
 * Revision 1.6  2005/02/22 11:53:57  yutakakn
 * ログ採取においてデフォルトファイル名を teraterm.log へ変更した。
 * 将来的には teraterm.ini でファイル名を指定できるようにする予定。
 *
 * Revision 1.5  2005/02/20 14:51:29  yutakakn
 * ログファイルの種別に"plain text"を追加。このオプションが有効の場合は、ログファイルに
 * ASCII非表示文字の採取をしない。
 *
 * 現在、無視するキャラクタは以下のとおり。
 * 　・BS
 * 　・ASCII(0x00-0x1f)のうち非表示なもの
 *
 * ただし、例外として以下のものはログ採取対象。
 * 　・HT
 * 　・CR
 * 　・LF
 *
 * Revision 1.4  2005/01/26 11:16:24  yutakakn
 * 初期ファイルディレクトリを読み込まれたteraterm.iniがあるディレクトリに固定するように、変更した。
 *
 * Revision 1.3  2005/01/21 07:46:41  yutakakn
 * ログ採取時のデフォルト名(log_YYYYMMDD_HHMMSS.txt)を設定するようにした。
 *
 * Revision 1.2  2005/01/06 13:06:45  yutakakn
 * "save setup"ダイアログの初期ファイルディレクトリをプログラム本体がある箇所に固定。
 * ログ採取のオープンダイアログをセーブダイアログへ変更した。
 *
 */
