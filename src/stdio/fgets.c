/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdio.h>
#include <internal/stdio.h>
#include <string.h>

size_t common_fread(void *restrict buffer, size_t size, size_t count, FILE *restrict stream);
int common_fseek(FILE *stream, ssize_t offset, int whence);

char *common_fgets(char *restrict buffer, size_t count, FILE *restrict stream)
{
	int result = common_fread(buffer, 1, count - 1, stream);
	for (int i = 0; i < result; i++)
	{
		if (buffer[i] == '\n')
		{
			buffer[i + 1] = '\0';                              // This will not cause a buffer overflow as i < count-1 always
			common_fseek(stream, -(result - i - 1), SEEK_CUR); // set stream pos, and clear eof
			return buffer;
		}
	}
	buffer[result] = '\0';
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
