/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SYS_SELECT_H
#define WLIBC_SYS_SELECT_H

#include <wlibc.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>

_WLIBC_BEGIN_DECLS

// This is the limitation on most implementations.
#define FD_SETSIZE 1024

typedef struct _fd_set
{
	unsigned long long masks[128];
} fd_set;

WLIBC_API void wlibc_fd_clr(int fd, fd_set *set);
WLIBC_API void wlibc_fd_set(int fd, fd_set *set);
WLIBC_API void wlibc_fd_zero(fd_set *set);
WLIBC_API int wlibc_fd_isset(int fd, fd_set *set);

WLIBC_INLINE void FD_CLR(int fd, fd_set *set)
{
	wlibc_fd_clr(fd, set);
}
WLIBC_INLINE void FD_SET(int fd, fd_set *set)
{
	wlibc_fd_set(fd, set);
}
WLIBC_INLINE void FD_ZERO(fd_set *set)
{
	wlibc_fd_zero(set);
}
WLIBC_INLINE int FD_ISSET(int fd, fd_set *set)
{
	return wlibc_fd_isset(fd, set);
}

WLIBC_API int wlibc_common_select(int nfds, fd_set *restrict readfds, fd_set *restrict writefds, fd_set *restrict exceptfds,
								  const struct timespec *restrict timeout, const sigset_t *restrict sigmask);

WLIBC_INLINE int select(int nfds, fd_set *restrict readfds, fd_set *restrict writefds, fd_set *restrict exceptfds,
						struct timeval *restrict timeout)
{
	struct timespec timespec_timeout;
	if (timeout)
	{
		timespec_timeout.tv_sec = timeout->tv_sec;
		timespec_timeout.tv_nsec = timeout->tv_usec * 1000;
	}

	return wlibc_common_select(nfds, readfds, writefds, exceptfds, timeout == NULL ? NULL : &timespec_timeout, NULL);
}

WLIBC_INLINE int pselect(int nfds, fd_set *restrict readfds, fd_set *restrict writefds, fd_set *restrict exceptfds,
						 const struct timespec *restrict timeout, const sigset_t *restrict sigmask)
{
	return wlibc_common_select(nfds, readfds, writefds, exceptfds, timeout, sigmask);
}

_WLIBC_END_DECLS

#endif
