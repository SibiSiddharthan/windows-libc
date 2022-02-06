/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <dlfcn.h>

void *wlibc_dlsym(void *restrict handle, const char *restrict symbol)
{
	if (symbol == NULL)
	{
		return NULL;
	}

	NTSTATUS status;
	UTF8_STRING u8_symbol;
	PVOID procedure = NULL;

	RtlInitUTF8String(&u8_symbol, symbol);
	status = LdrGetProcedureAddressForCaller(handle, &u8_symbol, 0, &procedure, 0, NULL);
	if (status != STATUS_SUCCESS)
	{
		_wlibc_last_dlfcn_error = status;
		return NULL;
	}

	return procedure;
}
