/*
 * $Id: Enumeration.h,v 1.3 2006-08-03 13:33:18 yutakakn Exp $
 */

#ifndef _YCL_ENUMERATION_H_
#define _YCL_ENUMERATION_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <YCL/Object.h>

namespace yebisuya {

// �I�u�W�F�N�g��񋓂��邽�߂̃N���X�B
template<class TYPE>
class Enumeration : public Object {
public:
	// �܂��񋓂���I�u�W�F�N�g�����݂��Ă��邩�ǂ����𔻒肷��B
	// �Ԓl:
	//	���̃I�u�W�F�N�g�����݂��Ă���ΐ^�B
	virtual bool hasMoreElements()const = 0;
	// ���Ɍ����I�u�W�F�N�g��Ԃ��B
	// �Ԓl:
	//	���̃I�u�W�F�N�g�B
	virtual TYPE nextElement()const = 0;
};

}

#endif//_YCL_ENUMERATION_H_

/*
 * $Changes
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2003/05/26 15:32:00  sugoroku
 * Initial revision
 *
 */
