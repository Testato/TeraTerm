	Source code of Tera Term Pro version 2.3 and
		       Tera Term version 1.4
	T. Teranishi Mar 10, 1998

	Copyright (C) 1994-1998 T. Teranishi
	All Rights Reserved.

-------------------------------------------------------------------------------
1. �T�v

Tera Term (Pro) �� MS-Windows �p�^�[�~�i���G�~�����[�^�[
(telnet �N���C�A���g)�ł��BTera Term Pro version 2.3 ��
Windows 95/NT �p�ATera Term version 1.4 �� Windows 3.1 �p�ł��B

���̃p�b�P�[�W�ɂ� Tera Term Pro 2.3 �� Tera Term 1.4 �̃\�[�X�R�[�h
(MS-Visual C/C++ 5.0 �� Watcom C/C++ 11.0 �ɑΉ�)���܂܂�Ă��܂��B

32-bit �� Tera Term Pro 2.3 �� MS-Visual C/C++ ����� Watcom C/C++
�̗����Ŗ��Ȃ��R���p�C���ł��܂����A16-bit �� Tera Term 1.4 ��
Watcom C/C++ �ł����R���p�C���ł��܂���B

16-bit �� Tera Term �Ɋւ��ďd��Ȗ�肪����܂��BWatcom C/C++ ��
����Đ������ꂽ���s�t�@�C�� TERATERM.EXE �͕����̃A�v���P�[�V����
�C���X�^���X�ɑΉ����Ă��܂���B�܂�A�����ɕ����� Tera Term
�C���X�^���X�����s���邱�Ƃ��ł��܂���B�����炭 Microsoft ��
16-bit �R���p�C���[���g���΂��̖��������邱�Ƃ��ł��邩�������
���񂪁A�m�F�͂���Ă��܂���B
���̖��̂��߁A���ۂɌ��J����Ă��� Tera Term 1.4 �� C/C++ �\�[�X
�Ƃقړ����� Turbo Pascal �\�[�X�R�[�h�ɂ���Đ�������܂����B
Turbo Pascal for Windows �ɂ͂��̕����C���X�^���X�̖�肪����܂���B
Turbo Pascal �\�[�X�́A�ʂ̃p�b�P�[�W TTSRCV14.ZIP �Ƃ��Č��J�����
���܂��B

���̃p�b�P�[�W�ɂ́ATera Term �̂��߂� add-on ���W���[���̗��
�܂܂�Ă��܂��BAdd-on ���W���[���� DLL �̌`�ō쐬����ATera Term
�̃��[�U�[�C���^�[�t�F�C�X�֐��A�ݒ�֐��AWinsock �֐����t�b�N
���邱�Ƃ��ł��܂��BTera Term �� add-on ���W���[���̊Ԃ�
Tera Term extension interface (TTX) �� Robert O'Callahan ��
Tera Term ���(���� ��)�ɂ��J������܂����B
Tera Term �������������ꍇ�́ATera Term �𒼐ډ������邱�Ƃ�
�l����O�ɁAadd-on ���W���[�������邩�ǂ������l���Ă݂Ă��������B
Add-on ���W���[������邱�ƂŁA���쌠�̖����ȒP�Ɏ�舵������
���ł��܂����A�݂��Ɍ݊����̂Ȃ������� Tera Term �������������
��h�����ƂɂȂ�܂��BAdd-on ���W���[���̊J���A�z�z�ɂ��Ă�
���́u2. ���ӎ����v���ǂ�ł��������B

-------------------------------------------------------------------------------
2. ���ӎ���

���쌠�́A���(���� ��)���ێ����܂��B���̃\�[�X�R�[�h�̎g�p�ɂ��A
�����Ȃ鑹�Q�ɂ������Ă���҂͐ӔC�𕉂��܂���B

���̃p�b�P�[�W�̓I���W�i���̌`�̂܂܂Ȃ�΁A�Ĕz�z���R�ł��B
�������A���K�I���v�𓾂邽�߂̔z�z�ɂ͍�҂̋����K�v�ł��B

���̃p�b�P�[�W�Ɋ܂܂��A�t�@�C���A���W���[���A�T�u���[�`���A���\�[�X����
�S���܂��́A�ꕔ���R�s�[���č쐬�����v���O�������A���K�I���v�𓾂邽�߂ɔz�z
����ꍇ�͍�҂̋����K�v�ł��B

������ Tera Term ��s���葽���̐l�ɔz�z����ꍇ�ɂ���҂̋����K�v�ł��B

�������A�ȉ��̃t�@�C�����g�p���� Tera Term �̂��߂� add-on ���W���[��
���쐬���A�z�z���邱�Ƃ́A��҂̋��Ȃ��ŉ\�ł��B

	ttxtest.c
	teraterm.h
	tttypes.h
	ttplugin.h
	ttsetup.h
	ttdialog.h
	ttwsk.h

Tera Term extension �̐����� ttxtest.c �̃R�����g��ǂ�ł��������B
Add-on ���W���[�����J�������ŁAttxtest.c �����������邱�Ƃ�
�\�ł����A���̃C���N���[�h�t�@�C�� (*.h) �͏��������Ȃ��ł��������B
Add-on ���W���[�����쐬�����ꍇ�ATera Term ��҂ɘA�����邱�Ƃ�
�����߂��܂��B�܂��Aadd-on ���W���[����z�z����ꍇ�́ATera Term
���s�t�@�C����t������ add-on ���W���[��������z�z���邱�Ƃ�
�����߂��܂��B�����łȂ���΁A������ Tera Term ��z�z���邽�߂�
������҂ɐ\������K�v������܂��B

-------------------------------------------------------------------------------
3. ��҂���̃R�����g

Tera Term (16-bit ��)�̌��X�̃\�[�X�� Turbo Pascal for Windows 1.5 (�p���)
�ŏ�����܂����B���̌� Tera Term Pro (32-bit ��)�p�� C/C++ �ɕϊ�����܂����B
��҂� C/C++ �ɂ��܂�Ȃ�Ă��Ȃ��̂ŁA�\�[�X�̃X�^�C���� C/C++ �I�ł�
�Ȃ���������܂���B

�\�[�X�̈ꕔ�� Microsoft Foundation Class Library (MFC) ���g�p���Ă��܂��B
Borland C/C++ �� Object Windows LIbrary (OWL) �ɑΉ�����\��͂���܂���B

�C���X�g�[���[�ƃA���C���X�g�[���[�̃\�[�X�͌��J���܂���B
��҂ɗv�����Ȃ��ł��������B

���̃p�b�P�[�W�ɂ͐ݒ�t�@�C���A�w���v�t�@�C���A"Tera Special" �t�H���g
���܂܂�Ă��܂���B�����K�v�Ȃ�z�z�p�b�P�[�W TTERMP23.ZIP �� TTERMV14.ZIP
����R�s�[���Ă��������B

����̃o�[�W�����A�b�v�ŁA�\�[�X�̑啔����������������\��������܂��B
�ύX�����ɂ��ăR�����g�����肷�邱�Ƃ͂���܂���B�o�[�W�����A�b�v����
�\�[�X����肵�āA�ǂ����ύX���ꂽ�̂��m�肽���ꍇ�́A�����Ńt�@�C�����e��
��r���Ă��������B

��҂ɁA�v���O���~���O�ATera Term �\�[�X�̍\���ɂ��āA�R���p�C�����@
���̎�������Ȃ��ł��������B

��҂ւ̘A����́A teranishi@rikaxp.riken.go.jp �ł��B

Tera Term �̍ŐV���ɂ��Ă� Tera Term home page ���������������B
	http://www.vector.co.jp/authors/VA002416/

-------------------------------------------------------------------------------
4. �C���X�g�[��

�z�z�t�@�C�� TTSRCP23.ZIP ���𓀂���Ƃ��́A�z�z�t�@�C���ɋL�^����Ă���
�f�B���N�g���[�\������������悤�ɂ��Ă��������B
���̃f�B���N�g���[�\���͈ȉ��̂悤�ɂȂ��Ă���͂��ł��B

[Base directory] (�Ⴆ�� C:\DEV\TERATERM)
	SOURCE
		COMMON
		KEYCODE
		TERATERM
		TTCMN
		TTDLG
		TTFILE
		TTMACRO
		TTSET
		TTTEK
		TTXTEST
	VISUALC
		BIN
			DEBUG
			RELEASE
		KEYCODE
			DEBUG
			RELEASE
		TTERMPRO
			DEBUG
			RELEASE
		TTPCMN
			DEBUG
			RELEASE
		TTPDLG
			DEBUG
			RELEASE
		TTPFILE
			DEBUG
			RELEASE
		TTPMACRO
			DEBUG
			RELEASE
		TTPSET
			DEBUG
			RELEASE
		TTPTEK
			DEBUG
			RELEASE
	WATCOM
		BIN
		KEYCODE
		LIB
		TTERMPRO
		TTPCMN
		TTPDLG
		TTPFILE
		TTPMACRO
		TTPSET
		TTPTEK
	WATCOM16
		BIN
		KEYCODE
		LIB
		TERATERM
		TTCMN
		TTDLG
		TTFILE
		TTMACRO
		TTSET
		TTTEK

-------------------------------------------------------------------------------
5. �R���p�C���̂�����

A) MS-Visual C/C++ 5.0 �̏ꍇ

1. Developer Studio ���N���B

2. �v���W�F�N�g���[�N�X�y�[�X�t�@�C�� "VISUALC\TTERMPRO.DSW" ���J���B

3. "[�r���h] �o�b�`�r���h" �R�}���h�ł��ׂẴv���W�F�N�g���r���h����B

4. �ȉ��̎��s�t�@�C���� "VISUALC\BIN\RELEASE" �� "VISUALC\BIN\DEBUG"
   �ɍ쐬����Ă���͂��B

	TTERMPRO.EXE
	TTPCMN.DLL
	TTPDLG.DLL
	TTPFILE.DLL
	TTPSET.DLL
	TTPTEK.DLL
	TTPMACRO.EXE
	KEYCODE.EXE

B) Watcom C/C++ 11.0 �̏ꍇ

1. IDE ���N���B

2. Project file "WATCOM\TTERMPRO.WPJ" (32-bit) �܂���
   "WATCOM16\TERATERM.WPJ" (16-bit) ���J���B

3. "[Actions] Make all" �R�}���h�ɂ��A���ׂĂ� target �� make ����B

4. �ȉ��̎��s�t�@�C�����쐬����Ă���͂��B

	WATCOM\BIN
		TTERMPRO.EXE
		TTPCMN.DLL
		TTPDLG.DLL
		TTPFILE.DLL
		TTPSET.DLL
		TTPTEK.DLL
		TTPMACRO.EXE
		KEYCODE.EXE

	WATCOM16\BIN
		TERATERM.EXE
		TTCMN.DLL
		TTDLG.DLL
		TTFILE.DLL
		TTSET.DLL
		TTTEK.DLL
		TTMACRO.EXE
		KEYCODE.EXE

-------------------------------------------------------------------------------
6. �t�@�C�����X�g

[Base directory]
README.TXT	���̕����̉p���
READMEJ.TXT	���̕���

[SOURCE\COMMON] -- common source files
DLGLIB.C/H	Dialog box control routines
HELPID.H	Help context IDs
STDAFX.CPP/H	Files for MFC applications
TEKTYPES.H	Type definitions for TEK window
TERATERM.H	Common header file
TT_RES.H	Resource IDs for VT window (32-bit)
TT_RES16.H	Resource IDs for VT window (16-bit)
TTCOMMON.H	TT(P)CMN.DLL interface
TTCTL3D.C/H	CTL3D interface (16-bit)
TTFTYPES.H	Constants and types for file transfer
TTLIB.C/H	Misc. routines
TTPLUGIN.H	Header file for Tera Term extension
TTTYPES.H	General constants and types

TEK.ICO 	TEK icon (32-bit)
TEK16.ICO	TEK icon (16-bit)
TERATERM.ICO	Tera Term icon (32-bit)
TTERM16.ICO	Tera Term icon (16-bit)
VT.ICO		VT icon (32-bit)
VT16.ICO	VT icon (16-bit)

[SOURCE\KEYCODE] -- source files of KEYCODE.EXE
KEYCODE.C	Main
KC_RES.H	Resource IDs (32-bit)
KC_RES16.H	Resource IDs (16-bit)

KEYCODE.RC	Resource file (32-bit)
KEYCOD16.RC	Resource file (16-bit)
KEYCODE.ICO	KEYCODE icon (32-bit)
KEYCOD16.ICO	KEYCODE icon (16-bit)

[SOURCE\TERATERM] -- source files of TTERMPRO.EXE (TERATERM.EXE)
BUFFER.C/H	Scroll buffer
CLIPBOAR.C/H	Clipboard
COMMLIB.C/H	Communication
FILESYS.CPP/H	File transfer (TT(P)FILE.DLL interface)
FTDLG.CPP/H	Log-file/send-file dialog box
KEYBOARD.C/H	Keyboard
PRNABORT.CPP/H	Print abort dialog box
PROTODLG.CPP/H	Protocol dialog box
TEKLIB.C/H	TT(P)TEK.DLL interface
TEKWIN.CPP/H	TEK window
TELNET.C/H	Telnet
TERAPRN.CPP/H	Print
TERATERM.CPP/TERAAPP.H	Main
TTIME.C/H	Japanese input system
TTDIALOG.C/H	TT(P)DLG.DLL interface
TTDDE.C/H	Communication with TT(P)MACRO.EXE
TTPLUG.C/H	Tera Term extension interface
TTSETUP.C/H	TT(P)SET.DLL interface
TTWINMAN.C/H	Common routines, variables and flags
		for VT and TEK window
TTWSK.C/H	Winsock interface
VTDISP.C/H	Display
VTTERM.C/H	Escape sequences
VTWIN.CPP/H	Main window (VT window)

TTERMPRO.RC	Resource file (32-bit)
TERATERM.RC	Resource file (16-bit)

[SOURCE\TTCMN] -- source files of TT(P)CMN.DLL
LANGUAGE.C/H	Japanese and Russian routines
TTCMN.C 	Main

[SOURCE\TTDLG] -- source files of TT(P)DLG.DLL
TTDLG.C 	Main
DLG_RES.H	Resource IDs (32-bit)
DLG_RE16.H	Resource IDs (16-bit)

TTPDLG.RC	Resource file (32-bit)
TTDLG.RC	Resource file (16-bit)

[SOURCE\TTFILE] -- source files of TT(P)FILE.DLL
BPLUS.C/H	B-Plus protocol
FTLIB.C/H	Routines for file transfer
KERMIT.C/H	Kermit protocol
QUICKVAN.C/H	Quick-VAN protocol
TTFILE.C	Main
XMODEM.C/H	XMODEM protocol
ZMODEM.C/H	ZMODEM protocol
FILE_RES.H	Resource IDs (32-bit)
FILE_R16.H	Resource IDs (16-bit)

TTPFILE.RC	Resource file (32-bit)
TTFILE.RC	Resource file (16-bit)

[SORUCE\TTMACRO] -- source files of TT(P)MACRO.EXE
ERRDLG.CPP/H	Error dialog box
INPDLG.CPP/H	Input dialog box
MSGDLG.CPP/H	Message dialog box
STATDLG.CPP/H	Status dialog box
TTL.C/H 	Script interpreter
TTMACRO.CPP/H	Main
TTMBUFF.C/H	Macro file buffer
TTMDDE.C/H	Communication with TTERMPRO.EXE (TERATERM.EXE)
TTMDLG.C/H	Dialog boxes
TTMENC.C/H	Password encryption/decryption
TTMLIB.C/H	Misc. routines
TTMMAIN.CPP/H	Main window
TTMPARSE.C/H	Script parser
TTM_RES.H	Resource IDs (32-bit)
TTM_RE16.H	Resource IDs (16-bit)
TTMMSG.H	Message IDs

TTPMACRO.RC	Resource file (32-bit)
TTMACRO.RC	Resource file (16-bit)
TTMACRO.ICO	TTPMACRO icon (32-bit)
TTMACR16.ICO	TTPMACRO icon (16-bit)

[SOURCE\TTSET] -- source file of TT(P)SET.DLL
TTSET.C 	Main

[SOURCE\TTTEK] -- source files of TT(P)TEK.DLL
TEKESC.C/H	TEK escape sequences
TTTEK.C 	Main

[SOURCE\TTXTEST] -- sample add-on module TTXTEST.DLL
TTXTEST.C	Main

[VISUALC]
TTERMPRO.DSP	TTERMPRO project file
TTERMPRO.DSW	Visual C/C++ project workspace file
TTERMPRO.MAK	Project make file

[VISUALC\KEYCODE]
KEYCODE.DSP	KEYCODE project file

[VISUALC\TTPCMN]
TTPCMN.DEF	Module definition file of TTPCMN.DLL
TTPCMN.DSP	TTPCMN project file

[VISUALC\TTPDLG]
TTPDLG.DEF	Module definition file of TTPDLG.DLL
TTPDLG.DSP	TTPDLG project file

[VISUALC\TTPFILE]
TTPFILE.DEF	Module definition file of TTPFILE.DLL
TTPFILE.DSP	TTPFILE project file

[VISUALC\TTPMACRO]
TTPMACRO.DSP	TTPMACRO project file

[VISUALC\TTPSET]
TTPSET.DEF	Module definition file of TTPSET.DLL
TTPSET.DSP	TTPSET project file

[VISUALC\TTPTEK]
TTPTEK.DEF	Module definition file of TTPTEK.DLL
TTPTEK.DSP	TTPTEK project file

[WATCOM]
TTERMPRO.WPJ	WATCOM Project file (32-bit)

[WATCOM\KEYCODE] -- KEYCODE target
KEYCODE.TGT	Target file

[WATCOM\LIB]
TTPCMN.LIB	Import library of TTPCMN.DLL

[WATCOM\TTERMPRO] -- TTERMPRO target
TTERMPRO.TGT	Target file

[WATCOM\TTPCMN] -- TTPCMN target
TTPCMN.TGT	Target file
TTPCMN.LBC	Librarian command file

[WATCOM\TTPDLG] -- TTPDLG target
TTPDLG.TGT	Target file
TTPDLG.LBC	Librarian command file

[WATCOM\TTPFILE] -- TTPFILE target
TTPFILE.TGT	Target file
TTPFILE.LBC	Librarian command file

[WATCOM\TTPMACRO] -- TTPMACRO target
TTPMACRO.TGT	Target file

[WATCOM\TTPSET] -- TTPSET target
TTPSET.TGT	Target file
TTPSET.LBC	Librarian command file

[WATCOM\TTPTEK] -- TTPTEK target
TTPTEK.TGT	Target file
TTPTEK.LBC	Librarian command file

[WATCOM16]
TERATERM.WPJ	WATCOM Project file (16-bit)

[WATCOM16\KEYCODE] -- KEYCODE target
KEYCODE.TGT	Target file

[WATCOM16\LIB]
TTCMN.LIB	Import library of TTCMN.DLL

[WATCOM16\TERATERM] -- TERATERM target
TERATERM.TGT	Target file

[WATCOM16\TTCMN] -- TTCMN target
TTCMN.TGT	Target file
TTCMN.LBC	Librarian command file

[WATCOM16\TTDLG] -- TTDLG target
TTDLG.TGT	Target file
TTDLG.LBC	Librarian command file

[WATCOM16\TTFILE] -- TTFILE target
TTFILE.TGT	Target file
TTFILE.LBC	Librarian command file

[WATCOM16\TTMACRO] -- TTMACRO target
TTMACRO.TGT	Target file

[WATCOM16\TTSET] -- TTPET target
TTSET.TGT	Target file
TTSET.LBC	Librarian command file

[WATCOM16\TTPTEK] -- TTTEK target
TTTEK.TGT	Target file
TTTEK.LBC	Librarian command file
