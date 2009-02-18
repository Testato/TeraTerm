/*
 * TTX KanjiMenu Plugin
 *    Copyright (C) 2007 Sunao HARA (naoh@nagoya-u.jp)
 */

//// ORIGINAL SOURCE CODE: ttxtest.c

/* Tera Term extension mechanism
   Robert O'Callahan (roc+tt@cs.cmu.edu)
   
   Tera Term by Takashi Teranishi (teranishi@rikaxp.riken.go.jp)
*/

#include "teraterm.h"
#include "tttypes.h"
#include "ttplugin.h"
#include "i18n.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// VS2005でビルドされたバイナリが Windows95 でも起動できるようにするために、
// IsDebuggerPresent()のシンボル定義を追加する。
//
// cf.http://jet2.u-abel.net/program/tips/forceimp.htm
// 装飾された名前のアドレスを作るための仮定義
// (これだけでインポートを横取りしている)
int WINAPI _imp__IsDebuggerPresent()
    { return PtrToInt((void*) &_imp__IsDebuggerPresent); }
// 実際に横取り処理を行う関数
BOOL WINAPI Cover_IsDebuggerPresent()
    { return FALSE; }
// 関数が実際に呼び出されたときに備えて
// 横取り処理関数を呼び出させるための下準備
void __stdcall DoCover_IsDebuggerPresent()
{
    DWORD dw;
    DWORD_PTR FAR* lpdw;
    // 横取り関数を設定するアドレスを取得
    lpdw = (DWORD_PTR FAR*) &_imp__IsDebuggerPresent;
    // このアドレスを書き込めるように設定
    // (同じプログラム内なので障害なく行える)
    VirtualProtect(lpdw, sizeof(DWORD_PTR), PAGE_READWRITE, &dw);
    // 横取り関数を設定
    *lpdw = (DWORD_PTR)(FARPROC) Cover_IsDebuggerPresent;
    // 読み書きの状態を元に戻す
    VirtualProtect(lpdw, sizeof(DWORD_PTR), dw, NULL);
}
// アプリケーションが初期化される前に下準備を呼び出す
// ※ かなり早くに初期化したいときは、このコードを
//  ファイルの末尾に書いて「#pragma init_seg(lib)」を、
//  この変数宣言の手前に書きます。
//  初期化を急ぐ必要が無い場合は WinMain 内から
//  DoCover_IsDebuggerPresent を呼び出して構いません。
//int s_DoCover_IsDebuggerPresent
//    = (int) (DoCover_IsDebuggerPresent(), 0);

#define IniSection "TTXKanjiMenu"
#define ORDER 5000

#define UpdateRecvMenu(val)	\
	CheckMenuRadioItem(pvar->hmEncode, \
	                   ID_MI_KANJIRECV + IdSJIS, \
	                   ID_MI_KANJIRECV + IdUTF8m, \
	                   ID_MI_KANJIRECV + (val), \
	                   MF_BYCOMMAND)
#define UpdateSendMenu(val)	\
	CheckMenuRadioItem(pvar->hmEncode, \
	                   ID_MI_KANJISEND + IdSJIS, \
	                   ID_MI_KANJISEND + IdUTF8, \
	                   ID_MI_KANJISEND + (val), \
	                   MF_BYCOMMAND)

static HANDLE hInst; /* Instance handle of TTX*.DLL */

typedef struct {
	PTTSet ts;
	PComVar cv;
	HMENU hmEncode;
} TInstVar;

static TInstVar FAR * pvar;

/* WIN32 allows multiple instances of a DLL */
static TInstVar InstVar;

/*
 * This function is called when Tera Term starts up.
 */
static void PASCAL FAR TTXInit(PTTSet ts, PComVar cv) {
	pvar->ts = ts;
	pvar->cv = cv;
}

// #define ID_MI_KANJIMASK 0xFF00
#define ID_MI_KANJIRECV 54009
#define ID_MI_KANJISEND 54109
/*
 * This function is called when Tera Term creates a new menu.
 */
