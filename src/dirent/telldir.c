/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <dirent.h>
#include <errno.h>
#include <fcntl_internal.h>

off_t wlibc_telldir(DIR *dirp)
{
	if (dirp == NULL || !validate_active_dirfd(dirp->fd))
	{
		errno = EBADF;
		return -1;
	}

	return dirp->offset;
}
