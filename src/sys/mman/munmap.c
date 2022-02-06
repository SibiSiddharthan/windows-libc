/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <sys/mman.h>

int wlibc_munmap(void *address, size_t size /* unused */)
{
	NTSTATUS status;

	status = NtUnmapViewOfSectionEx(NtCurrentProcess(), address, 0);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}
