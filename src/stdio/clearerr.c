/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdio.h>
#include <internal/stdio.h>

void common_clearerr(FILE *stream)
{
	stream->error = 0;
}

void wlibc_clearerr_unlocked(FILE *stream)
{
	if (stream == NULL || stream->magic != FILE_STREAM_MAGIC)
	{
		errno = EINVAL;
	}
	common_clearerr(stream);
}

void wlibc_clearerr(FILE *stream)
{
	if (stream == NULL || stream->magic != FILE_STREAM_MAGIC)
	{
		errno = EINVAL;
	}

	LOCK_FILE_STREAM(stream);
	common_clearerr(stream);
	UNLOCK_FILE_STREAM(stream);
}
