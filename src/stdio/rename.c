/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdio-ext.h>
#include <stdlib.h>
#include <errno.h>
#include <wchar.h>
#include <internal/misc.h>
#include <Windows.h>
#include <internal/error.h>

// Returns 1 if true, 0 otherwise (even for failures)
static int are_they_hardlinks(const wchar_t *restrict woldname, const wchar_t *restrict wnewname)
{
	HANDLE old = CreateFile(woldname, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
							FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, NULL);
	if (old == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	HANDLE new = CreateFile(wnewname, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
							FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, NULL);
	if (new == INVALID_HANDLE_VALUE)
	{
		CloseHandle(old);
		return 0;
	}

	// Check if they are Hardlinks of one another
	FILE_ID_INFO old_ino;
	if (!GetFileInformationByHandleEx(old, FileIdInfo, &old_ino, sizeof(FILE_ID_INFO)))
	{
		CloseHandle(old);
		CloseHandle(new);
		return 0;
	}

	FILE_ID_INFO new_ino;
	if (!GetFileInformationByHandleEx(new, FileIdInfo, &new_ino, sizeof(FILE_ID_INFO)))
	{
		CloseHandle(old);
		CloseHandle(new);
		return 0;
	}

	int cond = 0;

	if (memcmp(old_ino.FileId.Identifier, new_ino.FileId.Identifier, 16) == 0)
	{
		cond = 1;
	}

	CloseHandle(old);
	CloseHandle(new);
	return cond;
}

int common_rename(const wchar_t *restrict woldname, const wchar_t *restrict wnewname, int overwrite)
{

	if (are_they_hardlinks(woldname, wnewname))
	{
		// Do nothing and return 0.
		return 0;
	}

	int movefile_flags = MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH;
	if (overwrite)
	{
		movefile_flags |= MOVEFILE_REPLACE_EXISTING;
	}

	if (!MoveFileEx(woldname, wnewname, movefile_flags))
	{
		DWORD error = GetLastError();
		// If woldname refers to a directory and wnewname refers to an 'empty' directory try to remove wnewname and try again.
		if (error == ERROR_ACCESS_DENIED)
		{
			BOOL status = RemoveDirectory(wnewname);
			if (status)
			{
				if (!MoveFileEx(woldname, wnewname, movefile_flags))
				{
					map_win32_error_to_wlibc(GetLastError());
					return -1;
				}
			}
		}
		else
		{
			map_win32_error_to_wlibc(error);
			return -1;
		}
	}

	return 0;
}

int wlibc_rename(const char *restrict oldname, const char *restrict newname)
{
	if (oldname == NULL || newname == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	wchar_t *woldname = mb_to_wc(oldname);
	wchar_t *wnewname = mb_to_wc(newname);
	int status = common_rename(woldname, wnewname, 1);
	free(woldname);
	free(wnewname);
	return status;
}

int wlibc_wrename(const wchar_t *restrict woldname, const wchar_t *restrict wnewname)
{
	if (woldname == NULL || wnewname == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	return common_rename(woldname, wnewname, 1);
}
