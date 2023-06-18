/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/fcntl.h>
#include <errno.h>
#include <unistd.h>

int wlibc_isatty(int fd)
{
	handle_t type = get_fd_type(fd);

	if (type == INVALID_HANDLE)
	{
		errno = EBADF;
		return 0;
	}

	if (type == CONSOLE_HANDLE)
	{
		return 1;
	}

	// else
	errno = ENOTTY;
	return 0;
}
