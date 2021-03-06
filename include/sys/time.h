/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SYS_TIME_H
#define WLIBC_SYS_TIME_H

#include <wlibc-macros.h>
#include <time.h>
#include <sys/types.h>

_WLIBC_BEGIN_DECLS

struct timeval
{
	time_t tv_sec;       // seconds
	suseconds_t tv_usec; // microseconds
};

WLIBC_API int wlibc_gettimeofday(struct timeval *tp, void *tz /*unused*/);

WLIBC_INLINE int gettimeofday(struct timeval *tp, void *tz /*unused*/)
{
	return wlibc_gettimeofday(tp, tz);
}

_WLIBC_END_DECLS

#endif