/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <dlfcn.h>
#include <windows.h>
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