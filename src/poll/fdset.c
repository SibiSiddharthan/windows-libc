/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <errno.h>
#include <string.h>
#include <sys/select.h>

void wlibc_fd_clr(int fd, fd_set *set)
{
	if (set == NULL || fd >= FD_SETSIZE || fd < 0)
	{
		errno = EINVAL;
		return;
	}

	set->masks[fd / 64] &= ~(1ull << (fd % 64));
}

void wlibc_fd_set(int fd, fd_set *set)
{
	if (set == NULL || fd >= FD_SETSIZE || fd < 0)
	{
		errno = EINVAL;
		return;
	}

	set->masks[fd / 64] |= 1ull << (fd % 64);
}

void wlibc_fd_zero(fd_set *set)
{
	if (set == NULL)
	{
		errno = EINVAL;
		return;
	}

	memset(set, 0, sizeof(fd_set));
}

int wlibc_fd_isset(int fd, fd_set *set)
{
	if (set == NULL || fd >= FD_SETSIZE || fd < 0)
	{
		errno = EINVAL;
		return 0;
	}

	return ((set->masks[fd / 64] & (1ull << (fd % 64))) != 0);
}
