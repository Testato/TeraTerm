/*
 * $Id: libc.cpp,v 1.2 2006-08-02 14:40:19 yutakakn Exp $
 */

#include <YCL/common.h>

static HANDLE processHeap = NULL;
static HINSTANCE module_instance = NULL;

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#ifdef _DEBUG
static const char stack_level_unmatch[] = "Failed stack check!\n";
#endif//_DEBUG
void __declspec(naked) _chkesp() {
	__asm {
#ifdef _DEBUG
		jz ok;
		push offset stack_level_unmatch
		call dword ptr OutputDebugStringA
		int 3
	ok:
#endif//_DEBUG
		ret;
	};
}

#ifdef _DEBUG
static const char called_pure_virtual_function[] = "Pure virtual function was called!\n";
#endif//_DEBUG
int __declspec(naked) _purecall() {
	__asm {
#ifdef _DEBUG
		push offset called_pure_virtual_function
		call dword ptr OutputDebugStringA
		int 3
#endif//_DEBUG
		ret;
	}
}

#ifdef _DEBUG
//{

#ifdef _MT
static CRITICAL_SECTION malloc_section;
#endif//_MT

typedef struct tagMEMORY_ALLOCATION_DATA {
	void* pointer;
	size_t length;
	const char* filename;
	int lineno;
} MEMORY_ALLOCATION_DATA;

MEMORY_ALLOCATION_DATA* memory_allocation_data = NULL;
int memory_allocation_data_length = 0;

void* set_memory_allocation_data(void* pointer, size_t length, const char* filename, int lineno) {
	int index;
#ifdef _MT
	::EnterCriticalSection(&malloc_section);
#endif//_MT
	for (index = 0; index < memory_allocation_data_length; index++) {
		if (memory_allocation_data[index].pointer == NULL)
			break;
	}
	if (index >= memory_allocation_data_length) {
		int new_length = memory_allocation_data_length + 8;
		MEMORY_ALLOCATION_DATA* newdata;
		if (memory_allocation_data == NULL) {
			newdata = (MEMORY_ALLOCATION_DATA*) ::HeapAlloc(processHeap, 0, new_length * sizeof (MEMORY_ALLOCATION_DATA));
		}else{
			newdata = (MEMORY_ALLOCATION_DATA*) ::HeapReAlloc(processHeap, 0, memory_allocation_data, new_length * sizeof (MEMORY_ALLOCATION_DATA));
		}
		if (newdata != NULL) {
			memory_allocation_data = newdata;
			memset(memory_allocation_data + memory_allocation_data_length, 0, (new_length - memory_allocation_data_length) * sizeof (MEMORY_ALLOCATION_DATA));
			index = memory_allocation_data_length;
			memory_allocation_data_length = new_length;
		}
	}
	if (index < memory_allocation_data_length) {
		memory_allocation_data[index].pointer = pointer;
		memory_allocation_data[index].length = length;
		memory_allocation_data[index].filename = filename;
		memory_allocation_data[index].lineno = lineno;
	}
#ifdef _MT
	::LeaveCriticalSection(&malloc_section);
#endif//_MT
	return pointer;
}

void clear_memory_allocation_data(void* pointer, const char* filename, int lineno) {
	int index;
#ifdef _MT
	::EnterCriticalSection(&malloc_section);
#endif//_MT
	for (index = 0; index < memory_allocation_data_length; index++) {
		if (memory_allocation_data[index].pointer == pointer)
			break;
	}
	if (index < memory_allocation_data_length) {
		memory_allocation_data[index].pointer = NULL;
	}else{
		char buffer[1024];
		wsprintf(buffer, "%s(%d): free memory without malloc/realloc/calloc(%p)\n", filename, lineno, pointer);
		::OutputDebugString(buffer);
		__asm {
			int 3
		}
	}
#ifdef _MT
	::LeaveCriticalSection(&malloc_section);
#endif//_MT
}

void* _malloc_dbg(size_t length, const char* filename, int lineno) {
	return set_memory_allocation_data(::HeapAlloc(processHeap, 0, length), length, filename, lineno);
}

