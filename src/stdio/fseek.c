/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/stdio.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

int common_fflush(FILE *stream);

int common_fseek(FILE *stream, ssize_t offset, int whence)
{
	// If the stream was written to previously flush
	if (stream->prev_op == OP_WRITE)
	{
		common_fflush(stream);
	}

	// No need to seek
	if (whence == SEEK_CUR && stream->prev_op == OP_READ)
	{
		size_t new_pos = stream->pos + offset;
		if (new_pos >= stream->start && new_pos <= stream->end && stream->start != stream->end)
		{
			stream->pos = new_pos;
			return 0;
		}
	}

	off_t result = lseek(stream->fd, offset, whence);

	if (result != -1)
	{
		stream->pos = result;
		stream->start = stream->pos;
		stream->end = stream->pos;

		if (whence == SEEK_SET && offset == 0)
		{
			// Clear any error
			stream->error = 0;
		}
		else
		{
			// Clear eof only
			stream->error = stream->error & ~_IOEOF;
		}

		return 0;
	}

	return -1;
}

int wlibc_fseek(FILE *stream, ssize_t offset, int whence)
{
	VALIDATE_FILE_STREAM(stream, EOF);
	if (whence < 0 || whence > 2)
	{
		errno = EINVAL;
		return EOF;
	}

	int result;
	LOCK_FILE_STREAM(stream);
	result = common_fseek(stream, offset, whence);
	UNLOCK_FILE_STREAM(stream);
	return result;
}
