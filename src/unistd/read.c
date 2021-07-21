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

ssize_t wlibc_read(int fd, void *buf, size_t count)
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
	DWORD read_count;
	BOOL status = ReadFile(file, buf, count, &read_count, NULL);
	if (!status)
	{
		map_win32_error_to_wlibc(GetLastError());
		return -1;
	}

	if (_type == PIPE_HANDLE)
	{
		// ReadFile only reads till first CR for applications with ENABLE_LINE_INPUT
		// So we fill up the buffer with subsequent calls
		while (count != read_count)
		{
			DWORD new_count = 0;
			ReadFile(file, (char *)buf + read_count, count - read_count, &new_count, NULL);

			if (new_count == 0)
			{
				// We have read all the data;
				break;
			}
			else
			{
				read_count += new_count;
			}
		}
	}

	return read_count;
}
