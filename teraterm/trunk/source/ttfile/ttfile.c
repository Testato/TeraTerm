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

#ifdef I18N
static HFONT DlgFoptFont;
static HFONT DlgXoptFont;
static HFONT DlgGetfnFont;

char UILanguageFile[MAX_PATH];
#endif

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
#ifdef I18N
	char uimsg[MAX_UIMSG];
#else
	int Ptr;
#endif

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
#ifdef I18N
		memcpy(uimsg, "keyboard setup files (*.cnf)\0*.cnf\0\0", sizeof(uimsg));
		get_lang_msg("FILEDLG_KEYBOARD_FILTER", uimsg, UILanguageFile);
		memcpy(FNameFilter, uimsg, sizeof(FNameFilter));
#else
		strcpy(FNameFilter, "keyboard setup files (*.cnf)");
		Ptr = strlen(FNameFilter) + 1;
		strcpy(&(FNameFilter[Ptr]), "*.cnf");
#endif
	}
	else {
#ifdef I18N
		memcpy(uimsg, "setup files (*.ini)\0*.ini\0\0", sizeof(uimsg));
		get_lang_msg("FILEDLG_SETUP_FILTER", uimsg, UILanguageFile);
		memcpy(FNameFilter, uimsg, sizeof(FNameFilter));
#else
		strcpy(FNameFilter, "setup files (*.ini)");
		Ptr = strlen(FNameFilter) + 1;
		strcpy(&(FNameFilter[Ptr]), "*.ini");
#endif
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
		strcpy(Name,&(ts->KeyCnfFN[j]));
		memcpy(Dir,ts->KeyCnfFN,i);
		Dir[i] = 0;

		if ((strlen(Name)==0) || (_stricmp(Name,"KEYBOARD.CNF")==0))
			strcpy(Name,"KEYBOARD.CNF");
	}
	else {
		ofn.lpstrDefExt = "ini";
		GetFileNamePos(ts->SetupFName,&i,&j);
		strcpy(Name,&(ts->SetupFName[j]));
		memcpy(Dir,ts->SetupFName,i);
		Dir[i] = 0;

		if ((strlen(Name)==0) || (_stricmp(Name,"TERATERM.INI")==0))
			strcpy(Name,"TERATERM.INI");
	}

	if (strlen(Dir)==0)
		strcpy(Dir,ts->HomeDir);

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
#ifdef I18N
		strcpy(uimsg, "Tera Term: Save setup");
		get_lang_msg("FILEDLG_SAVE_SETUP_TITLE", uimsg, UILanguageFile);
		ofn.lpstrTitle = uimsg;
#else
		ofn.lpstrTitle = "Tera Term: Save setup";
#endif
		Ok = GetSaveFileName(&ofn);
		if (Ok)
			strcpy(ts->SetupFName,Name);
		break;
	case GSF_RESTORE:
		ofn.Flags = ofn.Flags | OFN_FILEMUSTEXIST;
#ifdef I18N
		strcpy(uimsg, "Tera Term: Restore setup");
		get_lang_msg("FILEDLG_RESTORE_SETUP_TITLE", uimsg, UILanguageFile);
		ofn.lpstrTitle = uimsg;
#else
		ofn.lpstrTitle = "Tera Term: Restore setup";
#endif
		Ok = GetOpenFileName(&ofn);
		if (Ok)
			strcpy(ts->SetupFName,Name);
		break;
	case GSF_LOADKEY:
		ofn.Flags = ofn.Flags | OFN_FILEMUSTEXIST;
#ifdef I18N
		strcpy(uimsg, "Tera Term: Load key map");
		get_lang_msg("FILEDLG_LOAD_KEYMAP_TITLE", uimsg, UILanguageFile);
		ofn.lpstrTitle = uimsg;
#else
		ofn.lpstrTitle = "Tera Term: Load key map";
#endif
		Ok = GetOpenFileName(&ofn);
		if (Ok)
			strcpy(ts->KeyCnfFN,Name);
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
#ifdef I18N
	char uimsg[MAX_UIMSG];
	LOGFONT logfont;
	HFONT font;
