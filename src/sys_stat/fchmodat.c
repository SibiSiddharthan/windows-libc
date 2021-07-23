/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <sys/stat.h>
#include <internal/misc.h>
#include <errno.h>
#include <internal/fcntl.h>
#include <fcntl.h>

int common_chmod(const wchar_t *wname, mode_t mode);

int common_fchmodat(int dirfd, const wchar_t *wname, mode_t mode, int flags)
{
	if (dirfd == AT_FDCWD || is_absolute_pathw(wname))
	{
		return common_chmod(wname, mode);
	}

	enum handle_type _type = get_fd_type(dirfd);
	if (_type != DIRECTORY_HANDLE || _type == INVALID_HANDLE)
	{
		errno = (_type == INVALID_HANDLE ? EBADF: ENOTDIR);
		return -1;
	}

	const wchar_t *dirpath = get_fd_path(dirfd);
	wchar_t *newname = wcstrcat(dirpath, wname);
	int status = common_chmod(newname, mode);
	free(newname);

	return status;
}

int wlibc_fchmodat(int dirfd, const char *name, mode_t mode, int flags)
{
	if (name == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	wchar_t *wname = mb_to_wc(name);
	int status = common_fchmodat(dirfd, wname, mode, flags);
	free(wname);

	return status;
}

int wlibc_wfchmodat(int dirfd, const wchar_t *wname, mode_t mode, int flags)
{
	if (wname == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	return common_fchmodat(dirfd, wname, mode, flags);
}
