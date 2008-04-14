#include "gettimeofday.h"

int gettimeofday(struct timeval *tv, struct timezone *tz) {
	FILETIME ft;
	__int64 t;
	int tzsec, dst;

	if (tv) {
		GetSystemTimeAsFileTime(&ft);
		t = (((__int64)ft.dwHighDateTime << 32 | ft.dwLowDateTime) - FTEPOCHDIFF) / 10;
		tv->tv_sec  = (long)(t / 1000000);
		tv->tv_usec = (long)(t % 1000000);
	}

	if (tz) {
		if (_get_timezone(&tzsec) == 0 && _get_daylight(&dst) == 0) {
			tz->tz_minuteswest = tzsec / 60;
			tz->tz_dsttime = dst;
		}
		else {
			return -1;
		}
	}

	return 0;
}
