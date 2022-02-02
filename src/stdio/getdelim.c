/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/fcntl.h>
#include <internal/stdio.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int common_fgetc(FILE *stream);
int common_fseek(FILE *stream, ssize_t offset, int whence);
size_t common_fread(void *restrict buffer, size_t size, size_t count, FILE *restrict stream);

ssize_t common_getdelim(char **restrict buffer, size_t *restrict size, int delimiter, FILE *restrict stream)
{
	char ch = 0;
	ssize_t result = 0;
	size_t buffer_size = *size;

	if (*buffer == NULL)
	{
		// Allocate 512 bytes initially
		buffer_size = 512;
		*buffer = (char *)malloc(buffer_size);
		*size = buffer_size;
	}

	/*
	 TODO : Improve preformance of this function.
	 NOTE : While doing so keep in mind that this needs to work with pipes as well.
	*/
	while (1)
	{
		ch = common_fgetc(stream);
		if (ch == EOF)
		{
			result = -1;
			break;
		}

		(*buffer)[result] = ch;
		++result;

		if (ch == delimiter)
		{
			break;
		}

		if (result == *size)
		{
			// Double the buffer
			char *temp = (char *)malloc(buffer_size * 2);
			memcpy(temp, *buffer, buffer_size);
			free(*buffer);
			*buffer = temp;
			buffer_size *= 2;
			*size = buffer_size;
		}
	}

	if (result != -1)
	{
		(*buffer)[result] = '\0';
	}

	return result;
}

ssize_t wlibc_getdelim(char **restrict buffer, size_t *restrict size, int delimiter, FILE *restrict stream)
{
	ssize_t result;

	if (buffer == NULL || size == NULL || (*buffer != NULL && *size == 0))
	{
		errno = EINVAL;
		return -1;
	}
	VALIDATE_FILE_STREAM(stream, -1);

	LOCK_FILE_STREAM(stream);
	result = common_getdelim(buffer, size, delimiter, stream);
	UNLOCK_FILE_STREAM(stream);

	return result;
}
