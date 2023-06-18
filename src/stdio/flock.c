/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/stdio.h>
#include <stdio.h>
#include <errno.h>

#define LOCK    0
#define UNLOCK  1
#define TRYLOCK 2

int wlibc_lockfile_op(FILE *stream, int op)
{
	VALIDATE_FILE_STREAM(stream, EOF);

	if (op == LOCK)
	{
		RtlEnterCriticalSection(&(stream->critical));
		return 0;
	}
	if (op == UNLOCK)
	{
		RtlLeaveCriticalSection(&(stream->critical));
		return 0;
	}
	if (op == TRYLOCK)
	{
		BOOLEAN result = RtlTryEnterCriticalSection(&(stream->critical));
		if (result == 0)
		{
			return -1; // Another thread holds the lock, fail by return non-zero
		}
		return 0;
	}

	// Unreachable
	return 0;
}
