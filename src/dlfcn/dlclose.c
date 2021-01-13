/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <dlfcn.h>
#include <windows.h>

int wlibc_dlclose(void *handle)
{
	if (FreeLibrary((HMODULE)handle)) // success -> non zero
	{
		return 0;
	}
	else
	{
		return -1;
	}
}
