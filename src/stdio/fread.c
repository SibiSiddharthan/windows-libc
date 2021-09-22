/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdio.h>
#include <internal/stdio.h>
#include <unistd.h>
#include <stdlib.h>

int common_fflush(FILE *stream);

size_t common_fread(void *buffer, size_t size, size_t count, FILE *stream)
{
	ssize_t result = 0;

	// unbuffered read stream
	if ((stream->buf_mode & _IONBF))
	{

		result = read(stream->fd, buffer, size * count);

		if (result != -1)
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
				read_result = read(stream->fd, (char *)buffer + bytes_read, stream->buf_size);
				if (read_result <= 0) // EOF or ERROR
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
				// read the last block into the stream buffer
				read_result = read(stream->fd, stream->buffer, stream->buf_size);

				if (read_result > 0)
				{
					size_t remaining_copy_size = (data_size - bytes_read) < read_result ? (data_size - bytes_read) : read_result;
					// copy only what we need into the target buffer;
					memcpy((char *)buffer + bytes_read, stream->buffer, remaining_copy_size);
					stream->end = stream->pos + read_result;
					stream->start = stream->pos;
					stream->pos += remaining_copy_size;
					result += remaining_copy_size;
				}
			}
		}
	}

	if (result == -1)
	{
		stream->error = _IOERROR;
	}

	else if (result < size * count)
	{
		// check this
		stream->error = _IOEOF;
	}

	return result / size;
}

size_t wlibc_fread_unlocked(void *buffer, size_t size, size_t count, FILE *stream)
{
	if (buffer == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	if (size * count == 0)
	{
		return 0;
	}

	VALIDATE_FILE_STREAM(stream, -1);
	return common_fread(buffer, size, count, stream);
}

size_t wlibc_fread(void *buffer, size_t size, size_t count, FILE *stream)
{
	if (buffer == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	if (size * count == 0)
	{
		return 0;
	}

	VALIDATE_FILE_STREAM(stream, -1);

	LOCK_FILE_STREAM(stream);
	size_t ret = common_fread(buffer, size, count, stream);
	UNLOCK_FILE_STREAM(stream);

	return ret;
}
