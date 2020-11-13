/*
   Copyright (c) 2020 Sibi Siddharthan

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
	if (dirp == NULL || !validate_active_dirfd(dirp->fd))
	{
		errno = EBADF;
		return;
	}

	if (dirp->buffer_length <= pos)
	{
		size_t length = dirp->buffer_length;
		WIN32_FIND_DATA *temp = (WIN32_FIND_DATA *)malloc(sizeof(WIN32_FIND_DATA) * length);
		memcpy(temp, dirp->data, sizeof(WIN32_FIND_DATA) * length);
		dirp->buffer_length = 2 * pos; // subject to change
		dirp->data = (WIN32_FIND_DATA *)realloc(dirp->data, sizeof(WIN32_FIND_DATA) * dirp->buffer_length);
		memcpy(dirp->data, temp, sizeof(WIN32_FIND_DATA) * length);
		free(temp);
		fill_dir_buffer(dirp);
	}
	dirp->offset = pos;
}
