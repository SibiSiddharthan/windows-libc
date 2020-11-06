/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
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