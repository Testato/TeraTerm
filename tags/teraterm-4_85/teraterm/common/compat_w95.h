// VS2005�Ńr���h���ꂽ�o�C�i���� Windows95 �ł��N���ł���悤�ɂ��邽�߂ɁA
// IsDebuggerPresent()�̃V���{����`��ǉ�����B
//
// cf.http://jet2.u-abel.net/program/tips/forceimp.htm


#if _MSC_VER == 1400

// �������ꂽ���O�̃A�h���X����邽�߂̉���`
// (���ꂾ���ŃC���|�[�g������肵�Ă���)
#ifdef __cplusplus
extern "C" {
#endif
int WINAPI _imp__IsDebuggerPresent()
    { return PtrToInt((void*) &_imp__IsDebuggerPresent); }
#ifdef __cplusplus
}
#endif

// ���ۂɉ���菈�����s���֐�
#ifdef __cplusplus
extern "C" {
#endif
BOOL WINAPI Cover_IsDebuggerPresent()
    { return FALSE; }
#ifdef __cplusplus
}
#endif

// �֐������ۂɌĂяo���ꂽ�Ƃ��ɔ�����
// ����菈���֐����Ăяo�����邽�߂̉�����
#ifdef __cplusplus
extern "C" {
#endif
void __stdcall DoCover_IsDebuggerPresent()
{
    DWORD dw;
    DWORD_PTR FAR* lpdw;
    OSVERSIONINFO osvi;
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    // Windows95 �łȂ���΂����ł����
    GetVersionEx(&osvi);
    if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT ||
        // VER_PLATFORM_WIN32_WINDOWS �Ȃ�A���Ƃ� Minor �����Ŕ���ł���
        // osvi.dwMajorVersion > 4 ||
        osvi.dwMinorVersion > 0) {
        return;
    }
    // �����֐���ݒ肷��A�h���X���擾
    lpdw = (DWORD_PTR FAR*) &_imp__IsDebuggerPresent;
    // ���̃A�h���X���������߂�悤�ɐݒ�
    // (�����v���O�������Ȃ̂ŏ�Q�Ȃ��s����)
    VirtualProtect(lpdw, sizeof(DWORD_PTR), PAGE_READWRITE, &dw);
    // �����֐���ݒ�
    *lpdw = (DWORD_PTR)(FARPROC) Cover_IsDebuggerPresent;
    // �ǂݏ����̏�Ԃ����ɖ߂�
    VirtualProtect(lpdw, sizeof(DWORD_PTR), dw, NULL);
}
#ifdef __cplusplus
}
#endif

// �A�v���P�[�V�����������������O�ɉ��������Ăяo��
// �� ���Ȃ葁���ɏ������������Ƃ��́A���̃R�[�h��
//  �t�@�C���̖����ɏ����āu#pragma init_seg(lib)�v���A
//  ���̕ϐ��錾�̎�O�ɏ����܂��B
//  ���������}���K�v�������ꍇ�� WinMain ������
//  DoCover_IsDebuggerPresent ���Ăяo���č\���܂���B
/* C����ł͈ȉ��̃R�[�h�́A�R���p�C���G���[�ƂȂ�̂ŁAWinMain, DllMain ����ĂԁB*/
#ifdef __cplusplus
extern "C" {
int s_DoCover_IsDebuggerPresent
    = (int) (DoCover_IsDebuggerPresent(), 0);
}
#endif

#else /* _MSC_VER */

#ifdef __cplusplus
extern "C" {
#endif
void __stdcall DoCover_IsDebuggerPresent()
{
	// NOP
}
#ifdef __cplusplus
}
#endif

#endif /* _MSC_VER */
