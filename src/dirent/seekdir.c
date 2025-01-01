/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/dirent.h>
#include <internal/fcntl.h>
#include <dirent.h>
#include <stdlib.h>

void wlibc_seekdir(DIR *dirstream, long long int pos)
{
	VALIDATE_DIR_STREAM(dirstream, );
	LOCK_DIR_STREAM(dirstream);
	// This value should be given by telldir
	dirstream->offset = pos;
	UNLOCK_DIR_STREAM(dirstream);
}
