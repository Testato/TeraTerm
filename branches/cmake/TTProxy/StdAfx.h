// stdafx.h : �W���̃V�X�e�� �C���N���[�h �t�@�C���A
//            �܂��͎Q�Ɖ񐔂������A�����܂�ύX����Ȃ�
//            �v���W�F�N�g��p�̃C���N���[�h �t�@�C�����L�q���܂��B
//

#if !defined(AFX_STDAFX_H__B24257E3_70C7_482E_8DB9_1A5C4AE8B2F6__INCLUDED_)
#define AFX_STDAFX_H__B24257E3_70C7_482E_8DB9_1A5C4AE8B2F6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#define _WINSOCKAPI_

/* VS2015(VC14.0)���ƁAWSASocketA(), inet_ntoa() �Ȃǂ�API��deprecated�ł����
* �x�����邽�߂ɁA�x����}�~����B��֊֐��ɒu������ƁAVS2005(VC8.0)�Ńr���h
* �ł��Ȃ��Ȃ邽�߁A�x����}�~���邾���Ƃ���B
*/
#if _MSC_VER >= 1800  // VSC2013(VC12.0) or later
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif
#endif

// ���̈ʒu�Ƀw�b�_�[��}�����Ă�������
#include <winsock2.h>
#include <windows.h>

#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509v3.h>

#include "teraterm.h"
#include "tttypes.h"
#include "ttplugin.h"

// TODO: �v���O�����ŕK�v�ȃw�b�_�[�Q�Ƃ�ǉ����Ă��������B

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_STDAFX_H__B24257E3_70C7_482E_8DB9_1A5C4AE8B2F6__INCLUDED_)
