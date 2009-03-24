/*
 * $Id: Array.h,v 1.4 2007-08-18 08:52:18 maya Exp $
 */

#ifndef _YCL_ARRAY_H_
#define _YCL_ARRAY_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <YCL/common.h>

#include <YCL/Object.h>

namespace yebisuya {

// �z����Ǘ�����N���X
template<class TYPE>
class Array : public Object {
private:
	// �z��̎���
	TYPE*const array;
	// ������Z�q�͎������Ȃ�
	void operator=(Array&);
public:
	// �z��̒���
	const int length;
	// �R���X�g���N�^
	Array(int length):array(new TYPE[length]), length(length) {
	}
	Array(const TYPE source[], int length):array(new TYPE[length]), length(length) {
		copyFrom(source);
	}
	Array(Array* source):array(new TYPE[source->length]), length(source->length) {
		copyFrom(source);
	}
protected:
	virtual ~Array() {
		delete[] array;
	}
public:
	// �z��v�f���Z�q(����p)
	TYPE& operator[](int index) {
		return array[index];
	}
	// �z��v�f���Z�q(�Q�Ɨp)
	TYPE operator[](int index)const {
		return array[index];
	}
	void copyFrom(const TYPE source[]) {
		for (int i = 0; i < length; i++) {
			array[i] = source[i];
		}
	}
	void copyFrom(Array* source) {
		copy(source->array[i]);
	}
	void copyTo(TYPE distination[])const {
		for (int i = 0; i < length; i++) {
			distination[i] = array[i];
		}
	}
	void copyTo(Array* distination)const {
		copyTo(distination->array[i]);
	}
};

}

#endif//_YCL_ARRAY_H_
