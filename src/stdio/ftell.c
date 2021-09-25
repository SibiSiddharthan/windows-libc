/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <internal/misc.h>
#include <errno.h>
#include <internal/stdio.h>
#include <wchar.h>
#include <fcntl.h>

ssize_t wlibc_ftell(FILE *stream)
{
	VALIDATE_FILE_STREAM(stream, EOF);
	ssize_t result;
	LOCK_FILE_STREAM(stream);
	result = stream->pos;
	UNLOCK_FILE_STREAM(stream);
	return result;
}
