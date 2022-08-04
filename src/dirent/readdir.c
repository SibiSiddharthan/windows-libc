/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/dirent.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

struct dirent *do_readdir(DIR *dirstream, struct dirent *entry)
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

	// Copy only the lower 8 bytes, the upper 8 bytes will be zero on NTFS
	entry->d_ino = *(ino_t *)(&direntry->FileId.Identifier);
	entry->d_off = dirstream->offset;

	DWORD attributes = direntry->FileAttributes;
	/* For a junction both FILE_ATTRIBUTE_DIRECTORY and FILE_ATTRIBUTE_REPARSE_POINT is set.
	   To have it as DT_LNK we put this condition first.
	*/
	if (attributes & FILE_ATTRIBUTE_REPARSE_POINT)
	{
		entry->d_type = DT_LNK;
	}
	else if (attributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		entry->d_type = DT_DIR;
	}
	else if ((attributes & ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE |
							 FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_SPARSE_FILE | FILE_ATTRIBUTE_COMPRESSED |
							 FILE_ATTRIBUTE_NOT_CONTENT_INDEXED | FILE_ATTRIBUTE_ENCRYPTED)) == 0)
	{
		entry->d_type = DT_REG;
	}
	else
	{
		entry->d_type = DT_UNKNOWN;
	}

	entry->d_reclen = (uint16_t)(offsetof(FILE_ID_EXTD_BOTH_DIR_INFORMATION, FileName) + direntry->FileNameLength);

	u16_path.Length = (USHORT)direntry->FileNameLength;
	u16_path.MaximumLength = (USHORT)direntry->FileNameLength;
	u16_path.Buffer = direntry->FileName;

	u8_path.Buffer = entry->d_name;
	u8_path.Length = 0;
	u8_path.MaximumLength = 260;

	status = RtlUnicodeStringToUTF8String(&u8_path, &u16_path, FALSE);

	if (status == STATUS_SUCCESS)
	{
		memcpy(entry->d_name, u8_path.Buffer, u8_path.MaximumLength);
		entry->d_namlen = (uint8_t)u8_path.Length; // This does not include the NULL character.
	}
	else
	{
		// Converting the UTF-16 name to UTF-8 has failed. Treat as if the entry has no name.
		// This really should never happen.
		entry->d_namlen = 0;
	}

	dirstream->offset += direntry->NextEntryOffset;
	// Each entry is aligned to a 8 byte boundary, except the last one
	dirstream->read_data += entry->d_reclen;
	if (direntry->NextEntryOffset != 0)
	{
		dirstream->read_data += (entry->d_reclen % sizeof(LONGLONG) == 0 ? 0 : (sizeof(LONGLONG) - entry->d_reclen % sizeof(LONGLONG)));
	}

	return entry;
}

struct dirent *wlibc_readdir(DIR *dirstream)
{
	VALIDATE_DIR_STREAM(dirstream, NULL);

	static struct dirent entry;
	struct dirent *result;

	LOCK_DIR_STREAM(dirstream);
	result = do_readdir(dirstream, &entry);
	UNLOCK_DIR_STREAM(dirstream);

	return result;
}

int wlibc_readdir_r(DIR *restrict dirstream, struct dirent *restrict entry, struct dirent **restrict result)
{
	VALIDATE_DIR_STREAM(dirstream, errno);

	// Save errno and restore it if no error has been encountered.
	errno_t old_errno, new_errno;
	old_errno = errno;
	errno = 0;

	*result = do_readdir(dirstream, entry);
	if (*result == NULL)
	{
		new_errno = errno;
		if (new_errno != 0)
		{
			return new_errno;
		}
	}

	errno = old_errno;
	return 0;
}
