/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
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