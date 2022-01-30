/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <errno.h>
#include <unistd.h>
#include <internal/error.h>
#include <Windows.h>
#include <internal/fcntl.h>

int wlibc_isatty(int fd)
{
	if (!validate_fd(fd))
	{
		errno = EBADF;
		return 0;
	}

	handle_t type = get_fd_type(fd);
	if(type == CONSOLE_HANDLE)
	{
		return 1;
	}
	else
	{
		errno = ENOTTY;
		return 0;
	}
}
