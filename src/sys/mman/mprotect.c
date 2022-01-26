/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <sys/mman.h>

// From mmap.c
ULONG determine_protection(int protection);

int wlibc_mprotect(void *address, size_t size, int protection)
{
	NTSTATUS status;
	ULONG new_protection, old_protection;

	if (protection > (PROT_READ | PROT_WRITE | PROT_EXEC))
	{
		errno = EINVAL;
		return -1;
	}

	new_protection = determine_protection(protection);

	status = NtProtectVirtualMemory(NtCurrentProcess(), &address, &size, new_protection, &old_protection);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}
