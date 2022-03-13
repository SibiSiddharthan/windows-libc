/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <errno.h>
#include <unistd.h>

ssize_t wlibc_pread(int fd, void *buf, size_t count, off_t offset)
{
	ssize_t result = 0;
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	HANDLE handle;
	LARGE_INTEGER byte_offset;
	FILE_POSITION_INFORMATION pos_info;
	fdinfo info;

	if (buf == NULL)
	{
		errno = EFAULT;
		return -1;
	}

	get_fdinfo(fd, &info);

	switch (info.type)
	{
	case FILE_HANDLE:
	case NULL_HANDLE:
		break;
	case DIRECTORY_HANDLE:
		errno = EISDIR;
		return -1;
	case CONSOLE_HANDLE:
	case PIPE_HANDLE:
		errno = ESPIPE;
		return -1;
	case INVALID_HANDLE:
		errno = EBADF;
		return -1;
	}

	handle = info.handle;

	status = NtQueryInformationFile(handle, &io, &pos_info, sizeof(FILE_POSITION_INFORMATION), FilePositionInformation);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	byte_offset.QuadPart = offset;
	status = NtReadFile(handle, NULL, NULL, NULL, &io, buf, (ULONG)count, &byte_offset, NULL);
	if (status != STATUS_SUCCESS && status != STATUS_PENDING && status != STATUS_END_OF_FILE)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}
	result = io.Information;

	status = NtSetInformationFile(handle, &io, &pos_info, sizeof(FILE_POSITION_INFORMATION), FilePositionInformation);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return result;
}
