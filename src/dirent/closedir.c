/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <dirent.h>
#include <windows.h>
#include <internal/error.h>
#include <stdlib.h>
#include <internal/fcntl.h>

int wlibc_closedir(DIR *dirp)
{
	if (dirp == NULL || get_fd_type(dirp->fd) != DIRECTORY_HANDLE)
	{
		errno = EBADF;
		return -1;
	}

	if (!close_fd(dirp->fd))
	{
		// Free the memory of DIR and it's components
		free(dirp->_wdirent);
		free(dirp->_dirent);
		free(dirp->buffer);
		free(dirp);
		return 0;
	}
	else
	{
		return -1;
	}
}
