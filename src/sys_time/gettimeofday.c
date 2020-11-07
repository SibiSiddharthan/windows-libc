/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <sys/time.h>
#include <Windows.h>
#include <errno.h>
#include <time.h>

/* 116444736000000000 is the number of 100 nanosecond intervals from
   January 1st 1601 to January 1st 1970 (UTC)
*/

int wlibc_gettimeofday(struct timeval *tp, void *tz /*unused*/)
{
	if (tp == NULL)
	{
		errno = EINVAL;
		return -1;
	}

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