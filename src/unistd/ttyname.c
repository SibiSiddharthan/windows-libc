/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/fcntl.h>
#include <Windows.h>
#include <errno.h>
#include <unistd.h>

char *wlibc_ttyname(int fd)
{
	static char wlibc_ttyname_buffer[256];
	if (wlibc_isatty(fd))
	{
		GetConsoleOriginalTitleA(wlibc_ttyname_buffer, 256);
		return wlibc_ttyname_buffer;
	}

	return NULL;
}

int wlibc_ttyname_r(int fd, char *buf, size_t bufsiz)
{
	if (wlibc_isatty(fd))
	{
		if (GetConsoleOriginalTitleA(buf, bufsiz) == 0) // bufsiz is too small
		{
			errno = ERANGE;
			return errno;
		}
		return 0;
	}

	return errno;
}
