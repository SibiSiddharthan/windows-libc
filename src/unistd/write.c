/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <Windows.h>
#include <wlibc_errors.h>
#include <errno.h>
#include <fcntl_internal.h>
#include <fcntl.h>

ssize_t wlibc_write(int fd, void *buf, size_t count)
{
	if (buf == NULL)
	{
		errno = EFAULT;
		return -1;
	}

	enum handle_type _type = get_fd_type(fd);
	if (_type == DIRECTORY_HANDLE || _type == INVALID_HANDLE)
	{
		errno = (_type == DIRECTORY_HANDLE ? EISDIR : EBADF);
		return -1;
	}

	HANDLE file = get_fd_handle(fd);
	int flags = get_fd_flags(fd);
	if ((flags & (O_WRONLY | O_RDWR)) == 0)
	{
		errno = EACCES;
		return -1;
	}

	if (flags & O_APPEND)
	{
		LARGE_INTEGER L;
		L.QuadPart = 0;
		SetFilePointerEx(file, L, NULL, FILE_END);
	}

	DWORD write_count;
	BOOL status = WriteFile(file, buf, count, &write_count, NULL);
	if (!status)
	{
		map_win32_error_to_wlibc(GetLastError());
		return -1;
	}

	return write_count;
}
