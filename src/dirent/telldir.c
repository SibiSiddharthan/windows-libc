/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <dirent.h>
#include <errno.h>
#include <internal/fcntl.h>

off_t wlibc_telldir(DIR *dirp)
{
	if (dirp == NULL || get_fd_type(dirp->fd) != DIRECTORY_HANDLE)
	{
		errno = EBADF;
		return -1;
	}
	// Return the offset in DIR->buffer.
	// NOTE: This is actually not the file offset in the directory entry.
	// You should treat this strictly as an opaque value.
	return dirp->offset;
}
