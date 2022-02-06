/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/dirent.h>
#include <dirent.h>
#include <errno.h>
#include <internal/fcntl.h>

off_t wlibc_telldir(DIR *dirstream)
{
	off_t offset;
	VALIDATE_DIR_STREAM(dirstream, -1);

	LOCK_DIR_STREAM(dirstream);
	// Return the offset in DIR->buffer.
	// NOTE: This is actually not the file offset in the directory entry.
	// You should treat this strictly as an opaque value.
	offset = dirstream->offset;
	UNLOCK_DIR_STREAM(dirstream);

	return offset;
}
