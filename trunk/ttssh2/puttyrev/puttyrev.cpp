// puttyrev.cpp : �R���\�[�� �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

#include "stdafx.h"
#include "puttyrev.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// �B��̃A�v���P�[�V���� �I�u�W�F�N�g�ł��B

CWinApp theApp;

using namespace std;

void write_putty_version(char *path)
{
	BOOL ret;
	CStdioFile csf;
	char *keywords[] = {
		//"AppVerName",
		"AppVersion",
		"VersionInfoTextVersion",
		NULL,
	};
	int i;
	CString filename, buf, cs;
	char revision[64] = {0};
	char header_line[64], *p;

	// PuTTY�̃o�[�W�������擾����B
	filename = path;
	filename += "\\libs\\putty\\windows\\putty.iss";

	ret = csf.Open(filename, CFile::modeRead);
	if (ret == FALSE) {
		goto write;
	}

	while (csf.ReadString(cs) != NULL) {
		CString tmp;
		for (i = 0 ; keywords[i] ; i++) {
			tmp = keywords[i];
			tmp += "=%[^\n]s";
			ret = sscanf_s(cs, tmp, revision, sizeof(revision));
			if (ret != 1) 
				continue;
			printf("%s\n", revision);
			goto close;
		}
	}

close:
	csf.Close();

	filename = path;
	filename += "\\ttssh2\\ttxssh\\puttyversion.h";

	// �o�[�W�������`�F�b�N���A�ύX���Ȃ���Δ�����
	ret = csf.Open(filename, CFile::modeRead);
	if (ret == FALSE) {
		goto write;
	}

	if (csf.ReadString(header_line, sizeof(header_line)) == NULL) {
		csf.Close();
		goto write;
	}

	if ( (p = strchr(header_line, '"')) == NULL ) {
		csf.Close();
		goto write;
	}

	p++;
	if (strncmp(p, revision, strlen(p)-2) == 0) {
		csf.Close();
		goto end;
	}

	csf.Close();

write:
	// �o�[�W�������w�b�_�ɏ������ށB
	ret = csf.Open(filename, CFile::modeWrite | CFile::modeCreate);
	if (ret == FALSE) {
		goto end;
	}

	if (revision[0] != '\0') {
		cs.Format("#define PUTTYVERSION \"%s\"\n", revision);
		csf.WriteString(cs);
	}
	else {
		cs.Format("#undef PUTTYVERSION\n");
		csf.WriteString(cs);
	}

	csf.Close();

end:;
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;
	char path[MAX_PATH * 2];
	int i, len;

	// MFC �����������āA�G���[�̏ꍇ�͌��ʂ�������܂��B
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: �K�v�ɉ����ăG���[ �R�[�h��ύX���Ă��������B
		_tprintf(_T("�v���I�ȃG���[: MFC �̏��������ł��܂���ł����B\n"));
		nRetCode = 1;
	}
	else
	{
		// TODO: �A�v���P�[�V�����̓�����L�q����R�[�h�������ɑ}�����Ă��������B
		GetModuleFileName(::GetModuleHandle(NULL), path, sizeof(path));
		len = (int)strlen(path);
		for (i=len; i>=0; i--) {
			if (path[i] == '\\') {
				break;
			}
			path[i] = '\0';
		}
		SetCurrentDirectory(path); // teraterm\debug or teraterm\release
		SetCurrentDirectory("..\\..\\..\\"); // top of source tree
		GetCurrentDirectory(sizeof(path), path);

		write_putty_version(path);
	}

	return nRetCode;
}
