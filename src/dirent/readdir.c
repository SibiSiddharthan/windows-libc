/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <dirent.h>
#include <windows.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl_internal.h>
#include <misc.h>
#include <wlibc_errors.h>

void fill_dir_buffer(DIR *dirp);

struct dirent *wlibc_readdir(DIR *dirp)
{
	struct wdirent *temp = wreaddir(dirp);
	if (temp == NULL)
		return NULL;
	// memcpy(dirp->_dirent,dirp->_wdirent,sizeof(ino_t) + sizeof(off_t) + sizeof(unsigned short int) + sizeof(unsigned char));
	dirp->_dirent->d_ino = dirp->_wdirent->d_ino;
	dirp->_dirent->d_off = dirp->_wdirent->d_off;
	dirp->_dirent->d_reclen = sizeof(struct dirent);
	dirp->_dirent->d_type = dirp->_wdirent->d_type;
	wcstombs(dirp->_dirent->d_name, dirp->_wdirent->d_name, wcslen(dirp->_wdirent->d_name) + 1);
	return dirp->_dirent;
}

struct wdirent *wlibc_wreaddir(DIR *dirp)
{
	if (dirp == NULL || !validate_active_dirfd(dirp->fd))
	{
		errno = EBADF;
		return NULL;
	}

	int index = dirp->offset;
	if (index >= dirp->buffer_length)
	{
		// refill the buffer if we reach its end
		WIN32_FIND_DATA *temp = (WIN32_FIND_DATA *)malloc(sizeof(WIN32_FIND_DATA) * dirp->buffer_length);
		memcpy(temp, dirp->data, sizeof(WIN32_FIND_DATA) * dirp->buffer_length);
		dirp->buffer_length *= 2;
		dirp->data = (WIN32_FIND_DATA *)realloc(dirp->data, sizeof(WIN32_FIND_DATA) * dirp->buffer_length);
		memcpy(dirp->data, temp, sizeof(WIN32_FIND_DATA) * (dirp->buffer_length / 2));
		free(temp);
		fill_dir_buffer(dirp);
	}

	if (index >= dirp->size)
		return NULL;

	dirp->_wdirent->d_ino = 0;
	dirp->_wdirent->d_off = dirp->offset;
	DWORD attributes = dirp->data[index].dwFileAttributes;
	/* For a junction both FILE_ATTRIBUTE_DIRECTORY and FILE_ATTRIBUTE_REPARSE_POINT is set.
	   To have it as DT_LNK we put this condition first.
	*/
	if (attributes & FILE_ATTRIBUTE_REPARSE_POINT)
	{
		dirp->_wdirent->d_type = DT_LNK;
	}
	else if (attributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		dirp->_wdirent->d_type = DT_DIR;
	}
	else if ((attributes & ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE |
							 FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_SPARSE_FILE | FILE_ATTRIBUTE_COMPRESSED |
							 FILE_ATTRIBUTE_NOT_CONTENT_INDEXED | FILE_ATTRIBUTE_ENCRYPTED)) == 0)
	{
		dirp->_wdirent->d_type = DT_REG;
	}
	else
	{
		dirp->_wdirent->d_type = DT_UNKNOWN;
	}

	dirp->_wdirent->d_reclen = sizeof(struct wdirent);
	wcscpy(dirp->_wdirent->d_name, dirp->data[index].cFileName);

	// Try to find the inode number
	const wchar_t *dirpath = get_fd_path(dirp->fd);
	wchar_t *wname = wcstrcat(dirpath, dirp->data[index].cFileName);

	HANDLE file = CreateFile(wname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
							 FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, NULL);
	if (file == INVALID_HANDLE_VALUE)
	{
		map_win32_error_to_wlibc(GetLastError());
		return NULL;
	}
	FILE_ID_INFO INO_INFO;
	if (!GetFileInformationByHandleEx(file, FileIdInfo, &INO_INFO, sizeof(FILE_ID_INFO)))
	{
		map_win32_error_to_wlibc(GetLastError());
		return NULL;
	}
	memcpy(&dirp->_wdirent->d_ino, INO_INFO.FileId.Identifier, sizeof(ino_t));

	free(wname);
	CloseHandle(file);

	dirp->offset++;
	return dirp->_wdirent;
}
