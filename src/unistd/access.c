/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <misc.h>
#include <errno.h>
#include <unistd.h>
#include <wlibc_errors.h>
#include <Windows.h>

int common_access(const wchar_t *wname, int mode, int deference_symlinks)
{

	if (mode < 0 || mode > 7)
	{
		errno = EINVAL;
		return -1;
	}

	int deference_symlinks_flags = 0;

	if (!deference_symlinks)
	{
		deference_symlinks_flags |= FILE_FLAG_OPEN_REPARSE_POINT;
	}

	HANDLE file = CreateFile(wname, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
							 FILE_FLAG_BACKUP_SEMANTICS | deference_symlinks_flags, NULL);
	if (file == INVALID_HANDLE_VALUE)
	{
		map_win32_error_to_wlibc(GetLastError());
		return -1;
	}

	BY_HANDLE_FILE_INFORMATION FILE_INFO;
	GetFileInformationByHandle(file, &FILE_INFO);
	DWORD attributes = FILE_INFO.dwFileAttributes;

	CloseHandle(file);

	// From readdir.c
	if (attributes & FILE_ATTRIBUTE_REPARSE_POINT)
	{
		return 0; // Have all permissions
	}
	else if (attributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		return 0; // Have all permissions
	}
	else if ((attributes & ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE |
							 FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_SPARSE_FILE | FILE_ATTRIBUTE_COMPRESSED |
							 FILE_ATTRIBUTE_NOT_CONTENT_INDEXED | FILE_ATTRIBUTE_ENCRYPTED)) == 0)
	{
		if ((attributes & FILE_ATTRIBUTE_READONLY) && (mode & W_OK))
		{
			return -1;
		}
		else if ((mode & X_OK) && !has_executable_extenstion(wname))
		{
			return -1;
		}
		else
		{
			return 0;
		}
	}

	return -1;
}

int wlibc_access(const char *name, int mode)
{
	if (name == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	wchar_t *wname = mb_to_wc(name);
	int status = common_access(wname, mode, 1);
	free(wname);

	return status;
}

int wlibc_waccess(const wchar_t *wname, int mode)
{
	if (wname == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	return common_access(wname, mode, 1);
}
