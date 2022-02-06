/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <unistd.h>

int common_sync(int fd, int sync_all)
{
	if (get_fd_type(fd) != FILE_HANDLE)
	{
		errno = EROFS;
		return -1;
	}

	IO_STATUS_BLOCK io;
	NTSTATUS status;
	HANDLE handle = get_fd_handle(fd);

	status = NtFlushBuffersFileEx(handle, sync_all == 1 ? 0 : FLUSH_FLAGS_FILE_DATA_SYNC_ONLY, NULL, 0, &io);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}

int wlibc_fdatasync(int fd)
{
	return common_sync(fd, 0);
}

int wlibc_fsync(int fd)
{
	return common_sync(fd, 1);
}
