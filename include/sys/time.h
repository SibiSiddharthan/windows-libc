/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SYS_TIME_H
#define WLIBC_SYS_TIME_H

#include <wlibc.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>

_WLIBC_BEGIN_DECLS

struct timeval
{
	time_t tv_sec;       // seconds
	suseconds_t tv_usec; // microseconds
};

#define ITIMER_REAL    0 // Realtime timer
#define ITIMER_VIRTUAL 1 // Process timer
#define ITIMER_PROF    2 // Profiling timer

struct itimerval
{
	struct timeval it_interval; // Interval for periodic timer
	struct timeval it_value;    // Time until next expiration
};

WLIBC_API int wlibc_gettimeofday(struct timeval *restrict tp);

#pragma warning(push)
#pragma warning(disable : 4100) // Unused parameter

WLIBC_INLINE int gettimeofday(struct timeval *restrict tp, void *restrict tz WLIBC_UNUSED)
{
	return wlibc_gettimeofday(tp);
}

#pragma warning(pop)

WLIBC_API int wlibc_common_utimes(int dirfd, const char *path, const struct timeval times[2], int flags);

WLIBC_INLINE int utimes(const char *path, const struct timeval times[2])
{
	return wlibc_common_utimes(AT_FDCWD, path, times, 0);
}

WLIBC_INLINE int lutimes(const char *path, const struct timeval times[2])
{
	return wlibc_common_utimes(AT_FDCWD, path, times, AT_SYMLINK_NOFOLLOW);
}

WLIBC_INLINE int futimes(int fd, const struct timeval times[2])
{
	return wlibc_common_utimes(fd, NULL, times, AT_EMPTY_PATH);
}

WLIBC_INLINE int futimesat(int dirfd, const char *path, const struct timeval times[2], int flags)
{
	return wlibc_common_utimes(dirfd, path, times, flags);
}

WLIBC_API int wlibc_getitimer(int which, struct itimerval *current_value);
WLIBC_API int wlibc_setitimer(int which, const struct itimerval *restrict new_value, struct itimerval *restrict old_value);

WLIBC_INLINE int getitimer(int which, struct itimerval *current_value)
{
	return wlibc_getitimer(which, current_value);
}

WLIBC_INLINE int setitimer(int which, const struct itimerval *restrict new_value, struct itimerval *restrict old_value)
{
	return wlibc_setitimer(which, new_value, old_value);
}

// Should be in time.h

typedef int clockid_t;
typedef void *timer_t;

struct itimerspec
{
	struct timespec it_interval;
	struct timespec it_value;
};

#define CLOCK_REALTIME  0 // Real time clock
#define CLOCK_MONOTONIC 1 // Monotonic clock

#define TIMER_ABSTIME 1 // Absolute time

WLIBC_API int wlibc_clock_getres(clockid_t id, struct timespec *res);
WLIBC_API int wlibc_clock_gettime(clockid_t id, struct timespec *ts);
WLIBC_API int wlibc_clock_settime(clockid_t id, const struct timespec *ts);

WLIBC_INLINE int clock_getres(clockid_t id, struct timespec *res)
{
	return wlibc_clock_getres(id, res);
}

WLIBC_INLINE int clock_gettime(clockid_t id, struct timespec *ts)
{
	return wlibc_clock_gettime(id, ts);
}

WLIBC_INLINE int clock_settime(clockid_t id, const struct timespec *ts)
{
	return wlibc_clock_settime(id, ts);
}

WLIBC_API int wlibc_timer_create(clockid_t id, struct sigevent *restrict event, timer_t *restrict timer);
WLIBC_API int wlibc_timer_delete(timer_t timer);
WLIBC_API int wlibc_timer_getoverrun(timer_t timer);
WLIBC_API int wlibc_timer_gettime(timer_t timer, struct itimerspec *current_value);
WLIBC_API int wlibc_timer_settime(timer_t timer, int flags, const struct itimerspec *restrict new_value,
								  struct itimerspec *restrict old_value);

WLIBC_INLINE int timer_create(clockid_t id, struct sigevent *restrict event, timer_t *restrict timer)
{
	return wlibc_timer_create(id, event, timer);
}

WLIBC_INLINE int timer_delete(timer_t timer)
{
	return wlibc_timer_delete(timer);
}

WLIBC_INLINE int timer_getoverrun(timer_t timer)
{
	return wlibc_timer_getoverrun(timer);
}

WLIBC_INLINE int timer_gettime(timer_t timer, struct itimerspec *current_value)
{
	return wlibc_timer_gettime(timer, current_value);
}

WLIBC_INLINE int timer_settime(timer_t timer, int flags, const struct itimerspec *restrict new_value, struct itimerspec *restrict old_value)
{
	return wlibc_timer_settime(timer, flags, new_value, old_value);
}

_WLIBC_END_DECLS

#endif
