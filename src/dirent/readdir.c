/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/dirent.h>
#include <dirent.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>
#include <internal/fcntl.h>
#include <internal/misc.h>
#include <internal/error.h>

#if 0
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

#endif

struct dirent *do_readdir(DIR *dirstream)
{
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	UTF8_STRING u8_path;
	UNICODE_STRING u16_path;

	if (dirstream->read_data == dirstream->received_data)
	{
		memset(dirstream->buffer, 0, DIRENT_DIR_BUFFER_SIZE);
		status = NtQueryDirectoryFileEx(get_fd_handle(dirstream->fd), NULL, NULL, NULL, &io, dirstream->buffer, DIRENT_DIR_BUFFER_SIZE,
										FileIdExtdBothDirectoryInformation, 0, NULL);
		if (status != STATUS_SUCCESS)
		{
			if (status != STATUS_NO_MORE_FILES)
			{
				map_ntstatus_to_errno(status);
			}
			return NULL;
		}
		dirstream->offset = 0;
		dirstream->received_data = io.Information;
	}

	PFILE_ID_EXTD_BOTH_DIR_INFORMATION direntry = (PFILE_ID_EXTD_BOTH_DIR_INFORMATION)((char *)dirstream->buffer + dirstream->offset);

	// copy only the lower 8 bytes, the upper 8 bytes will be zero on NTFS
	dirstream->info->d_ino = *(ino_t *)(&direntry->FileId.Identifier);
	dirstream->info->d_off = dirstream->offset;

	DWORD attributes = direntry->FileAttributes;
	/* For a junction both FILE_ATTRIBUTE_DIRECTORY and FILE_ATTRIBUTE_REPARSE_POINT is set.
	   To have it as DT_LNK we put this condition first.
	*/
	if (attributes & FILE_ATTRIBUTE_REPARSE_POINT)
	{
		dirstream->info->d_type = DT_LNK;
	}
	else if (attributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		dirstream->info->d_type = DT_DIR;
	}
	else if ((attributes & ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE |
							 FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_SPARSE_FILE | FILE_ATTRIBUTE_COMPRESSED |
							 FILE_ATTRIBUTE_NOT_CONTENT_INDEXED | FILE_ATTRIBUTE_ENCRYPTED)) == 0)
	{
		dirstream->info->d_type = DT_REG;
	}
	else
	{
		dirstream->info->d_type = DT_UNKNOWN;
	}

	dirstream->info->d_reclen = offsetof(FILE_ID_EXTD_BOTH_DIR_INFORMATION, FileName) + direntry->FileNameLength;

	u16_path.Length = direntry->FileNameLength;
	u16_path.MaximumLength = direntry->FileNameLength;
	u16_path.Buffer = direntry->FileName;

	RtlUnicodeStringToUTF8String(&u8_path, &u16_path, TRUE);
	memcpy(dirstream->info->d_name, u8_path.Buffer, u8_path.MaximumLength);
	RtlFreeUTF8String(&u8_path);
	// dirstream->info->d_name[direntry->FileNameLength / sizeof(WCHAR)] = L'\0';

	dirstream->offset += direntry->NextEntryOffset;
	// each entry is aligned to a 8 byte boundary, except the last one
	dirstream->read_data += dirstream->info->d_reclen;
	if (direntry->NextEntryOffset != 0)
	{
		dirstream->read_data +=
			(dirstream->info->d_reclen % sizeof(LONGLONG) == 0 ? 0 : (sizeof(LONGLONG) - dirstream->info->d_reclen % sizeof(LONGLONG)));
	}

	return dirstream->info;
}

struct dirent *wlibc_readdir(DIR *dirstream)
{
	VALIDATE_DIR_STREAM(dirstream, NULL);

	struct dirent *info = NULL;

	LOCK_DIR_STREAM(dirstream);
	info = do_readdir(dirstream);
	UNLOCK_DIR_STREAM(dirstream);

	return info;
}
