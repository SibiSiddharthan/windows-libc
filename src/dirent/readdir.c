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
#include <internal/fcntl.h>
#include <internal/misc.h>
#include <internal/error.h>

int fill_dir_buffer(DIR *dirp)
{
	memset(dirp->buffer, 0, DIRENT_DIR_BUFFER_SIZE);
	// TODO use NtQueryDirectoryFile for better performance in rewinddir
	if (!GetFileInformationByHandleEx(dirp->d_handle, dirp->called_rewinddir ? FileIdBothDirectoryRestartInfo : FileIdBothDirectoryInfo,
									  dirp->buffer, DIRENT_DIR_BUFFER_SIZE))
	{
		dirp->called_rewinddir = 0;
		DWORD error = GetLastError();
		if (error != ERROR_NO_MORE_FILES)
		{
			map_win32_error_to_wlibc(GetLastError());
		}
		return 0;
	}
	dirp->called_rewinddir = 0;
	return 1;
}

struct dirent *wlibc_readdir(DIR *dirp)
{
	if (wreaddir(dirp) == NULL)
		return NULL;
	memcpy(dirp->_dirent, dirp->_wdirent, offsetof(struct dirent, d_name));
	wcstombs(dirp->_dirent->d_name, dirp->_wdirent->d_name, wcslen(dirp->_wdirent->d_name) + 1);
	return dirp->_dirent;
}

struct wdirent *wlibc_wreaddir(DIR *dirp)
{
	if (dirp == NULL || get_fd_type(dirp->fd) != DIRECTORY_HANDLE)
	{
		errno = EBADF;
		return NULL;
	}

	if (dirp->offset == 0)
	{
		if (!fill_dir_buffer(dirp))
		{
			return NULL;
		}
		dirp->offset = 0;
	}

	PFILE_ID_BOTH_DIR_INFO B = (PFILE_ID_BOTH_DIR_INFO)((char *)dirp->buffer + dirp->offset);

	dirp->_wdirent->d_ino = B->FileId.QuadPart;
	dirp->_wdirent->d_off = dirp->offset;

	DWORD attributes = B->FileAttributes;
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

	dirp->_wdirent->d_reclen = sizeof(FILE_ID_BOTH_DIR_INFO) - sizeof(WCHAR) + B->FileNameLength;
	memcpy(dirp->_wdirent->d_name, B->FileName, B->FileNameLength);
	dirp->_wdirent->d_name[B->FileNameLength / sizeof(wchar_t)] = L'\0';

	dirp->offset = (B->NextEntryOffset == 0 ? 0 : (dirp->offset + B->NextEntryOffset));
	return dirp->_wdirent;
}
