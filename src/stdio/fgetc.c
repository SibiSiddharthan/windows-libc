/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdio.h>
#include <internal/stdio.h>

size_t common_fread(void *restrict buffer, size_t size, size_t count, FILE *restrict stream);

int common_fgetc(FILE *stream)
{
	char ch;
	if (((stream->buf_mode & _IONBF) == 0) && stream->prev_op == OP_READ)
	{
		if (stream->start != stream->end && stream->pos != stream->end)
		{
			ch = stream->buffer[stream->pos++ - stream->start];
			return ch;
		}
	}
	common_fread(&ch, 1, 1, stream);
	if (stream->error == _IOEOF)
	{
		return EOF;
	}
	return ch;
}

int wlibc_fgetc_unlocked(FILE *stream)
{
	VALIDATE_FILE_STREAM(stream, EOF);
	return common_fgetc(stream);
}

int wlibc_fgetc(FILE *stream)
{
	VALIDATE_FILE_STREAM(stream, EOF);
	LOCK_FILE_STREAM(stream);
	int ch = common_fgetc(stream);
	UNLOCK_FILE_STREAM(stream);
	return ch;
}
