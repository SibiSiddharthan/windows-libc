/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <dlfcn.h>
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