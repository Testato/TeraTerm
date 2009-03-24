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
	Outline			: �w�肵�����W�X�g���L�[���쐬�i�܂��̓I�[�v���j����
	Arguments		: HKEY		hCurrentKey		(in)	���݂̃I�[�v���L�[
					: LPCTSTR	lpszKeyName		(in)	�I�[�v������T�u�L�[��
					: 									���O
	Return Value	: ����	�I�[�v���܂��͍쐬���ꂽ�L�[�̃n���h��
					: ���s	NULL
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
	Outline			: �w�肵�����W�X�g���L�[���I�[�v������
	Arguments		: HKEY		hCurrentKey		(in)	���݂̃I�[�v���L�[
					: LPCTSTR	lpszKeyName		(in)	�I�[�v������T�u�L�[��
					: 									���O
	Return Value	: ����	�I�[�v���܂��͍쐬���ꂽ�L�[�̃n���h��
					: ���s	NULL
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
	Outline			: �w�肵�����W�X�g���L�[���N���[�Y����
	Arguments		: HKEY		hKey			(in)	�N���[�Y����L�[�̃n���h��
	Return Value	: ����	TRUE
					: ���s	FALSE
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
	Outline			: ���W�X�g���L�[�̒l�ɕ��������������
	Arguments		: HKEY		hKey			(in)	�l��ݒ肷��L�[�̃n���h��
					: LPCTSTR	lpszValueName	(in)	�ݒ肷��l
					: char		*buf			(in)	�l�f�[�^
	Return Value	: ����	TRUE
					: ���s	FALSE
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
	Outline			: ���W�X�g���L�[�̒l���當�����ǂݍ���
	Arguments		: HKEY		hKey			(in)		�l��ݒ肷��L�[��
					: 										�n���h��
					: LPCTSTR	lpszValueName	(in)		�ݒ肷��l
					: char		*buf			(out)		�l�f�[�^���i�[����
					: 										�o�b�t�@
					: DWORD		dwSize			(in/out)	������
	Return Value	: ����	TRUE
					: ���s	FALSE
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
	Outline			: ���W�X�g���L�[�̒l�� DWORD����������
	Arguments		: HKEY		hKey			(in)	�l��ݒ肷��L�[�̃n���h��
					: LPCTSTR	lpszValueName	(in)	�ݒ肷��l
					: DWORD		dwValue			(in)	�l�f�[�^
	Return Value	: ����	TRUE
					: ���s	FALSE
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
	Outline			: ���W�X�g���L�[�̒l���� DWORD��ǂݍ���
	Arguments		: HKEY		hKey			(in)	�l��ݒ肷��L�[�̃n���h��
					: LPCTSTR	lpszValueName	(in)	�ݒ肷��l
					: DWORD		*dwValue		(out)	�l�f�[�^
	Return Value	: ����	TRUE
					: ���s	FALSE
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
	Outline			: ���W�X�g���L�[�̒l���� BINARY��ǂݍ���
	Arguments		: HKEY		hKey			(in)	�l��ݒ肷��L�[�̃n���h��
					: LPCTSTR	lpszValueName	(in)	�ݒ肷��l
					: void		*buf			(out)	�l�f�[�^
	Return Value	: ����	TRUE
					: ���s	FALSE
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
	Outline			: ���W�X�g���L�[�̒l���� BINARY��ǂݍ���
	Arguments		: HKEY		hKey			(in)	�l��ݒ肷��L�[�̃n���h��
					: LPCTSTR	lpszValueName	(in)	�ݒ肷��l
					: int		*buf			(out)	�l�f�[�^
	Return Value	: ����	TRUE
					: ���s	FALSE
	Reference		: 
	Renewal			: 
	Notes			: 
	Attention		: 
	Up Date			: 
   ======1=========2=========3=========4=========5=========6=========7======= */
// �֐��̕Ԓl�̌^��ǉ� (2006.2.18 yutaka)
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
