/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <internal/misc.h>
#include <Windows.h>
#include <internal/error.h>
#include <internal/nt.h>
#include <fcntl.h>
#include <internal/fcntl.h>

wchar_t *get_absolute_ntpath(int dirfd, const char *path);
HANDLE just_open(const wchar_t *u16_ntpath, ACCESS_MASK access, ULONG attributes, ULONG disposition, ULONG options);

int common_removeat(int dirfd, const char *path, int flags)
{
	wchar_t *u16_ntpath = get_absolute_ntpath(dirfd, path);
	if (u16_ntpath == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	ULONG options = FILE_OPEN_REPARSE_POINT;

	switch (flags)
	{
	case 0:
		options |= FILE_NON_DIRECTORY_FILE;
		break;
	case AT_REMOVEDIR:
		options |= FILE_DIRECTORY_FILE;
		break;
	default: // AT_REMOVEANY
		break;
	}

	HANDLE handle = just_open(u16_ntpath, DELETE, 0, FILE_OPEN, options);
	free(u16_ntpath);

	if (handle == INVALID_HANDLE_VALUE)
	{
		// errno wil be set by just_open
		return -1;
	}

	IO_STATUS_BLOCK I;
	FILE_DISPOSITION_INFORMATION_EX dispostion;
	dispostion.Flags = FILE_DISPOSITION_DELETE | FILE_DISPOSITION_POSIX_SEMANTICS | FILE_DISPOSITION_IGNORE_READONLY_ATTRIBUTE;
	NTSTATUS status = NtSetInformationFile(handle, &I, &dispostion, sizeof(FILE_DISPOSITION_INFORMATION_EX), FileDispositionInformationEx);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}
	NtClose(handle);

	return 0;
}

int wlibc_common_removeat(int dirfd, const char *path, int flags)
{
	if (path == NULL || path[0] == '\0')
	{
		errno = ENOENT;
		return -1;
	}

	if (flags != 0 && flags != AT_REMOVEDIR && flags != AT_REMOVEANY)
	{
		errno = EINVAL;
		return -1;
	}

	if (dirfd != AT_FDCWD && get_fd_type(dirfd) != DIRECTORY_HANDLE)
	{
		errno = ENOTDIR;
		return -1;
	}

	return common_removeat(dirfd, path, flags);
}
