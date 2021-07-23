/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <dlfcn.h>
#include <internal/dlfcn.h>
#include <Windows.h>
#include <wchar.h>
#include <internal/error.h>
#include <internal/misc.h>

void *wlibc_dlopen(const char *filename, int flags /*unused*/)
{
	if (filename == NULL)
	{
		return NULL;
	}

	wchar_t *wfilename = mb_to_wc(filename);

	HMODULE module = LoadLibrary(wfilename);
	if (module == NULL)
	{
		_last_dlfcn_error = GetLastError();
		map_win32_error_to_wlibc(_last_dlfcn_error);
		return NULL;
	}

	free(wfilename);
	return (void *)module;
}
