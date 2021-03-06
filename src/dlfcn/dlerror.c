/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <dlfcn.h>
#include <dlfcn_internal.h>
#include <Windows.h>
#include <wlibc_errors.h>

char *wlibc_dlerror()
{
	if (_last_dlfcn_error == ERROR_SUCCESS)
	{
		return NULL;
	}

	// Call the ANSI version explicitly as we are always returning char*
	DWORD length = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, _last_dlfcn_error,
								  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), _dlfcn_error_message, 65535, NULL);

	_last_dlfcn_error = ERROR_SUCCESS;

	if (length == 0)
	{
		map_win32_error_to_wlibc(GetLastError());
		return NULL;
	}

	return _dlfcn_error_message;
}
