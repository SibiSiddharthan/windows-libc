/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/fcntl.h>
#include <internal/stdio.h>
#include <stdlib.h>

FILE *_wlibc_stdio_head = NULL;

FILE *_wlibc_stdin = NULL;
FILE *_wlibc_stdout = NULL;
FILE *_wlibc_stderr = NULL;

CRITICAL_SECTION _wlibc_stdio_critical;

static void insert_stream(FILE *stream);

void initialize_stdio()
{
	InitializeCriticalSection(&_wlibc_stdio_critical);

	_wlibc_stdin = create_stream(0, _IOFBF | _IOBUFFER_INTERNAL | _IOBUFFER_RDONLY, 512);
	_wlibc_stdout = create_stream(1, _IOFBF | _IOBUFFER_INTERNAL | _IOBUFFER_WRONLY, 512);
	_wlibc_stderr = create_stream(2, _IONBF | _IOBUFFER_WRONLY, 0);
}

int common_fflush(FILE *stream);

void cleanup_stdio()
{
	while (_wlibc_stdio_head != NULL) //&& _wlibc_stdio_head->prev != NULL)
	{
		FILE *prev = _wlibc_stdio_head->prev;

		// flush buffered data
		common_fflush(_wlibc_stdio_head);

		// free internal buffers if any
		if (_wlibc_stdio_head->buffer != NULL && (_wlibc_stdio_head->buf_mode & _IOBUFFER_INTERNAL))
		{
			free(_wlibc_stdio_head->buffer);
		}
		DeleteCriticalSection(&(_wlibc_stdio_head->critical));

		// close the underlying file descriptor
		close_fd(_wlibc_stdio_head->fd);

		free(_wlibc_stdio_head);
		_wlibc_stdio_head = prev;
	}

	DeleteCriticalSection(&_wlibc_stdio_critical);
}

FILE *create_stream(int fd, int buf_mode, int buf_size)
{
	FILE *stream = (FILE *)malloc(sizeof(FILE));
	stream->magic = FILE_STREAM_MAGIC;
	stream->fd = fd;
	stream->error = 0;
	stream->buf_mode = buf_mode;
	stream->buffer = NULL;
	stream->buf_size = buf_size;
	stream->start = 0;
	stream->end = 0;
	stream->pos = 0;
	stream->prev_op = 0;
	stream->phandle = NULL;
	RtlInitializeCriticalSection(&(stream->critical));
	insert_stream(stream);
	return stream;
}

void insert_stream(FILE *stream)
{
	// stream won't be null here
	EnterCriticalSection(&_wlibc_stdio_critical);
	stream->prev = _wlibc_stdio_head;
	stream->next = NULL;
	if (_wlibc_stdio_head != NULL)
	{
		_wlibc_stdio_head->next = stream;
	}
	_wlibc_stdio_head = stream;
	LeaveCriticalSection(&_wlibc_stdio_critical);
}

void delete_stream(FILE *stream)
{
	// stream won't be null here
	EnterCriticalSection(&_wlibc_stdio_critical);

	if (stream->prev != NULL && stream->next != NULL)
	{
		stream->prev->next = stream->next;
		stream->next->prev = stream->prev;
	}
	else if (stream->prev != NULL && stream->next == NULL)
	{
		stream->prev->next = NULL;
		_wlibc_stdio_head = stream->prev;
	}
	else if (stream->prev == NULL && stream->next != NULL)
	{
		stream->next->prev = NULL;
	}
	// else (stream->prev == NULL && stream->next == NULL) fallthrough
	RtlDeleteCriticalSection(&(stream->critical));
	free(stream);

	LeaveCriticalSection(&_wlibc_stdio_critical);
}
