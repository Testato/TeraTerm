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
	int Ptr;
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
		strcpy(FNameFilter, "keyboard setup files (*.cnf)");
		Ptr = strlen(FNameFilter) + 1;
		strcpy(&(FNameFilter[Ptr]), "*.cnf");
	}
	else {
		strcpy(FNameFilter, "setup files (*.ini)");
		Ptr = strlen(FNameFilter) + 1;
		strcpy(&(FNameFilter[Ptr]), "*.ini");
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
		// �����t�@�C���f�B���N�g�����v���O�����{�̂�����ӏ��ɌŒ肷�� (2005.1.6 yutaka)
		// �ǂݍ��܂ꂽteraterm.ini������f�B���N�g���ɌŒ肷��B
		// ����ɂ��A/F= �Ŏw�肳�ꂽ�ʒu�ɕۑ������悤�ɂȂ�B(2005.1.26 yutaka)
//		ofn.lpstrInitialDir = __argv[0];
		ofn.lpstrInitialDir = ts->SetupFName;
		ofn.lpstrTitle = "Tera Term: Save setup";
		Ok = GetSaveFileName(&ofn);
		if (Ok)
			strcpy(ts->SetupFName,Name);
		break;
	case GSF_RESTORE:
		ofn.Flags = ofn.Flags | OFN_FILEMUSTEXIST;
		ofn.lpstrTitle = "Tera Term: Restore setup";
		Ok = GetOpenFileName(&ofn);
		if (Ok)
			strcpy(ts->SetupFName,Name);
		break;
	case GSF_LOADKEY:
		ofn.Flags = ofn.Flags | OFN_FILEMUSTEXIST;
		ofn.lpstrTitle = "Tera Term: Load key map";
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

	switch (Message) {
	case WM_INITDIALOG:
		ofn = (LPOPENFILENAME)lParam;
		pl = (LPLONG)(ofn->lCustData);
		SetWindowLong(Dialog, DWL_USER, (LONG)pl);
		Lo = LOWORD(*pl) & 1;
		Hi = HIWORD(*pl);
		SetRB(Dialog,Lo,IDC_FOPTBIN,IDC_FOPTBIN);
		if (Hi!=0xFFFF)
		{
			ShowDlgItem(Dialog,IDC_FOPTAPPEND,IDC_FOPTAPPEND);
			SetRB(Dialog,Hi & 1,IDC_FOPTAPPEND,IDC_FOPTAPPEND);

			// plain text�`�F�b�N�{�b�N�X�̓f�t�H���g��ON (2005.2.20 yutaka)
			ShowDlgItem(Dialog,IDC_PLAINTEXT,IDC_PLAINTEXT);
			if (Hi & 0x1000) {
				SetRB(Dialog,1,IDC_PLAINTEXT,IDC_PLAINTEXT);
			}

			// timestamp�`�F�b�N�{�b�N�X (2006.7.23 maya)
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

			// timestamp�`�F�b�N�{�b�N�X (2006.7.23 maya)
			GetRB(Dialog,&val,IDC_TIMESTAMP,IDC_TIMESTAMP);
			if (val > 0) {
				Hi |= 0x2000;
			}

			*pl = MAKELONG(Lo,Hi);
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
	char FNFilter[11];
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
		strcat(fv->DlgCaption,"Send file");
		break;
	case GTF_LOG:
		strcat(fv->DlgCaption,"Log");
		break;
	case GTF_BP:
		strcat(fv->DlgCaption,"B-Plus Send");
		break;
	default: return FALSE;
	}

	strcpy(FNFilter, "all");
	strcpy(&(FNFilter[strlen(FNFilter)+1]), "*.*");      

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

	// logging�̏ꍇ�A�I�[�v���_�C�A���O���Z�[�u�_�C�A���O�֕ύX (2005.1.6 yutaka)
	if (FuncId == GTF_LOG) {
		// ���O�̃f�t�H���g�l(log_YYYYMMDD_HHMMSS.txt)��ݒ肷�� (2005.1.21 yutaka)
		// �f�t�H���g�t�@�C������ teraterm.log �֕ύX (2005.2.22 yutaka)
		// �f�t�H���g�t�@�C�����̐ݒ�ꏊ���Ăяo�����ֈړ� (2006.8.28 maya)
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

  switch (Message) {
    case WM_INITDIALOG:
      ofn = (LPOPENFILENAME)lParam;
      pw = (LPWORD)ofn->lCustData;
      SetWindowLong(Dialog, DWL_USER, (LONG)pw);
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
  char FNFilter[11];
  OPENFILENAME ofn;
  char TempDir[MAXPATHLEN];
  BOOL Ok;

  /* save current dir */
  _getcwd(TempDir,sizeof(TempDir));

  fv->NumFname = 0;

  strcpy(fv->DlgCaption,"Tera Term: ");
  switch (FuncId) {
    case GMF_KERMIT:
      strcat(fv->DlgCaption,"Kermit Send");
      break;
    case GMF_Z:
      strcat(fv->DlgCaption,"ZMODEM Send");
      break;
    case GMF_QV:
      strcat(fv->DlgCaption,"Quick-VAN Send");
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

  memset(FNFilter, 0, sizeof(FNFilter));  /* Set up for double null at end */
  strcpy(FNFilter, "all");
  strcpy(&(FNFilter[strlen(FNFilter)+1]), "*.*");
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

  switch (Message) {
    case WM_INITDIALOG:
      fv = (PFileVar)lParam;
      SetWindowLong(Dialog, DWL_USER, lParam);
      SendDlgItemMessage(Dialog, IDC_GETFN, EM_LIMITTEXT, sizeof(TempFull)-1,0);
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
	  return TRUE;
	case IDCANCEL:
	  EndDialog(Dialog, 0);
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

  GetFileNamePos(fv->FullName,&(fv->DirLen),&i);
  if (fv->FullName[fv->DirLen]=='\\') fv->DirLen++;
  strcpy(fv->DlgCaption,"Tera Term: ");
  switch (fv->OpId) {
    case OpLog:
      strcat(fv->DlgCaption,TitLog);
      break;
    case OpSendFile:
      strcat(fv->DlgCaption,TitSendFile);
      break;
    case OpKmtRcv:
      strcat(fv->DlgCaption,TitKmtRcv);
      break;
    case OpKmtGet:
      strcat(fv->DlgCaption,TitKmtGet);
      break;
    case OpKmtSend:
      strcat(fv->DlgCaption,TitKmtSend);
      break;
    case OpKmtFin:
      strcat(fv->DlgCaption,TitKmtFin);
      break;
    case OpXRcv:
      strcat(fv->DlgCaption,TitXRcv);
      break;
    case OpXSend:
      strcat(fv->DlgCaption,TitXSend);
      break;
    case OpZRcv:
      strcat(fv->DlgCaption,TitZRcv);
      break;
    case OpZSend:
      strcat(fv->DlgCaption,TitZSend);
      break;
    case OpBPRcv:
      strcat(fv->DlgCaption,TitBPRcv);
      break;
    case OpBPSend:
      strcat(fv->DlgCaption,TitBPSend);
      break;
    case OpQVRcv:
      strcat(fv->DlgCaption,TitQVRcv);
      break;
    case OpQVSend:
      strcat(fv->DlgCaption,TitQVSend);
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

  switch (Message) {
    case WM_INITDIALOG:
      ofn = (LPOPENFILENAME)lParam;
      pl = (LPLONG)ofn->lCustData;
      SetWindowLong(Dialog, DWL_USER, (LONG)pl);
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
  char FNFilter[11];
  OPENFILENAME ofn;
  LONG opt;
  char TempDir[MAXPATHLEN];
  BOOL Ok;

  /* save current dir */
  _getcwd(TempDir,sizeof(TempDir));

  fv->FullName[0] = 0;
  memset(FNFilter, 0, sizeof(FNFilter));  /* Set up for double null at end */
  memset(&ofn, 0, sizeof(OPENFILENAME));

  strcpy(fv->DlgCaption,"Tera Term: XMODEM ");
  if (Receive)
    strcat(fv->DlgCaption,"Receive");
  else
    strcat(fv->DlgCaption,"Send");

  strcpy(FNFilter, "all");
  strcpy(&(FNFilter[strlen(FNFilter)+1]), "*.*");

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
 * Revision 1.10  2006/07/22 16:15:54  maya
 * ���O�L�^���Ɏ������������ދ@�\��ǉ������B
 *
 * Revision 1.9  2006/02/18 08:40:07  yutakakn
 *   �E�R���p�C���� Visual Studio 2005 Standard Edition �ɐ؂�ւ����B
 *   �Estricmp()��_stricmp()�֒u������
 *   �Estrnicmp()��_strnicmp()�֒u������
 *   �Estrdup()��_strdup()�֒u������
 *   �Echdir()��_chdir()�֒u������
 *   �Egetcwd()��_getcwd()�֒u������
 *   �Estrupr()��_strupr()�֒u������
 *   �Etime_t��64�r�b�g���ɂƂ��Ȃ��x�����b�Z�[�W��}�~����
 *   �ETeraTerm Menu���r���h�G���[�ƂȂ錻�ۂɑΏ�����
 *   �EOniguruma 4.0.1�֍����ւ���
 *
 * Revision 1.8  2005/05/07 09:49:24  yutakakn
 * teraterm.ini�� LogTypePlainText ��ǉ������B
 *
 * Revision 1.7  2005/02/22 11:55:39  yutakakn
 * �x���̗}�~�i���g�p�̃��[�J���ϐ����폜�j
 *
 * Revision 1.6  2005/02/22 11:53:57  yutakakn
 * ���O�̎�ɂ����ăf�t�H���g�t�@�C������ teraterm.log �֕ύX�����B
 * �����I�ɂ� teraterm.ini �Ńt�@�C�������w��ł���悤�ɂ���\��B
 *
 * Revision 1.5  2005/02/20 14:51:29  yutakakn
 * ���O�t�@�C���̎�ʂ�"plain text"��ǉ��B���̃I�v�V�������L���̏ꍇ�́A���O�t�@�C����
 * ASCII��\�������̍̎�����Ȃ��B
 *
 * ���݁A��������L�����N�^�͈ȉ��̂Ƃ���B
 * �@�EBS
 * �@�EASCII(0x00-0x1f)�̂�����\���Ȃ���
 *
 * �������A��O�Ƃ��Ĉȉ��̂��̂̓��O�̎�ΏہB
 * �@�EHT
 * �@�ECR
 * �@�ELF
 *
 * Revision 1.4  2005/01/26 11:16:24  yutakakn
 * �����t�@�C���f�B���N�g����ǂݍ��܂ꂽteraterm.ini������f�B���N�g���ɌŒ肷��悤�ɁA�ύX�����B
 *
 * Revision 1.3  2005/01/21 07:46:41  yutakakn
 * ���O�̎掞�̃f�t�H���g��(log_YYYYMMDD_HHMMSS.txt)��ݒ肷��悤�ɂ����B
 *
 * Revision 1.2  2005/01/06 13:06:45  yutakakn
 * "save setup"�_�C�A���O�̏����t�@�C���f�B���N�g�����v���O�����{�̂�����ӏ��ɌŒ�B
 * ���O�̎�̃I�[�v���_�C�A���O���Z�[�u�_�C�A���O�֕ύX�����B
 *
 */
