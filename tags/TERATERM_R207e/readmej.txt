	Source code of Tera Term Pro version 2.3 and
		       Tera Term version 1.4
	T. Teranishi Mar 10, 1998

	Copyright (C) 1994-1998 T. Teranishi
	All Rights Reserved.

-------------------------------------------------------------------------------
1. 概要

Tera Term (Pro) は MS-Windows 用ターミナルエミュレーター
(telnet クライアント)です。Tera Term Pro version 2.3 は
Windows 95/NT 用、Tera Term version 1.4 は Windows 3.1 用です。

このパッケージには Tera Term Pro 2.3 と Tera Term 1.4 のソースコード
(MS-Visual C/C++ 5.0 と Watcom C/C++ 11.0 に対応)が含まれています。

32-bit 版 Tera Term Pro 2.3 は MS-Visual C/C++ および Watcom C/C++
の両方で問題なくコンパイルできますが、16-bit 版 Tera Term 1.4 は
Watcom C/C++ でしかコンパイルできません。

16-bit 版 Tera Term に関して重大な問題があります。Watcom C/C++ に
よって生成された実行ファイル TERATERM.EXE は複数のアプリケーション
インスタンスに対応していません。つまり、同時に複数の Tera Term
インスタンスを実行することができません。おそらく Microsoft の
16-bit コンパイラーを使えばこの問題をさけることができるかもしれま
せんが、確認はされていません。
この問題のため、実際に公開されている Tera Term 1.4 は C/C++ ソース
とほぼ等価な Turbo Pascal ソースコードによって生成されました。
Turbo Pascal for Windows にはこの複数インスタンスの問題がありません。
Turbo Pascal ソースは、別のパッケージ TTSRCV14.ZIP として公開されて
います。

このパッケージには、Tera Term のための add-on モジュールの例も
含まれています。Add-on モジュールは DLL の形で作成され、Tera Term
のユーザーインターフェイス関数、設定関数、Winsock 関数をフック
することができます。Tera Term と add-on モジュールの間の
Tera Term extension interface (TTX) は Robert O'Callahan と
Tera Term 作者(寺西 高)により開発されました。
Tera Term を改造したい場合は、Tera Term を直接改造することを
考える前に、add-on モジュールが作れるかどうかを考えてみてください。
Add-on モジュールを作ることで、著作権の問題を簡単に取り扱うこと
ができますし、互いに互換性のない改造版 Tera Term が多数つくられるの
を防ぐことになります。Add-on モジュールの開発、配布については
次の「2. 注意事項」も読んでください。

-------------------------------------------------------------------------------
2. 注意事項

著作権は、作者(寺西 高)が保持します。このソースコードの使用による、
いかなる損害にたいしても作者は責任を負いません。

このパッケージはオリジナルの形のままならば、再配布自由です。
ただし、金銭的利益を得るための配布には作者の許可が必要です。

このパッケージに含まれる、ファイル、モジュール、サブルーチン、リソース等の
全部または、一部をコピーして作成したプログラムを、金銭的利益を得るために配布
する場合は作者の許可が必要です。

改造版 Tera Term を不特定多数の人に配布する場合にも作者の許可が必要です。

ただし、以下のファイルを使用して Tera Term のための add-on モジュール
を作成し、配布することは、作者の許可なしで可能です。

	ttxtest.c
	teraterm.h
	tttypes.h
	ttplugin.h
	ttsetup.h
	ttdialog.h
	ttwsk.h

Tera Term extension の説明は ttxtest.c のコメントを読んでください。
Add-on モジュールを開発する上で、ttxtest.c を書き換えることは
可能ですが、他のインクルードファイル (*.h) は書き換えないでください。
Add-on モジュールを作成した場合、Tera Term 作者に連絡することを
お勧めします。また、add-on モジュールを配布する場合は、Tera Term
実行ファイルを付けずに add-on モジュールだけを配布することを
お勧めします。そうでなければ、改造版 Tera Term を配布するための
許可を作者に申請する必要があります。

-------------------------------------------------------------------------------
3. 作者からのコメント

Tera Term (16-bit 版)の元々のソースは Turbo Pascal for Windows 1.5 (英語版)
で書かれました。その後 Tera Term Pro (32-bit 版)用に C/C++ に変換されました。
作者は C/C++ にあまりなれていないので、ソースのスタイルは C/C++ 的では
ないかもしれません。

ソースの一部は Microsoft Foundation Class Library (MFC) を使用しています。
Borland C/C++ の Object Windows LIbrary (OWL) に対応する予定はありません。

インストーラーとアンインストーラーのソースは公開しません。
作者に要求しないでください。

このパッケージには設定ファイル、ヘルプファイル、"Tera Special" フォント
が含まれていません。もし必要なら配布パッケージ TTERMP23.ZIP と TTERMV14.ZIP
からコピーしてください。

今後のバージョンアップで、ソースの大部分が書き換えられる可能性があります。
変更部分についてコメントしたりすることはありません。バージョンアップした
ソースを入手して、どこが変更されたのか知りたい場合は、自分でファイル内容を
比較してください。

作者に、プログラミング、Tera Term ソースの構造について、コンパイル方法
等の質問をしないでください。

作者への連絡先は、 teranishi@rikaxp.riken.go.jp です。

Tera Term の最新情報については Tera Term home page をご覧ください。
	http://www.vector.co.jp/authors/VA002416/

-------------------------------------------------------------------------------
4. インストール

配布ファイル TTSRCP23.ZIP を解凍するときは、配布ファイルに記録されている
ディレクトリー構造も復元するようにしてください。
そのディレクトリー構造は以下のようになっているはずです。

[Base directory] (例えば C:\DEV\TERATERM)
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
5. コンパイルのしかた

A) MS-Visual C/C++ 5.0 の場合

1. Developer Studio を起動。

2. プロジェクトワークスペースファイル "VISUALC\TTERMPRO.DSW" を開く。

3. "[ビルド] バッチビルド" コマンドですべてのプロジェクトをビルドする。

4. 以下の実行ファイルが "VISUALC\BIN\RELEASE" と "VISUALC\BIN\DEBUG"
   に作成されているはず。

	TTERMPRO.EXE
	TTPCMN.DLL
	TTPDLG.DLL
	TTPFILE.DLL
	TTPSET.DLL
	TTPTEK.DLL
	TTPMACRO.EXE
	KEYCODE.EXE

B) Watcom C/C++ 11.0 の場合

1. IDE を起動。

2. Project file "WATCOM\TTERMPRO.WPJ" (32-bit) または
   "WATCOM16\TERATERM.WPJ" (16-bit) を開く。

3. "[Actions] Make all" コマンドにより、すべての target を make する。

4. 以下の実行ファイルが作成されているはず。

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
6. ファイルリスト

[Base directory]
README.TXT	この文書の英語版
READMEJ.TXT	この文書

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
