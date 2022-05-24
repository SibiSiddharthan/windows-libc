/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int common_fflush(FILE *stream);

static ssize_t read_wrapper(FILE *restrict stream, void *restrict buffer, size_t size)
{
	ssize_t result = read(stream->fd, buffer, size);
	// Set the stream error states
	if (result == 0)
	{
		stream->error = _IOEOF;
	}
	if (result == -1)
	{
		stream->error = _IOERROR;
		result = 0;
	}
	return result; // never -1
}

size_t common_fread(void *restrict buffer, size_t size, size_t count, FILE *restrict stream)
{
	ssize_t result = 0;

	// Stream has reached it's end or an error has occured.
	if (stream->error == _IOEOF || stream->error == _IOERROR)
	{
		return 0;
	}

	// Stream was opened for writing only.
	if (stream->buf_mode & _IOBUFFER_WRONLY)
	{
		errno = EACCES;
		stream->error = _IOERROR;
		return 0;
	}

	// unbuffered read stream
	if ((stream->buf_mode & _IONBF))
	{

		result = read_wrapper(stream, buffer, size * count);
		stream->prev_op = OP_READ;

		if (result != 0)
		{
			stream->pos += result;
			stream->start = stream->pos;
			stream->end = stream->pos;
		}
	}
	// buffered read stream
	else
	{
		// Flush the stream if we have written to it previously
		if (stream->prev_op == OP_WRITE)
		{
			common_fflush(stream);
			stream->end = stream->start;
		}
		stream->prev_op = OP_READ;

		// allocate the buffer if not allocated already
		if ((stream->buf_mode & _IOBUFFER_INTERNAL) && ((stream->buf_mode & _IOBUFFER_ALLOCATED) == 0))
		{
			stream->buffer = (char *)malloc(sizeof(char) * stream->buf_size);
			stream->buf_mode |= _IOBUFFER_ALLOCATED;
		}

		size_t data_size = size * count;

		if (data_size <= stream->end - stream->pos)
		{
			memcpy(buffer, stream->buffer + (stream->pos - stream->start), data_size);
			stream->pos += data_size;
			return count;
		}
		else // if (data_size > stream->end - stream->pos)
		{

			// copy already read data first
			size_t bytes_read = 0;
			ssize_t read_result = 1;
			if (stream->pos < stream->end)
			{
				memcpy(buffer, stream->buffer + (stream->pos - stream->start), stream->end - stream->pos);
				bytes_read = stream->end - stream->pos;
			}

			while (bytes_read + stream->buf_size < data_size)
			{
				// Store the reads directly into the target buffer
				read_result = read_wrapper(stream, (char *)buffer + bytes_read, stream->buf_size);
				if (read_result == 0) // EOF or ERROR
				{
					break;
				}
				bytes_read += read_result;
			}
			result = bytes_read;
			stream->pos += bytes_read;
			// reads until this point are not buffered by us
			stream->start = stream->pos;
			stream->end = stream->pos;

			if (read_result > 0)
			{
				size_t last_read_count = 0;
				while (last_read_count < stream->buf_size && last_read_count < (data_size - bytes_read))
				{
					// read the last block into the stream buffer
					read_result = read_wrapper(stream, stream->buffer + last_read_count, stream->buf_size - last_read_count);
					if (read_result == 0) // EOF or ERROR
					{
						break;
					}
					last_read_count += read_result;
				}

				if (last_read_count > 0)
				{
					size_t remaining_copy_size = (data_size - bytes_read) < last_read_count ? (data_size - bytes_read) : last_read_count;
					// copy only what is required into the target buffer;
					memcpy((char *)buffer + bytes_read, stream->buffer, remaining_copy_size);
					stream->end = stream->pos + last_read_count;
					stream->start = stream->pos;
					stream->pos += remaining_copy_size;
					result += remaining_copy_size;
				}
			}
		}
	}

	return result / size;
}

size_t wlibc_fread_unlocked(void *restrict buffer, size_t size, size_t count, FILE *restrict stream)
{
	if (buffer == NULL)
	{
		errno = EINVAL;
		return 0;
	}

	if (size * count == 0)
	{
		return 0;
	}

	VALIDATE_FILE_STREAM(stream, 0);
	return common_fread(buffer, size, count, stream);
}

size_t wlibc_fread(void *restrict buffer, size_t size, size_t count, FILE *restrict stream)
{
	if (buffer == NULL)
	{
		errno = EINVAL;
		return 0;
	}

	if (size * count == 0)
	{
		return 0;
	}

	VALIDATE_FILE_STREAM(stream, 0);

	LOCK_FILE_STREAM(stream);
	size_t ret = common_fread(buffer, size, count, stream);
	UNLOCK_FILE_STREAM(stream);

	return ret;
}
