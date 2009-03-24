	Source code of Tera Term Pro version 2.3 and
		       Tera Term version 1.4
	T. Teranishi Mar 10, 1998

	Copyright (C) 1994-1998 T. Teranishi
	All Rights Reserved.

DESCRIPTION
~~~~~~~~~~~
Tera Term (Pro) is a terminal emulator (telnet client)
for MS-Windows. Tera Term Pro version 2.3 is for Windows 95/NT
and Tera Term version 1.4 is for Windows 3.1.

This package contains the source code for both Tera Term Pro 2.3 and
Tera Term 1.4. The source code can be compiled with
MS-Visual C/C++ 5.0 and Watcom C/C++ 11.0.
The 32-bit Tera Term Pro 2.3 can be compiled by both MS-Visual C/C++
and Watcom C/C++ without any problem while the 16-bit Tera Term 1.4
can be compiled by only Watcom C/C++.

There is one serious problem with the 16-bit Tera Term:
the executable file TERATERM.EXE compiled by Watcom C/C++ does not
support multiple application instances. Namely, you can not run
multiple Tera Term instances at the same time. Probably, the problem
is solved by using the Microsoft 16-bit compiler, but it has never
been tested.
Because of this problem, the released package of Tera Term 1.4 was
actually produced by the Turbo Pascal source code which is equivalent
to the C/C++ code. The Turbo Pascal for Windows does not have the
multiple instance problem. You can find the Turbo Pascal source code
in another package TTSRCV14.ZIP.

This package also contains a demonstration of the add-on module for
Tera Term. An add-on module is provided as a DLL which can hook
Tera Term user interface functions, setup functions and Winsock
functions. The Tera Term extension interface (TTX) between
Tera Term and add-on modules was developed by Robert O'Callahan
and the Tera Term author, Takashi Teranishi.
If you want to modify Tera Term, please consider a possibility of
making an add-on module instead of modifying Tera Term directly.
Making add-on module simplifies the treatment of copyrights problem
and avoids producing Tera Term variants which does not
compatible with each other. See also the next section for making
and distributing add-on modules.

NOTICE
~~~~~~
There is no warranty for damages caused by using this package.

Without written permission by the author (Takashi Teranishi), you may
not distribute modified versions of this package, and may not distribute
this package for profit.

You may not copy any file, module, subroutine and resource
in this package to create commercial products (including sharewares),
without written permission by the author.

If you want to distribute modified versions of Tera Term widely,
you need also the permission.

There is only one exception to these copyrights rules.
You can make and distribute add-on modules for Tera Term
by using the following files without any permission by the author:

	ttxtest.c
	teraterm.h
	tttypes.h
	ttplugin.h
	ttsetup.h
	ttdialog.h
	ttwsk.h

See comments in ttxtest.c for the basic idea of Tera Term extension.
In developing your module, you can modify the file ttxtest.c
while other include files (*.h) should not be modified.
It is recommended that you inform the author of the development of
your add-on module. It is also recommended that you distribute
only the add-on module without Tera Term executable files. Otherwise,
you need a permission by the author to distribute a modified package
of Tera Term.

COMMENTS FROM THE AUTHOR
~~~~~~~~~~~~~~~~~~~~~~~
The original source code of Tera Term (16-bit version)
is written in Turbo Pascal for Windows.
For Tera Term Pro (32-bit version), it is converted into C/C++.
My coding style may not be C/C++-like because I am not
so familiar with C/C++.

Some parts of the source code use the Microsoft Foundation Class
Library (MFC). I do not have a plan to convert the source
to Object Windows Library (OWL) for Borland C/C++.

The source code files of installer and uninstaller are not
included in this package. I can not distribute them to you.
So, please do not ask me to give them.

This package does not contain setup files, help files and
"Tera Special" font. If you need them, copy them from the
Tera Term distribution packages "TTERMV14.ZIP" and "TTERMP23.ZIP".

Please do not ask the author questions about programming,
structure of the Tera Term source code, how to compile
the source code and so on.

You can contact the author by e-mail at the following address:

	teranishi@rikaxp.riken.go.jp

You may see the current status of Tera Term at Tera Term home page:
	http://www.vector.co.jp/authors/VA002416/teraterm.html

INSTALLATION
~~~~~~~~~~~~
Extract the distribution file TTSRCP23.ZIP onto your hard disk
with keeping the directory structure recorded in the file.
The directory structure should be like the following:

[Base directory] (for example C:\DEV\TERATERM)
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

HOW TO MAKE TERA TERM
~~~~~~~~~~~~~~~~~~~~~
A) Using MS-Visual C/C++ 5.0

1. Run Developer Studio.

2. Open the project workspace file "VISUALC\TTERMPRO.DSW".

3. Make all the projects by the "[Build] Batch build" command.

4. You can find the following executable files at
"VISUALC\BIN\RELEASE" and "VISUALC\BIN\DEBUG":

	TTERMPRO.EXE
	TTPCMN.DLL
	TTPDLG.DLL
	TTPFILE.DLL
	TTPSET.DLL
	TTPTEK.DLL
	TTPMACRO.EXE
	KEYCODE.EXE

B) Using Watcom C/C++ 11.0

1. Run IDE.

2. Open the project file "WATCOM\TTERMPRO.WPJ" for the 32-bit Tera Term or
   "WATCOM16\TERATERM.WPJ" for the 16-bit Tera Term.

3. Make all the targets in the project by the "[Actions] Make all"
   command.

4. You can find the following executable files:

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

FILE LIST
~~~~~~~~~
[Base directory]
README.TXT	This document
READMEJ.TXT	Document written in Japanese

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
