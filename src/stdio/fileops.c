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

size_t wlibc_fileops(FILE *stream, WLIBC_FILE_STREAM_OPERATIONS operation)
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
	case bufsize:
		return stream->buf_size;
	case reading:
		if (stream->prev_op == OP_READ || ((flags & (O_WRONLY | O_APPEND | O_RDWR)) == 0))
		{
			return 1;
		}
		return 0;
	case writing:
		if (stream->prev_op == OP_WRITE || (flags & (O_WRONLY | O_APPEND)))
		{
			return 1;
		}
		return 0;
	case readable:
		if (flags == O_RDONLY || flags & O_RDWR)
		{
			return 1;
		}
		return 0;
	case writeable:
		if (flags != 0)
		{
			return 1;
		}
		return 0;
	case buffered:
		if (stream->buf_mode & (_IOFBF | _IOLBF))
		{
			return 1;
		}
		return 0;
	case pending:
		if (stream->prev_op == OP_WRITE)
		{
			return stream->pos - stream->start;
		}
		return 0;
	case purge:
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
	case locking:
	// Don't bother changing the locking state. The CRITICAL_SECTION locks are recursive anyway.
		return FSETLOCKING_INTERNAL;
	// Unreachable (To avoid -Wswitch)
	case maxop:
		return -1;
	}

	// Unreachable
	return 0;
}
