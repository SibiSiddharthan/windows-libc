/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>

int do_chflags(HANDLE handle, uint32_t attributes)
{
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	FILE_BASIC_INFORMATION basic_info;

	status = NtQueryInformationFile(handle, &io, &basic_info, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	basic_info.FileAttributes = attributes;

	status = NtSetInformationFile(handle, &io, &basic_info, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}

int common_chflags(int dirfd, const char *path, uint32_t attributes, int flags)
{
	HANDLE handle = just_open(dirfd, path, FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES, flags == AT_SYMLINK_NOFOLLOW ? FILE_OPEN_REPARSE_POINT : 0);
	if (handle == INVALID_HANDLE_VALUE)
	{
		// errno wil be set by just_open
		return -1;
	}

	int result = do_chflags(handle, attributes);
	NtClose(handle);
	return result;
}

int wlibc_common_chflags(int dirfd, const char *path, uint32_t attributes, int flags)
{
	if (flags != 0 && flags != AT_EMPTY_PATH && flags != AT_SYMLINK_NOFOLLOW)
	{
		errno = EINVAL;
		return -1;
	}

	if (attributes > S_IA_SETTABLE)
	{
		errno = EINVAL;
		return -1;
	}

	if (flags != AT_EMPTY_PATH)
	{
		VALIDATE_PATH_AND_DIRFD(path, dirfd);
		return common_chflags(dirfd, path, attributes, flags);
	}
	else
	{
		enum handle_type type = get_fd_type(dirfd);
		if (type != FILE_HANDLE && type != DIRECTORY_HANDLE)
		{
			errno = EBADF;
			return -1;
		}

		HANDLE handle = get_fd_handle(dirfd);
		return do_chflags(handle, attributes);
	}
}
