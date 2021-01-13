/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <dirent.h>

int wlibc_dirfd(DIR *dirp)
{
	return dirp->fd;
}
