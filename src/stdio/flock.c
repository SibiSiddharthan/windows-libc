/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdio.h>
#include <internal/stdio.h>
#include <errno.h>
#include <Windows.h>

#define LOCK    0
#define UNLOCK  1
#define TRYLOCK 2

int wlibc_lockfile_op(FILE *stream, int op)
{
	VALIDATE_FILE_STREAM(stream, EOF);

	if (op == LOCK)
	{
		EnterCriticalSection(&(stream->critical));
		return 0;
	}
	if (op == UNLOCK)
	{
		LeaveCriticalSection(&(stream->critical));
		return 0;
	}
	if (op == TRYLOCK)
	{
		BOOL result = TryEnterCriticalSection(&(stream->critical));
		if (result == 0)
		{
			return -1; // Another thread holds the lock, fail by return non-zero
		}
		return 0;
	}

	// Unreachable
	return 0;
}
