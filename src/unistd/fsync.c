/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <unistd.h>

int common_sync(int fd, int sync_all)
{
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	HANDLE handle;
	fdinfo info;

	get_fdinfo(fd, &info);

	if (info.type != FILE_HANDLE)
	{
		errno = EROFS;
		return -1;
	}

	handle = info.handle;

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
