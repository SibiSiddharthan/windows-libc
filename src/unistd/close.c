/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/fcntl.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

int wlibc_close(int fd)
{
	if(fd == AT_FDCWD)
	{
		// Special fd value.
		return 0;
	}

	if (!validate_fd(fd))
	{
		errno = EBADF;
		return -1;
	}

	return close_fd(fd);
}
