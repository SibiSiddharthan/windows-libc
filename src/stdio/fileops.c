/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/fcntl.h>
#include <internal/stdio.h>
#include <errno.h>
#include <stdio.h>
#include <stdio_ext.h>

int common_fileno(FILE *stream);

// Buffer queries
size_t wlibc_fbufsize(FILE *stream)
{
	VALIDATE_FILE_STREAM(stream, -1);
	return stream->buf_size;
}

int wlibc_fbufmode(FILE *stream)
{
	VALIDATE_FILE_STREAM(stream, -1);
	return stream->buf_mode & (_IONBF | _IOLBF | _IOFBF);
}

const char *wlibc_freadptr(FILE *stream, size_t *bufsize)
{
	VALIDATE_FILE_STREAM(stream, NULL);
	*bufsize = stream->buf_size;
	return stream->buffer;
}

// Mode query
int wlibc_freading(FILE *stream)
{
	VALIDATE_FILE_STREAM(stream, -1);
	int flags = get_fd_flags(common_fileno(stream));

	if (stream->prev_op == OP_READ || ((flags & (O_WRONLY | O_RDWR | O_APPEND)) == 0))
	{
		return 1;
	}
	return 0;
}

int wlibc_fwriting(FILE *stream)
{
	VALIDATE_FILE_STREAM(stream, -1);
	int flags = get_fd_flags(common_fileno(stream));

	if (stream->prev_op == OP_WRITE || (flags & (O_WRONLY | O_APPEND)))
	{
		return 1;
	}
	return 0;
}

int wlibc_freadable(FILE *stream)
{
	VALIDATE_FILE_STREAM(stream, -1);
	int flags = get_fd_flags(common_fileno(stream));

	if ((flags & O_WRONLY) == 0 || flags & O_RDWR)
	{
		return 1;
	}
	return 0;
}

int wlibc_fwritable(FILE *stream)
{
	VALIDATE_FILE_STREAM(stream, -1);
	int flags = get_fd_flags(common_fileno(stream));

	if ((flags & (O_WRONLY | O_RDWR | O_APPEND)) != 0)
	{
		return 1;
	}
	return 0;
}

size_t wlibc_freadahead(FILE *stream)
{
	VALIDATE_FILE_STREAM(stream, -1);

	if (stream->prev_op == OP_READ)
	{
		return stream->end - stream->pos;
	}
	return 0;
}

size_t wlibc_fpending(FILE *stream)
{
	VALIDATE_FILE_STREAM(stream, -1);

	if (stream->prev_op == OP_WRITE)
	{
		return stream->pos - stream->start;
	}
	return 0;
}

// Buffer operations
void wlibc_fpurge(FILE *stream)
{
	VALIDATE_FILE_STREAM(stream, );

	if (stream->buf_mode != _IONBF)
	{
		if (stream->prev_op == OP_WRITE)
		{
			stream->start = stream->pos;
		}
		if (stream->prev_op == OP_READ)
		{
			stream->pos = stream->end;
		}
	}
}

void wlibc_freadptrinc(FILE *stream, size_t size)
{
	VALIDATE_FILE_STREAM(stream, );

	if (stream->buf_mode != _IONBF)
	{
		stream->pos += size;
		// If we seek past the buffer, behave like fseek.
		if (stream->pos > stream->end)
		{
			stream->start = stream->pos;
			stream->end = stream->pos;
		}
	}
}

// Stream control
void wlibc_fseterr(FILE *stream)
{
	VALIDATE_FILE_STREAM(stream, );
	stream->error |= _IOERROR;
}

int wlibc_fsetlocking(FILE *stream, int type /*unused*/)
{
	// Don't bother changing the locking state. The CRITICAL_SECTION locks are recursive anyway.
	VALIDATE_FILE_STREAM(stream, -1);
	return FSETLOCKING_INTERNAL;
}
