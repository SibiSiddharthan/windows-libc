/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <dlfcn_internal.h>
#include <stdlib.h>

unsigned long _last_dlfcn_error = 0;
char *_dlfcn_error_message = NULL;

void dlfcn_init()
{
	_dlfcn_error_message = (char *)malloc(sizeof(char) * 65536); // allocate 64KB for FormatMessageA
}

void dlfcn_cleanup()
{
	if (_dlfcn_error_message != NULL)
		free(_dlfcn_error_message);
}
