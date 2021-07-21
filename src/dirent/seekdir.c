/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <dirent.h>
#include <windows.h>
#include <stdlib.h>
#include <fcntl_internal.h>

void fill_dir_buffer(DIR *dirp);

void wlibc_seekdir(DIR *dirp, long long int pos)
{
	if (dirp == NULL || get_fd_type(dirp->fd) != DIRECTORY_HANDLE)
	{
		errno = EBADF;
		return;
	}
	// This value should be given by telldir
	dirp->offset = pos;
}
