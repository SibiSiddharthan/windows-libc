/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/fcntl.h>
#include <internal/stdio.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int common_fflush(FILE *stream);

int common_setvbuf(FILE *restrict stream, char *restrict buffer, int mode, size_t size)
{
	int buf_type = stream->buf_mode & (_IOBUFFER_RDONLY | _IOBUFFER_WRONLY | _IOBUFFER_RDWR);
	int prev_mode = stream->buf_mode & (_IONBF | _IOLBF | _IOFBF);

	// Always seek first
	lseek(stream->fd, stream->pos, SEEK_SET);
	stream->start = stream->pos;
	stream->end = stream->pos;

	if (mode == _IONBF) // no buffering
	{
		stream->buf_mode = _IONBF;
		stream->buf_size = 0;
		stream->buffer = NULL;

		// free any internal buffer
		if (stream->buffer != NULL && (stream->buf_mode & _IOBUFFER_INTERNAL))
		{
			free(stream->buffer);
		}
	}
	else // buffering
	{
		if (buffer == NULL)
		{
			if (size != 0)
			{
				if (stream->buf_mode & _IOBUFFER_EXTERNAL)
				{
					stream->buffer = malloc(size);
					stream->buf_size = size;
					stream->buf_mode = mode | _IOBUFFER_INTERNAL | _IOBUFFER_ALLOCATED;
				}
				// resize internal buffer
				if (stream->buf_mode & _IOBUFFER_INTERNAL)
				{
					if (size != stream->buf_size)
					{
						if (stream->buf_mode & _IOBUFFER_ALLOCATED)
						{
							stream->buffer = realloc(stream->buffer, size);
							stream->buf_size = size;
							stream->buf_mode = mode | _IOBUFFER_INTERNAL | _IOBUFFER_ALLOCATED;
						}
						else
						{
							stream->buffer = malloc(size);
							stream->buf_size = size;
							stream->buf_mode = mode | _IOBUFFER_INTERNAL | _IOBUFFER_ALLOCATED;
						}
					}
				}
			}
			else
			{
				// Buffer was NULL, size = 0, mode was not _IONBF
				errno = EINVAL;
				return -1;
			}
		}
		else
		{
			// Use the user provided buffer
			if (stream->buffer != NULL && (stream->buf_mode & _IOBUFFER_INTERNAL))
			{
				free(stream->buffer);
			}
			stream->buffer = buffer;
			stream->buf_size = size;
			stream->buf_mode = mode | _IOBUFFER_EXTERNAL;
		}
	}

	stream->buf_mode |= buf_type;

	return 0;
}

int wlibc_setvbuf(FILE *restrict stream, char *restrict buffer, int mode, size_t size)
{
	VALIDATE_FILE_STREAM(stream, -1);

	if (!(mode == _IONBF || mode == _IOFBF || mode == _IOLBF))
	{
		errno = EINVAL;
		return -1;
	}

	LOCK_FILE_STREAM(stream);
	common_fflush(stream);
	int result = common_setvbuf(stream, buffer, mode, size);
	UNLOCK_FILE_STREAM(stream);

	return result;
}
