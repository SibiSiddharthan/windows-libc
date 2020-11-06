/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <unistd.h>
#include <misc.h>
#include <Windows.h>
#include <wlibc_errors.h>

int common_unlink(const wchar_t *wpath)
{
	if (!DeleteFile(wpath))
	{
		DWORD error = GetLastError();
		if (error == ERROR_ACCESS_DENIED) // Try to remove the Readonly Attribute and try again
		{
			DWORD attributes;
			attributes = GetFileAttributes(wpath);
			attributes &= ~FILE_ATTRIBUTE_READONLY;
			if (!SetFileAttributes(wpath, attributes))
			{
				map_win32_error_to_wlibc(GetLastError());
				return -1;
			}

			if (!DeleteFile(wpath))
			{
				map_win32_error_to_wlibc(GetLastError());
				return -1;
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

int wlibc_unlink(const char *path)
{
	if (path == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	wchar_t *wpath = mb_to_wc(path);
	int status = common_unlink(wpath);
	free(wpath);
	return status;
}

int wlibc_wunlink(const wchar_t *wpath)
{
	if (wpath == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	return common_unlink(wpath);
}