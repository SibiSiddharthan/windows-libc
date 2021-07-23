/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <internal/misc.h>
#include <Windows.h>
#include <internal/error.h>
#include <errno.h>

int common_rmdir(const wchar_t *wpath)
{
	if (!RemoveDirectory(wpath))
	{
		map_win32_error_to_wlibc(GetLastError());
		return -1;
	}

	return 0;
}

int wlibc_rmdir(const char *path)
{
	if (path == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	wchar_t *wpath = mb_to_wc(path);
	int status = common_rmdir(wpath);
	free(wpath);
	return status;
}

int wlibc_wrmdir(const wchar_t *wpath)
{
	if (wpath == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	return common_rmdir(wpath);
}
