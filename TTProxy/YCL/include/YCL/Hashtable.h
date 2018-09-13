/*
 * $Id: Hashtable.h,v 1.4 2007-08-18 08:52:18 maya Exp $
 */

#ifndef _YCL_HASHTABLE_H_
#define _YCL_HASHTABLE_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <YCL/common.h>

#include <YCL/HASHCODE.h>
#include <YCL/ValueCtrl.h>
#include <YCL/Enumeration.h>
#include <YCL/Pointer.h>

namespace yebisuya {

// �n�b�V���e�[�u�����Ǘ�����N���X�B
template<class TYPE_KEY, class TYPE_VALUE, class KEYCTRL = ValueCtrl<TYPE_KEY>, class VALCTRL = ValueCtrl<TYPE_VALUE> >
class Hashtable {
	// TYPE_KEY, TYPE_VALUE�ɂƂ��^:
	// �E�f�t�H���g�R���X�g���N�^������
	// �E�R�s�[�R���X�g���N�^������
	// �E==���Z�q������
	// �E=���Z�q������(���̏����̂��ߎQ�Ƃ͎w��ł��Ȃ�)
	// KEYCTRL�AVALCTRL�Ƀf�t�H���g�̂��̂��g�p����ꍇ��
	// �E�f�t�H���g�R���X�g���N�^�Ő������ꂽ�C���X�^���X��NULL���r����Ɛ^�ɂȂ�
	// �ENULL�C���X�^���X��NULL���r����Ɛ^�ɂȂ�
	// �ETYPE_KEY�̓n�b�V���l�𐶐�����hashCode���\�b�h������
	// �������ɉ����B
public:
	typedef Enumeration<TYPE_KEY> KeyEnumeration;
	typedef Enumeration<TYPE_VALUE> ElementEnumeration;
private:
	// �R�s�[�R���X�g���N�^�͎g�p�֎~
	Hashtable(Hashtable&);
	// ������Z�q�͎g�p�֎~
	void operator=(Hashtable&);

	// �n�b�V���e�[�u���̃G���g���B
	struct Entry {
		TYPE_KEY key;
		TYPE_VALUE value;
		Entry() {
			KEYCTRL::initialize(key);
			VALCTRL::initialize(value);
		}
	};
	// �n�b�V���e�[�u���̃G���g����񋓂��邽�߂̃N���X�B
	class EnumEntries {
	private:
		const Hashtable& table;
		mutable int index;
		void skip()const {
			while (index < table.backetSize && VALCTRL::isEmpty(table.backet[index].value))
				index++;
		}
	public:
		EnumEntries(const Hashtable& table)
		:table(table), index(0) {
			skip();
		}
		bool hasMoreEntries()const {
			return index < table.backetSize;
		}
		const Entry& nextEntry()const {
			const Entry& entry = table.backet[index++];
			skip();
			return entry;
		}
	};
	friend class EnumEntries;
	// �n�b�V���e�[�u���̃L�[��񋓂��邽�߂̃N���X�B
	class EnumKeys : public KeyEnumeration {
	private:
		EnumEntries entries;
	public:
		EnumKeys(const Hashtable& table)
		:entries(table) {
		}
		virtual bool hasMoreElements()const {
			return entries.hasMoreEntries();
		}
		virtual TYPE_KEY nextElement()const {
			return entries.nextEntry().key;
		}
	};
	// �n�b�V���e�[�u���̒l��񋓂��邽�߂̃N���X�B
	class EnumValues : public ElementEnumeration {
	private:
		EnumEntries entries;
	public:
		EnumValues(const Hashtable& table)
		:entries(table) {
		}
		virtual bool hasMoreElements()const {
			return entries.hasMoreEntries();
		}
		virtual TYPE_VALUE nextElement()const {
			return entries.nextEntry().value;
		}
	};

