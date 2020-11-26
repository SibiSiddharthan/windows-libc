/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <dlfcn.h>
#include <dlfcn_internal.h>
#include <Windows.h>
#include <wlibc_errors.h>

void *wlibc_dlsym(void *handle, const char *symbol)
{
	void *ptr = GetProcAddress((HMODULE)handle, symbol);
	if (ptr == NULL)
	{
		_last_dlfcn_error = GetLastError();
		map_win32_error_to_wlibc(_last_dlfcn_error);
		return NULL;
	}
	return ptr;
}
