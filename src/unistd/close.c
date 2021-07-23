/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <fcntl.h>
#include <internal/fcntl.h>
#include <internal/error.h>

int wlibc_close(int fd)
{
	if(fd == AT_FDCWD)
	{
		// We do
		return 0;
	}

	if (!validate_fd(fd))
	{
		errno = EBADF;
		return -1;
	}

	return close_fd(fd);
}
