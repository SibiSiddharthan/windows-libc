/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <errno.h>
#include <poll.h>
#include <string.h>
#include <sys/select.h>

#define FAST_FDISSET(fd, set) ((set->masks[fd / 64] & (1ull << (fd % 64))) != 0)
#define FAST_FDSET(fd, set)   ((set).masks[fd / 64] |= 1ull << (fd % 64))
#define FAST_FDZERO(set)      (memset(set, 0, sizeof(fd_set)))

int wlibc_common_select(int nfds, fd_set *restrict readfds, fd_set *restrict writefds, fd_set *restrict exceptfds,
						const struct timespec *restrict timeout, const sigset_t *restrict sigmask)
{
	struct pollfd *pollfds = NULL;
	fd_set setfds, zerofds;
	nfds_t countfds = 0;

	memset(&zerofds, 0, sizeof(fd_set));

	if (nfds < 0 || nfds > FD_SETSIZE)
	{
		errno = EINVAL;
		return -1;
	}

	if (readfds == NULL && writefds == NULL && exceptfds == NULL)
	{
		// Nothing to do.
		return 0;
	}

	// Assigning the null fd sets to the zerofd set avoids needless branching for the FAST_FD* macros.
	if (readfds == NULL)
	{
		readfds = &zerofds;
	}

	if (writefds == NULL)
	{
		writefds = &zerofds;
	}

	if (exceptfds == NULL)
	{
		exceptfds = &zerofds;
	}

	for (int i = 0; i < nfds; ++i)
	{
		if (FAST_FDISSET(i, readfds) || FAST_FDISSET(i, writefds) || FAST_FDISSET(i, exceptfds))
		{
			FAST_FDSET(i, setfds);
			++countfds;
		}
	}

	if (countfds == 0)
	{
		// Again nothing to do.
		return 0;
	}

	pollfds = (struct pollfd *)RtlAllocateHeap(NtCurrentProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct pollfd) * countfds);
	if (pollfds == NULL)
	{
		errno = ENOMEM;
		return -1;
	}

	size_t index = 0;
	for (int i = 0; i < nfds; ++i)
	{
		if (FAST_FDISSET(i, readfds) || FAST_FDISSET(i, writefds) || FAST_FDISSET(i, exceptfds))
		{
			// No need to initialize events and revents as we are zeroing the memory first.
			pollfds[index].fd = i;

			if (FAST_FDISSET(i, readfds))
			{
				pollfds[index].events |= POLLIN;
			}

			if (FAST_FDISSET(i, writefds))
			{
				pollfds[index].events |= POLLOUT;
			}

			if (FAST_FDISSET(i, exceptfds))
			{
				pollfds[index].events |= POLLPRI;
			}

			++index;
		}
	}

	int poll_result;
	int select_result = 0;
	int error = 0;

	poll_result = wlibc_common_poll(pollfds, countfds, timeout, sigmask);
	if (poll_result == -1)
	{
		RtlFreeHeap(NtCurrentProcessHeap(), 0, pollfds);
		return -1;
	}

	// Zero all the input fd sets.
	FAST_FDZERO(readfds);
	FAST_FDZERO(writefds);
	FAST_FDZERO(exceptfds);

	// Fill the fd sets according to revents.
	for (nfds_t i = 0; i < countfds; ++i)
	{
		if (pollfds[i].revents == 0 || pollfds[i].revents & (POLLERR | POLLHUP))
		{
			continue;
		}

		if (pollfds[i].revents & POLLNVAL)
		{
			error = 1;
		}

		if (pollfds[i].revents & POLLIN)
		{
			FAST_FDSET(pollfds[i].fd, *readfds);
			++select_result;
		}

		if (pollfds[i].revents & POLLOUT)
		{
			FAST_FDSET(pollfds[i].fd, *writefds);
			++select_result;
		}

		if (pollfds[i].revents & POLLPRI)
		{
			FAST_FDSET(pollfds[i].fd, *exceptfds);
			++select_result;
		}
	}

	RtlFreeHeap(NtCurrentProcessHeap(), 0, pollfds);

	if (error == 1)
	{
		errno = EBADF;
		return -1;
	}

	return select_result;
}
