/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdio-ext.h>
#include <stdlib.h>
#include <errno.h>
#include <wchar.h>
#include <misc.h>
#include <fcntl.h>
#include <fcntl_internal.h>
#include <Windows.h>
#include <wlibc_errors.h>

int common_rename(const wchar_t *woldname, const wchar_t *wnewname, int overwrite);

int rename_exchange(const wchar_t *final_oldname, const wchar_t *final_newname)
{
	wchar_t temp_path[MAX_PATH];
	if (!GetTempFileName(L".", NULL, 0, temp_path))
	{
		map_win32_error_to_wlibc(GetLastError());
		return -1;
	}

	// Delete the file, so we can use the name for files and directories
	DeleteFile(temp_path);

	int status;
	status = common_rename(final_oldname, temp_path, 1);
	if (status == -1)
	{
		return -1;
	}

	status = common_rename(final_newname, final_oldname, 1);
	if (status == -1)
	{
		// rollback
		common_rename(temp_path, final_oldname, 1);
		return -1;
	}

	status = common_rename(temp_path, final_newname, 1);
	if (status == -1)
	{
		// rollback
		common_rename(final_oldname, final_newname, 1);
		common_rename(temp_path, final_oldname, 1);
		return -1;
	}

	return 0;
}

int common_renameat(int olddirfd, const wchar_t *woldname, int newdirfd, const wchar_t *wnewname, int flags)
{
	int use_olddirfd = 1, use_newdirfd = 1;
	wchar_t *final_oldname = NULL;
	wchar_t *final_newname = NULL;
	enum handle_type _type;

	if (olddirfd == AT_FDCWD || is_absolute_pathw(woldname))
	{
		use_olddirfd = 0;
		final_oldname = (wchar_t *)woldname;
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

	if (newdirfd == AT_FDCWD || is_absolute_pathw(wnewname))
	{
		use_newdirfd = 0;
		final_newname = (wchar_t *)wnewname;
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

	if (use_olddirfd)
	{
		const wchar_t *old_dirpath = get_fd_path(olddirfd);
		final_oldname = wcstrcat(old_dirpath, woldname);
	}

	if (use_newdirfd)
	{
		const wchar_t *new_dirpath = get_fd_path(newdirfd);
		final_newname = wcstrcat(new_dirpath, wnewname);
	}

	int overwrite = 1;
	if (flags & RENAME_NOREPLACE)
	{
		overwrite = 0;
	}

	int status;
	if (flags & RENAME_EXCHANGE)
	{
		status = rename_exchange(final_oldname, final_newname);
	}
	else
	{
		status = common_rename(final_oldname, final_newname, overwrite);
	}

	if (use_olddirfd)
	{
		free(final_oldname);
	}

	if (use_newdirfd)
	{
		free(final_newname);
	}

	return status;
}

int wlibc_renameat(int olddirfd, const char *oldname, int newdirfd, const char *newname)
{
	if (oldname == NULL || newname == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	wchar_t *woldname = mb_to_wc(oldname);
	wchar_t *wnewname = mb_to_wc(newname);
	int status = common_renameat(olddirfd, woldname, newdirfd, wnewname, 0);
	free(woldname);
	free(wnewname);
	return status;
}

int wlibc_wrenameat(int olddirfd, const wchar_t *woldname, int newdirfd, const wchar_t *wnewname)
{
	if (woldname == NULL || wnewname == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	return common_renameat(olddirfd, woldname, newdirfd, wnewname, 0);
}

int common_renameat2(int olddirfd, const wchar_t *woldname, int newdirfd, const wchar_t *wnewname, unsigned int flags)
{
	if ((flags & (RENAME_EXCHANGE | RENAME_NOREPLACE)) == (RENAME_EXCHANGE | RENAME_NOREPLACE))
	{
		errno = EINVAL;
		return -1;
	}

	if (flags < 0 || flags > 7)
	{
		errno = EINVAL;
		return -1;
	}

	return common_renameat(olddirfd, woldname, newdirfd, wnewname, flags);
}

int wlibc_renameat2(int olddirfd, const char *oldname, int newdirfd, const char *newname, unsigned int flags)
{
	if (oldname == NULL || newname == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	wchar_t *woldname = mb_to_wc(oldname);
	wchar_t *wnewname = mb_to_wc(newname);
	int status = common_renameat2(olddirfd, woldname, newdirfd, wnewname, flags);
	free(woldname);
	free(wnewname);
	return status;
}

int wlibc_wrenameat2(int olddirfd, const wchar_t *woldname, int newdirfd, const wchar_t *wnewname, unsigned int flags)
{
	if (woldname == NULL || wnewname == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	return common_renameat2(olddirfd, woldname, newdirfd, wnewname, flags);
}
