/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/fcntl.h>
#include <internal/validate.h>
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

int wlibc_ttyname_r(int fd, char *buffer, size_t bufsiz)
{
	VALIDATE_PTR(buffer, EINVAL, EINVAL);

	if (wlibc_isatty(fd))
	{
		if (GetConsoleOriginalTitleA(buffer, (DWORD)bufsiz) == 0) // bufsiz is too small
		{
			errno = ERANGE;
			return errno;
		}
		return 0;
	}

	return errno;
}
