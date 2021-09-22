/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdio.h>
#include <internal/stdio.h>

size_t common_fread(void *restrict buffer, size_t size, size_t count, FILE *restrict stream);

char *common_fgets(void *restrict buffer, size_t count, FILE *restrict stream)
{
	common_fread(buffer, 1, count, stream);
	return buffer;
}

char *wlibc_fgets_unlocked(void *restrict buffer, size_t count, FILE *restrict stream)
{
	if(buffer == NULL)
	{
		errno = EINVAL;
		return NULL;
	}

	VALIDATE_FILE_STREAM(stream, NULL);
	return common_fgets(buffer, count, stream);
}

char *wlibc_fgets(void *restrict buffer, size_t count, FILE *restrict stream)
{
	if(buffer == NULL)
	{
		errno = EINVAL;
		return NULL;
	}

	VALIDATE_FILE_STREAM(stream, NULL);
	LOCK_FILE_STREAM(stream);
	char *buf = common_fgets(buffer, count, stream);
	UNLOCK_FILE_STREAM(stream);
	return buf;
}
