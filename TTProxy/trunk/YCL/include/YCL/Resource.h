/*
 * $Id: Resource.h,v 1.4 2007-03-08 13:33:47 maya Exp $
 */

#ifndef _YCL_RESOURCE_H_
#define _YCL_RESOURCE_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <YCL/common.h>

#include <YCL/String.h>

namespace yebisuya {

class Resource {
public:
	static String loadString(HINSTANCE instance, UINT id, int bufsize) {
		char* buffer = (char*) alloca(bufsize);
		int length = loadString(instance, id, buffer, bufsize);
		if (length >= bufsize - 1)
			return NULL;
		if (length == 0)
			buffer[0] = '\0';
		return buffer;
	}

public:
	static int loadString(UINT id, char* buffer, int bufsize) {
		return loadString(GetInstanceHandle(), id, buffer, bufsize);
	}
	static int loadString(HINSTANCE instance, UINT id, char* buffer, int bufsize) {
		return LoadString(instance, id, buffer, bufsize);
	}
#if 0
	static String loadString(int id) {
		return loadString(GetInstanceHandle(), id);
	}
	static String loadString(HINSTANCE instance, int id) {
		int bufsize = 256;
		String string;
		while (string == NULL) {
			string = loadString(instance, id, bufsize);
			bufsize += 256;
		}
		return string;
	}
#endif
	static String loadString(int id, LCID lcid) {
		return loadString(GetInstanceHandle(), id, lcid);
	}
	static String loadString(HINSTANCE instance, int id, LCID lcid) {
		String string;
		UINT idRsrcBlk = id / 16 + 1;
		int strIndex  = id % 16;
		HRSRC hResource = NULL;
		char *p, buf[256];
		int i, len, destlen;

		hResource = FindResourceEx(instance, RT_STRING,
								   MAKEINTRESOURCE(idRsrcBlk), (WORD)lcid);
		if (hResource == NULL) {
			hResource = FindResourceEx(instance, RT_STRING,
									   MAKEINTRESOURCE(idRsrcBlk),
									   (WORD)1033); // 0x409 English(US)
		}
		p = (char *)LockResource(LoadResource(instance, hResource));
		for (i=0; i<(strIndex & 15); i++) {
			p += 2 + *(LPWORD)p*2;
		}
		len = *(LPWORD)p;

		destlen = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(p+2), len,
									  buf, sizeof(buf), 0, 0);
		buf[destlen] = 0;

		string = buf;
		return string;
	}

	static HICON loadIcon(int id) {
		return loadIcon(GetInstanceHandle(), id);
	}
	static HICON loadIcon(const char* id) {
		return loadIcon(GetInstanceHandle(), id);
	}
	static HICON loadIcon(HINSTANCE instance, int id) {
		return loadIcon(instance, MAKEINTRESOURCE(id));
	}
	static HICON loadIcon(HINSTANCE instance, const char* id) {
		return ::LoadIcon(instance, id);
	}

	static HICON loadIcon(int id, int cx, int cy) {
		return loadIcon(GetInstanceHandle(), id, cx, cy);
	}
	static HICON loadIcon(const char* id, int cx, int cy) {
		return loadIcon(GetInstanceHandle(), id, cx, cy);
	}
	static HICON loadIcon(HINSTANCE instance, int id, int cx, int cy) {
		return loadIcon(instance, MAKEINTRESOURCE(id), cx, cy);
	}
	static HICON loadIcon(HINSTANCE instance, const char* id, int cx, int cy) {
		return loadIcon(instance, id, cx, cy, LR_DEFAULTCOLOR | LR_SHARED);
	}
	static HICON loadIcon(int id, int cx, int cy, int flags) {
		return loadIcon(GetInstanceHandle(), id, cx, cy, flags);
	}
	static HICON loadIcon(const char* id, int cx, int cy, int flags) {
		return loadIcon(GetInstanceHandle(), id, cx, cy, flags);
	}
	static HICON loadIcon(HINSTANCE instance, int id, int cx, int cy, int flags) {
		return loadIcon(instance, MAKEINTRESOURCE(id), cx, cy, flags);
	}
	static HICON loadIcon(HINSTANCE instance, const char* id, int cx, int cy, int flags) {
		return (HICON) ::LoadImage(instance, id, IMAGE_ICON, cx, cx, flags);
	}

	static HCURSOR loadCursor(int id) {
		return loadCursor(GetInstanceHandle(), id);
	}
	static HCURSOR loadCursor(const char* id) {
		return loadCursor(GetInstanceHandle(), id);
	}
	static HCURSOR loadCursor(HINSTANCE instance, int id) {
		return loadCursor(instance, MAKEINTRESOURCE(id));
	}
	static HCURSOR loadCursor(HINSTANCE instance, const char* id) {
		return ::LoadCursor(instance, id);
	}

	static HCURSOR loadCursor(int id, int cx, int cy) {
		return loadCursor(GetInstanceHandle(), id, cx, cy);
	}
	static HCURSOR loadCursor(const char* id, int cx, int cy) {
		return loadCursor(GetInstanceHandle(), id, cx, cy);
	}
	static HCURSOR loadCursor(HINSTANCE instance, int id, int cx, int cy) {
		return loadCursor(instance, MAKEINTRESOURCE(id), cx, cy);
	}
	static HCURSOR loadCursor(HINSTANCE instance, const char* id, int cx, int cy) {
		return loadCursor(instance, id, cx, cy, LR_DEFAULTCOLOR | LR_SHARED);
	}
	static HCURSOR loadCursor(int id, int cx, int cy, int flags) {
		return loadCursor(GetInstanceHandle(), id, cx, cy, flags);
	}
	static HCURSOR loadCursor(const char* id, int cx, int cy, int flags) {
		return loadCursor(GetInstanceHandle(), id, cx, cy, flags);
	}
	static HCURSOR loadCursor(HINSTANCE instance, int id, int cx, int cy, int flags) {
		return loadCursor(instance, MAKEINTRESOURCE(id), cx, cy, flags);
	}
	static HCURSOR loadCursor(HINSTANCE instance, const char* id, int cx, int cy, int flags) {
		return (HCURSOR) ::LoadImage(instance, id, IMAGE_CURSOR, cx, cx, flags);
	}

	static HMENU loadMenu(int id) {
		return loadMenu(GetInstanceHandle(), id);
	}
	static HMENU loadMenu(const char* id) {
		return loadMenu(GetInstanceHandle(), id);
	}
	static HMENU loadMenu(HINSTANCE instance, int id) {
		return loadMenu(instance, MAKEINTRESOURCE(id));
	}
	static HMENU loadMenu(HINSTANCE instance, const char* id) {
		return ::LoadMenu(instance, id);
	}
};

}

#endif//_YCL_RESOURCE_H_

/*
 * $Changes
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2006/08/03 13:33:18  yutakakn
 * (none)
 *
 * Revision 1.2  2003/06/06 15:49:21  sugoroku
 * 文字列リソースをバッファに直接読み込むメソッドを追加
 *
 * Revision 1.1.1.1  2003/05/26 15:32:04  sugoroku
 * 新規作成
 *
 */
