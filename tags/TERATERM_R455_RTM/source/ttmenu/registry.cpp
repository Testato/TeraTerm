#define		STRICT
static char *Registry_id = 
	"@(#)Copyright (C) NTT-IT 1998  -- registry.cpp --  Ver1.00b1";
/* ==========================================================================
	Project Name		: Universal Library
	Outline				: Registory Function
	Create				: 1998-02-17(Tue)
	Update				: 
	Copyright			: S.Hayakawa				NTT-IT
	Reference			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
#include	"registry.h"


/* ==========================================================================
	Function Name	: (HKEY) RegCreate()
	Outline			: 指定したレジストリキーを作成（またはオープン）する
	Arguments		: HKEY		hCurrentKey		(in)	現在のオープンキー
					: LPCTSTR	lpszKeyName		(in)	オープンするサブキーの
					: 									名前
	Return Value	: 成功	オープンまたは作成されたキーのハンドル
					: 失敗	NULL
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
HKEY RegCreate(HKEY hCurrentKey, LPCTSTR lpszKeyName)
{
	long	lError;
	HKEY	hkResult;
	DWORD	dwDisposition;

	lError = ::RegCreateKeyEx(hCurrentKey,
							lpszKeyName,
							0,
							NULL,
							REG_OPTION_NON_VOLATILE,
							KEY_ALL_ACCESS,
							NULL,
							&hkResult,
							&dwDisposition);
	if (lError != ERROR_SUCCESS) {
		::SetLastError(lError);
		return (HKEY) INVALID_HANDLE_VALUE;
	}

	return hkResult;
}

/* ==========================================================================
	Function Name	: (HKEY) RegOpen()
	Outline			: 指定したレジストリキーをオープンする
	Arguments		: HKEY		hCurrentKey		(in)	現在のオープンキー
					: LPCTSTR	lpszKeyName		(in)	オープンするサブキーの
					: 									名前
	Return Value	: 成功	オープンまたは作成されたキーのハンドル
					: 失敗	NULL
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
HKEY RegOpen(HKEY hCurrentKey, LPCTSTR lpszKeyName)
{
	long	lError;
	HKEY	hkResult;

	lError = ::RegOpenKeyEx(hCurrentKey,
							lpszKeyName,
							0,
							KEY_ALL_ACCESS,
							&hkResult);
	if (lError != ERROR_SUCCESS) {
		::SetLastError(lError);
		return (HKEY) INVALID_HANDLE_VALUE;
	}

	return hkResult;
}

/* ==========================================================================
	Function Name	: (BOOL) RegClose()
	Outline			: 指定したレジストリキーをクローズする
	Arguments		: HKEY		hKey			(in)	クローズするキーのハンドル
	Return Value	: 成功	TRUE
					: 失敗	FALSE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL RegClose(HKEY hKey)
{
	long	lError;

	lError = ::RegCloseKey(hKey);
	if (lError != ERROR_SUCCESS) {
		::SetLastError(lError);
		return FALSE;
	}

	return TRUE;
}

/* ==========================================================================
	Function Name	: (BOOL) RegSetStr()
	Outline			: レジストリキーの値に文字列を書き込む
	Arguments		: HKEY		hKey			(in)	値を設定するキーのハンドル
					: LPCTSTR	lpszValueName	(in)	設定する値
					: char		*buf			(in)	値データ
	Return Value	: 成功	TRUE
					: 失敗	FALSE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL RegSetStr(HKEY hKey, LPCTSTR lpszValueName, TCHAR *buf)
{
	long	lError;

	lError = ::RegSetValueEx(hKey,
							lpszValueName,
							0,
							REG_SZ,
							(CONST BYTE *) buf,
							(::lstrlen(buf) + 1) * sizeof(TCHAR));
	if (lError != ERROR_SUCCESS) {
		::SetLastError(lError);
		return FALSE;
	}

	return TRUE;
}

/* ==========================================================================
	Function Name	: (BOOL) RegGetStr()
	Outline			: レジストリキーの値から文字列を読み込む
	Arguments		: HKEY		hKey			(in)		値を設定するキーの
					: 										ハンドル
					: LPCTSTR	lpszValueName	(in)		設定する値
					: char		*buf			(out)		値データを格納する
					: 										バッファ
					: DWORD		dwSize			(in/out)	文字数
	Return Value	: 成功	TRUE
					: 失敗	FALSE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL RegGetStr(HKEY hKey, LPCTSTR lpszValueName, TCHAR *buf, DWORD dwSize)
{
	LONG	lError;
	DWORD	dwWriteSize;
	DWORD	dwType = REG_SZ;

	dwWriteSize = dwSize * sizeof(TCHAR);

	lError = ::RegQueryValueEx(hKey, lpszValueName, 0, &dwType, (LPBYTE) buf, &dwWriteSize);
	if (lError != ERROR_SUCCESS) {
		::SetLastError(lError);
		return FALSE;
	}

	buf[dwSize - 1] = '\0';

	return TRUE;
}

/* ==========================================================================
	Function Name	: (BOOL) RegSetDword()
	Outline			: レジストリキーの値に DWORDを書き込む
	Arguments		: HKEY		hKey			(in)	値を設定するキーのハンドル
					: LPCTSTR	lpszValueName	(in)	設定する値
					: DWORD		dwValue			(in)	値データ
	Return Value	: 成功	TRUE
					: 失敗	FALSE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL RegSetDword(HKEY hKey, LPCTSTR lpszValueName, DWORD dwValue)
{
	long	lError;

	lError = ::RegSetValueEx(hKey,
							lpszValueName,
							0,
							REG_DWORD,
							(CONST BYTE *) &dwValue,
							sizeof(DWORD));
	if (lError != ERROR_SUCCESS) {
		::SetLastError(lError);
		return FALSE;
	}

	return TRUE;
}

/* ==========================================================================
	Function Name	: (BOOL) RegGetDword()
	Outline			: レジストリキーの値から DWORDを読み込む
	Arguments		: HKEY		hKey			(in)	値を設定するキーのハンドル
					: LPCTSTR	lpszValueName	(in)	設定する値
					: DWORD		*dwValue		(out)	値データ
	Return Value	: 成功	TRUE
					: 失敗	FALSE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL RegGetDword(HKEY hKey, LPCTSTR lpszValueName, DWORD *dwValue)
{
	long	lError;
	DWORD	dwType = REG_DWORD;
	DWORD	dwSize = sizeof(DWORD);

	lError = ::RegQueryValueEx(hKey,
								lpszValueName,
								0,
								&dwType,
								(LPBYTE) dwValue,
								&dwSize);
	if (lError != ERROR_SUCCESS) {
		::SetLastError(lError);
		return FALSE;
	}

	return TRUE;
}

/* ==========================================================================
	Function Name	: (BOOL) RegSetBinary()
	Outline			: レジストリキーの値から BINARYを読み込む
	Arguments		: HKEY		hKey			(in)	値を設定するキーのハンドル
					: LPCTSTR	lpszValueName	(in)	設定する値
					: void		*buf			(out)	値データ
	Return Value	: 成功	TRUE
					: 失敗	FALSE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
BOOL RegSetBinary(HKEY hKey, LPCTSTR lpszValueName, void *buf, DWORD dwSize)
{
	long	lError;
	DWORD	dwWriteSize;

	dwWriteSize = dwSize * sizeof(TCHAR);

	if ((lError = ::RegSetValueEx(hKey,
								lpszValueName,
								0,
								REG_BINARY,
								(CONST BYTE *) buf,
								dwWriteSize)) != ERROR_SUCCESS) {
		::SetLastError(lError);
		return FALSE;
	}

	return TRUE;
}

/* ==========================================================================
	Function Name	: (BOOL) RegGetBinary()
	Outline			: レジストリキーの値から BINARYを読み込む
	Arguments		: HKEY		hKey			(in)	値を設定するキーのハンドル
					: LPCTSTR	lpszValueName	(in)	設定する値
					: int		*buf			(out)	値データ
	Return Value	: 成功	TRUE
					: 失敗	FALSE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
// 関数の返値の型を追加 (2006.2.18 yutaka)
int RegGetBinary(HKEY hKey, LPCTSTR lpszValueName, void *buf, LPDWORD lpdwSize)
{
	long	lError;
	DWORD	dwType = REG_BINARY;
	DWORD	dwWriteSize;

	dwWriteSize = *lpdwSize * sizeof(TCHAR);

	if ((lError = ::RegQueryValueEx(hKey,
									lpszValueName,
									NULL,
									&dwType,
									(LPBYTE) buf,
									&dwWriteSize)) != ERROR_SUCCESS) {
		::SetLastError(lError);
		return FALSE;
	}

	return TRUE;
}
