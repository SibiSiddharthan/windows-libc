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

static int common_truncate(HANDLE handle, off_t length)
{
	// Let's keep this below snippet for reference
#if 0
	LARGE_INTEGER offset, newpos;
	offset.QuadPart = 0;

	if (!SetFilePointerEx(file, offset, &newpos, FILE_END))
	{
		map_doserror_to_errno(GetLastError());
		return -1;
	}

	// length <= file size. Move to specified position and set end of file
	if (newpos.QuadPart >= length)
	{
		offset.QuadPart = length;
		if (!SetFilePointerEx(file, offset, &newpos, FILE_BEGIN))
		{
			map_doserror_to_errno(GetLastError());
			return -1;
		}
		if (!SetEndOfFile(file))
		{
			map_doserror_to_errno(GetLastError());
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
			map_doserror_to_errno(GetLastError());
			free(buf);
			return -1;
		}
		free(buf);
	}
#endif

	NTSTATUS status;
	IO_STATUS_BLOCK io;
	FILE_END_OF_FILE_INFORMATION eof_info;

	eof_info.EndOfFile.QuadPart = length;
	// This does all the work neccessary.
	// `SetFileInformationByHandle` with `FileEndofFileInfo` can also be used.
	status = NtSetInformationFile(handle, &io, &eof_info, sizeof(FILE_END_OF_FILE_INFORMATION), FileEndOfFileInformation);
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
	IO_STATUS_BLOCK io;
	FILE_POSITION_INFORMATION pos_info;

	status = NtQueryInformationFile(handle, &io, &pos_info, sizeof(FILE_POSITION_INFORMATION), FilePositionInformation);
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
	status = NtSetInformationFile(handle, &io, &pos_info, sizeof(FILE_POSITION_INFORMATION), FilePositionInformation);
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

	HANDLE handle =
		just_open(AT_FDCWD, path, FILE_WRITE_DATA | FILE_APPEND_DATA | SYNCHRONIZE, FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);
	if (handle == NULL)
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
	fdinfo info;

	get_fdinfo(fd, &info);

	if (info.type != FILE_HANDLE)
	{
		errno = EBADF;
		return -1;
	}

	if ((info.flags & (O_WRONLY | O_RDWR)) == 0)
	{
		errno = EACCES;
		return -1;
	}

	return do_ftruncate(info.handle, length);
}
