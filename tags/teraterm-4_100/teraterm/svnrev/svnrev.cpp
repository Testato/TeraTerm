/*
 * Copyright (C) 2009-2018 TeraTerm Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <windows.h>
#include <stdio.h>
#include "svnrev.h"

int get_svn_revision(char *svnversion, char *path) {
	FILE *fp;
	char command[MAX_PATH*2];
	char result[32]= "";
	int revision = -1;
	char arg1[MAX_PATH], arg2[MAX_PATH];

	// subversion 1.7 ���� .svn\entries �̃t�H�[�}�b�g���ς�������߁A
	// .svn\entries �𒼐ړǂݍ��ނ̂���߁A
	// svnversion.exe �R�}���h���Ăяo�������ʂ�Ԃ�

	// _popen �̓X�y�[�X���܂܂��ꍇ�Ƀ_�u���N�H�[�g�ň͂�ł�
	// ���܂������Ȃ����� 8.3 �`���ɕϊ�
	DWORD r = GetShortPathName(svnversion, arg1, sizeof(arg1));
	if (r == 0) {
		return -1;	// svn did not exist
	}
	GetShortPathName(path, arg2, sizeof(arg2));

	_snprintf_s(command, sizeof(command), _TRUNCATE, "%s -n %s", arg1, arg2);
	if ((fp = _popen(command, "rt")) == NULL ) {
		return -1;
	}

	fread(result, sizeof(result), sizeof(result)-1, fp);
	revision = atoi(result);
	_pclose(fp);

	if (revision == 0) {
		revision = -1;
	}

	return revision;
}

BOOL write_svn_revesion(char *filename, int revision) {
	FILE *fp;
	char buf[64] = "";
	int file_revision = -1;

	// print to stdout
	if (strcmp(filename, "-") == 0) {
		fprintf(stdout, "#define SVNVERSION %d\n", revision);
		return TRUE;
	}

	// read current file
	if (fopen_s(&fp, filename, "r") == 0) {
		fread(buf, sizeof(char), sizeof(buf)-1, fp);
		fclose(fp);

		sscanf_s(buf, "#define SVNVERSION %d", &file_revision);
	}

	// compare revisions
	// .svn �z���̃t�@�C�������݂��Ȃ�(-1)�ꍇ�ɂ����Ă��A�r���h���ł���悤�ɁA
	// �w�b�_�t�@�C���͍쐬����B��Ƃ̃C���g���l�b�g���� SourceForge ��SVN���|�W�g����
	// ���ڃA�N�Z�X�ł��Ȃ��ꍇ�Atarball���_�E�����[�h���邵���Ȃ��A���̍� .svn �f�B���N�g����
	// ���݂��Ȃ��B
	if (file_revision != -1 &&
		(file_revision >= revision)) {
		return TRUE;
	}

	if (fopen_s(&fp, filename, "w+") != 0) {
		return FALSE;
	}

	if (revision >= 1) {
		fprintf(fp, "#define SVNVERSION %d\n", revision);
	}
	else {
		fprintf(fp, "#undef SVNVERSION\n");
	}

	fclose(fp);

	return TRUE;
}

int main(int argc, char* argv[])
{
	int revision = -1;
	char *svnversion, *input, *output;

	if (argc != 4) {
		printf("USAGE: %s svnversion path output\n", argv[0]);
		return -1;
	}

	svnversion = argv[1]; // svnversion.exe
	input = argv[2];      // top of source tree
	output = argv[3];     // output to
	revision = get_svn_revision(svnversion, input);

	if (!write_svn_revesion(output, revision)) {
		return 1;
	}

	return 0;
}
