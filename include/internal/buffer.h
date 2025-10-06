/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_BUFFER_INTERNAL_H
#define WLIBC_BUFFER_INTERNAL_H

#include <internal/minmax.h>
#include <internal/ptr.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct _buffer_t
{
	uint8_t *data;
	size_t pos;
	size_t size;

	void *ctx;
	size_t (*read)(struct _buffer_t *buffer, size_t size);
	size_t (*write)(struct _buffer_t *buffer, size_t size);
	uint32_t error;
} buffer_t;

static inline void advance(buffer_t *buffer, size_t step)
{
	buffer->pos = MIN(buffer->pos + step, buffer->size);
}

static inline size_t pending(buffer_t *buffer)
{
	return buffer->size - buffer->pos;
}

static inline void *current(buffer_t *buffer)
{
	return PTR_OFFSET(buffer->data, buffer->pos);
}

static inline void flush(buffer_t *buffer)
{
	buffer->write(buffer, 0);
}

static inline uint8_t readbyte(buffer_t *buffer)
{
	if ((buffer->pos + 1) > buffer->size)
	{
		return 0;
	}

	return buffer->data[buffer->pos++];
}

static inline uint8_t peekbyte(buffer_t *buffer, uint32_t offset)
{
	if ((buffer->pos + offset) >= buffer->size)
	{
		return 0;
	}

	return buffer->data[buffer->pos + offset];
}

static inline size_t writebyte(buffer_t *buffer, uint8_t byte)
{
	if ((buffer->pos + 1) > buffer->size)
	{
		if (buffer->write == NULL)
		{
			return 0;
		}

		buffer->write(buffer, 1);

		if (buffer->error)
		{
			return 0;
		}
	}

	buffer->data[buffer->pos] = byte;
	buffer->pos += 1;

	return 1;
}

static inline size_t writen(buffer_t *buffer, void *in, size_t size)
{
	if ((buffer->pos + size) > buffer->size)
	{
		if (buffer->write == NULL)
		{
			return 0;
		}

		buffer->write(buffer, size);

		if (buffer->error)
		{
			return 0;
		}
	}

	memcpy(buffer->data + buffer->pos, in, size);
	buffer->pos += size;

	return size;
}

size_t memory_buffer_write(buffer_t *buffer, size_t size);