static void PASCAL FAR TTXModifyMenu(HMENU menu) {
	UINT flag = MF_ENABLED;

	// 言語が日本語のときのみメニューに追加されるようにした。 (2007.7.14 maya)
	if (pvar->ts->Language != IdJapanese) {
		return;
	}

	{
		MENUITEMINFO mi;

		pvar->hmEncode = CreateMenu();

		memset(&mi, 0, sizeof(MENUITEMINFO));
		mi.cbSize = sizeof(MENUITEMINFO);
		mi.fMask  = MIIM_TYPE | MIIM_SUBMENU;
		mi.fType  = MFT_STRING;
		mi.hSubMenu = pvar->hmEncode;
		GetI18nStr(IniSection, "MENU_KANJI", pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg),
		           "&KanjiCode", pvar->ts->UILanguageFile);
		mi.dwTypeData = pvar->ts->UIMsg;
		InsertMenuItem(menu, ID_HELPMENU, FALSE, &mi);

		flag = MF_STRING|MF_CHECKED;
		GetI18nStr(IniSection, "MENU_RECV_SJIS", pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg),
		           "Recv: &Shift_JIS", pvar->ts->UILanguageFile);
		AppendMenu(pvar->hmEncode, flag, ID_MI_KANJIRECV+IdSJIS,  pvar->ts->UIMsg);
		GetI18nStr(IniSection, "MENU_RECV_EUCJP", pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg),
		           "Recv: &EUC-JP", pvar->ts->UILanguageFile);
		AppendMenu(pvar->hmEncode, flag, ID_MI_KANJIRECV+IdEUC,   pvar->ts->UIMsg);
		GetI18nStr(IniSection, "MENU_RECV_JIS", pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg),
		           "Recv: &JIS", pvar->ts->UILanguageFile);
		AppendMenu(pvar->hmEncode, flag, ID_MI_KANJIRECV+IdJIS,   pvar->ts->UIMsg);
		GetI18nStr(IniSection, "MENU_RECV_UTF8", pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg),
		           "Recv: &UTF-8", pvar->ts->UILanguageFile);
		AppendMenu(pvar->hmEncode, flag, ID_MI_KANJIRECV+IdUTF8,  pvar->ts->UIMsg);
		GetI18nStr(IniSection, "MENU_RECV_UTF8m", pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg),
		           "Recv: UTF-8&m", pvar->ts->UILanguageFile);
		AppendMenu(pvar->hmEncode, flag, ID_MI_KANJIRECV+IdUTF8m, pvar->ts->UIMsg);
		AppendMenu(pvar->hmEncode, MF_SEPARATOR, 0, NULL);
		GetI18nStr(IniSection, "MENU_SEND_SJIS", pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg),
		           "Send: S&hift_JIS", pvar->ts->UILanguageFile);
		AppendMenu(pvar->hmEncode, flag, ID_MI_KANJISEND+IdSJIS,  pvar->ts->UIMsg);
		GetI18nStr(IniSection, "MENU_SEND_EUCJP", pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg),
		           "Send: EU&C-JP", pvar->ts->UILanguageFile);
		AppendMenu(pvar->hmEncode, flag, ID_MI_KANJISEND+IdEUC,   pvar->ts->UIMsg);
		GetI18nStr(IniSection, "MENU_SEND_JIS", pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg),
		           "Send: J&IS", pvar->ts->UILanguageFile);
		AppendMenu(pvar->hmEncode, flag, ID_MI_KANJISEND+IdJIS,   pvar->ts->UIMsg);
		GetI18nStr(IniSection, "MENU_SEND_UTF8", pvar->ts->UIMsg, sizeof(pvar->ts->UIMsg),
		           "Send: U&TF-8", pvar->ts->UILanguageFile);
		AppendMenu(pvar->hmEncode, flag, ID_MI_KANJISEND+IdUTF8,  pvar->ts->UIMsg);

		UpdateRecvMenu(pvar->ts->KanjiCode);
		UpdateSendMenu(pvar->ts->KanjiCodeSend);
	}
}


