/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <Windows.h>
#include <wlibc_errors.h>
#include <fcntl_internal.h>

int common_sync(int fd)
{
	if (!validate_active_ffd(fd))
	{
		return -1;
	}

	HANDLE file = get_fd_handle(fd);
	// Fail if we are not a disk file
	if (GetFileType(file) != FILE_TYPE_DISK)
	{
		errno = EPIPE;
		return -1;
	}

	if (!FlushFileBuffers(file))
	{
		map_win32_error_to_wlibc(GetLastError());
		return -1;
	}

	return 0;
}

/* Hack. There isn't a way (that I know of) to
   flush the data to disk without updating it's metadata
*/
int wlibc_fdatasync(int fd)
{
	return common_sync(fd);
}

int wlibc_fsync(int fd)
{
	return common_sync(fd);
}
