// �h���t�g�łȂ��Ȃ����ꍇ�� ifdef ���폜���邽�߂Ɏg�p����B
// �R���p�C���ɂ����� on/off ���邽�߂ɗ��p����ƁATERATERM.INI��
// �ۑ�����Ƃ��̋L��������Ă��܂��̂ł���Ă͂����Ȃ��B

// Camellia support draft
// http://tools.ietf.org/html/draft-kanno-secsh-camellia-02
// https://bugzilla.mindrot.org/show_bug.cgi?id=1340
#undef WITH_CAMELLIA_DRAFT
#undef WITH_CAMELLIA_PRIVATE

#if defined(WITH_CAMELLIA_PRIVATE) && !defined(WITH_CAMELLIA_DRAFT)
#define WITH_CAMELLIA_DRAFT
#endif
