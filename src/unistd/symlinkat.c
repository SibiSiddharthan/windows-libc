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

int common_symlink(const wchar_t *wsource, const wchar_t *wtarget);

int common_symlinkat(const wchar_t *wsource, int newdirfd, const wchar_t *wtarget)
{
	if (newdirfd == AT_FDCWD || is_absolute_pathw(wtarget))
	{
		return common_symlink(wsource, wtarget);
	}

	if (!validate_dirfd(newdirfd))
	{
		return -1;
	}

	const wchar_t *dirpath = get_fd_path(newdirfd);
	wchar_t *newtarget = wcstrcat(dirpath, wtarget);
	int status = common_symlink(wsource, newtarget);
	free(newtarget);

	return status;
}

int wlibc_symlinkat(const char *source, int newdirfd, const char *target)
{
	if (source == NULL || target == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	wchar_t *wsource = mb_to_wc(source);
	wchar_t *wtarget = mb_to_wc(target);
	int status = common_symlinkat(wsource, newdirfd, wtarget);
	free(wsource);
	free(wtarget);

	return status;
}

int wlibc_wsymlinkat(const wchar_t *wsource, int newdirfd, const wchar_t *wtarget)
{
	if (wsource == NULL || wtarget == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	return common_symlinkat(wsource, newdirfd, wtarget);
}
