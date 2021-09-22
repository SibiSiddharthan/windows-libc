/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/stdio.h>
#include <internal/fcntl.h>
#include <errno.h>
#include <wchar.h>
#include <internal/misc.h>
#include <fcntl.h>

int parse_mode(const char *mode);
int get_buf_mode(int flags);
int common_fflush(FILE *stream);
int common_open(const wchar_t *wname, const int oflags, const mode_t perm);

FILE *wlibc_freopen(const char *name, const char *mode, FILE *stream)
{
	VALIDATE_FILE_STREAM(stream, NULL);

	const wchar_t *old_name = get_fd_path(stream->fd);
	int new_fd;

	// Flush the stream first
	common_fflush(stream);

	// Close the underlying file handle
	close_fd(stream->fd);

	int flags = parse_mode(mode);

	if (name == NULL)
	{
		new_fd = common_open(old_name, flags, 0);
	}
	else
	{
		wchar_t *wname = mb_to_wc(name);
		new_fd = common_open(wname, flags, 0);
		free(wname);
	}

	if (new_fd == -1)
	{
		// Failed to create a new stream, cleanup the current stream properly
		if ((stream->buf_mode & _IOBUFFER_INTERNAL) && (stream->buf_mode & _IOBUFFER_ALLOCATED))
		{
			free(stream->buffer);
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
}
