/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <wchar.h>
#include <Windows.h>
#include <errno.h>
#include <stdbool.h>
#include <internal/fcntl.h>

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

wchar_t *wlibc_wttyname(int fd)
{
	static wchar_t wlibc_wttyname_buffer[256];
	if (wlibc_isatty(fd))
	{
		GetConsoleOriginalTitleW(wlibc_wttyname_buffer, 256);
		return wlibc_wttyname_buffer;
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

int wlibc_wttyname_r(int fd, wchar_t *wbuf, size_t bufsiz)
{
	if (wlibc_isatty(fd))
	{
		if (GetConsoleOriginalTitleW(wbuf, bufsiz) == 0) // bufsiz is too small
		{
			errno = ERANGE;
			return errno;
		}
		return 0;
	}

	return errno;
}