void* _realloc_dbg(void* pointer, size_t length, const char* filename, int lineno) {
	void* new_pointer;
	if (pointer != NULL) {
		new_pointer = ::HeapReAlloc(processHeap, 0, pointer, length);
	}else{
		new_pointer = ::HeapAlloc(processHeap, 0, length);
	}
	if (new_pointer != NULL) {
		clear_memory_allocation_data(pointer, filename, lineno);
		set_memory_allocation_data(new_pointer, length, filename, lineno);
	}
	return new_pointer;
}

void* _calloc_dbg(size_t num, size_t size, const char* filename, int lineno) {
	return set_memory_allocation_data(::HeapAlloc(processHeap, HEAP_ZERO_MEMORY, num * size), num * size, filename, lineno);
}

void _free_dbg(void* pointer, const char* filename, int lineno) {
	if (pointer != NULL) {
		clear_memory_allocation_data(pointer, filename, lineno);
		::HeapFree(processHeap, 0, pointer);
	}
}

//}
#endif//_DEBUG

#ifdef malloc
#undef malloc
#endif//malloc

void* malloc(size_t length) {
	return ::HeapAlloc(processHeap, 0, length);
}

#ifdef realloc
#undef realloc
#endif//realloc

void* realloc(void* pointer, size_t length) {
	if (pointer != NULL) {
		return ::HeapReAlloc(processHeap, 0, pointer, length);
	}else{
		return ::HeapAlloc(processHeap, 0, length);
	}
}

#ifdef calloc
#undef calloc
#endif//calloc

void* calloc(size_t num, size_t size) {
	return ::HeapAlloc(processHeap, 0, num * size);
}

#ifdef free
#undef free
#endif//free

void free(void* pointer) {
	::HeapFree(processHeap, 0, pointer);
}

// ●CランタイムやC++でのグローバル変数の初期化および終了処理
// 
// ".CRT$XIC"というセグメントにCランタイムライブラリの初期化ルーチンへの
// ポインタが格納される。このセグメントは".CRT$XIA"と".CRT$XIZ"に挟まれる形で
// 配置されるので".CRT$XIA"に配置した__xi_aと".CRT$XIZ"に配置した__xi_zの間にある
// NULLでないポインタを実行することによりCランタイムライブラリの初期化が行われる。
//
// C++のグローバル変数の初期化ルーチンは".CRT$XCA"と".CRT$XCZ"の間に配置される
// セグメント(".CRT$XCC"?)に格納される。こコに格納されたルーチンも".CRT$XI?"と
// 同様に実行することにより初期化が行われる。
//
// 終了処理が必要な場合、例えばC++でデストラクタを持つオブジェクトがグローバル変数
// だった場合などは、その終了処理を行うルーチンをatexitを用いて登録する。
// これらの登録したルーチンを終了時に実行することにより終了処理が行われる。
//
// これらのセグメントは初期化処理が全く必要ない場合、1KBほど占有してしまう。
// このためこのライブラリでは_YCL_NO_CRTSTARTUP_を定義しておけば初期化ルーチン
// を全く参照しないようにした。ただし、必要な初期化が行われなくなる可能性が
// あるので、_DEBUG版では使用せずに_RELEASE版で使用するなど、充分に注意して
// 使用すること。

typedef void (*vfuncv)();

static int _callfunclist(vfuncv* begin, vfuncv* end) {
#ifdef _DEBUG
	int count = 0;
#endif
	while (begin < end){
		if (*begin != NULL) {
			(**begin)();
#ifdef _DEBUG
			count++;
#endif
		}
		begin++;
	}
#ifdef _DEBUG
	return count;
#else
	return 0;
#endif
}

#if defined _DEBUG || !defined _YCL_NO_CRTSTARTUP_
//{

