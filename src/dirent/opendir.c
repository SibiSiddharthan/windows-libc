/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <dirent.h>
#include <windows.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>
#include <wlibc_errors.h>
#include <errno.h>
#include <fcntl_internal.h>
#include <misc.h>

void fill_dir_buffer(DIR *dirp);

static DIR *common_opendir(const wchar_t *wname, int fd)
{

	int length = wcslen(wname);
	wchar_t last = wname[length - 1];
	wchar_t *wname_proper = (wchar_t *)malloc(sizeof(wchar_t) * (length + 3)); // '/0','/','*'
	wcscpy(wname_proper, wname);

	// Append "/*" or "*" as needed
	if (last == L'/' || last == L'\\')
	{
		wcscat(wname_proper, L"*");
	}
	else
	{
		wcscat(wname_proper, L"/*");
	}

	WIN32_FIND_DATA find_file;
	HANDLE directory_handle;
	DIR *dirp = NULL;

	directory_handle =
		FindFirstFileEx(wname_proper, FindExInfoStandard, &find_file, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);

	if (directory_handle == INVALID_HANDLE_VALUE)
	{
		map_win32_error_to_wlibc(GetLastError());
		return NULL;
	}

	dirp = (DIR *)malloc(sizeof(DIR));
	dirp->d_handle = directory_handle;
	dirp->buffer_length = 8;
	dirp->data = (WIN32_FIND_DATA *)malloc(sizeof(WIN32_FIND_DATA) * dirp->buffer_length);
	dirp->_dirent = (struct dirent *)malloc(sizeof(struct dirent));
	dirp->_wdirent = (struct wdirent *)malloc(sizeof(struct wdirent));
	dirp->offset = 0;
	dirp->data[0] = find_file;
	dirp->size = 1;

	// fill a buffer of directory entries
	fill_dir_buffer(dirp);

	// Make/Update an entry in the fd table
	if (fd == -1)
	{
		dirp->fd = register_to_fd_table(directory_handle, wname, DIRECTORY_ACTIVE, 0);
	}
	else
	{
		set_fd_handle(fd, directory_handle);
		set_fd_type(fd, DIRECTORY_ACTIVE);
	}

	free(wname_proper);
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
	if (!validate_fd(fd) || get_fd_type(fd) != DIRECTORY_INACTIVE)
	{
		errno = EBADF;
		return NULL;
	}

	const wchar_t *path = get_fd_path(fd);
	DIR *dirp = common_opendir(path, fd);
	return dirp;
}