/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/stdio.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

int common_ungetc(int ch, FILE *stream)
{
	// Only perform on an input stream
	if(stream->prev_op != OP_READ)
	{
		return EOF;
	}

	if (stream->pos > stream->start)
	{
		stream->pos--;
		stream->buffer[stream->pos - stream->start] = (char)ch;
		// clear eof flag
		stream->error = stream->error & ~_IOEOF;
		return ch;
	}

	return EOF;
}

int wlibc_ungetc(int ch, FILE *stream)
{
	if(ch == EOF)
	{
		return EOF;
	}

	VALIDATE_FILE_STREAM(stream, EOF);
	LOCK_FILE_STREAM(stream);
	int result = common_ungetc(ch, stream);
	UNLOCK_FILE_STREAM(stream);

	return result;
}
