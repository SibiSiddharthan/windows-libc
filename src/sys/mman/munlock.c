/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <sys/mman.h>

int wlibc_munlock(const void *address, size_t size)
{
	NTSTATUS status;

	status = NtUnlockVirtualMemory(NtCurrentProcess(), (PVOID *)&address, &size, MAP_PROCESS);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}
