/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SYS_TIMES_H
#define WLIBC_SYS_TIMES_H

#include <wlibc.h>
#include <time.h>

struct tms
{
	clock_t tms_utime;  // User CPU time.
	clock_t tms_stime;  // System CPU time.
	clock_t tms_cutime; // User CPU time of dead children.
	clock_t tms_cstime; // System CPU time of dead children.
};

WLIBC_API clock_t wlibc_times(struct tms *tmsbuf);

WLIBC_INLINE clock_t times(struct tms *tmsbuf)
{
	return wlibc_times(tmsbuf);
}

#endif
