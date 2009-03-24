/*
 * $Id: DynamicLinkLibrary.h,v 1.3 2006-08-03 13:33:18 yutakakn Exp $
 */

#ifndef _YCL_DYNAMICLINKLIBRARY_H_
#define _YCL_DYNAMICLINKLIBRARY_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

namespace yebisuya {

template<class IMPLEMENT>
class DynamicLinkLibrary {
protected:
	typedef DynamicLinkLibrary<IMPLEMENT> super;

	DynamicLinkLibrary() {
	}
public:
	static IMPLEMENT& getInstance() {
		static IMPLEMENT instance;
		return instance;
	}
	bool processAttach() {
		return true;
	}
	bool processDetach() {
		return true;
	}
	bool threadAttach() {
		return true;
	}
	bool threadDetach() {
		return true;
	}
};

}

#endif//_YCL_DYNAMICLINKLIBRARY_H_

/*
 * $Changes
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2003/12/19 19:59:33  sugoroku
 * コンパイルエラーが出ていたので修正
 *
 * Revision 1.1.1.1  2003/05/26 15:32:00  sugoroku
 * 新規作成
 *
 */
