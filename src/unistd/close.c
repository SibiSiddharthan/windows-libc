/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
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
