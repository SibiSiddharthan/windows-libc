/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <string.h>

void *wlibc_memrchr(const void *str, int character, size_t size)
{
	// A simple memrchr implementation.
	// TODO make this fast.

	const char *start = (const char *)str;
	const char *end = (const char *)str + size;

	while (1)
	{
		--end;

		if (end < start)
		{
			break;
		}

		if (*end == character)
		{
			return (void *)end;
		}
	}

	return NULL;
}
