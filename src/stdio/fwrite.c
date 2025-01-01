/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/fcntl.h>
#include <internal/stdio.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

size_t common_fwrite(const void *restrict buffer, size_t size, size_t count, FILE *restrict stream)
{
	ssize_t result = 0;

	// An error has occured.
	if (stream->error == _IOERROR)
	{
		return 0;
	}

	// Stream was opened for reading only.
	if (stream->buf_mode & _IOBUFFER_RDONLY)
	{
		errno = EACCES;
		stream->error = _IOERROR;
		return 0;
	}

	// unbuffered stream
	if ((stream->buf_mode & _IONBF))
	{

		result = write(stream->fd, buffer, size * count);
		stream->prev_op = OP_WRITE;

		if (result != -1)
		{
			stream->pos += result;
			stream->start = stream->pos;
			stream->end = stream->pos;
		}
	}
	else
	{

		if (stream->prev_op != OP_WRITE) // OP_READ or 'nothing'
		{
			// Seek to end of file if we are 'starting' to append.
			if (get_fd_flags(stream->fd) & O_APPEND)
			{
				stream->pos = lseek(stream->fd, 0, SEEK_END);
			}
			// If the previous operation was a read, seek to where the stream position actually is.
			else if (stream->prev_op == OP_READ) // not appending
			{
				lseek(stream->fd, stream->pos, SEEK_SET);
			}
			stream->start = stream->pos;
			stream->end = stream->pos;
		}
		stream->prev_op = OP_WRITE;

		// allocate the buffer if not allocated already
		if ((stream->buf_mode & _IOBUFFER_INTERNAL) && ((stream->buf_mode & _IOBUFFER_ALLOCATED) == 0))
		{
			stream->buffer = (char *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(char) * stream->buf_size);
			if (stream->buffer == NULL)
			{
				errno = ENOMEM;
				return 0;
			}

			stream->buf_mode |= _IOBUFFER_ALLOCATED;
		}

		if (stream->start == stream->end && ((stream->buf_mode & _IOBUFFER_ALLOCATED) || (stream->buf_mode & _IOBUFFER_EXTERNAL)))
		{
			stream->start = stream->pos;
			stream->end = stream->pos + stream->buf_size;
		}

		size_t data_size = size * count;

		if (data_size <= stream->end - stream->pos)
		{
			memcpy(stream->buffer + (stream->pos - stream->start), buffer, data_size);
			stream->pos += data_size;
			return count;
		}

		else // if (data_size > stream->end - stream->pos)
		{
			size_t bytes_written = 0;
			ssize_t write_result = 1;

			// fill out the write buffer first if not empty.
			if (stream->end - stream->pos != stream->buf_size)
			{
				memcpy(stream->buffer + (stream->pos - stream->start), buffer, stream->end - stream->pos);
				if (write(stream->fd, stream->buffer, stream->buf_size) == -1)
				{
					stream->error = _IOERROR;
					return 0;
				}

				bytes_written = stream->end - stream->pos;
			}

			while (bytes_written + stream->buf_size < data_size)
			{
				// Write directly from the target buffer
				write_result = write(stream->fd, (char *)buffer + bytes_written, stream->buf_size);
				if (write_result == -1) // ERROR
				{
					break;
				}
				bytes_written += write_result;
			}
			result = bytes_written;
			stream->pos += bytes_written;
			// writes until this point are not buffered by us
			stream->start = stream->pos;
			stream->end = stream->pos;

			if (write_result > 0)
			{
				// copy the remaining bytes into the stream buffer
				size_t remaining_copy_size = (data_size - bytes_written);
				memcpy(stream->buffer, (char *)buffer + bytes_written, remaining_copy_size);
				stream->end = stream->pos + stream->buf_size;
				stream->start = stream->pos;
				stream->pos += remaining_copy_size;
				result += remaining_copy_size;
			}
		}
	}

	if (result == -1)
	{
		stream->error = _IOERROR;
	}

	return result / size;
}

size_t wlibc_fwrite_unlocked(const void *restrict buffer, size_t size, size_t count, FILE *restrict stream)
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
	return common_fwrite(buffer, size, count, stream);
}

size_t wlibc_fwrite(const void *restrict buffer, size_t size, size_t count, FILE *restrict stream)
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
	size_t ret = common_fwrite(buffer, size, count, stream);
	UNLOCK_FILE_STREAM(stream);

	return ret;
}
