/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/dirent.h>
#include <dirent.h>
#include <windows.h>
#include <stdlib.h>
#include <internal/fcntl.h>

void fill_dir_buffer(DIR *dirstream);

void wlibc_seekdir(DIR *dirstream, long long int pos)
{
	VALIDATE_DIR_STREAM(dirstream, );
	LOCK_DIR_STREAM(dirstream);
	// This value should be given by telldir
	dirstream->offset = pos;
	UNLOCK_DIR_STREAM(dirstream);
}
