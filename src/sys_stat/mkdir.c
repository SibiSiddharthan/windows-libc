/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <sys/stat.h>
#include <Windows.h>
#include <wlibc_errors.h>
#include <misc.h>
#include <errno.h>

int common_mkdir(const wchar_t *wpath, mode_t mode)
{
	if(!CreateDirectory(wpath,NULL))
	{
		map_win32_error_to_wlibc(GetLastError());
		return -1;
	}

	return 0;
}

int wlibc_mkdir(const char *path, mode_t mode)
{
	if (path == NULL)
	{
		errno = ENOENT;
		return -1;
	}
	wchar_t *wpath = mb_to_wc(path);
	int status = common_mkdir(wpath, mode);
	free(wpath);
	return status;
}

int wlibc_wmkdir(const wchar_t *wpath, mode_t mode)
{
	if (wpath == NULL)
	{
		errno = ENOENT;
		return -1;
	}
	return common_mkdir(wpath, mode);
}
