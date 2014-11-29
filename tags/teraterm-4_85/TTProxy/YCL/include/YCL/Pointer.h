/*
 * $Id: Pointer.h,v 1.4 2007-08-18 08:52:18 maya Exp $
 */

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef _YCL_POINTER_H_
#define _YCL_POINTER_H_

#include <YCL/Array.h>

namespace yebisuya {
    
// �|�C���^�����b�v���邽�߂̃N���X�B
template<class TYPE, class REFCTRL = Object >
class Pointer {
private:
    // �i�[����|�C���^
    TYPE* value;
protected:
    // �|�C���^��u��������
    void set(TYPE* ptr) {
	value = (TYPE*) REFCTRL::set(value, ptr);
    }
public:
    // �f�t�H���g�R���X�g���N�^
    Pointer():value(NULL) {
    }
    // �����l�t���R���X�g���N�^
    Pointer(TYPE* ptr):value(NULL) {
	set(ptr);
    }
    // �R�s�[�R���X�g���N�^
    Pointer(const Pointer& ptr):value(NULL) {
	set(ptr.value);
    }
    // �f�X�g���N�^
    ~Pointer() {
	set(NULL);
    }
    // ������Z�q
    Pointer& operator=(TYPE* ptr) {
	set(ptr);
	return *this;
    }
    // �R�s�[������Z�q
    Pointer& operator=(const Pointer& ptr) {
	set(ptr.value);
	return *this;
    }
    // �Ԑډ��Z�q
    TYPE& operator*()const {
	return *value;
    }
    // �����o�I�����Z�q
    TYPE* operator->()const {
	return value;
    }
    // �L���X�g���Z�q
    operator TYPE*()const {
	return value;
    }
};
    
// Pointer��Array<TYPE>�^�ւ̓��ꉻ
// �z��v�f���Z�q[]��񋟂���
template<class TYPE>
class PointerArray : public Pointer< Array<TYPE> > {
public:
    // �f�t�H���g�R���X�g���N�^
    PointerArray() {
    }
    // �����l�t���R���X�g���N�^
    PointerArray(Array<TYPE>* ptr):Pointer< Array<TYPE> >(ptr) {
    }
    // �R�s�[�R���X�g���N�^
    PointerArray(const PointerArray& ptr):Pointer< Array<TYPE> >(ptr) {
    }
    // ������Z�q
    PointerArray& operator=(Array<TYPE>* ptr) {
	set(ptr);
	return *this;
    }
    // �z��v�f���Z�q
    TYPE& operator[](int index) {
	Array<TYPE>* ptr = *this;
	return (**this)[index];
    }
    TYPE operator[](int index)const {
	Array<TYPE>* ptr = *this;
	return (**this)[index];
    }
};
    
}

#endif//_YCL_POINTER_H_
