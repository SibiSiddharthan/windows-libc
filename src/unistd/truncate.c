/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <wchar.h>
#include <errno.h>
#include <fcntl.h>
#include <fcntl_internal.h>
#include <Windows.h>
#include <wlibc_errors.h>
#include <stdlib.h>

static int common_truncate(HANDLE file, off_t length)
{
	LARGE_INTEGER offset, newpos;
	offset.QuadPart = 0;

	if (!SetFilePointerEx(file, offset, &newpos, FILE_END))
	{
		map_win32_error_to_wlibc(GetLastError());
		return -1;
	}

	// length <= file size. Move to specified position and set end of file
	if (newpos.QuadPart >= length)
	{
		offset.QuadPart = length;
		if (!SetFilePointerEx(file, offset, &newpos, FILE_BEGIN))
		{
			map_win32_error_to_wlibc(GetLastError());
			return -1;
		}
		if (!SetEndOfFile(file))
		{
			map_win32_error_to_wlibc(GetLastError());
			return -1;
		}
	}

	// length > file size. Move to end of file and write '0' till length is filled
	else
	{
		char *buf = (char *)malloc(sizeof(char) * (length - newpos.QuadPart));
		memset(buf, 0, length - newpos.QuadPart);
		DWORD bytes_written;
		BOOL status = WriteFile(file, buf, length - newpos.QuadPart, &bytes_written, NULL);
		if (!status)
		{
			map_win32_error_to_wlibc(GetLastError());
			free(buf);
			return -1;
		}
		free(buf);
	}

	return 0;
}

int wlibc_truncate(const char *path, off_t length)
{
	int fd = open(path, O_RDWR | O_BINARY | O_EXCL);
	if (fd == -1)
	{
		return -1;
	}

	HANDLE file = get_fd_handle(fd);
	int result = common_truncate(file, length);

	close(fd);
	return result;
}

int wlibc_wtruncate(const wchar_t *wpath, off_t length)
{
	int fd = wopen(wpath, O_RDWR | O_BINARY | O_EXCL);
	if (fd == -1)
	{
		return -1;
	}

	HANDLE file = get_fd_handle(fd);
	int result = common_truncate(file, length);

	close(fd);
	return result;
}

int wlibc_ftruncate(int fd, off_t length)
{
	if (get_fd_type(fd) != FILE_HANDLE)
	{
		return -1;
	}
	int flags = get_fd_flags(fd);
	if ((flags & (O_WRONLY | O_RDWR)) == 0)
	{
		errno = EACCES;
	}

	HANDLE file = get_fd_handle(fd);
	// Fail if we are not a disk file
	if (GetFileType(file) != FILE_TYPE_DISK)
	{
		errno = EPIPE;
		return -1;
	}

	return common_truncate(file, length);
}
