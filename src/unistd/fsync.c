/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <internal/error.h>
#include <internal/nt.h>
#include <internal/fcntl.h>

int common_sync(int fd, int sync_all)
{
	if (get_fd_type(fd) != FILE_HANDLE)
	{
		errno = EROFS;
		return -1;
	}

	HANDLE file = get_fd_handle(fd);
	IO_STATUS_BLOCK I;
	NTSTATUS status = NtFlushBuffersFileEx(file, sync_all == 1 ? 0 : FLUSH_FLAGS_FILE_DATA_SYNC_ONLY, NULL, 0, &I);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}

/* Hack. There isn't a way (that I know of) to
   flush the data to disk without updating it's metadata
*/
int wlibc_fdatasync(int fd)
{
	return common_sync(fd, 0);
}

int wlibc_fsync(int fd)
{
	return common_sync(fd, 1);
}
