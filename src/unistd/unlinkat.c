/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <misc.h>
#include <wlibc_errors.h>
#include <errno.h>
#include <fcntl.h>
#include <fcntl_internal.h>

int common_unlink(const wchar_t *wpath);
int common_rmdir(const wchar_t *wpath);

int common_unlinkat(int dirfd, const wchar_t *wpath, int flags)
{
	if (flags != 0 && flags != AT_REMOVEDIR)
	{
		errno = EINVAL;
		return -1;
	}

	if (dirfd == AT_FDCWD || is_absolute_pathw(wpath))
	{
		if (flags == AT_REMOVEDIR)
		{
			return common_rmdir(wpath);
		}
		else
		{
			return common_unlink(wpath);
		}
	}

	if (get_fd_type(dirfd) != DIRECTORY_HANDLE)
	{
		return -1;
	}

	const wchar_t *dirpath = get_fd_path(dirfd);
	wchar_t *newpath = wcstrcat(dirpath, wpath);
	int status;
	if (flags == AT_REMOVEDIR)
	{
		status = common_rmdir(newpath);
	}
	else
	{
		status = common_unlink(newpath);
	}
	free(newpath);

	return status;
}

int wlibc_unlinkat(int dirfd, const char *path, int flags)
{
	if (path == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	wchar_t *wpath = mb_to_wc(path);
	int status = common_unlinkat(dirfd, wpath, flags);
	free(wpath);

	return status;
}

int wlibc_wunlinkat(int dirfd, const wchar_t *wpath, int flags)
{
	if (wpath == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	return common_unlinkat(dirfd, wpath, flags);
}