#endif

	switch (Message) {
	case WM_INITDIALOG:
		ofn = (LPOPENFILENAME)lParam;
		pl = (LPLONG)(ofn->lCustData);
		SetWindowLong(Dialog, DWL_USER, (LONG)pl);

#ifdef I18N
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

      GetDlgItemText(Dialog, IDC_FOPT, uimsg, sizeof(uimsg));
      get_lang_msg("DLG_FOPT", uimsg, UILanguageFile);
      SetDlgItemText(Dialog, IDC_FOPT, uimsg);
      GetDlgItemText(Dialog, IDC_FOPTBIN, uimsg, sizeof(uimsg));
      get_lang_msg("DLG_FOPT_BINARY", uimsg, UILanguageFile);
      SetDlgItemText(Dialog, IDC_FOPTBIN, uimsg);
      GetDlgItemText(Dialog, IDC_FOPTAPPEND, uimsg, sizeof(uimsg));
      get_lang_msg("DLG_FOPT_APPEND", uimsg, UILanguageFile);
      SetDlgItemText(Dialog, IDC_FOPTAPPEND, uimsg);
      GetDlgItemText(Dialog, IDC_PLAINTEXT, uimsg, sizeof(uimsg));
      get_lang_msg("DLG_FOPT_PLAIN", uimsg, UILanguageFile);
      SetDlgItemText(Dialog, IDC_PLAINTEXT, uimsg);
      GetDlgItemText(Dialog, IDC_TIMESTAMP, uimsg, sizeof(uimsg));
      get_lang_msg("DLG_FOPT_TIMESTAMP", uimsg, UILanguageFile);
      SetDlgItemText(Dialog, IDC_TIMESTAMP, uimsg);
#endif

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
#ifdef I18N
		if (DlgFoptFont != NULL) {
			DeleteObject(DlgFoptFont);
		}
#endif
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
#ifdef I18N
	char uimsg[MAX_UIMSG];
	char FNFilter[40];
#else
	char FNFilter[11];
#endif
	OPENFILENAME ofn;
	LONG opt;
	char TempDir[MAXPATHLEN];
	BOOL Ok;

	/* save current dir */
	_getcwd(TempDir,sizeof(TempDir));

	memset(FNFilter, 0, sizeof(FNFilter));  /* Set up for double null at end */
	memset(&ofn, 0, sizeof(OPENFILENAME));

	strcpy(fv->DlgCaption,"Tera Term: ");
	switch (FuncId) {
	case GTF_SEND:
#ifdef I18N
		strcpy(uimsg, TitSendFile);
		get_lang_msg("FILEDLG_TRANS_TITLE_SENDFILE", uimsg, UILanguageFile);
		strncat(fv->DlgCaption, uimsg, strlen(fv->DlgCaption)-1);
#else
		strcat(fv->DlgCaption,"Send file");
#endif
		break;
	case GTF_LOG:
#ifdef I18N
		strcpy(uimsg, TitLog);
		get_lang_msg("FILEDLG_TRANS_TITLE_LOG", uimsg, UILanguageFile);
		strncat(fv->DlgCaption, uimsg, strlen(fv->DlgCaption)-1);
#else
		strcat(fv->DlgCaption,"Log");
#endif
		break;
	case GTF_BP:
#ifdef I18N
		strcpy(uimsg, TitBPSend);
		get_lang_msg("FILEDLG_TRANS_TITLE_BPSEND", uimsg, UILanguageFile);
		strncat(fv->DlgCaption, uimsg, strlen(fv->DlgCaption)-1);
#else
		strcat(fv->DlgCaption,"B-Plus Send");
#endif
		break;
	default: return FALSE;
	}

#ifdef I18N
	memcpy(uimsg, "All(*.*)\0*.*\0\0", sizeof(FNFilter));
	get_lang_msg("FILEDLG_ALL_FILTER", uimsg, UILanguageFile);
	memcpy(FNFilter, uimsg, sizeof(FNFilter));
#else
	strcpy(FNFilter, "all");
	strcpy(&(FNFilter[strlen(FNFilter)+1]), "*.*");
#endif

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
		GetLongFName(fv->FullName,&fv->FullName[fv->DirLen]);
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
#ifdef I18N
	char uimsg[MAX_UIMSG];
	LOGFONT logfont;
	HFONT font;
#endif

  switch (Message) {
    case WM_INITDIALOG:
      ofn = (LPOPENFILENAME)lParam;
      pw = (LPWORD)ofn->lCustData;
      SetWindowLong(Dialog, DWL_USER, (LONG)pw);

#ifdef I18N
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

      GetDlgItemText(Dialog, IDC_FOPT, uimsg, sizeof(uimsg));
      get_lang_msg("DLG_FOPT", uimsg, UILanguageFile);
      SetDlgItemText(Dialog, IDC_FOPT, uimsg);
      GetDlgItemText(Dialog, IDC_FOPTBIN, uimsg, sizeof(uimsg));
      get_lang_msg("DLG_FOPT_BINARY", uimsg, UILanguageFile);
      SetDlgItemText(Dialog, IDC_FOPTBIN, uimsg);
#endif

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
#ifndef I18N
		if (DlgFoptFont != NULL) {
			DeleteObject(DlgFoptFont);
		}
#endif
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
#ifdef I18N
  char uimsg[MAX_UIMSG];
  char FNFilter[40];
#else
  char FNFilter[11];
#endif
  OPENFILENAME ofn;
  char TempDir[MAXPATHLEN];
  BOOL Ok;

  /* save current dir */
  _getcwd(TempDir,sizeof(TempDir));

  fv->NumFname = 0;

  strcpy(fv->DlgCaption,"Tera Term: ");
  switch (FuncId) {
    case GMF_KERMIT:
#ifdef I18N
      strcpy(uimsg, TitKmtSend);
      get_lang_msg("FILEDLG_TRANS_TITLE_KMTSEND", uimsg, UILanguageFile);
      strncat(fv->DlgCaption, uimsg, strlen(fv->DlgCaption)-1);
#else
      strcat(fv->DlgCaption,"Kermit Send");
#endif
      break;
    case GMF_Z:
#ifdef I18N
      strcpy(uimsg, TitZSend);
      get_lang_msg("FILEDLG_TRANS_TITLE_ZSEND", uimsg, UILanguageFile);
      strncat(fv->DlgCaption, uimsg, strlen(fv->DlgCaption)-1);
#else
      strcat(fv->DlgCaption,"ZMODEM Send");
#endif
      break;
    case GMF_QV:
#ifdef I18N
      strcpy(uimsg, TitQVSend);
	  get_lang_msg("FILEDLG_TRANS_TITLE_QVSEND", uimsg, UILanguageFile);
      strncat(fv->DlgCaption, uimsg, strlen(fv->DlgCaption)-1);
#else
      strcat(fv->DlgCaption,"Quick-VAN Send");
#endif
      break;
    default: return FALSE;
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

#ifdef I18N
  memcpy(uimsg, "All(*.*)\0*.*\0\0", sizeof(uimsg));
  get_lang_msg("FILEDLG_ALL_FILTER", uimsg, UILanguageFile);
  memcpy(FNFilter, uimsg, sizeof(FNFilter));
#else
  memset(FNFilter, 0, sizeof(FNFilter));  /* Set up for double null at end */
  strcpy(FNFilter, "all");
  strcpy(&(FNFilter[strlen(FNFilter)+1]), "*.*");
#endif
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
      strcpy(fv->FullName,fv->FnStrMem);
      fv->FnPtr = 0;
#ifdef TERATERM32
      // for Win NT 3.5: short name -> long name
      GetLongFName(fv->FullName,&fv->FullName[fv->DirLen]);
#endif
    }
    else { // multiple selection
      strcpy(fv->FullName,fv->FnStrMem);
      AppendSlash(fv->FullName);
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
#ifdef I18N
	char uimsg[MAX_UIMSG];
	LOGFONT logfont;
	HFONT font;
#endif

  switch (Message) {
    case WM_INITDIALOG:
      fv = (PFileVar)lParam;
      SetWindowLong(Dialog, DWL_USER, lParam);
      SendDlgItemMessage(Dialog, IDC_GETFN, EM_LIMITTEXT, sizeof(TempFull)-1,0);

#ifdef I18N
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

	  GetWindowText(Dialog, uimsg, sizeof(uimsg));
	  get_lang_msg("DLG_GETFN_TITLE", uimsg, UILanguageFile);
	  SetWindowText(Dialog, uimsg);
	  GetDlgItemText(Dialog, IDC_FILENAME, uimsg, sizeof(uimsg));
	  get_lang_msg("DLG_GETFN_FILENAME", uimsg, UILanguageFile);
	  SetDlgItemText(Dialog, IDC_FILENAME, uimsg);
	  GetDlgItemText(Dialog, IDOK, uimsg, sizeof(uimsg));
	  get_lang_msg("BTN_OK", uimsg, UILanguageFile);
	  SetDlgItemText(Dialog, IDOK, uimsg);
	  GetDlgItemText(Dialog, IDCANCEL, uimsg, sizeof(uimsg));
	  get_lang_msg("BTN_CANCEL", uimsg, UILanguageFile);
	  SetDlgItemText(Dialog, IDCANCEL, uimsg);
	  GetDlgItemText(Dialog, IDC_GETFNHELP, uimsg, sizeof(uimsg));
	  get_lang_msg("BTN_HELP", uimsg, UILanguageFile);
	  SetDlgItemText(Dialog, IDC_GETFNHELP, uimsg);
#endif

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
	    FitFileName(&(TempFull[j]),NULL);
	    strcat(fv->FullName,&(TempFull[j]));
	  }
	  EndDialog(Dialog, 1);
#ifdef I18N
	  if (DlgGetfnFont != NULL) {
	    DeleteObject(DlgGetfnFont);
	  }
#endif
	  return TRUE;
	case IDCANCEL:
	  EndDialog(Dialog, 0);
#ifdef I18N
	  if (DlgGetfnFont != NULL) {
	    DeleteObject(DlgGetfnFont);
	  }
#endif
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
#ifdef I18N
  char uimsg[MAX_UIMSG];
#endif

  GetFileNamePos(fv->FullName,&(fv->DirLen),&i);
  if (fv->FullName[fv->DirLen]=='\\') fv->DirLen++;
  strcpy(fv->DlgCaption,"Tera Term: ");
  switch (fv->OpId) {
    case OpLog:
#ifdef I18N
      strcpy(uimsg, TitLog);
      get_lang_msg("FILEDLG_TRANS_TITLE_LOG", uimsg, UILanguageFile);
      strncat(fv->DlgCaption, uimsg, strlen(fv->DlgCaption)-1);
#else
      strcat(fv->DlgCaption,TitLog);
#endif
    break;
    case OpSendFile:
#ifdef I18N
      strcpy(uimsg, TitSendFile);
      get_lang_msg("FILEDLG_TRANS_TITLE_SENDFILE", uimsg, UILanguageFile);
      strncat(fv->DlgCaption, uimsg, strlen(fv->DlgCaption)-1);
#else
      strcat(fv->DlgCaption,TitSendFile);
#endif
      break;
    case OpKmtRcv:
#ifdef I18N
      strcpy(uimsg, TitKmtRcv);
      get_lang_msg("FILEDLG_TRANS_TITLE_KMTRCV", uimsg, UILanguageFile);
      strncat(fv->DlgCaption, uimsg, strlen(fv->DlgCaption)-1);
#else
      strcat(fv->DlgCaption,TitKmtRcv);
#endif
      break;
    case OpKmtGet:
#ifdef I18N
      strcpy(uimsg, TitKmtGet);
      get_lang_msg("FILEDLG_TRANS_TITLE_KMTGET", uimsg, UILanguageFile);
      strncat(fv->DlgCaption, uimsg, strlen(fv->DlgCaption)-1);
#else
      strcat(fv->DlgCaption,TitKmtGet);
#endif
      break;
    case OpKmtSend:
#ifdef I18N
      strcpy(uimsg, TitKmtSend);
      get_lang_msg("FILEDLG_TRANS_TITLE_KMTSEND", uimsg, UILanguageFile);
      strncat(fv->DlgCaption, uimsg, strlen(fv->DlgCaption)-1);
#else
      strcat(fv->DlgCaption,TitKmtSend);
#endif
      break;
    case OpKmtFin:
#ifdef I18N
      strcpy(uimsg, TitKmtFin);
      get_lang_msg("FILEDLG_TRANS_TITLE_KMTFIN", uimsg, UILanguageFile);
      strncat(fv->DlgCaption, uimsg, strlen(fv->DlgCaption)-1);
#else
      strcat(fv->DlgCaption,TitKmtFin);
#endif
      break;
    case OpXRcv:
#ifdef I18N
      strcpy(uimsg, TitXRcv);
      get_lang_msg("FILEDLG_TRANS_TITLE_XRCV", uimsg, UILanguageFile);
      strncat(fv->DlgCaption, uimsg, strlen(fv->DlgCaption)-1);
#else
      strcat(fv->DlgCaption,TitXRcv);
#endif
      break;
    case OpXSend:
#ifdef I18N
      strcpy(uimsg, TitXSend);
      get_lang_msg("FILEDLG_TRANS_TITLE_XSEND", uimsg, UILanguageFile);
      strncat(fv->DlgCaption, uimsg, strlen(fv->DlgCaption)-1);
#else
      strcat(fv->DlgCaption,TitXSend);
#endif
      break;
    case OpZRcv:
#ifdef I18N
      strcpy(uimsg, TitZRcv);
      get_lang_msg("FILEDLG_TRANS_TITLE_ZRCV", uimsg, UILanguageFile);
      strncat(fv->DlgCaption, uimsg, strlen(fv->DlgCaption)-1);
#else
      strcat(fv->DlgCaption,TitZRcv);
#endif
      break;
    case OpZSend:
#ifdef I18N
      strcpy(uimsg, TitZSend);
      get_lang_msg("FILEDLG_TRANS_TITLE_ZSEND", uimsg, UILanguageFile);
      strncat(fv->DlgCaption, uimsg, strlen(fv->DlgCaption)-1);
#else
      strcat(fv->DlgCaption,TitZSend);
#endif
      break;
    case OpBPRcv:
#ifdef I18N
      strcpy(uimsg, TitBPRcv);
      get_lang_msg("FILEDLG_TRANS_TITLE_BPRCV", uimsg, UILanguageFile);
      strncat(fv->DlgCaption, uimsg, strlen(fv->DlgCaption)-1);
#else
      strcat(fv->DlgCaption,TitBPRcv);
#endif
      break;
    case OpBPSend:
#ifdef I18N
      strcpy(uimsg, TitBPSend);
      get_lang_msg("FILEDLG_TRANS_TITLE_BPSEND", uimsg, UILanguageFile);
      strncat(fv->DlgCaption, uimsg, strlen(fv->DlgCaption)-1);
#else
      strcat(fv->DlgCaption,TitBPSend);
#endif
      break;
    case OpQVRcv:
#ifdef I18N
      strcpy(uimsg, TitQVRcv);
      get_lang_msg("FILEDLG_TRANS_TITLE_QVRCV", uimsg, UILanguageFile);
      strncat(fv->DlgCaption, uimsg, strlen(fv->DlgCaption)-1);
#else
      strcat(fv->DlgCaption,TitQVRcv);
#endif
      break;
    case OpQVSend:
#ifdef I18N
      strcpy(uimsg, TitQVSend);
      get_lang_msg("FILEDLG_TRANS_TITLE_QVSEND", uimsg, UILanguageFile);
      strncat(fv->DlgCaption, uimsg, strlen(fv->DlgCaption)-1);
#else
      strcat(fv->DlgCaption,TitQVSend);
#endif
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
#ifdef I18N
	char uimsg[MAX_UIMSG];
	LOGFONT logfont;
	HFONT font;
#endif

  switch (Message) {
    case WM_INITDIALOG:
      ofn = (LPOPENFILENAME)lParam;
      pl = (LPLONG)ofn->lCustData;
      SetWindowLong(Dialog, DWL_USER, (LONG)pl);

#ifdef I18N
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

      GetDlgItemText(Dialog, IDC_XOPT, uimsg, sizeof(uimsg));
      get_lang_msg("DLG_XOPT", uimsg, UILanguageFile);
      SetDlgItemText(Dialog, IDC_XOPT, uimsg);
      GetDlgItemText(Dialog, IDC_XOPTCHECK, uimsg, sizeof(uimsg));
      get_lang_msg("DLG_XOPT_CHECKSUM", uimsg, UILanguageFile);
      SetDlgItemText(Dialog, IDC_XOPTCHECK, uimsg);
      GetDlgItemText(Dialog, IDC_XOPTCRC, uimsg, sizeof(uimsg));
      get_lang_msg("DLG_XOPT_CRC", uimsg, UILanguageFile);
      SetDlgItemText(Dialog, IDC_XOPTCRC, uimsg);
      GetDlgItemText(Dialog, IDC_XOPT1K, uimsg, sizeof(uimsg));
      get_lang_msg("DLG_XOPT_1K", uimsg, UILanguageFile);
      SetDlgItemText(Dialog, IDC_XOPT1K, uimsg);
      GetDlgItemText(Dialog, IDC_XOPTBIN, uimsg, sizeof(uimsg));
      get_lang_msg("DLG_XOPT_BINARY", uimsg, UILanguageFile);
      SetDlgItemText(Dialog, IDC_XOPTBIN, uimsg);
#endif


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
#ifdef I18N
	if (DlgXoptFont != NULL) {
		DeleteObject(DlgXoptFont);
	}
#endif
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
#ifdef I18N
  char uimsg[MAX_UIMSG];
  char FNFilter[40];
#else
  char FNFilter[11];
#endif
  OPENFILENAME ofn;
  LONG opt;
  char TempDir[MAXPATHLEN];
  BOOL Ok;

  /* save current dir */
  _getcwd(TempDir,sizeof(TempDir));

  fv->FullName[0] = 0;
  memset(FNFilter, 0, sizeof(FNFilter));  /* Set up for double null at end */
  memset(&ofn, 0, sizeof(OPENFILENAME));

#ifdef I18N
  strcpy(fv->DlgCaption,"Tera Term: ");
#else
  strcpy(fv->DlgCaption,"Tera Term: XMODEM ");
#endif
  if (Receive)
#ifdef I18N
  {
    strcpy(uimsg, TitXRcv);
	get_lang_msg("FILEDLG_TRANS_TITLE_XRCV", uimsg, UILanguageFile);
    strncat(fv->DlgCaption, uimsg, strlen(fv->DlgCaption)-1);
  }
#else
    strcat(fv->DlgCaption,"Receive");
#endif
  else
#ifdef I18N
  {
    strcpy(uimsg, TitXSend);
    get_lang_msg("FILEDLG_TRANS_TITLE_XSEND", uimsg, UILanguageFile);
    strncat(fv->DlgCaption, uimsg, strlen(fv->DlgCaption)-1);
  }
#else
    strcat(fv->DlgCaption,"Send");
#endif

#ifdef I18N
  memcpy(uimsg, "All(*.*)\0*.*\0\0", sizeof(FNFilter));
  get_lang_msg("FILEDLG_ALL_FILTER", uimsg, UILanguageFile);
  memcpy(FNFilter, uimsg, sizeof(FNFilter));
#else
  strcpy(FNFilter, "all");
  strcpy(&(FNFilter[strlen(FNFilter)+1]), "*.*");
#endif

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
#ifdef I18N
  PMap pm;
  HANDLE HMap = NULL;
#endif

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
#ifdef I18N
      HMap = CreateFileMapping(
        (HANDLE) 0xFFFFFFFF, NULL, PAGE_READONLY,
        0, sizeof(TMap), "ttset_memfilemap");
      if (HMap != NULL) {
        pm = (PMap)MapViewOfFile(
        HMap,FILE_MAP_READ,0,0,0);
        if (pm != NULL) {
          strncpy(UILanguageFile, pm->ts.UILanguageFile, sizeof(UILanguageFile)-1);
          UILanguageFile[sizeof(UILanguageFile)-1] = 0;
	    }
      }
#endif
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