// VCではこれらのセグメントに初期化処理を行う関数へのポインタを格納する
#pragma data_seg(".CRT$XIA")
static vfuncv __xi_a[] = {NULL};
#pragma data_seg(".CRT$XIZ")
static vfuncv __xi_z[] = {NULL};
#pragma data_seg(".CRT$XCA")
static vfuncv __xc_a[] = {NULL};
#pragma data_seg(".CRT$XCZ")
static vfuncv __xc_z[] = {NULL};
#pragma data_seg()

//}
#endif//defined _DEBUG || !defined _YCL_NO_CRTSTARTUP_

static vfuncv* _atexit_a = NULL;
static vfuncv* _atexit_c = NULL;
static vfuncv* _atexit_z = NULL;

#ifdef _MT
static CRITICAL_SECTION atexit_section;
#endif//_MT

#define _ATEXIT_BOUNDARY 16

int atexit(vfuncv f) {
	if (f == NULL)
		return 0;

	int result = -1;
#ifdef _MT
	::EnterCriticalSection(&atexit_section);
#endif//_MT
	if (_atexit_a == _atexit_c) {
		vfuncv* newfuncs = (vfuncv*) malloc((_atexit_z - _atexit_a + _ATEXIT_BOUNDARY) * sizeof (vfuncv));
		if (newfuncs != NULL) {
			if (_atexit_a != NULL) {
				memcpy(newfuncs + _ATEXIT_BOUNDARY, _atexit_a, (_atexit_z - _atexit_a) * sizeof (vfuncv));
				free(_atexit_a);
			}
			_atexit_z = newfuncs + (_atexit_z - _atexit_a) + _ATEXIT_BOUNDARY;
			_atexit_c = newfuncs + _ATEXIT_BOUNDARY;
			_atexit_a = newfuncs;
		}
	}
	if (_atexit_a < _atexit_c) {
		*--_atexit_c = f;
		result = 0;
	}
#ifdef _MT
	::LeaveCriticalSection(&atexit_section);
#endif//_MT
	return result;
}

#ifdef __cplusplus
} // extern "C"
#endif//__cplusplus

#ifdef __cplusplus
//{
#ifdef _DEBUG
//{

#ifdef new
#undef new
#endif//new

void* operator new(size_t length, const char* filename, int lineno) {
	return _malloc_dbg(length, filename, lineno);
}

#ifdef delete
#undef delete
#endif//delete

void operator delete(void* pointer, const char* filename, int lineno) {
	_free_dbg(pointer, filename, lineno);
}

//}
#endif//_DEBUG

void* operator new(size_t length) {
	return malloc(length);
}

void operator delete(void* pointer) {
#ifdef _DEBUG
	_free_dbg(pointer, "unknwon", -1);
#else
	free(pointer);
#endif//_DEBUG
}
//}
#endif//__cplusplus

static inline void ycl_startup(HINSTANCE module) {
	processHeap = ::GetProcessHeap();
	module_instance = module;

#ifdef _MT
	::InitializeCriticalSection(&atexit_section);
#ifdef _DEBUG
	::InitializeCriticalSection(&malloc_section);
#endif // _DEBUG
#endif // _MT

#if defined _DEBUG || !defined _YCL_NO_CRTSTARTUP_
	int countI = _callfunclist(__xi_a, __xi_z);
	int countC = _callfunclist(__xc_a, __xc_z);
#ifdef _YCL_NO_CRTSTARTUP_
	YCLASSERT(countI + countC == 0, "C++ runtime startup is needed!\n_YCL_NO_CRTSTARTUP_ must not be defined.");
#else
	YCLASSERT(countI + countC > 0, "C++ runtime startup is not needed!\nPerhaps _YCL_NO_CRTSTARTUP_ should be defined.");
#endif//_YCL_NO_CRTSTARTUP_
#endif//defined _DEBUG || !defined _YCL_NO_CRTSTARTUP_
}

