/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <dirent.h>
#include <windows.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>
#include <internal/error.h>
#include <errno.h>
#include <internal/fcntl.h>
#include <internal/misc.h>

static void initialize_dirp(DIR **dirp, HANDLE directory_handle)
{
	*dirp = (DIR *)malloc(sizeof(DIR));
	(*dirp)->d_handle = directory_handle;
	(*dirp)->buffer = malloc(DIRENT_DIR_BUFFER_SIZE);
	(*dirp)->offset = 0;
	(*dirp)->called_rewinddir = 0;
	(*dirp)->_dirent = (struct dirent *)malloc(sizeof(struct dirent));
	(*dirp)->_wdirent = (struct wdirent *)malloc(sizeof(struct wdirent));
}

static DIR *common_opendir(const wchar_t *wname, int fd)
{
	HANDLE directory_handle =
		CreateFile(wname, FILE_READ_ATTRIBUTES | FILE_LIST_DIRECTORY | FILE_TRAVERSE,
				   FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (directory_handle == INVALID_HANDLE_VALUE)
	{
		map_win32_error_to_wlibc(GetLastError());
		return NULL;
	}

	FILE_STANDARD_INFO INFO;
	if (!GetFileInformationByHandleEx(directory_handle, FileStandardInfo, &INFO, sizeof(FILE_STANDARD_INFO)))
	{
		map_win32_error_to_wlibc(GetLastError());
	}
	if(!INFO.Directory)
	{
		CloseHandle(directory_handle);
		errno = ENOTDIR;
		return NULL;
	}

	DIR *dirp = NULL;
	initialize_dirp(&dirp, directory_handle);
	dirp->fd = register_to_fd_table(directory_handle, wname, DIRECTORY_HANDLE, 0);
	return dirp;
}

DIR *wlibc_opendir(const char *name)
{
	if (name == NULL)
	{
		errno = ENOENT;
		return NULL;
	}

	wchar_t *wname = mb_to_wc(name);
	DIR *dirp = common_opendir(wname, -1);
	free(wname);
	return dirp;
}

DIR *wlibc_wopendir(const wchar_t *wname)
{
	if (wname == NULL)
	{
		errno = ENOENT;
		return NULL;
	}

	return common_opendir(wname, -1);
}

DIR *wlibc_fdopendir(int fd)
{
	enum handle_type _type = get_fd_type(fd);
	if (_type != DIRECTORY_HANDLE || _type == INVALID_HANDLE)
	{
		errno = (_type == INVALID_HANDLE ? EBADF : ENOTDIR);
		return NULL;
	}

	HANDLE directory_handle = get_fd_handle(fd);
	DIR *dirp = NULL;
	initialize_dirp(&dirp, directory_handle);
	return dirp;
}
