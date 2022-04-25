/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/convert.h>

/* 116444736000000000 is the number of 100 nanosecond intervals from
   January 1st 1601 to January 1st 1970 (UTC)
*/

struct timespec LARGE_INTEGER_to_timespec(LARGE_INTEGER LT)
{
	struct timespec result;
	time_t epoch = LT.QuadPart - 116444736000000000LL;
	result.tv_sec = epoch / 10000000;
	result.tv_nsec = (epoch % 10000000) * 100;
	return result;
}

LARGE_INTEGER timespec_to_LARGE_INTEGER(const struct timespec *time)
{
	LARGE_INTEGER L;
	L.QuadPart = time->tv_sec * 10000000 + time->tv_nsec / 100;
	L.QuadPart += 116444736000000000LL;
	return L;
}