static inline void ycl_exit() {
#ifdef _MT
	::EnterCriticalSection(&atexit_section);
#endif//_MT
	_callfunclist(_atexit_c, _atexit_z);
#ifdef _MT
	::LeaveCriticalSection(&atexit_section);
	::DeleteCriticalSection(&atexit_section);
#endif // _MT
	free(_atexit_a);

#ifdef _DEBUG
#ifdef _MT
	::EnterCriticalSection(&malloc_section);
#endif//_MT
	for (int i = 0; i < memory_allocation_data_length; i++) {
		if (memory_allocation_data[i].pointer != NULL) {
			char buffer[1024];
			wsprintf(buffer, "%s(%d): memory leak detected!\n  %08x |", memory_allocation_data[i].filename, memory_allocation_data[i].lineno, memory_allocation_data[i].pointer);
			::OutputDebugString(buffer);
			int count = memory_allocation_data[i].length;
			if (count > 16)
				count = 16;
			const char* data = (const char*) memory_allocation_data[i].pointer;
			while (count-- > 0) {
				wsprintf(buffer, " %02x", (unsigned char) *data++);
				::OutputDebugString(buffer);
			}
			::OutputDebugString("\n");
		}
	}
#ifdef _MT
	::DeleteCriticalSection(&malloc_section);
#endif // _MT
	::HeapFree(processHeap, 0, memory_allocation_data);
#endif // _DEBUG

}

#ifdef __cplusplus
namespace yebisuya {
#endif//__cplusplus

HINSTANCE GetInstanceHandle() {
	return module_instance;
}

#ifdef __cplusplus
}
#endif//__cplusplus

#ifdef _YCL_DATULAPLUGIN_H_
//{
#pragma comment(linker, "/entry:DllEntryPoint")
#pragma comment(linker, "/dll")


///////////////////////////////////////////////////////////////////////
// Datulaプラグインとしてエクスポートする関数群
///////////////////////////////////////////////////////////////////////

// DatulaPlugin_Init
//	引数:	なし
//	返値:	初期化成功時には0以外/失敗時には0
//	備考:	プラグインがロードされたときに呼び出されます
BOOL WINAPI DatulaPlugin_Init() {
	ycl_startup(module_instance);
    if (!YCL_DatulaPlugin_Init()) {
	    YCL_DatulaPlugin_Exit();
	    ycl_exit();
    }
    return true;
}

// DatulaPlugin_Exit
//	引数:	なし
//	返値:	終了処理成功時には0以外/失敗時には0
//	備考:	プラグインがアンロードされたときに呼び出されます
BOOL WINAPI DatulaPlugin_Exit() {
	if (!YCL_DatulaPlugin_Exit())
		return false;
	ycl_exit();
	return true;
}

// DatulaPlugin_OnUpdateView
//	引数:	なし
//	返値:	なし
//	備考:	ドキュメントが更新されたときに呼び出されます
void WINAPI DatulaPlugin_OnUpdateView() {
	YCL_DatulaPlugin_OnUpdateView();
}

// DatulaPlugin_OnActivate
//	引数:	HWND hWnd		アクティブになったウィンドウのハンドル
//			UINT nState		WA_INACTIVE
//							WA_ACTIVE
//							WA_CLICKACTIVE
//			BOOL bMinimized	最小化されたかどうか
//	返値:	なし
//	備考:	Datulaのメインフレーム、もしくはポストマンがアクティブに
//			なったときに呼び出されます
void WINAPI DatulaPlugin_OnActivate(HWND hWnd, UINT nState, BOOL bMinimized) {
	YCL_DatulaPlugin_OnActivate(hWnd, nState, bMinimized != FALSE);
}

// DatulaPlugin_OnNewPostmanInstance
//	引数:	HWND hWnd	ポストマンのウィンドウハンドル
//	返値:	なし
//	備考:	ポストマンのウィンドウが作成されたときに呼び出されます
void WINAPI DatulaPlugin_OnNewPostmanInstance(HWND hWnd) {
	YCL_DatulaPlugin_OnNewPostmanInstance(hWnd);
}

// DatulaPlugin_OnDeletePostmanInstance
//	引数:	HWND hWnd	ポストマンのウィンドウハンドル
//	返値:	なし
//	備考:	ポストマンのウィンドウが破棄されたときに呼び出されます
void WINAPI DatulaPlugin_OnDeletePostmanInstance(HWND hWnd) {
	YCL_DatulaPlugin_OnDeletePostmanInstance(hWnd);
}