/*
 * This function is called when Tera Term pops up a submenu menu.
 */
static void PASCAL FAR TTXModifyPopupMenu(HMENU menu) {
	// メニューが呼び出されたら、最新の設定に更新する。(2007.5.25 yutaka)
	UpdateRecvMenu(pvar->ts->KanjiCode);
	UpdateSendMenu(pvar->ts->KanjiCodeSend);
}


/*
 * This function is called when Tera Term receives a command message.
 */
static int PASCAL FAR TTXProcessCommand(HWND hWin, WORD cmd) {
	WORD val;

	if( (cmd > ID_MI_KANJIRECV) && (cmd <= ID_MI_KANJIRECV+IdUTF8m)) {
		// 範囲チェックを追加 
		// TTProxyのバージョンダイアログを開くと、当該ハンドラが呼ばれ、誤動作していたのを修正。
		// (2007.7.13 yutaka)
		val = cmd - ID_MI_KANJIRECV;
		pvar->cv->KanjiCodeEcho = pvar->ts->KanjiCode = val;
		return UpdateRecvMenu(pvar->ts->KanjiCode)?1:0;
	}
	else
	if( (cmd > ID_MI_KANJISEND) && (cmd <= ID_MI_KANJISEND+IdUTF8) ) {
		val = cmd - ID_MI_KANJISEND;
		pvar->cv->KanjiCodeSend = pvar->ts->KanjiCodeSend = val;
		return UpdateSendMenu(pvar->ts->KanjiCodeSend)?1:0;
	}

	return 0;
}


/*
 * This record contains all the information that the extension forwards to the
 * main Tera Term code. It mostly consists of pointers to the above functions.
 * Any of the function pointers can be replaced with NULL, in which case
 * Tera Term will just ignore that function and assume default behaviour, which
 * means "do nothing".
 */
static TTXExports Exports = {
/* This must contain the size of the structure. See below for its usage. */
	sizeof(TTXExports),

/* This is the load order number of this DLL. */
	ORDER,

/* Now we just list the functions that we've implemented. */
	TTXInit,
	NULL, /* TTXGetUIHooks */
	NULL, /* TTXGetSetupHooks */
	NULL, /* TTXOpenTCP */
	NULL, /* TTXCloseTCP */
	NULL, /* TTXSetWinSize */
	TTXModifyMenu,
	TTXModifyPopupMenu,
	TTXProcessCommand,
	NULL, /* TTXEnd */
	NULL  /* TTXSetCommandLine */
};

BOOL __declspec(dllexport) PASCAL FAR TTXBind(WORD Version, TTXExports FAR * exports) {
	int size = sizeof(Exports) - sizeof(exports->size);
	/* do version checking if necessary */
	/* if (Version!=TTVERSION) return FALSE; */

	if (size > exports->size) {
		size = exports->size;
	}
	memcpy((char FAR *)exports + sizeof(exports->size),
	       (char FAR *)&Exports + sizeof(exports->size),
	       size);
	return TRUE;
}

BOOL WINAPI DllMain(HANDLE hInstance, 
                    ULONG ul_reason_for_call,
                    LPVOID lpReserved)
{
	switch( ul_reason_for_call ) {
		case DLL_THREAD_ATTACH:
			/* do thread initialization */
			break;
		case DLL_THREAD_DETACH:
			/* do thread cleanup */
			break;
		case DLL_PROCESS_ATTACH:
			/* do process initialization */
			DoCover_IsDebuggerPresent();
			hInst = hInstance;
			pvar = &InstVar;
			break;
		case DLL_PROCESS_DETACH:
			/* do process cleanup */
			break;
	}
	return TRUE;
}
