/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#define _CRT_RAND_S

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <wchar.h>
#include <internal/misc.h>
#include <internal/error.h>
#include <internal/nt.h>
#include <internal/fcntl.h>

static bool are_they_hardlinks(HANDLE a, HANDLE b)
{
	IO_STATUS_BLOCK I;
	NTSTATUS status;
	FILE_ID_INFORMATION id_a, id_b;

	status = NtQueryInformationFile(a, &I, &id_a, sizeof(FILE_ID_INFORMATION), FileIdInformation);
	if (status != STATUS_SUCCESS)
	{
		return false;
	}

	status = NtQueryInformationFile(b, &I, &id_b, sizeof(FILE_ID_INFORMATION), FileIdInformation);
	if (status != STATUS_SUCCESS)
	{
		return false;
	}

	if (id_a.VolumeSerialNumber != id_b.VolumeSerialNumber)
	{
		return false;
	}

	// just compare the lower 8 bytes, the upper 8 bytes will be zero on NTFS
	if (*(ULONGLONG *)(&id_a.FileId) != *(ULONGLONG *)(&id_b.FileId))
	{
		return false;
	}

	return true;
}

int do_rename(HANDLE handle, const wchar_t *path, int flags)
{
	int length = wcslen(path) * sizeof(wchar_t);
	size_t size_of_rename_info = sizeof(FILE_RENAME_INFORMATION) - sizeof(WCHAR) + length;
	IO_STATUS_BLOCK I;
	PFILE_RENAME_INFORMATION rename_info = (PFILE_RENAME_INFORMATION)malloc(size_of_rename_info);
	memset(rename_info, 0, size_of_rename_info);
	// No need to set RootDirectory as we are zeroing the memory
	rename_info->Flags = FILE_RENAME_POSIX_SEMANTICS | FILE_RENAME_IGNORE_READONLY_ATTRIBUTE |
						 (flags == RENAME_NOREPLACE ? 0 : FILE_RENAME_REPLACE_IF_EXISTS);
	rename_info->FileNameLength = length;
	memcpy(rename_info->FileName, path, length);

	NTSTATUS status = NtSetInformationFile(handle, &I, rename_info, size_of_rename_info, FileRenameInformationEx);
	free(rename_info);

	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}
	return 0;
}

int common_rename(int olddirfd, const char *restrict oldpath, int newdirfd, const char *restrict newpath, int flags)
{
	wchar_t *u16_ntoldpath = get_absolute_ntpath(olddirfd, oldpath);
	if (u16_ntoldpath == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	HANDLE old_handle = just_open(u16_ntoldpath, FILE_READ_ATTRIBUTES | DELETE, 0, FILE_OPEN, FILE_OPEN_REPARSE_POINT);
	free(u16_ntoldpath);
	if (old_handle == INVALID_HANDLE_VALUE)
	{
		// errno wil be set by just_open
		return -1;
	}

	wchar_t *u16_ntnewpath = get_absolute_ntpath(newdirfd, newpath);
	if (u16_ntnewpath == NULL)
	{
		errno = ENOENT;
		NtClose(old_handle);
		return -1;
	}
	HANDLE new_handle = just_open(u16_ntnewpath, FILE_READ_ATTRIBUTES, 0, FILE_OPEN, FILE_OPEN_REPARSE_POINT);
	if (new_handle != INVALID_HANDLE_VALUE)
	{
		if (are_they_hardlinks(old_handle, new_handle))
		{
			// Do nothing and return 0
			NtClose(old_handle);
			NtClose(new_handle);
			return 0;
		}
		NtClose(new_handle);
	}
	else
	{
		// newpath does not exist, clear errno
		errno = 0;
	}

	int status = do_rename(old_handle, u16_ntnewpath, flags);

	free(u16_ntnewpath);
	NtClose(old_handle);

	return status;
}

int common_exchange(int olddirfd, const char *restrict oldpath, int newdirfd, const char *restrict newpath)
{
	unsigned int rn;
	rand_s(&rn);
	char temp[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	_ultoa_s(rn, temp, 8, 36);

	int status;
	status = common_rename(olddirfd, oldpath, AT_FDCWD, temp, 0);
	if (status == -1)
	{
		return -1;
	}

	status = common_rename(newdirfd, newpath, olddirfd, oldpath, 0);
	if (status == -1)
	{
		// rollback
		common_rename(AT_FDCWD, temp, olddirfd, oldpath, 0);
		return -1;
	}

	status = common_rename(AT_FDCWD, temp, newdirfd, newpath, 0);
	if (status == -1)
	{
		// rollback
		common_rename(olddirfd, oldpath, newdirfd, newpath, 0);
		common_rename(AT_FDCWD, temp, olddirfd, oldpath, 0);
		return -1;
	}

	return 0;
}

int wlibc_common_rename(int olddirfd, const char *restrict oldpath, int newdirfd, const char *restrict newpath, int flags)
{
	if (flags != 0 && flags != RENAME_EXCHANGE && flags != RENAME_NOREPLACE)
	{
		errno = EINVAL;
		return -1;
	}

	VALIDATE_PATH_AND_DIRFD(oldpath, olddirfd);
	VALIDATE_PATH_AND_DIRFD(newpath, newdirfd);

	if (flags == RENAME_EXCHANGE)
	{
		return common_exchange(olddirfd, oldpath, newdirfd, newpath);
	}

	return common_rename(olddirfd, oldpath, newdirfd, newpath, flags);
}
