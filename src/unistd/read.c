/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

ssize_t wlibc_read(int fd, void *buffer, size_t count)
{
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	HANDLE handle;
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
	io.Information = 0;

	status = NtReadFile(handle, NULL, NULL, NULL, &io, buffer, (ULONG)count, NULL, NULL);
	if (status != STATUS_SUCCESS && status != STATUS_PENDING && status != STATUS_END_OF_FILE && status != STATUS_PIPE_BROKEN &&
		status != STATUS_PIPE_EMPTY)
	{
		// When status is set to STATUS_PIPE_BROKEN , it is not treated as an error.
		// Reading from a pipe with no write end is not an error apparently???.
		// Lots of software depend on this behaviour, changing this would lead to breakage.
		map_ntstatus_to_errno(status);
		return -1;
	}
	// NOTE: When we are reading from a pipe, if the write end is duplicated, only the result of one of those
	// writes will be read in.
	// `read` on linux reads all the data, even from the duplicated write ends in one shot(provided the given buffer is big enough).
	// We strictly don't have to conform to this as applications will check for the result of read, if 0 it means no more data is left.
	return io.Information;
}
