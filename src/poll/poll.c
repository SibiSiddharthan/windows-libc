/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/fcntl.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdlib.h>

int do_poll(const fdinfo *pinfo, struct pollfd *fds, nfds_t nfds, const struct timespec *timeout, const sigset_t *sigmask)
{
	int result = 0;
	// TODO make use of sigmask.
	for (nfds_t i = 0; i < nfds; ++i)
	{
		switch (pinfo[i].type)
		{
		case INVALID_HANDLE:
		{
			// Don't set errno to EBADF in this case.
			fds[i].revents = POLLNVAL;
			++result;
			break;
		}
		case CONSOLE_HANDLE:
		{
			// TODO
			break;
		}
		case NULL_HANDLE:
		case FILE_HANDLE:
		case DIRECTORY_HANDLE:
		{
			if (pinfo[i].flags & O_RDWR)
			{
				fds[i].revents = fds[i].events & (POLLIN | POLLOUT | POLLRDNORM | POLLRDBAND | POLLWRNORM | POLLWRBAND);
			}
			else if (pinfo[i].flags & O_WRONLY)
			{
				fds[i].revents = fds[i].events & (POLLOUT | POLLWRNORM | POLLWRBAND);
			}
			else // pinfo[i].flags == O_RDONLY
			{
				fds[i].revents = fds[i].events & (POLLIN | POLLRDNORM | POLLRDBAND);
			}
			break;
		}

		case PIPE_HANDLE:
		{
			// TODO
			break;
		}
		}
	}

	return result;
}

int wlibc_common_poll(struct pollfd *fds, nfds_t nfds, const struct timespec *timeout, const sigset_t *sigmask)
{
	int result;
	fdinfo *pinfo;

	if (fds == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	pinfo = (fdinfo *)malloc(sizeof(fdinfo) * nfds);

	SHARED_LOCK_FD_TABLE();

	for (nfds_t i = 0; i < nfds; ++i)
	{
		get_fdinfo(fds[i].fd, &pinfo[i]);
	}

	SHARED_UNLOCK_FD_TABLE();

	result = do_poll(pinfo, fds, nfds, timeout, sigmask);

	free(pinfo);

	return result;
}
