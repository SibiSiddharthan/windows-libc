/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdlib.h>
#include <string.h>

int main()
{
	char *value = getenv("TEST_ENV");

	if (value == NULL)
	{
		return 1;
	}

	if (memcmp(value, "Hello World", 11) != 0)
	{
		return 2;
	}

	return 0;
}
