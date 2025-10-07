/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/buffer.h>
#include <stdlib.h>
#include <string.h>

size_t memory_buffer_write(buffer_t *buffer, size_t size)
{
	size_t old_size = buffer->size;

	buffer->error = 0;

	if (size == 0)
	{
		// nop
		return 0;
	}

	if (buffer->size == 0)
	{
		// Allocate 64 bytes initially
		buffer->pos = 0;
		buffer->size = 64;

		// Grow to power of 2
		while (size > buffer->size)
		{
			buffer->size *= 2;
		}

		buffer->data = malloc(buffer->size);

		if (buffer->data == NULL)
		{
			buffer->error = 1;
			return 0;
		}

		memset(buffer->data, 0, buffer->size);
		return buffer->size;
	}

	// Grow to power of 2
	while (size > buffer->size - buffer->pos)
	{
		buffer->size *= 2;
	}

	buffer->data = realloc(buffer->data, buffer->size);

	if (buffer->data == NULL)
	{
		buffer->error = 1;
		return 0;
	}

	memset(PTR_OFFSET(buffer->data, old_size), 0, buffer->size - old_size);

	return buffer->size;
}
