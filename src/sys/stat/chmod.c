/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <internal/path.h>
#include <internal/security.h>
#include <stdlib.h>
#include <sys/stat.h>

int do_chmod(HANDLE handle, mode_t mode)
{
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	FILE_ATTRIBUTE_TAG_INFORMATION attr_info;
	PSECURITY_DESCRIPTOR security_descriptor = NULL;
	int is_directory = 0; // assume regular file

	status = NtQueryInformationFile(handle, &io, &attr_info, sizeof(FILE_ATTRIBUTE_TAG_INFORMATION), FileAttributeTagInformation);
	if (status == STATUS_SUCCESS)
	{
		if (attr_info.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			is_directory = 1;
		}
	}
	else
	{
		map_ntstatus_to_errno(status);
		// do not return yet
	}

	security_descriptor = get_security_descriptor(mode, is_directory);

	status = NtSetSecurityObject(handle, DACL_SECURITY_INFORMATION, security_descriptor);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}

int common_chmod(int dirfd, const char *path, mode_t mode, int flags)
{

	HANDLE handle =
		just_open(dirfd, path, FILE_READ_ATTRIBUTES | READ_CONTROL | WRITE_DAC, flags == AT_SYMLINK_NOFOLLOW ? FILE_OPEN_REPARSE_POINT : 0);
	if (handle == INVALID_HANDLE_VALUE)
	{
		// errno wil be set by just_open
		return -1;
	}

	int result = do_chmod(handle, mode);
	NtClose(handle);
	return result;
}

int wlibc_common_chmod(int dirfd, const char *path, mode_t mode, int flags)
{
	if (flags != 0 && flags != AT_EMPTY_PATH && flags != AT_SYMLINK_NOFOLLOW)
	{
		errno = EINVAL;
		return -1;
	}

	if (flags != AT_EMPTY_PATH)
	{
		VALIDATE_PATH_AND_DIRFD(path, dirfd);
		return common_chmod(dirfd, path, mode, flags);
	}
	else
	{
		handle_t type = get_fd_type(dirfd);
		if (type == PIPE_HANDLE || type == CONSOLE_HANDLE)
		{
			errno = EBADF;
			return -1;
		}

		// The fd is validated, this call will not fail.
		UNICODE_STRING *ntpath = xget_fd_ntpath(dirfd);
		// 'open' does not give WRITE_DAC permission, reopen the file with 'WRITE_DAC'.
		HANDLE handle = just_open2(ntpath, FILE_READ_ATTRIBUTES | READ_CONTROL | WRITE_DAC, 0);
		if (handle == INVALID_HANDLE_VALUE)
		{
			// errno wil be set by `just_open2`.
			return -1;
		}

		int result = do_chmod(handle, mode);
		free(ntpath);
		NtClose(handle);

		return result;
	}
}
