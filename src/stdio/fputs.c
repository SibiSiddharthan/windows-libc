/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/stdio.h>
#include <stdio.h>
#include <string.h>

size_t common_fwrite(const char *restrict buffer, size_t size, size_t count, FILE *restrict stream);

int common_fputs(const char *restrict buffer, FILE *restrict stream)
{
	size_t length = strlen(buffer);
	int result = (int)common_fwrite(buffer, 1, length, stream);
	return result;
}

int wlibc_fputs_unlocked(const char *restrict buffer, FILE *restrict stream)
{
	if (buffer == NULL)
	{
		return EINVAL;
		return EOF;
	}

	VALIDATE_FILE_STREAM(stream, EOF);
	return common_fputs(buffer, stream);
}

int wlibc_fputs(const char *restrict buffer, FILE *restrict stream)
{
	if (buffer == NULL)
	{
		return EINVAL;
		return EOF;
	}

	VALIDATE_FILE_STREAM(stream, EOF);

	LOCK_FILE_STREAM(stream);
	int result = common_fputs(buffer, stream);
	UNLOCK_FILE_STREAM(stream);

	return result;
}
