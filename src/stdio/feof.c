/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/stdio.h>
#include <stdio.h>

int common_feof(FILE *stream)
{
	return stream->error == _IOEOF ? _IOEOF : 0;
}

int wlibc_feof_unlocked(FILE *stream)
{
	VALIDATE_FILE_STREAM(stream, 0);
	return common_feof(stream);
}

int wlibc_feof(FILE *stream)
{
	VALIDATE_FILE_STREAM(stream, 0);

	LOCK_FILE_STREAM(stream);
	int error = common_feof(stream);
	UNLOCK_FILE_STREAM(stream);

	return error;
}
