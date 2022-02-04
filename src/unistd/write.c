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
#include <internal/nt.h>
#include <fcntl.h>

ssize_t wlibc_write(int fd, const void *buf, size_t count)
{
	if (buf == NULL)
	{
		errno = EFAULT;
		return -1;
	}

	handle_t _type = get_fd_type(fd);
	if (_type == DIRECTORY_HANDLE || _type == INVALID_HANDLE)
	{
		errno = (_type == DIRECTORY_HANDLE ? EISDIR : EBADF);
		return -1;
	}

	HANDLE file = get_fd_handle(fd);
	int flags = get_fd_flags(fd);
	LARGE_INTEGER offset;
	offset.HighPart = -1;

	if (flags & O_APPEND)
	{
		offset.LowPart = FILE_WRITE_TO_END_OF_FILE;
	}
	else
	{
		offset.LowPart = FILE_USE_FILE_POINTER_POSITION;
	}

	IO_STATUS_BLOCK I;
	NTSTATUS status = NtWriteFile(file, NULL, NULL, NULL, &I, (PVOID)buf, (ULONG)count, &offset, NULL);
	if (status != STATUS_SUCCESS && status != STATUS_PENDING)
	{
		// NOTE: According to POSIX when status is STATUS_PIPE_BROKEN the signal SIGPIPE should be raised.
		// The default behaviour of SIGPIPE is 'abort'. We will just set errno to EPIPE and return -1.
		map_ntstatus_to_errno(status);
		return -1;
	}

	return I.Information;
}
