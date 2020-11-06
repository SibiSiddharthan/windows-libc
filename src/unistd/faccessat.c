/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <misc.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <fcntl_internal.h>
#include <wlibc_errors.h>

int common_access(const wchar_t *wname, int mode, int deference_symlinks);

int common_faccessat(int dirfd, const wchar_t *wname, int mode, int flags)
{
	if (flags != 0 && flags != AT_SYMLINK_NOFOLLOW)
	{
		errno = EINVAL;
		return -1;
	}

	int deference_symlinks = 1;
	if (flags == AT_SYMLINK_NOFOLLOW)
	{
		deference_symlinks = 0;
	}

	if (dirfd == AT_FDCWD || is_absolute_pathw(wname))
	{
		return common_access(wname, mode, deference_symlinks);
	}

	if (!validate_dirfd(dirfd))
	{
		return -1;
	}

	const wchar_t *dirpath = get_fd_path(dirfd);
	wchar_t *newname = wcstrcat(dirpath, wname);
	int status = common_access(newname, mode, deference_symlinks);
	free(newname);

	return status;
}

int wlibc_faccessat(int dirfd, const char *name, int mode, int flags)
{
	if (name == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	wchar_t *wname = mb_to_wc(name);
	int status = common_faccessat(dirfd, wname, mode, flags);
	free(wname);

	return status;
}

int wlibc_wfaccessat(int dirfd, const wchar_t *wname, int mode, int flags)
{
	if (wname == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	return common_faccessat(dirfd, wname, mode, flags);
}