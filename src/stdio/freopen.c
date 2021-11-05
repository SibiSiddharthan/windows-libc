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
#include <internal/nt.h>
#include <fcntl.h>

int parse_mode(const char *mode);
int get_buf_mode(int flags);
int common_fflush(FILE *stream);
int do_open(int dirfd, const char *name, int oflags, mode_t perm);

FILE *wlibc_freopen(const char *restrict name, const char *restrict mode, FILE *restrict stream)
{
	VALIDATE_FILE_STREAM(stream, NULL);

	// Treat name = NULL as an error for now
	VALIDATE_PATH(name, EINVAL, NULL);

	// const wchar_t *oldname = get_fd_path(stream->fd);
	int new_fd;

	// Flush the stream first
	common_fflush(stream);

	// Close the underlying file handle
	close_fd(stream->fd);

	int flags = parse_mode(mode);
#if 0
	if (name == NULL)
	{
		UTF8_STRING u8_oldname;
		UNICODE_STRING u16_oldname;
		RtlInitUnicodeString(&u16_oldname, oldname);
		RtlUnicodeStringToUTF8String(&u8_oldname, &u16_oldname, TRUE);
		new_fd = do_open(AT_FDCWD, u8_oldname.Buffer, flags | O_NOTDIR, 0700);
		RtlFreeUTF8String(&u8_oldname);
	}
	else
#endif
	{
		new_fd = do_open(AT_FDCWD, name, flags | O_NOTDIR, 0700);
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
