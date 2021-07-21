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
#include <stdlib.h>

int common_link(const wchar_t *wsource, const wchar_t *wtarget);
ssize_t common_readlink(const wchar_t *wpath, wchar_t *wbuf, size_t bufsiz, int give_absolute);
ssize_t common_readlinkat(int dirfd, const wchar_t *wpath, wchar_t *wbuf, size_t bufsiz);

int common_linkat(int olddirfd, const wchar_t *wsource, int newdirfd, const wchar_t *wtarget, int flags)
{
	wchar_t *newsource = NULL;
	wchar_t *finalsource = NULL;
	wchar_t *newtarget = NULL;
	wchar_t *wbuf = NULL;
	enum handle_type _type;
	int use_olddirfd = 1, use_newdirfd = 1;

	if (flags != 0 && flags != AT_SYMLINK_FOLLOW)
	{
		errno = EINVAL;
		return -1;
	}

	if (olddirfd == AT_FDCWD || is_absolute_pathw(wsource))
	{
		use_olddirfd = 0;
		newsource = (wchar_t *)wsource;
	}

	if (use_olddirfd)
	{
		_type = get_fd_type(olddirfd);
		if (_type != DIRECTORY_HANDLE || _type == INVALID_HANDLE)
		{
			errno = (_type == INVALID_HANDLE ? EBADF : ENOTDIR);
			return -1;
		}
	}

	if (newdirfd == AT_FDCWD || is_absolute_pathw(wtarget))
	{
		use_newdirfd = 0;
		newtarget = (wchar_t *)wtarget;
	}

	if (use_newdirfd)
	{
		_type = get_fd_type(newdirfd);
		if (_type != DIRECTORY_HANDLE || _type == INVALID_HANDLE)
		{
			errno = (_type == INVALID_HANDLE ? EBADF : ENOTDIR);
			return -1;
		}
	}

	// Handle AT_SYMLINK_FOLLOW
	if (flags == AT_SYMLINK_FOLLOW)
	{
		wchar_t *wbuf = (wchar_t *)malloc(sizeof(wchar_t) * MAX_PATH);
		if (use_olddirfd)
		{
			const wchar_t *source_dirpath = get_fd_path(olddirfd);
			ssize_t length = common_readlinkat(olddirfd, wsource, wbuf, MAX_PATH);
			if (length == -1)
			{
				return -1;
			}
			wbuf[length] = L'\0';
			finalsource = wcstrcat(source_dirpath, wbuf);
		}
		else
		{
			ssize_t length = common_readlink(wsource, wbuf, MAX_PATH, 1);
			if (length == -1)
			{
				return -1;
			}
			wbuf[length] = L'\0';
			finalsource = wbuf;
		}
	}
	else
	{
		if (use_olddirfd)
		{
			const wchar_t *source_dirpath = get_fd_path(olddirfd);
			newsource = wcstrcat(source_dirpath, wsource);
		}
		finalsource = newsource;
	}

	if (use_newdirfd)
	{
		const wchar_t *target_dirpath = get_fd_path(newdirfd);
		newtarget = wcstrcat(target_dirpath, wtarget);
	}

	int status = common_link(finalsource, newtarget);

	if (flags == AT_SYMLINK_FOLLOW)
	{
		if (use_olddirfd)
		{
			free(finalsource);
		}
		else
		{
		}
		free(wbuf);
	}
	else
	{
		if (use_olddirfd)
		{
			free(newsource);
		}
	}

	if (use_newdirfd)
	{
		free(newtarget);
	}

	return status;
}

int wlibc_linkat(int olddirfd, const char *source, int newdirfd, const char *target, int flags)
{
	if (source == NULL || target == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	wchar_t *wsource = mb_to_wc(source);
	wchar_t *wtarget = mb_to_wc(target);
	int status = common_linkat(olddirfd, wsource, newdirfd, wtarget, flags);
	free(wsource);
	free(wtarget);
	return status;
}

int wlibc_wlinkat(int olddirfd, const wchar_t *wsource, int newdirfd, const wchar_t *wtarget, int flags)
{
	if (wsource == NULL || wtarget == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	return common_linkat(olddirfd, wsource, newdirfd, wtarget, flags);
}
