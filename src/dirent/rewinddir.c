/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/dirent.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <dirent.h>
#include <errno.h>

void wlibc_rewinddir(DIR *dirstream)
{
	VALIDATE_DIR_STREAM(dirstream, );

	NTSTATUS status;
	IO_STATUS_BLOCK io;

	LOCK_DIR_STREAM(dirstream);

	memset(dirstream->buffer, 0, DIRENT_DIR_BUFFER_SIZE);
	status = NtQueryDirectoryFileEx(get_fd_handle(dirstream->fd), NULL, NULL, NULL, &io, dirstream->buffer, DIRENT_DIR_BUFFER_SIZE,
									FileIdExtdBothDirectoryInformation, FILE_QUERY_RESTART_SCAN, NULL);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
	}

	dirstream->received_data = io.Information;
	dirstream->read_data = 0;
	dirstream->offset = 0;

	UNLOCK_DIR_STREAM(dirstream);
}
