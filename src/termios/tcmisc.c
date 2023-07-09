/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/fcntl.h>
#include <internal/error.h>
#include <termios.h>
#include <unistd.h>

int wlibc_tcsendbreak(int fd, int duration)
{
	fdinfo info;

	get_fdinfo(fd, &info);

	if (info.type != CONSOLE_HANDLE)
	{
		errno = EBADF;
		return -1;
	}

	if(duration < 0)
	{
		errno = EINVAL;
		return -1;
	}

	// Just validate and return.
	return 0;
}

int wlibc_tcdrain(int fd)
{
	fdinfo info;

	get_fdinfo(fd, &info);

	if (info.type != CONSOLE_HANDLE)
	{
		errno = EBADF;
		return -1;
	}

	// Just validate and return.
	return 0;
}

int wlibc_tcflow(int fd, int action)
{
	fdinfo info;

	get_fdinfo(fd, &info);

	if (info.type != CONSOLE_HANDLE)
	{
		errno = EBADF;
		return -1;
	}

	if (action < TCOOFF || action > TCION)
	{
		errno = EINVAL;
		return -1;
	}

	// Just validate and return.
	return 0;
}

int wlibc_tcflush(int fd, int queue)
{
	if (queue < TCIFLUSH || queue > TCIOFLUSH)
	{
		errno = EINVAL;
		return -1;
	}

	return wlibc_tcdrain(fd);
}
