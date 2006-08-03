/*
 * $Id: HASHCODE.h,v 1.1.1.1 2006-08-01 15:01:47 yutakakn Exp $
 */
#ifndef _YCL_HASHCODE_H_
#define _YCL_HASHCODE_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

namespace yebisuya {

class HASHCODE {
private:
	static int generateHashCode(const char* value) {
		int h = 0;
		while (*value != '\0')
			h = h * 31 + *value++;
		return h;
	}

	int value;
public:
	HASHCODE(const char* value):value(generateHashCode(value)) {
	}
	HASHCODE(const HASHCODE& code):value(code.value) {
	}
	HASHCODE(void* value):value((int) value) {
	}
	HASHCODE(int value):value(value) {
	}
	operator int()const {
		return value;
	}
};

}

#endif//_YCL_HASHCODE_H_
/*
 * Changes:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2003/11/28 11:14:10  sugoroku
 * CVSキーワードを追加
 *
 */
