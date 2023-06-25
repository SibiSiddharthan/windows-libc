/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SYS_TIME_H
#define WLIBC_SYS_TIME_H

#include <wlibc.h>
#include <fcntl.h>
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

_WLIBC_END_DECLS

#endif