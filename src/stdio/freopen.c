/*
   Copyright (c) 2020-2022 Sibi Siddharthan

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

int parse_mode(const char *mode);
int get_buf_mode(int flags);
int common_fflush(FILE *stream);
int do_open(int dirfd, const char *name, int oflags, mode_t perm);

HANDLE reopen_file(HANDLE old_handle, int flags)
{
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	OBJECT_ATTRIBUTES object;
	UNICODE_STRING empty = {0, 0, NULL};
	HANDLE new_handle = INVALID_HANDLE_VALUE;
	ACCESS_MASK access = SYNCHRONIZE | FILE_READ_ATTRIBUTES | FILE_READ_EA | READ_CONTROL | FILE_WRITE_ATTRIBUTES;
	ULONG share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
	ULONG options = FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT;
	ULONG disposition;

	// TODO move this to open.c itself.
	// From open.c
	// access
	if (flags & O_WRONLY)
	{
		access |= FILE_WRITE_DATA | FILE_WRITE_EA;
	}
	else if (flags & O_RDWR)
	{
		access |= FILE_READ_DATA | FILE_WRITE_DATA | FILE_WRITE_EA;
	}
	else // nothing is given i.e O_RDONLY by default
	{
		access |= FILE_READ_DATA;
	}

	if (flags & O_APPEND)
	{
		access |= FILE_APPEND_DATA;
	}

	// options
	if (flags & O_RANDOM)
	{
		options |= FILE_RANDOM_ACCESS;
	}
	if (flags & O_SEQUENTIAL)
	{
		options |= FILE_SEQUENTIAL_ONLY;
	}

	// disposition
	switch ((flags & (O_CREAT | O_EXCL | O_TRUNC)))
	{
	case 0x0:
	case O_EXCL:
		disposition = FILE_OPEN;
	case O_CREAT:
		disposition = FILE_OPEN_IF;
	case O_TRUNC:
	case O_TRUNC | O_EXCL:
		disposition = FILE_OVERWRITE;
	case O_CREAT | O_EXCL:
	case O_CREAT | O_TRUNC | O_EXCL:
		disposition = FILE_CREATE;
	case O_CREAT | O_TRUNC:
		disposition = FILE_OVERWRITE_IF;
	}

	InitializeObjectAttributes(&object, &empty, (flags & O_NOINHERIT) == 0 ? OBJ_INHERIT : 0, old_handle, NULL);
	status = NtCreateFile(&new_handle, access, &object, &io, NULL, 0, share, FILE_OPEN, options, NULL, 0);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
	}

	return new_handle;
}

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

		new_handle = reopen_file(info.handle, flags);
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

reopen_same_file_fail:
	close_fd(stream->fd);
	return NULL;
}
