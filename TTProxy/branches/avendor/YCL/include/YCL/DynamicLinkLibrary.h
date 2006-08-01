/*
 * $Id: DynamicLinkLibrary.h,v 1.1.1.1 2006-08-01 15:01:47 yutakakn Exp $
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
	friend bool YCL_DllMain(int reason) {
		switch (reason) {
		case DLL_PROCESS_ATTACH:
			return DynamicLinkLibrary<IMPLEMENT>::getInstance().processAttach();
		case DLL_PROCESS_DETACH:
			return DynamicLinkLibrary<IMPLEMENT>::getInstance().processDetach();
		case DLL_THREAD_ATTACH:
			return DynamicLinkLibrary<IMPLEMENT>::getInstance().threadAttach();
		case DLL_THREAD_DETACH:
			return DynamicLinkLibrary<IMPLEMENT>::getInstance().threadDetach();
		}
		return false;
	}
};

}

#endif//_YCL_DYNAMICLINKLIBRARY_H_

/*
 * $Changes
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2003/12/19 19:59:33  sugoroku
 * �R���p�C���G���[���o�Ă����̂ŏC��
 *
 * Revision 1.1.1.1  2003/05/26 15:32:00  sugoroku
 * �V�K�쐬
 *
 */
