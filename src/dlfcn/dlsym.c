/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/validate.h>
#include <dlfcn.h>
#include <errno.h>

void *wlibc_dlsym(void *restrict handle, const char *restrict symbol)
{
	NTSTATUS status;
	UTF8_STRING u8_symbol;
	PVOID procedure = NULL;

	VALIDATE_PTR(handle, EINVAL, NULL); // We know that handle can't be 0.
	VALIDATE_STRING(symbol, EINVAL, NULL);

	RtlInitUTF8String(&u8_symbol, symbol);
	status = LdrGetProcedureAddressForCaller(handle, &u8_symbol, 0, &procedure, 0, NULL);
	if (status != STATUS_SUCCESS)
	{
		_wlibc_last_dlfcn_error = status;
		return NULL;
	}

	return procedure;
}
