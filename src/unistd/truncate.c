/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <internal/fcntl.h>
#include <Windows.h>
#include <internal/error.h>
#include <stdlib.h>
#include <internal/nt.h>

static int common_truncate(HANDLE handle, off_t length)
{
	// Let's keep this below snippet for reference
#if 0
	LARGE_INTEGER offset, newpos;
	offset.QuadPart = 0;

	if (!SetFilePointerEx(file, offset, &newpos, FILE_END))
	{
		map_win32_error_to_wlibc(GetLastError());
		return -1;
	}

	// length <= file size. Move to specified position and set end of file
	if (newpos.QuadPart >= length)
	{
		offset.QuadPart = length;
		if (!SetFilePointerEx(file, offset, &newpos, FILE_BEGIN))
		{
			map_win32_error_to_wlibc(GetLastError());
			return -1;
		}
		if (!SetEndOfFile(file))
		{
			map_win32_error_to_wlibc(GetLastError());
			return -1;
		}
	}

	// length > file size. Move to end of file and write '0' till length is filled
	else
	{
		char *buf = (char *)malloc(sizeof(char) * (length - newpos.QuadPart));
		memset(buf, 0, length - newpos.QuadPart);
		DWORD bytes_written;
		BOOL status = WriteFile(file, buf, length - newpos.QuadPart, &bytes_written, NULL);
		if (!status)
		{
			map_win32_error_to_wlibc(GetLastError());
			free(buf);
			return -1;
		}
		free(buf);
	}
#endif

	IO_STATUS_BLOCK I;
	FILE_END_OF_FILE_INFORMATION eof_info;
	eof_info.EndOfFile.QuadPart = length;
	// This does all the work neccessary.
	// `SetFileInformationByHandle` with `FileEndofFileInfo` can also be used.
	NTSTATUS status = NtSetInformationFile(handle, &I, &eof_info, sizeof(FILE_END_OF_FILE_INFORMATION), FileEndOfFileInformation);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}

static int do_ftruncate(HANDLE handle, off_t length)
{
	NTSTATUS status;
	IO_STATUS_BLOCK I;
	FILE_POSITION_INFORMATION pos_info;

	status = NtQueryInformationFile(handle, &I, &pos_info, sizeof(FILE_POSITION_INFORMATION), FilePositionInformation);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	status = common_truncate(handle, length);
	if (status != 0)
	{
		return -1;
	}

	// truncate does not alter the file position
	status = NtSetInformationFile(handle, &I, &pos_info, sizeof(FILE_POSITION_INFORMATION), FilePositionInformation);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}

int wlibc_truncate(const char *path, off_t length)
{
	VALIDATE_PATH(path, ENOENT, -1);

	wchar_t *u16_ntpath = get_absolute_ntpath(AT_FDCWD, path);
	if (u16_ntpath == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	HANDLE handle = just_open(u16_ntpath, FILE_WRITE_DATA | FILE_APPEND_DATA | SYNCHRONIZE, 0, FILE_OPEN,
							  FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);
	if (handle == INVALID_HANDLE_VALUE)
	{
		// errno wil be set by just_open
		return -1;
	}

	int result = common_truncate(handle, length);

	NtClose(handle);
	return result;
}

int wlibc_ftruncate(int fd, off_t length)
{
	if (get_fd_type(fd) != FILE_HANDLE)
	{
		errno = EBADF;
		return -1;
	}

	if ((get_fd_flags(fd) & (O_WRONLY | O_RDWR)) == 0)
	{
		errno = EACCES;
		return -1;
	}

	HANDLE handle = get_fd_handle(fd);
	return do_ftruncate(handle, length);
}
