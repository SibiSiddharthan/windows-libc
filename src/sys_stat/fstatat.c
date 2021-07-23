/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <sys/stat.h>
#include <internal/misc.h>
#include <errno.h>
#include <fcntl.h>
#include <internal/fcntl.h>

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

	if (get_fd_type(dirfd) != DIRECTORY_HANDLE)
	{
		errno = EBADF;
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
