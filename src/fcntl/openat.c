/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <fcntl.h>
#include <errno.h>
#include <wchar.h>
#include <string.h>
#include <windows.h>
#include <fcntl_internal.h>
#include <stdlib.h>
#include <misc.h>

int common_openat(int dirfd, const wchar_t *wname, int flags, va_list perm_args)
{
	if (dirfd == AT_FDCWD || is_absolute_pathw(wname))
	{
		return wlibc_wopen(wname, flags, perm_args);
	}

	if (!validate_dirfd(dirfd))
	{
		return -1;
	}

	const wchar_t *dirpath = get_fd_path(dirfd);
	wchar_t *newpath = wcstrcat(dirpath, wname);
	int status = wlibc_wopen(newpath, flags, perm_args);
	free(newpath);

	return status;
}

int wlibc_openat(int dirfd, const char *name, int flags, va_list perm_args)
{
	if (name == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	wchar_t *wname = mb_to_wc(name);
	int status = common_openat(dirfd, wname, flags, perm_args);
	free(wname);

	return status;
}

int wlibc_wopenat(int dirfd, const wchar_t *wname, int flags, va_list perm_args)
{
	if (wname == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	return common_openat(dirfd, wname, flags, perm_args);
}