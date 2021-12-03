/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/stdio.h>
#include <internal/fcntl.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <errno.h>

int common_fileno(FILE *stream);

size_t wlibc_fileops(FILE *stream, WLIBC_FILE_STREAM_OPERATIONS operation, void *arg)
{
	VALIDATE_FILE_STREAM(stream, -1);
	if (operation < 0 || operation >= maxop)
	{
		errno = EINVAL;
		return -1;
	}

	int flags = get_fd_flags(common_fileno(stream));

	switch (operation)
	{
	case bufsize: // __fbufsize
		return stream->buf_size;
	case bufmode: // __fbufmode
		return stream->buf_mode & (_IONBF | _IOLBF | _IOFBF);
	case reading: // __freading
		if (stream->prev_op == OP_READ || ((flags & (O_WRONLY | O_APPEND | O_RDWR)) == 0))
		{
			return 1;
		}
		return 0;
	case writing: // __fwriting
		if (stream->prev_op == OP_WRITE || (flags & (O_WRONLY | O_APPEND)))
		{
			return 1;
		}
		return 0;
	case readable: // __freadable
		if (flags == O_RDONLY || flags & O_RDWR)
		{
			return 1;
		}
		return 0;
	case writeable: // __fwritable
		if (flags != 0)
		{
			return 1;
		}
		return 0;
	case pending_read: // __freadahead
		if (stream->prev_op == OP_READ)
		{
			return stream->end - stream->pos;
		}
		return 0;
	case pending_write: // __fpending
		if (stream->prev_op == OP_WRITE)
		{
			return stream->pos - stream->start;
		}
		return 0;
	case purge: // __fpurge
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
		return 0;
	case increment: // __freadptrinc
		if (stream->buf_mode != _IONBF)
		{
			stream->pos += *(size_t *)arg;
			// If we seek past the buffer, behave like fseek.
			if (stream->pos > stream->end)
			{
				stream->start = stream->pos;
				stream->end = stream->pos;
			}
		}
		return 0;
	case locking: // __fsetlocking
		// Don't bother changing the locking state. The CRITICAL_SECTION locks are recursive anyway.
		return FSETLOCKING_INTERNAL;
	case buffer: // __freadptr
		// For unbuffered streams, buf_size -> 0, buffer -> NULL.
		*(size_t *)arg = stream->buf_size;
		// This will be cast to const char* in __freadptr
		return (size_t)stream->buffer;
	case seterr: // __fseterr
		stream->error |= _IOERROR;
		return 0;
	// Unreachable (To avoid -Wswitch)
	case maxop:
		return -1;
	}

	// Unreachable
	return 0;
}
