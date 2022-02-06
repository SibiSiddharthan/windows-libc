/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/dirent.h>
#include <dirent.h>

int wlibc_dirfd(DIR *dirstream)
{
	int fd;
	VALIDATE_DIR_STREAM(dirstream, -1);
	LOCK_DIR_STREAM(dirstream);
	fd = dirstream->fd;
	UNLOCK_DIR_STREAM(dirstream);
	return fd;
}
