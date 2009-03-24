/*
 * $Id: Object.h,v 1.3 2006-08-03 13:33:18 yutakakn Exp $
 */

#ifndef _YCL_OBJECT_H_
#define _YCL_OBJECT_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

namespace yebisuya {

// �Q�ƃJ�E���^�t���̃I�u�W�F�N�g�̊�{�N���X
class Object {
private:
	// �Q�ƃJ�E���^
	unsigned int refCount;
protected:
	// �f�t�H���g�R���X�g���N�^
	Object():refCount(0) {
	}
	// �f�X�g���N�^
	// protected�Ȃ̂Ŕh���N���X�Ńf�X�g���N�^��public�ɂ��Ȃ�����
	// �����ϐ��Ƃ��Ă͐����ł��Ȃ�
	virtual ~Object() {
	}
	// �����I�ȎQ�Ƃ̐��Ǝ��ۂ̎Q�ƃJ�E���^�����������ǂ����𔻒肷��
	bool existsOuterReference(unsigned int innerRefCount)const {
		return refCount != innerRefCount;
	}
public:
	// �Q�ƃJ�E���^�𑝂₷
	virtual int refer() {
		return ++refCount;
	}
	// �Q�ƃJ�E���^�����炵�A0�ɂȂ�����delete����
	virtual int release() {
		if (--refCount != 0) {
			return refCount;
		}else{
			delete this;
			return 0;
		}
	}
	static Object* set(Object* value, Object* ptr) {
		if (value != ptr) {
			if (value != NULL)
				value->release();
			value = ptr;
			if (value != NULL)
				value->refer();
		}
		return value;
	}
};

}

#endif//_YCL_OBJECT_H_

/*
 * $Changes
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2003/06/12 08:34:01  sugoroku
 * �����|�C���^���Z�b�g���ꂽ�Ƃ��ɉ�����Ă��܂�Ȃ��悤�ɏC��
 *
 * Revision 1.1.1.1  2003/05/26 15:32:04  sugoroku
 * �V�K�쐬
 *
 */
