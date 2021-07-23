/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/misc.h>
#include <errno.h>
#include <unistd.h>
#include <internal/fcntl.h>
#include <fcntl.h>

int common_chown(const wchar_t *wname, uid_t owner, gid_t group, int do_lchown);

int common_chownat(int dirfd, const wchar_t *wname, uid_t owner, gid_t group, int flags)
{
	if (flags != 0 && flags != AT_SYMLINK_NOFOLLOW)
	{
		errno = EINVAL;
		return -1;
	}

	int do_lchown = 0;
	if (flags == AT_SYMLINK_NOFOLLOW)
	{
		do_lchown = 1;
	}

	if (dirfd == AT_FDCWD || is_absolute_pathw(wname))
	{
		return common_chown(wname, owner, group, do_lchown);
	}

	enum handle_type _type = get_fd_type(dirfd);
	if (_type != DIRECTORY_HANDLE || _type == INVALID_HANDLE)
	{
		errno = (_type == INVALID_HANDLE ? EBADF: ENOTDIR);
		return -1;
	}

	const wchar_t *dirpath = get_fd_path(dirfd);
	wchar_t *newname = wcstrcat(dirpath, wname);
	int status = common_chown(wname, owner, group, do_lchown);
	free(newname);

	return status;
}

int wlibc_fchownat(int dirfd, const char *name, uid_t owner, gid_t group, int flags)
{
	if (name == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	wchar_t *wname = mb_to_wc(name);
	int status = common_chownat(dirfd, wname, owner, group, flags);
	free(wname);

	return status;
}

int wlibc_wfchownat(int dirfd, const wchar_t *wname, uid_t owner, gid_t group, int flags)
{

	if (wname == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	return common_chownat(dirfd, wname, owner, group, flags);
}
