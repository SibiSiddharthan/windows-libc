/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

ssize_t wlibc_write(int fd, const void *buffer, size_t count)
{
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	HANDLE handle;
	LARGE_INTEGER offset;
	fdinfo info;

	if (buffer == NULL)
	{
		errno = EFAULT;
		return -1;
	}

	get_fdinfo(fd, &info);

	if (info.type == DIRECTORY_HANDLE || info.type == INVALID_HANDLE)
	{
		errno = (info.type == DIRECTORY_HANDLE ? EISDIR : EBADF);
		return -1;
	}

	handle = info.handle;
	offset.HighPart = -1;

	if (info.flags & O_APPEND)
	{
		offset.LowPart = FILE_WRITE_TO_END_OF_FILE;
	}
	else
	{
		offset.LowPart = FILE_USE_FILE_POINTER_POSITION;
	}

	status = NtWriteFile(handle, NULL, NULL, NULL, &io, (PVOID)buffer, (ULONG)count, &offset, NULL);
	if (status != STATUS_SUCCESS && status != STATUS_PENDING)
	{
		// NOTE: According to POSIX when status is STATUS_PIPE_BROKEN the signal SIGPIPE should be raised.
		// The default behaviour of SIGPIPE is 'abort'. We will just set errno to EPIPE and return -1.
		map_ntstatus_to_errno(status);
		return -1;
	}

	return io.Information;
}
