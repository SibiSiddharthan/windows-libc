/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdio.h>
#include <internal/stdio.h>

size_t common_fwrite(void *buffer, size_t size, size_t count, FILE *stream);

int common_fputc(int ch, FILE *stream)
{
	size_t res = common_fwrite(&ch, 1, 1, stream);
	if (res != 1)
	{
		return EOF;
	}

	return ch;
}

int wlibc_fputc_unlocked(int ch, FILE *stream)
{
	VALIDATE_FILE_STREAM(stream, EOF);
	return common_fputc(ch, stream);
}

int wlibc_fputc(int ch, FILE *stream)
{
	VALIDATE_FILE_STREAM(stream, EOF);
	LOCK_FILE_STREAM(stream);
	int result = common_fputc(ch, stream);
	UNLOCK_FILE_STREAM(stream);
	return result;
}
