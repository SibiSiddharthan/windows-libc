/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/misc.h>
#include <errno.h>
#include <unistd.h>
#include <internal/error.h>
#include <internal/fcntl.h>
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
	if (get_fd_type(fd) != DIRECTORY_HANDLE)
	{
		return -1;
	}

	const wchar_t *dirpath = get_fd_path(fd);
	return common_chdir(dirpath);
}
