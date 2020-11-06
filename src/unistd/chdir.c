/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <misc.h>
#include <errno.h>
#include <unistd.h>
#include <wlibc_errors.h>
#include <fcntl_internal.h>
#include <Windows.h>

int common_chdir(const wchar_t *wname)
{
	if (!SetCurrentDirectory(wname))
	{
		map_win32_error_to_wlibc(GetLastError());
		return -1;
	}

	return 0;
}

int wlibc_chdir(const char *name)
{
	if (name == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	wchar_t *wname = mb_to_wc(name);
	int status = common_chdir(wname);
	free(wname);

	return status;
}

int wlibc_wchdir(const wchar_t *wname)
{
	if (wname == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	return common_chdir(wname);
}

int wlibc_fchdir(int fd)
{
	if (!validate_dirfd(fd))
	{
		return -1;
	}

	const wchar_t *dirpath = get_fd_path(fd);
	return common_chdir(dirpath);
}