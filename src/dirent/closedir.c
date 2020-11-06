/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <dirent.h>
#include <windows.h>
#include <wlibc_errors.h>
#include <stdlib.h>
#include <fcntl_internal.h>

int wlibc_closedir(DIR *dirp)
{
	if (dirp == NULL || !validate_fd(dirp->fd))
	{
		errno = EBADF;
		return -1;
	}

	if (!close_fd(dirp->fd))
	{
		// Free the memory of DIR and it's components
		free(dirp->_wdirent);
		free(dirp->_dirent);
		free(dirp->data);
		free(dirp);
		return 0;
	}
	else
	{
		return -1;
	}
}
