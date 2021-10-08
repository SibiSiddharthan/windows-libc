/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <Windows.h>
#include <internal/error.h>
#include <errno.h>
#include <internal/fcntl.h>
#include <internal/nt.h>

off_t wlibc_lseek(int fd, off_t offset, int whence)
{
	enum handle_type type = get_fd_type(fd);
	if (type == INVALID_HANDLE)
	{
		errno = EBADF;
		return -1;
	}
	// Fail if a pipe fd is given
	if (type == PIPE_HANDLE)
	{
		errno = ESPIPE;
		return -1;
	}

	if (whence < 0 || whence > 2)
	{
		errno = EINVAL;
		return -1;
	}

	HANDLE handle = get_fd_handle(fd);
	IO_STATUS_BLOCK I;
	NTSTATUS status;
	FILE_POSITION_INFORMATION pos_info;
	LONGLONG current_pos = 0;
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
		status = NtQueryInformationFile(handle, &I, &curpos_info, sizeof(FILE_POSITION_INFORMATION), FilePositionInformation);
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}
		current_pos = curpos_info.CurrentByteOffset.QuadPart;
		break;

	case SEEK_END:
		FILE_STANDARD_INFORMATION standard_info;
		status = NtQueryInformationFile(handle, &I, &standard_info, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation);
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}
		current_pos = standard_info.EndOfFile.QuadPart;
		break;
	}

	pos_info.CurrentByteOffset.QuadPart = current_pos + offset;
	status = NtSetInformationFile(handle, &I, &pos_info, sizeof(FILE_POSITION_INFORMATION), FilePositionInformation);
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
		map_win32_error_to_wlibc(GetLastError());
		return -1;
	}
	return _newpos.QuadPart;
#endif
	return pos_info.CurrentByteOffset.QuadPart;
}
