/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <dlfcn.h>

int wlibc_dlclose(void *handle)
{
	NTSTATUS status;

	status = LdrUnloadDll(handle);
	if (status != STATUS_SUCCESS)
	{
		_wlibc_last_dlfcn_error = status;
		return -1;
	}

	return 0;
}