// DatulaPlugin_GetPluginVersion
//	引数:	なし
//	返値:	プラグインのバージョンを返す
//	備考:	いつでも呼び出されます
DWORD WINAPI DatulaPlugin_GetPluginVersion() {
	return YCL_DatulaPlugin_GetPluginVersion();
}

BOOL WINAPI DllEntryPoint(HINSTANCE instance, DWORD reason, LPVOID reserved) {
	if (reason == DLL_PROCESS_ATTACH) {
		module_instance = instance;
		::DisableThreadLibraryCalls(instance);
	}
	return TRUE;
}

//}
#endif// _YCL_DATULAPLUGIN_H_

#ifdef _YCL_DYNAMICLINKLIBRARY_H_
//{

#pragma comment(linker, "/entry:DllEntryPoint")
#pragma comment(linker, "/dll")

BOOL WINAPI DllEntryPoint(HINSTANCE instance, DWORD reason, LPVOID reserved) {
	if (reason == DLL_PROCESS_ATTACH)
		ycl_startup(instance);
	// VS2005(VC++8.0)では error C3767 となるため、呼ばないようにする（暫定処置）。
	// (2006.8.2 yutaka)
#if 0
	BOOL result = YCL_DllMain(reason);
#else
	BOOL result = true;
#endif
	if (reason == DLL_PROCESS_DETACH)
		ycl_exit();
	return result;
}

//}
#endif//_YCL_DYNAMICLINKLIBRARY_H_

#ifdef _YCL_APPLICATION_H_

#pragma comment(linker, "/entry:WinStartup")
void WINAPI WinStartup() {
	ycl_startup(::GetModuleHandle(NULL));
	int exitCode = YCL_WinMain();
	ycl_exit();
	::ExitProcess(exitCode);
}

#endif//_YCL_APPLICATION_H_

#if !defined(_YCL_APPLICATION_H_) && !defined(_YCL_DYNAMICLINKLIBRARY_H_) && !defined(_YCL_DATULAPLUGIN_H_)
//{
#pragma comment(linker, "/entry:WinStartup")
void WINAPI WinStartup() {
	ycl_startup(::GetModuleHandle(NULL));
#ifdef _YCL_CONSOLE
//{

#pragma comment(linker, "/subsystem:console")
	// int exitCode = main(0, NULL, NULL);
	// mainがどんな形式で定義されるか分からないので
	// インラインアセンブラで記述
	// ただし、どんな形式でも引数はダミー
	int exitCode;
	__asm {
		push 0
		push 0
		push 0
		call main
		add  esp, 12
		mov  dword ptr[exitCode], eax
	}

//}
#else//_YCL_CONSOLE
//{

#pragma comment(linker, "/subsystem:windows")
	// WinMainの形式ながら引数の値はすべてダミー
	int exitCode = WinMain(module_instance, NULL, NULL, 0);

//}
#endif//_YCL_CONSOLE
	ycl_exit();
	::ExitProcess(exitCode);
}

//}
#endif//!defined(_YCL_APPLICATION_H_) && !defined(_YCL_DYNAMICLINKLIBRARY_H_) && !defined(_YCL_DATULAPLUGIN_H_)

// stricmpはCランタイムの初期化を必要とするためlstricmpを使用するように変更
extern "C" int stricmp(const char* s1, const char* s2) {
	return lstrcmpi(s1, s2);
}

#pragma comment(linker, "/opt:nowin98")
#pragma comment(linker, "/merge:.rdata=.text")
#ifndef _YCL_NO_CRTSTARTUP_
#pragma comment(linker, "/merge:.CRT=.text")
#endif//_YCL_NO_CRTSTARTUP_
#pragma comment(linker, "/ignore:4078")
#pragma comment(lib, "kernel32.lib")

/*
 * Changes:
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2006/08/01 15:01:48  yutakakn
 * no message
 *
 * Revision 1.6  2003/11/28 11:14:10  sugoroku
 * CVSキーワードを追加
 *
 */
