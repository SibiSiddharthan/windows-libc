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

	handle_t _type = get_fd_type(fd);
	if (_type == DIRECTORY_HANDLE || _type == INVALID_HANDLE)
	{
		errno = (_type == DIRECTORY_HANDLE ? EISDIR : EBADF);
		return -1;
	}

	HANDLE file = get_fd_handle(fd);
	IO_STATUS_BLOCK I;
	I.Information = 0;
	NTSTATUS status = NtReadFile(file, NULL, NULL, NULL, &I, buf, count, NULL, NULL);
	if (status != STATUS_SUCCESS && status != STATUS_PENDING && status != STATUS_END_OF_FILE && status != STATUS_PIPE_BROKEN)
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
	return I.Information;
}