	// �G���g���̔z��B
	Entry* backet;
	// �G���g���̔z��̃T�C�Y
	int backetSize;
	// �L���ȃG���g���̐�
	int count;
	enum {
		// �G���g���̔z���傫������Ƃ��Ɏg�p����T�C�Y
		BOUNDARY = 8,
	};
	// key�Ɠ������L�[�����G���g���̃C���f�b�N�X��Ԃ��B
	// ����:
	//	key	��������L�[
	// �Ԓl:
	//	key�Ɠ������L�[�������A�܂��ݒ肳��Ă��Ȃ��G���g���̃C���f�b�N�X�B
	//	�S�ẴG���g�����ݒ�ς݂�key�Ɠ��������̂��Ȃ����-1��Ԃ��B
	int find(const TYPE_KEY& key)const {
		int found = -1;
		int h = HASHCODE(&key);
		for (int i = 0; i < backetSize; i++) {
			int index = ((unsigned) h + i) % backetSize;
			const TYPE_KEY& bkey = backet[index].key;
			if (KEYCTRL::isEmpty(bkey) || bkey == key) {
				found = index;
				break;
			}
		}
		return found;
	}
	// �G���g���̔z����w��̃T�C�Y�ɕς��A������x�e�[�u������蒼���B
	// ����:
	//	newSize	�V�����z��̃T�C�Y�B
	//			�L���ȃG���g���̐��������Ȃ����Ă͂����Ȃ��B
	void rehash(int newSize) {
		Entry* oldBacket = backet;
		int oldSize = backetSize;
		backet = new Entry[newSize];
		backetSize = newSize;
		for (int i = 0; i < oldSize; i++) {
			if (!VALCTRL::isEmpty(oldBacket[i].value)) {
				backet[find(oldBacket[i].key)] = oldBacket[i];
			}
		}
		delete[] oldBacket;
	}
public:
	// �f�t�H���g�R���X�g���N�^�B
	// ��̃n�b�V���e�[�u�������B
	Hashtable()
	:backet(NULL), backetSize(0), count(0) {
	}
	// �G���g���̔z��̏����T�C�Y���w�肷��R���X�g���N�^�B
	// ����:
	//	backetSize	�G���g���̔z��̑傫���B
	Hashtable(int backetSize)
	:backet(new Entry[backetSize]), backetSize(backetSize), count(0) {
	}
	// �f�X�g���N�^�B
	// �G���g���̔z����폜����B
	~Hashtable() {
		delete[] backet;
	}
	// �w��̃L�[�ɑΉ�����l���擾����B
	// ����:
	//	key	��������L�[
	// �Ԓl:
	//	�L�[�ɑΉ�����l�B������Ȃ����NULL�Ɠ������l��Ԃ��B
	TYPE_VALUE get(const TYPE_KEY& key)const {
		TYPE_VALUE value;
		VALCTRL::initialize(value);
		int index = find(key);
		if (index != -1)
			value = backet[index].value;
		return value;
	}
	// �w��̃L�[�ɑΉ�����l��ݒ肷��B
	// ����:
	//	key	�ݒ肷��L�[
	//	value �ݒ肷��l
	// �Ԓl:
	//	�O�ɃL�[�ɐݒ肳��Ă����l�B���ݒ肾�����ꍇ��NULL�Ɠ������l��Ԃ��B
	TYPE_VALUE put(const TYPE_KEY& key, const TYPE_VALUE& value) {
		int index = find(key);
		if (index == -1) {
			rehash(backetSize + BOUNDARY);
			index = find(key);
		}
		TYPE_VALUE old = backet[index].value;
		if (VALCTRL::isEmpty(old)) {
			count++;
			backet[index].key = key;
		}
		backet[index].value = value;
		return old;
	}
	// �w��̃L�[�ɑΉ�����l���폜����B
	// ����:
	//	key	�폜����L�[
	// �Ԓl:
	//	�L�[�ɐݒ肳��Ă����l�B���ݒ肾�����ꍇ��NULL�Ɠ������l��Ԃ��B
	TYPE_VALUE remove(const TYPE_KEY& key) {
		TYPE_VALUE old;
		VALCTRL::initialize(old);
		int index = find(key);
		if (index != -1) {
			old = backet[index].value;
			if (!VALCTRL::isEmpty(old)) {
				count--;
				VALCTRL::initialize(backet[index].value);
				if (backetSize - count > BOUNDARY)
					rehash(count);
			}
		}
		return old;
	}
	// �L�[��񋓂��邽�߂̃C���X�^���X�𐶐�����B
	// �g�p���delete��Y��Ȃ��悤�ɒ��ӁB
	// �Ԓl:
	//	�L�[��񋓂��邽�߂̃C���X�^���X�B
	Pointer<KeyEnumeration> keys()const {
		return new EnumKeys(*this);
	}
	// �l��񋓂��邽�߂̃C���X�^���X�𐶐�����B
	// �g�p���delete��Y��Ȃ��悤�ɒ��ӁB
	// �Ԓl:
	//	�l��񋓂��邽�߂̃C���X�^���X�B
	Pointer<ElementEnumeration> elements()const {
		return new EnumValues(*this);
	}
	// �L���Ȓl�����L�[�̐����擾����B
	// �Ԓl:
	//	�L���Ȓl�����L�[�̐��B
	int size()const {
		return count;
	}
	// �w��̃L�[���L���Ȓl�������Ă��邩�ǂ����𔻒肷��B
	// ����:
	//	���肷��L�[�B
	// �Ԓl:
	//	�L���Ȓl�������Ă���ΐ^�B
	bool contains(const TYPE_KEY& key)const {
		int index = find(key);
		return index != -1 && !VALCTRL::isEmpty(backet[index].value);
	}
	// �L���Ȓl����������Ă��Ȃ����ǂ����𔻒肷��B
	// �Ԓl:
	//	�L���Ȓl����������Ă��Ȃ���ΐ^�B
	bool isEmpty()const {
		return count == 0;
	}
	// �L���Ȓl����������Ă��Ȃ���Ԃɖ߂��B
	void empty() {
		delete[] backet;
		backet = NULL;
		backetSize = 0;
		count = 0;
	}
};

}

#endif//_YCL_HASHTABLE_H_
