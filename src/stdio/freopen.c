/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <internal/stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int common_fflush(FILE *stream);
int do_open(int dirfd, const char *name, int oflags, mode_t perm);

FILE *wlibc_freopen(const char *restrict name, const char *restrict mode, FILE *restrict stream)
{
	VALIDATE_FILE_STREAM(stream, NULL);

	int new_fd;
	int flags = parse_mode(mode);

	// Flush the stream first
	common_fflush(stream);

	if (name == NULL)
	{
		HANDLE new_handle;
		fdinfo info;

		get_fdinfo(stream->fd, &info);

		if (info.type != FILE_HANDLE)
		{
			errno = EBADF;
			goto reopen_same_file_fail;
		}

		// Remove the O_CREAT and O_TMPFILE bits aas we are reopening an existing file.
		new_handle = reopen_handle(info.handle, ((flags | O_NOTDIR) & ~(O_CREAT | O_TMPFILE)));
		if (new_handle == INVALID_HANDLE_VALUE)
		{
			goto reopen_same_file_fail;
		}

		// It is safe to close the streams handle now.
		close_fd(stream->fd);
		new_fd = register_to_fd_table(new_handle, FILE_HANDLE, flags | O_NOTDIR);
	}
	else
	{
		// Close the underlying file handle first if name is not NULL.
		close_fd(stream->fd);
		new_fd = do_open(AT_FDCWD, name, flags | O_NOTDIR, 0700);
	}

	if (new_fd == -1)
	{
		// Failed to create a new stream, cleanup the current stream properly
		if ((stream->buf_mode & _IOBUFFER_INTERNAL) && (stream->buf_mode & _IOBUFFER_ALLOCATED))
		{
			RtlFreeHeap(NtCurrentProcessHeap(), 0, stream->buffer);
		}
		delete_stream(stream);
		return NULL;
	}

	stream->fd = new_fd;
	stream->error = 0;
	stream->start = 0;
	stream->end = 0;
	stream->pos = 0;
	stream->prev_op = 0;
	// Remove these buffer bits as we are setting them below based on mode
	stream->buf_mode = stream->buf_mode & ~(_IOBUFFER_RDONLY | _IOBUFFER_WRONLY | _IOBUFFER_RDWR);
	stream->buf_mode |= get_buf_mode(flags);

	return stream;

reopen_same_file_fail:
	close_fd(stream->fd);
	return NULL;
}
