/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdlib.h>
#include <string.h>

char *wlibc_strndup(const char *str, size_t size)
{
	size_t length = strnlen(str, size);
	char *result = (char *)malloc(length + 1);

	if (result == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}

	// Null terminate the string.
	result[length] = '\0';
	memcpy(result, str, length);

	return result;
}
