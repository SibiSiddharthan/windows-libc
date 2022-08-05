/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/fcntl.h>
#include <internal/stdio.h>

FILE *_wlibc_stdio_head = NULL;

FILE *_wlibc_stdin = NULL;
FILE *_wlibc_stdout = NULL;
FILE *_wlibc_stderr = NULL;

RTL_CRITICAL_SECTION _wlibc_stdio_critical;

static void insert_stream(FILE *stream);

void initialize_stdio(void)
{
	RtlInitializeCriticalSection(&_wlibc_stdio_critical);

	_wlibc_stdin = create_stream(0, _IOFBF | _IOBUFFER_INTERNAL | _IOBUFFER_RDONLY, 512);
	_wlibc_stdout = create_stream(1, _IOFBF | _IOBUFFER_INTERNAL | _IOBUFFER_WRONLY, 512);
	_wlibc_stderr = create_stream(2, _IONBF | _IOBUFFER_WRONLY, 0);
}

int common_fflush(FILE *stream);

void close_all_streams(void)
{
	LOCK_STDIO();

	while (_wlibc_stdio_head != NULL)
	{
		FILE *prev = _wlibc_stdio_head->prev;

		// flush buffered data
		common_fflush(_wlibc_stdio_head);

		// free internal buffers if any
		if (_wlibc_stdio_head->buffer != NULL && (_wlibc_stdio_head->buf_mode & _IOBUFFER_INTERNAL))
		{
			RtlFreeHeap(NtCurrentProcessHeap(), 0, _wlibc_stdio_head->buffer);
		}
		RtlDeleteCriticalSection(&(_wlibc_stdio_head->critical));

		// close the underlying file descriptor
		close_fd(_wlibc_stdio_head->fd);

		RtlFreeHeap(NtCurrentProcessHeap(), 0, _wlibc_stdio_head);
		_wlibc_stdio_head = prev;
	}

	UNLOCK_STDIO();
}

void cleanup_stdio(void)
{
	close_all_streams();
	RtlDeleteCriticalSection(&_wlibc_stdio_critical);
}

FILE *create_stream(int fd, int buf_mode, int buf_size)
{
	FILE *stream = (FILE *)RtlAllocateHeap(NtCurrentProcessHeap(), HEAP_ZERO_MEMORY, sizeof(FILE));

	if (stream == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}

	stream->magic = FILE_STREAM_MAGIC;
	stream->fd = fd;
	stream->buf_mode = buf_mode;
	stream->buf_size = buf_size;

	RtlInitializeCriticalSection(&(stream->critical));
	insert_stream(stream);

	return stream;
}

void insert_stream(FILE *stream)
{
	// stream won't be null here.
	LOCK_STDIO();

	stream->prev = _wlibc_stdio_head;
	stream->next = NULL;

	if (_wlibc_stdio_head != NULL)
	{
		_wlibc_stdio_head->next = stream;
	}

	_wlibc_stdio_head = stream;

	UNLOCK_STDIO();
}

void delete_stream(FILE *stream)
{
	// stream won't be null here.
	LOCK_STDIO();

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
	else // if(stream->prev == NULL && stream->next == NULL)
	{
		_wlibc_stdio_head = NULL;
	}

	RtlDeleteCriticalSection(&(stream->critical));
	RtlFreeHeap(NtCurrentProcessHeap(), 0, stream);

	UNLOCK_STDIO();
}
