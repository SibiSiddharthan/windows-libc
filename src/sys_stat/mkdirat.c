/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <sys/stat.h>
#include <fcntl.h>
#include <Windows.h>
#include <wlibc_errors.h>
#include <misc.h>
#include <errno.h>
#include <fcntl_internal.h>
#include <stdlib.h>

int common_mkdir(const wchar_t *wpath, mode_t mode);

int common_mkdirat(int dirfd, const wchar_t *wpath, mode_t mode)
{
	if (dirfd == AT_FDCWD || is_absolute_pathw(wpath))
	{
		return common_mkdir(wpath, mode);
	}

	if (!validate_dirfd(dirfd))
	{
		return -1;
	}

	const wchar_t *dirpath = get_fd_path(dirfd);
	wchar_t *newpath = wcstrcat(dirpath, wpath);
	int status = common_mkdir(newpath, mode);
	free(newpath);

	return status;
}

int wlibc_mkdirat(int dirfd, const char *path, mode_t mode)
{
	if (path == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	wchar_t *wpath = mb_to_wc(path);
	int status = common_mkdirat(dirfd, wpath, mode);
	free(wpath);

	return status;
}

int wlibc_wmkdirat(int dirfd, const wchar_t *wpath, mode_t mode)
{
	if (wpath == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	return common_mkdirat(dirfd, wpath, mode);
}
