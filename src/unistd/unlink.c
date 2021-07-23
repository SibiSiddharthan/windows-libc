/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <internal/misc.h>
#include <Windows.h>
#include <internal/error.h>

int common_unlink(const wchar_t *wpath)
{
	if (!DeleteFile(wpath))
	{
		DWORD error = GetLastError();
		if (error == ERROR_ACCESS_DENIED)
		{
			// Try to remove the read-only attribute if it is set and try again
			DWORD attributes;
			attributes = GetFileAttributes(wpath);
			if (attributes & FILE_ATTRIBUTE_READONLY)
			{
				attributes &= ~FILE_ATTRIBUTE_READONLY;
				if (!SetFileAttributes(wpath, attributes))
				{
					map_win32_error_to_wlibc(GetLastError());
					return -1;
				}

				if (!DeleteFile(wpath))
				{
					map_win32_error_to_wlibc(GetLastError());
					// re-set the read-only attribute
					attributes |= FILE_ATTRIBUTE_READONLY;
					SetFileAttributes(wpath, attributes);
					return -1;
				}
			}
			else
			{
				map_win32_error_to_wlibc(error);
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
