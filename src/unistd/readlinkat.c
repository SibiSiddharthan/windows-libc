/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <fcntl.h>
#include <Windows.h>
#include <wlibc_errors.h>
#include <misc.h>
#include <errno.h>
#include <fcntl_internal.h>
#include <stdlib.h>
#include <unistd.h>

ssize_t common_readlink(const wchar_t *wpath, wchar_t *wbuf, size_t bufsiz, int give_absolute);

ssize_t common_readlinkat(int dirfd, const wchar_t *wpath, wchar_t *wbuf, size_t bufsiz)
{
	if (dirfd == AT_FDCWD || is_absolute_pathw(wpath))
	{
		return common_readlink(wpath, wbuf, bufsiz, 0);
	}

	if (!validate_dirfd(dirfd))
	{
		return -1;
	}

	const wchar_t *dirpath = get_fd_path(dirfd);
	wchar_t *newpath = wcstrcat(dirpath, wpath);
	ssize_t length = common_readlink(newpath, wbuf, bufsiz, 0);
	free(newpath);

	return length;
}

ssize_t wlibc_readlinkat(int dirfd, const char *path, char *buf, size_t bufsiz)
{
	if (path == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	wchar_t *wpath = mb_to_wc(path);
	wchar_t *wbuf = (wchar_t *)malloc(sizeof(wchar_t) * bufsiz);
	ssize_t length = common_readlinkat(dirfd, wpath, wbuf, bufsiz);
	if (length != -1)
	{
		wcstombs(buf, wbuf, length);
	}
	free(wpath);
	free(wbuf);

	return length;
}

ssize_t wlibc_wreadlinkat(int dirfd, const wchar_t *wpath, wchar_t *wbuf, size_t bufsiz)
{
	if (wpath == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	return common_readlinkat(dirfd, wpath, wbuf, bufsiz);
}
