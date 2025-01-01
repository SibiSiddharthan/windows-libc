/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/stdio.h>
#include <internal/fcntl.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int common_fflush(FILE *stream)
{
	if (stream->buf_mode & _IONBF)
	{
		// unbuffered stream, nothing to flush
		return 0;
	}

	if (stream->buf_mode & _IOBUFFER_RDONLY)
	{
		// readonly buffer, no need to flush
		return 0;
	}

	// CHECK this might be redundant with the above condition
	if (stream->prev_op == OP_READ)
	{
		// previous operation was a read, no need to flush
		return 0;
	}

	// flush only write portion
	if (stream->start != stream->pos)
	{
		ssize_t result = write(stream->fd, stream->buffer, stream->pos - stream->start);
		if (result == -1)
		{
			stream->error = _IOERROR;
			return -1;
		}

		stream->start = stream->pos;
	}

	return 0;
}

int common_flushall(int lock)
{
	FILE *start = _wlibc_stdio_head;
	int status = 0;

	LOCK_STDIO();
	while (start != NULL)
	{
		if (lock)
		{
			LOCK_FILE_STREAM(start);
		}
		status |= common_fflush(start);
		if (lock)
		{
			UNLOCK_FILE_STREAM(start);
		}
		start = start->prev;
	}
	UNLOCK_STDIO();

	return status;
}

int wlibc_fflush_unlocked(FILE *stream)
{
	if (stream == NULL)
	{
		return common_flushall(0);
	}

	VALIDATE_FILE_STREAM(stream, EOF);
	return common_fflush(stream);
}

int wlibc_fflush(FILE *stream)
{
	if (stream == NULL)
	{
		return common_flushall(1);
	}

	VALIDATE_FILE_STREAM(stream, EOF);
	LOCK_FILE_STREAM(stream);
	int status = common_fflush(stream);
	UNLOCK_FILE_STREAM(stream);
	return status;
}
