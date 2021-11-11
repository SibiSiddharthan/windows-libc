/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <wchar.h>
#include <internal/misc.h>
#include <errno.h>
#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <stdlib.h>

int common_symlink(const char *restrict source, int dirfd, const char *restrict target)
{
	wchar_t *u16_nttarget = get_absolute_ntpath(dirfd, target);
	if (u16_nttarget == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	bool is_absolute = false;
	if (isalpha(source[0]) && source[1] == ':')
	{
		is_absolute = true;
	}

	// Try opening the source
	wchar_t *u16_ntsource = get_absolute_ntpath(dirfd, source);
	if (u16_ntsource == NULL)
	{
		errno = ENOENT;
		free(u16_nttarget);
		return -1;
	}

	HANDLE source_handle = just_open(u16_ntsource, FILE_READ_ATTRIBUTES, 0, FILE_OPEN, FILE_OPEN_REPARSE_POINT | FILE_NON_DIRECTORY_FILE);
	free(u16_ntsource);

	ULONG options = FILE_NON_DIRECTORY_FILE;
	if (source_handle == INVALID_HANDLE_VALUE)
	{
		if (errno == EISDIR)
		{
			errno = 0; // clear errno
			options = FILE_DIRECTORY_FILE;
		}
		if (errno == ENOENT)
		{
			errno = 0; // clear errno
		}
	}
	else // file exists and is a normal file, close the handle
	{
		NtClose(source_handle);
	}

	HANDLE target_handle = just_open(u16_nttarget, FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES, 0, FILE_CREATE, options);
	free(u16_nttarget);
	if (target_handle == INVALID_HANDLE_VALUE)
	{
		// errno wil be set by just_open
		return -1;
	}

	UTF8_STRING u8_source;
	UNICODE_STRING u16_source;
	RtlInitUTF8String(&u8_source, source);
	RtlUTF8StringToUnicodeString(&u16_source, &u8_source, TRUE);

	// convert all forward slashes to back slashes
	for (int i = 0; u16_source.Buffer[i] != L'\0'; i++)
	{
		if (u16_source.Buffer[i] == L'/')
		{
			u16_source.Buffer[i] = L'\\';
		}
	}

	size_t total_reparse_data_length =
		REPARSE_DATA_BUFFER_HEADER_SIZE + 12 + 2 * u16_source.Length + (is_absolute ? 8 : 0); // For SubstituteName, PrintName
	IO_STATUS_BLOCK I;
	PREPARSE_DATA_BUFFER reparse_data = (PREPARSE_DATA_BUFFER)malloc(total_reparse_data_length);
	memset(reparse_data, 0, total_reparse_data_length);

	reparse_data->ReparseTag = IO_REPARSE_TAG_SYMLINK;
	reparse_data->ReparseDataLength = total_reparse_data_length - REPARSE_DATA_BUFFER_HEADER_SIZE;
	reparse_data->SymbolicLinkReparseBuffer.Flags = is_absolute ? 0 : SYMLINK_FLAG_RELATIVE;
	reparse_data->SymbolicLinkReparseBuffer.PrintNameOffset = 0;
	reparse_data->SymbolicLinkReparseBuffer.PrintNameLength = u16_source.Length;
	reparse_data->SymbolicLinkReparseBuffer.SubstituteNameOffset = u16_source.Length;
	reparse_data->SymbolicLinkReparseBuffer.SubstituteNameLength = u16_source.Length + (is_absolute ? 8 : 0); // '\??\'

	// copy PrintName
	memcpy(reparse_data->SymbolicLinkReparseBuffer.PathBuffer, u16_source.Buffer, u16_source.Length);
	// copy SubstituteName
	if (is_absolute)
	{
		memcpy(reparse_data->SymbolicLinkReparseBuffer.PathBuffer + u16_source.Length / sizeof(WCHAR), L"\\??\\", 4 * sizeof(WCHAR));
	}
	memcpy(reparse_data->SymbolicLinkReparseBuffer.PathBuffer + (u16_source.Length + (is_absolute ? 8 : 0)) / sizeof(WCHAR),
		   u16_source.Buffer, u16_source.Length);
	RtlFreeUnicodeString(&u16_source);

	NTSTATUS status =
		NtFsControlFile(target_handle, NULL, NULL, NULL, &I, FSCTL_SET_REPARSE_POINT, reparse_data, total_reparse_data_length, NULL, 0);

	free(reparse_data);
	NtClose(target_handle);

	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}

int wlibc_common_symlink(const char *restrict source, int dirfd, const char *restrict target)
{
	VALIDATE_PATH(source, EINVAL, -1);
	VALIDATE_PATH_AND_DIRFD(target, dirfd);

	return common_symlink(source, dirfd, target);
}

#if 0

int wlibc_symlink(const char *restrict source, const char *restrict target)
{
	if (source == NULL || target == NULL)
	{
		errno = ENOENT;
		return -1;
	}
	wchar_t *wsource = mb_to_wc(source);
	wchar_t *wtarget = mb_to_wc(target);
	int status = common_symlink(wsource, wtarget);
	free(wsource);
	free(wtarget);

	return status;
}

int wlibc_wsymlink(const wchar_t *restrict wsource, const wchar_t *restrict wtarget)
{
	if (wsource == NULL || wtarget == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	return common_symlink(wsource, wtarget);
}

#endif
