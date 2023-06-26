/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/validate.h>
#include <internal/error.h>
#include <sys/time.h>
#include <time.h>

#define VALIDATE_CLOCK(id)                                 \
	{                                                      \
		if (id != CLOCK_REALTIME && id != CLOCK_MONOTONIC) \
		{                                                  \
			errno = EINVAL;                                \
			return -1;                                     \
		}                                                  \
	}

int wlibc_clock_getres(clockid_t id, struct timespec *res)
{
	VALIDATE_CLOCK(id);

	if (res != NULL)
	{
		// Resolution of clocks in Windows is 100ns.
		res->tv_nsec = 100;
		res->tv_sec = 0;
	}

	return 0;
}

int wlibc_clock_gettime(clockid_t id, struct timespec *ts)
{
	LARGE_INTEGER epoch;

	VALIDATE_CLOCK(id);
	VALIDATE_PTR(ts, EINVAL, -1);

	switch (id)
	{
	case CLOCK_REALTIME:
		GetSystemTimePreciseAsFileTime((LPFILETIME)&epoch);
		break;
	case CLOCK_MONOTONIC:
		QueryPerformanceCounter(&epoch);
		break;
	}

	// The values reported here should be from January 1st 1601 UTC.
	ts->tv_sec = epoch.QuadPart / 10000000;
	ts->tv_nsec = (epoch.QuadPart % 10) * 100;

	return 0;
}

int wlibc_clock_settime(clockid_t id, const struct timespec *ts)
{
	VALIDATE_CLOCK(id);
	VALIDATE_PTR(ts, EINVAL, -1);

	// We don't support changing the time.
	errno = ENOTSUP;
	return -1;
}
