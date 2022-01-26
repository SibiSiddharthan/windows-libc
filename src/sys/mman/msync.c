/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <sys/mman.h>

int wlibc_msync(void *address, size_t size, int flags)
{
	NTSTATUS status;
	IO_STATUS_BLOCK io;

	if (flags > MS_SYNC)
	{
		errno = EINVAL;
		return -1;
	}

	status = NtFlushVirtualMemory(NtCurrentProcess(), &address, &size, &io);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}
