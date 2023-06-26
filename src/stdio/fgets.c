/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/stdio.h>
#include <stdio.h>
#include <string.h>

int common_fgetc(FILE *stream);

char *common_fgets(char *restrict buffer, size_t count, FILE *restrict stream)
{
	char ch = 0;
	size_t read_count = 0;

	while (ch != '\n' && read_count + 1 < count)
	{
		ch = (char)common_fgetc(stream);
		if (ch == EOF)
		{
			break;
		}
		buffer[read_count++] = ch;
	}

	buffer[read_count] = '\0';
	return buffer;
}

char *wlibc_fgets_unlocked(char *restrict buffer, size_t count, FILE *restrict stream)
{
	if (buffer == NULL || count < 1)
	{
		errno = EINVAL;
		return NULL;
	}

	if (count == 1)
	{
		buffer[0] = '\0';
		return buffer;
	}

	VALIDATE_FILE_STREAM(stream, NULL);
	return common_fgets(buffer, count, stream);
}

char *wlibc_fgets(char *restrict buffer, size_t count, FILE *restrict stream)
{
	if (buffer == NULL || count < 1)
	{
		errno = EINVAL;
		return NULL;
	}

	if (count == 1)
	{
		buffer[0] = '\0';
		return buffer;
	}

	VALIDATE_FILE_STREAM(stream, NULL);
	LOCK_FILE_STREAM(stream);
	char *buf = common_fgets(buffer, count, stream);
	UNLOCK_FILE_STREAM(stream);
	return buf;
}
