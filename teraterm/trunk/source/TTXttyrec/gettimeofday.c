#include "gettimeofday.h"

int gettimeofday(struct timeval *tv, struct timezone *tz) {
	FILETIME ft;
	__int64 t;

	if (tv) {
		GetSystemTimeAsFileTime(&ft);
		t = (((__int64)ft.dwHighDateTime << 32 | ft.dwLowDateTime) - FTEPOCHDIFF) / 10;
		tv->tv_sec  = (long)(t / 1000000);
		tv->tv_usec = (long)(t % 1000000);
	}

	if (tz) {
		// timezone is not supported.
		return -1;
	}

	return 0;
}
