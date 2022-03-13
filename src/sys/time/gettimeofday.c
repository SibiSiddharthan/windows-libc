/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/validate.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>

/* 116444736000000000 is the number of 100 nanosecond intervals from
   January 1st 1601 to January 1st 1970 (UTC)
*/

int wlibc_gettimeofday(struct timeval *restrict tp, void *restrict tz /*unused*/)
{
	VALIDATE_PTR(tp, EINVAL, -1);

	SYSTEMTIME systemtime;
	FILETIME filetime;
	GetSystemTime(&systemtime);
	SystemTimeToFileTime(&systemtime, &filetime);
	time_t epoch = ((time_t)filetime.dwHighDateTime << 32) + filetime.dwLowDateTime;
	epoch -= 116444736000000000LL;
	tp->tv_sec = epoch / 10000000;
	tp->tv_usec = (epoch % 10000000) / 10;

	return 0;
}
