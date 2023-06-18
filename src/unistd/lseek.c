/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <errno.h>
#include <unistd.h>

off_t wlibc_lseek(int fd, off_t offset, int whence)
{
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	HANDLE handle;
	FILE_POSITION_INFORMATION pos_info;
	LONGLONG current_pos = 0;
	fdinfo info;

	get_fdinfo(fd, &info);

	if (info.type == INVALID_HANDLE)
	{
		errno = EBADF;
		return -1;
	}
	// Fail if a pipe fd is given
	if (info.type == PIPE_HANDLE)
	{
		errno = ESPIPE;
		return -1;
	}

	if (whence < 0 || whence > 2)
	{
		errno = EINVAL;
		return -1;
	}

	handle = info.handle;

	switch (whence)
	{
	case SEEK_SET:
		if (offset < 0)
		{
			errno = EINVAL;
			return -1;
		}
		current_pos = 0;
		break;
	case SEEK_CUR:
		FILE_POSITION_INFORMATION curpos_info;
		status = NtQueryInformationFile(handle, &io, &curpos_info, sizeof(FILE_POSITION_INFORMATION), FilePositionInformation);
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}
		current_pos = curpos_info.CurrentByteOffset.QuadPart;
		break;

	case SEEK_END:
		FILE_STANDARD_INFORMATION standard_info;
		status = NtQueryInformationFile(handle, &io, &standard_info, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation);
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}
		current_pos = standard_info.EndOfFile.QuadPart;
		break;
	}

	pos_info.CurrentByteOffset.QuadPart = current_pos + offset;
	status = NtSetInformationFile(handle, &io, &pos_info, sizeof(FILE_POSITION_INFORMATION), FilePositionInformation);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

#if 0
	LARGE_INTEGER _offset, _newpos;
	_offset.QuadPart = offset;
	if (!SetFilePointerEx(file, _offset, &_newpos, whence))
	{
		map_doserror_to_errno(GetLastError());
		return -1;
	}
	return _newpos.QuadPart;
#endif
	return pos_info.CurrentByteOffset.QuadPart;
}
