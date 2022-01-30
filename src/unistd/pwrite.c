/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <unistd.h>
#include <internal/error.h>
#include <errno.h>
#include <internal/fcntl.h>
#include <fcntl.h>

ssize_t wlibc_pwrite(int fd, const void *buf, size_t count, off_t offset)
{
	if (buf == NULL)
	{
		errno = EFAULT;
		return -1;
	}

	handle_t _type = get_fd_type(fd);
	switch (_type)
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
	
	ssize_t result = 0;
	HANDLE file = get_fd_handle(fd);
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	LARGE_INTEGER byte_offset;
	FILE_POSITION_INFORMATION pos_info;

	status = NtQueryInformationFile(file, &io, &pos_info, sizeof(FILE_POSITION_INFORMATION), FilePositionInformation);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	byte_offset.QuadPart = offset;
	status = NtWriteFile(file, NULL, NULL, NULL, &io, (PVOID)buf, count, &byte_offset, NULL);
	if (status != STATUS_SUCCESS && status != STATUS_PENDING)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}
	result = io.Information;

	status = NtSetInformationFile(file, &io, &pos_info, sizeof(FILE_POSITION_INFORMATION), FilePositionInformation);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return result;
}
