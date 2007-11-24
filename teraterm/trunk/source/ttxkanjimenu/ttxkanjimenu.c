/*
 * TTX KanjiMenu Plugin
 *    Copyright (C) 2007 Sunao HARA (naoh@nagoya-u.jp)
 */

//// ORIGINAL SOURCE CODE: ttxtest.c

/* Teraterm extension mechanism
   Robert O'Callahan (roc+tt@cs.cmu.edu)
   
   Teraterm by Takashi Teranishi (teranishi@rikaxp.riken.go.jp)
*/

#include "teraterm.h"
#include "tttypes.h"
#include "ttplugin.h"
#include "i18n.h"
#define IniSection "TTXKanjiMenu"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
 * This function is called when TeraTerm starts up.
 */
static void PASCAL FAR TTXInit(PTTSet ts, PComVar cv) {
	pvar->ts = ts;
	pvar->cv = cv;
}

// #define ID_MI_KANJIMASK 0xFF00
#define ID_MI_KANJIRECV 54009
#define ID_MI_KANJISEND 54109
/*
 * This function is called when Teraterm creates a new menu.
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
 * This function is called when Teraterm pops up a submenu menu.
 */
static void PASCAL FAR TTXModifyPopupMenu(HMENU menu) {
	// メニューが呼び出されたら、最新の設定に更新する。(2007.5.25 yutaka)
	UpdateRecvMenu(pvar->ts->KanjiCode);
	UpdateSendMenu(pvar->ts->KanjiCodeSend);
}


/*
 * This function is called when Teraterm receives a command message.
 */
static int PASCAL FAR TTXProcessCommand(HWND hWin, WORD cmd) {
	WORD val;

	if( (cmd > ID_MI_KANJIRECV) && (cmd <= ID_MI_KANJIRECV+IdUTF8)) {
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
 * main Teraterm code. It mostly consists of pointers to the above functions.
 * Any of the function pointers can be replaced with NULL, in which case
 * Teraterm will just ignore that function and assume default behaviour, which
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
			hInst = hInstance;
			pvar = &InstVar;
			break;
		case DLL_PROCESS_DETACH:
			/* do process cleanup */
			break;
	}
	return TRUE;
}
