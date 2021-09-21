/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdio.h>
#include <internal/stdio.h>

int common_ferror(FILE *stream)
{
	return stream->error == _IOERROR ? _IOERROR : 0;
}

int wlibc_ferror_unlocked(FILE *stream)
{
	VALIDATE_FILE_STREAM(stream, 0);
	return common_ferror(stream);
}

int wlibc_ferror(FILE *stream)
{
	VALIDATE_FILE_STREAM(stream, 0);

	LOCK_FILE_STREAM(stream);
	int error = common_ferror(stream);
	UNLOCK_FILE_STREAM(stream);

	return error;
}
