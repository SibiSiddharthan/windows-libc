/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <unistd.h>
#include <fcntl_internal.h>
#include <wlibc_errors.h>

int wlibc_close(int fd)
{
	if (!validate_fd(fd))
	{
		errno = EBADF;
		return -1;
	}

	return close_fd(fd);
}