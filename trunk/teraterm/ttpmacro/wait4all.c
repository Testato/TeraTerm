/*
 * "wait4all"�}�N���R�}���h�p���[�`��
 *
 */
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "wait4all.h"
#include "ttlib.h"

//  1: disable
//  0: enable
static int function_disable = 1;  

// ���L�������t�H�[�}�b�g�g�����́A�ȉ��̖��̂�ύX���邱�ƁB
#define TTM_FILEMAPNAME "ttm_memfilemap_1"

// ���L�������̃t�H�[�}�b�g
typedef struct {
	HWND WinList[MAXNWIN];
	int NWin;
	struct mbuf {
		char RingBuf[RingBufSize];
		int RBufStart;
		int RBufPtr;
		int RBufCount;
	} mbufs[MAXNWIN];
} TMacroShmem;

static HANDLE HMap = NULL;
static BOOL FirstInstance = FALSE;
static TMacroShmem *pm = NULL;
static int mindex = -1;
static BOOL QuoteFlag;

// �r������
#define MUTEX_NAME "Mutex Object for macro shmem"
static HANDLE hMutex = NULL;

// ���L�������C���f�b�N�X
int macro_shmem_index = -1;


// wait4all�R�}���h���L�����ǂ�����Ԃ�
int is_wait4all_enabled(void)
{
	return !function_disable;
}


// ���L�������̃}�b�s���O
static int open_macro_shmem(void)
{
	HMap = CreateFileMapping(
		(HANDLE) 0xFFFFFFFF, NULL, PAGE_READWRITE,
		0, sizeof(TMacroShmem), TTM_FILEMAPNAME);
	if (HMap == NULL)
		return FALSE;

	FirstInstance = (GetLastError() != ERROR_ALREADY_EXISTS);

	pm = (TMacroShmem *)MapViewOfFile(HMap,FILE_MAP_WRITE,0,0,0);
	if (pm == NULL)
		return FALSE;

	if (FirstInstance) { // ����ɃA�^�b�`�����l���A�ӔC�������ăN���A���Ă����B
		memset(pm, 0, sizeof(TMacroShmem));
	}

	hMutex = CreateMutex(NULL, FALSE, MUTEX_NAME);

	return TRUE;
}

static void close_macro_shmem(void)
{
	if (pm) {
		UnmapViewOfFile(pm);
		pm = NULL;
	}
	if (HMap) {
		CloseHandle(HMap);
		HMap = NULL;
	}
	if (hMutex)
		CloseHandle(hMutex);
}

static HANDLE lock_shmem(void)
{
	return OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEX_NAME);
}

static void unlock_shmem(HANDLE hd)
{
	ReleaseMutex(hd);
}

// �}�N���E�B���h�E��o�^����
int register_macro_window(HWND hwnd)
{
	int i;
	int ret = FALSE;
	HANDLE hd;
	char buf[20];

	GetOnOffEntryInifile("Wait4allMacroCommand", buf, sizeof(buf));
	if (_stricmp(buf, "on") == 0) {
		function_disable = 0;
	} else {
		function_disable = 1;
	}

	open_macro_shmem();

	hd = lock_shmem();

	for (i = 0 ; i < MAXNWIN ; i++) {
		if (pm->WinList[i] == NULL) {
			pm->NWin++;
			pm->WinList[i] = hwnd;
			macro_shmem_index = mindex = i;   // �C���f�b�N�X��ۑ�
			ret = TRUE;
			break;
		}
	}

	unlock_shmem(hd);

	return (ret);
}

// �}�N���E�B���h�E��o�^��������
int unregister_macro_window(HWND hwnd)
{
	int i;
	int ret = FALSE;
	HANDLE hd;

	hd = lock_shmem();

	for (i = 0 ; i < MAXNWIN ; i++) {
		if (pm->WinList[i] == hwnd) {
			pm->NWin--;
			pm->WinList[i] = NULL;

			pm->mbufs[i].RBufCount = 0;
			pm->mbufs[i].RBufPtr = 0;
			pm->mbufs[i].RBufStart = 0;
			ret = TRUE;
			break;
		}
	}

	unlock_shmem(hd);

	close_macro_shmem();

	return (ret);
}

// �A�N�e�B�u�ɂȂ��Ă���ttpmacro�̃C���f�b�N�X�z���Ԃ��B
void get_macro_active_info(int *num, int *index)
{
	int i;
	HANDLE hd;

	hd = lock_shmem();

	*num = pm->NWin;

	for (i = 0 ; i < MAXNWIN ; i++) {
		if (pm->WinList[i]) {
			*index++ = i;
		}
	}

	unlock_shmem(hd);
}

// ���݂̃A�N�e�B�uttpmacro����Ԃ�
int get_macro_active_num(void)
{
	return pm->NWin;
}


void put_macro_1byte(BYTE b)
{
	char *RingBuf;
	int RBufPtr;
	int RBufCount;
	int RBufStart;
	HANDLE hd;

	if (function_disable)
		return;

	RingBuf = pm->mbufs[mindex].RingBuf;
	RBufPtr = pm->mbufs[mindex].RBufPtr;
	RBufCount = pm->mbufs[mindex].RBufCount;
	RBufStart = pm->mbufs[mindex].RBufStart;

	hd = lock_shmem();

	RingBuf[RBufPtr] = b;
	RBufPtr++;
	if (RBufPtr>=RingBufSize) {
		RBufPtr = RBufPtr-RingBufSize;
	}
	if (RBufCount>=RingBufSize) {
		RBufCount = RingBufSize;
		RBufStart = RBufPtr;
	}
	else {
		RBufCount++;
	}

	// push back
	pm->mbufs[mindex].RBufPtr = RBufPtr;
	pm->mbufs[mindex].RBufCount = RBufCount;
	pm->mbufs[mindex].RBufStart = RBufStart;

	unlock_shmem(hd);
}

int read_macro_1byte(int index, LPBYTE b)
{
	char *RingBuf;
	int RBufPtr;
	int RBufCount;
	int RBufStart;
	HANDLE hd;

	if (function_disable)
		return FALSE;

	RingBuf = pm->mbufs[index].RingBuf;
	RBufPtr = pm->mbufs[index].RBufPtr;
	RBufCount = pm->mbufs[index].RBufCount;
	RBufStart = pm->mbufs[index].RBufStart;

	if (RBufCount<=0) {
		return FALSE;
	}
	hd = lock_shmem();

	*b = RingBuf[RBufStart];
	RBufStart++;
	if (RBufStart>=RingBufSize) {
		RBufStart = RBufStart-RingBufSize;
	}
	RBufCount--;
	if (QuoteFlag) {
		*b= *b-1;
		QuoteFlag = FALSE;
	}
	else {
		QuoteFlag = (*b==0x01);
	}

	// push back
	pm->mbufs[index].RBufPtr = RBufPtr;
	pm->mbufs[index].RBufCount = RBufCount;
	pm->mbufs[index].RBufStart = RBufStart;

	unlock_shmem(hd);

	return (! QuoteFlag);
}
