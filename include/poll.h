/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_POLL_H
#define WLIBC_POLL_H

#include <wlibc.h>
#include <signal.h>
#include <time.h>

_WLIBC_BEGIN_DECLS

// Requested events
#define POLLIN     0x001 // There is data to read.
#define POLLPRI    0x002 // There is urgent data to read.
#define POLLOUT    0x004 // Writing now will not block.
#define POLLRDNORM 0x040 // Normal data may be read.
#define POLLRDBAND 0x080 // Priority data may be read.
#define POLLWRNORM 0x100 // Writing now will not block.
#define POLLWRBAND 0x200 // Priority data may be written.

// Return events
#define POLLERR  0x008 // Error condition.
#define POLLHUP  0x010 // Hung up.
#define POLLNVAL 0x020 // Invalid polling request.

typedef unsigned long long int nfds_t;

struct pollfd
{
	int fd;        // File descriptor.
	short events;  // Requested events.
	short revents; // Returned events.
};

WLIBC_API int wlibc_common_poll(struct pollfd *fds, nfds_t nfds, const struct timespec *timeout, const sigset_t *sigmask);

WLIBC_INLINE int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
	struct timespec timespec_timeout;
	if (timeout >= 0)
	{
		timespec_timeout.tv_sec = timeout / 1000;
		timespec_timeout.tv_nsec = ((timeout % 1000) * 1000000) % 1000000000;
	}

	return wlibc_common_poll(fds, nfds, timeout < 0 ? NULL : &timespec_timeout, NULL);
}

WLIBC_INLINE int ppoll(struct pollfd *fds, nfds_t nfds, const struct timespec *timeout, const sigset_t *sigmask)
{
	return wlibc_common_poll(fds, nfds, timeout, sigmask);
}

_WLIBC_END_DECLS

#endif
