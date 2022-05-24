/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <internal/security.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

// From stat.c
mode_t get_permissions(ACCESS_MASK access);

int have_required_access(mode_t given_access, mode_t mode)
{
	mode_t required_access = 0;
	if (mode & X_OK)
	{
		required_access |= S_IEXEC;
	}
	if (mode & W_OK)
	{
		required_access |= S_IWRITE;
	}
	if (mode & R_OK)
	{
		required_access |= S_IREAD;
	}

	// given_access should be a superset of required_access
	if ((required_access & ~given_access) != 0)
	{
		return -1;
	}
	return 0;
}

int do_access(HANDLE handle, int mode, int flags)
{
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	FILE_STAT_INFORMATION stat_info;

	UNREFERENCED_PARAMETER(flags);

	// Use the 'EffectiveAccess' field of FILE_STAT_INFORMATION
	status = NtQueryInformationFile(handle, &io, &stat_info, sizeof(FILE_STAT_INFORMATION), FileStatInformation);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return have_required_access(get_permissions(stat_info.EffectiveAccess), mode);
}

int common_access(int dirfd, const char *path, int mode, int flags)
{
	int result = -1;

	HANDLE handle = just_open(dirfd, path, FILE_READ_ATTRIBUTES | READ_CONTROL, flags == AT_SYMLINK_NOFOLLOW ? FILE_OPEN_REPARSE_POINT : 0);
	if (handle == INVALID_HANDLE_VALUE)
	{
		// errno wil be set by just_open
		return -1;
	}

	if (mode == F_OK)
	{
		// Avoid doing any expensive checks if we are checking whether the file exists or not only.
		result = 0;
	}
	else
	{
		result = do_access(handle, mode, flags & AT_EACCESS);
	}

	NtClose(handle);
	return result;
}

int wlibc_common_access(int dirfd, const char *path, int mode, int flags)
{
	if (flags != 0 && flags != AT_SYMLINK_NOFOLLOW)
	{
		errno = EINVAL;
		return -1;
	}

	if (mode < F_OK || mode > (F_OK | R_OK | W_OK | X_OK))
	{
		errno = EINVAL;
		return -1;
	}

	VALIDATE_PATH_AND_DIRFD(path, dirfd);
	return common_access(dirfd, path, mode, flags);
}
