/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <sys/mman.h>

ULONG determine_protection(int protection)
{
	if (protection == PROT_NONE)
	{
		return PAGE_NOACCESS;
	}
	if (protection == PROT_READ)
	{
		return PAGE_READONLY;
	}
	if (protection == PROT_WRITE)
	{
		return PAGE_WRITECOPY;
	}
	if (protection == PROT_EXEC)
	{
		return PAGE_EXECUTE;
	}
	if (protection == (PROT_READ | PROT_WRITE))
	{
		return PAGE_READWRITE;
	}
	if (protection == (PROT_EXEC | PROT_READ))
	{
		return PAGE_EXECUTE_READ;
	}
	if (protection == (PROT_EXEC | PROT_WRITE))
	{
		return PAGE_EXECUTE_WRITECOPY;
	}
	if (protection == (PROT_EXEC | PROT_READ | PROT_WRITE))
	{
		return PAGE_EXECUTE_READWRITE;
	}
	// Unreachable
	return 0;
}

ULONG determine_attibutes(int flags)
{
	ULONG attributes = SEC_COMMIT;

	if (flags & MAP_HUGETLB)
	{
#if 0
		//	Requires SeLockMemoryPrivelege. Put this feature on standby for now. TODO
		attributes |= SEC_LARGE_PAGES;
#endif
	}

	return attributes;
}

void *wlibc_mmap(void *address, size_t size, int protection, int flags, int fd, off_t offset)
{
	NTSTATUS status;
	HANDLE section_handle, file_handle;
	ULONG page_protection, allocation_attributes;
	LARGE_INTEGER max_size, section_offset;
	fdinfo info;

	get_fdinfo(fd, &info);

	if ((flags & MAP_ANONYMOUS) == 0 && info.type != FILE_HANDLE)
	{
		errno = EBADF;
		return MAP_FAILED;
	}

	if (protection > (PROT_READ | PROT_WRITE | PROT_EXEC))
	{
		errno = EINVAL;
		return MAP_FAILED;
	}

	if (flags & MAP_ANONYMOUS)
	{
		file_handle = NULL;
	}
	else
	{
		file_handle = info.handle;
	}

	max_size.QuadPart = size;
	page_protection = determine_protection(protection);
	allocation_attributes = determine_attibutes(flags);

	status = NtCreateSectionEx(&section_handle, SECTION_ALL_ACCESS, NULL, &max_size, page_protection, allocation_attributes, file_handle,
							   NULL, 0);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return MAP_FAILED;
	}

	section_offset.QuadPart = offset;

	status = NtMapViewOfSectionEx(section_handle, NtCurrentProcess(), &address, &section_offset, &size, 0, page_protection, NULL, 0);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return MAP_FAILED;
	}

	// The section handle can be closed here itself as no subsequent memory operations require it.
	// In future if we try to implement mremap we might need this.
	NtClose(section_handle);

	return address;
}
