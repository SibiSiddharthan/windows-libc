/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <langinfo_internal.h>
#include <stdlib.h>

char **__nl_langinfo_buf = NULL;

void langinfo_init()
{
	__nl_langinfo_buf = (char **)malloc(sizeof(char *) * MAX_LANGINFO_INVOKE_COUNT);
	for (int i = 0; i < MAX_LANGINFO_INVOKE_COUNT; i++)
	{
		__nl_langinfo_buf[i] = (char *)malloc(sizeof(char) * MAX_LANGINFO_LENGTH);
	}
}

void langinfo_cleanup()
{
	for (int i = 0; i < MAX_LANGINFO_INVOKE_COUNT; i++)
	{
		free(__nl_langinfo_buf[i]);
	}
	free(__nl_langinfo_buf);
}
