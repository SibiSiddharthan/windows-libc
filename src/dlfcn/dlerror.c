/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <dlfcn.h>

unsigned long _wlibc_last_dlfcn_error = 0;

char *wlibc_dlerror(void)
{
	switch (_wlibc_last_dlfcn_error)
	{
	case STATUS_INVALID_FILE_FOR_SECTION:
		return "This file is not a valid Win32 application.";
	case STATUS_DLL_NOT_FOUND:
		return "The specified module could not be found.";
	case STATUS_ENTRYPOINT_NOT_FOUND:
	case STATUS_ORDINAL_NOT_FOUND:
		return "The specified procedure could not be found.";
	case STATUS_SUCCESS: // No error.
	default:
		return NULL;
	}
}
