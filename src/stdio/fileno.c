/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/stdio.h>
#include <stdio.h>

int common_fileno(FILE *stream)
{
	return stream->fd;
}

int wlibc_fileno_unlocked(FILE *stream)
{
	VALIDATE_FILE_STREAM(stream, -1);
	return common_fileno(stream);
}

int wlibc_fileno(FILE *stream)
{
	VALIDATE_FILE_STREAM(stream, -1);

	LOCK_FILE_STREAM(stream);
	int fd = common_fileno(stream);
	UNLOCK_FILE_STREAM(stream);

	return fd;
}
