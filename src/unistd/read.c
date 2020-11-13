/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <Windows.h>
#include <wlibc_errors.h>
#include <errno.h>
#include <fcntl_internal.h>
#include <fcntl.h>

ssize_t wlibc_read(int fd, void *buf, size_t count)
{
	if (buf == NULL)
	{
		errno = EFAULT;
		return -1;
	}

	if (!validate_active_ffd(fd))
	{
		return -1;
	}

	HANDLE file = get_fd_handle(fd);
	int flags = get_fd_flags(fd);
	if ((flags & (O_RDONLY | O_RDWR)) == 0)
	{
		errno = EACCES;
		return -1;
	}

	DWORD read_count;
	BOOL status = ReadFile(file, buf, count, &read_count, NULL);
	if (!status)
	{
		map_win32_error_to_wlibc(GetLastError());
		return -1;
	}

	return read_count;
}
