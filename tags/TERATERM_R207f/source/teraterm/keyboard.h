/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* proto types */
#ifdef __cplusplus
extern "C" {
#endif

void SetKeyMap();
void ClearUserKey();
void DefineUserKey(int NewKeyId, PCHAR NewKeyStr, int NewKeyLen);
BOOL KeyDown(HWND HWin, WORD VKey, WORD Count, WORD Scan);
void KeyCodeSend(WORD KCode, WORD Count);
void KeyUp(WORD VKey);
BOOL ShiftKey();
BOOL ControlKey();
BOOL AltKey();
void InitKeyboard();
void EndKeyboard();

#define FuncKeyStrMax 32

extern BOOL AutoRepeatMode;
extern BOOL AppliKeyMode, AppliCursorMode;
extern BOOL DebugFlag;

#ifdef __cplusplus
}
#endif
