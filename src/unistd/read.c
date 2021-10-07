/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <Windows.h>
#include <internal/error.h>
#include <errno.h>
#include <internal/fcntl.h>
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
	IO_STATUS_BLOCK I;
	I.Information = 0;
	NTSTATUS status = NtReadFile(file, NULL, NULL, NULL, &I, buf, count, NULL, NULL);
	if (status != STATUS_SUCCESS && status != STATUS_PENDING && status != STATUS_END_OF_FILE)
	{
		// When status is set to STATUS_PIPE_BROKEN set errno as EPIPE and return 0 as glibc does.
		// Lots of software depend on this behaviour, changing this would lead to breakage.
		map_ntstatus_to_errno(status);
		if (errno != EPIPE)
		{
			return -1;
		}
	}
	// When we are reading from a pipe, if the write end is duplicated, only the result of one of those
	// writes will be read in. As a workaround we try reading until the requested count is satisfied or
	// until we encounter STATUS_PIPE_BROKEN. The below code belongs to the old implementation, the new
	// implementation will be done at a later date.
#if 0
	DWORD read_count;
	BOOL status = ReadFile(file, buf, count, &read_count, NULL);
	if (!status)
	{
		map_win32_error_to_wlibc(GetLastError());
		if(errno != EPIPE)
			return -1;
	}

	if (_type == PIPE_HANDLE)
	{
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
#endif

	return I.Information;
}
