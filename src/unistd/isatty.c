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

	HANDLE handle = get_fd_handle(fd);
	DWORD type = GetFileType(handle);
	if (type == FILE_TYPE_CHAR)
	{
		return 1;
	}
	else
	{
		GetLastError(); // Clear Last Error if any
		errno = ENOTTY;
		return 0;
	}
}
