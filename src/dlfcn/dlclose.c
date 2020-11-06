/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
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