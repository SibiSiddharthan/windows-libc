/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <sys/stat.h>
#include <misc.h>
#include <errno.h>
#include <fcntl.h>
#include <fcntl_internal.h>

int common_stat(const wchar_t *wname, struct stat *statbuf, int do_lstat);

int common_fstatat(int dirfd, const wchar_t *wname, struct stat *statbuf, int flags)
{
	if (flags != 0 && flags != AT_SYMLINK_NOFOLLOW)
	{
		errno = EINVAL;
		return -1;
	}

	int do_lstat = 0;
	if (flags == AT_SYMLINK_NOFOLLOW)
	{
		do_lstat = 1;
	}

	if (dirfd == AT_FDCWD || is_absolute_pathw(wname))
	{
		return common_stat(wname, statbuf, do_lstat);
	}

	if (!validate_dirfd(dirfd))
	{
		return -1;
	}

	const wchar_t *dirpath = get_fd_path(dirfd);
	wchar_t *newname = wcstrcat(dirpath, wname);
	int status = common_stat(newname, statbuf, do_lstat);
	free(newname);

	return status;
}

int wlibc_fstatat(int dirfd, const char *name, struct stat *statbuf, int flags)
{
	if (name == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	wchar_t *wname = mb_to_wc(name);
	int status = common_fstatat(dirfd, wname, statbuf, flags);
	free(wname);

	return status;
}

int wlibc_wfstatat(int dirfd, const wchar_t *wname, struct stat *statbuf, int flags)
{
	if (wname == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	return common_fstatat(dirfd, wname, statbuf, flags);
}
