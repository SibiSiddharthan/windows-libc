/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <wchar.h>
#include <internal/misc.h>
#include <errno.h>
#include <Windows.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <internal/nt.h>
#include <stdlib.h>

#if 0
int common_symlink(const wchar_t *restrict wsource, const wchar_t *restrict wtarget)
{
	DWORD attributes;
	DWORD flags = SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;

	attributes = GetFileAttributes(wtarget);
	if (attributes != INVALID_FILE_ATTRIBUTES)
	{
		errno = EEXIST;
		return -1;
	}

	int wtarget_length = wcslen(wtarget);
	int last_slash_pos = 0;
	for (int i = wtarget_length - 1; i > 0; i--)
	{
		if (wtarget[i] == L'/' || wtarget[i] == L'\\')
		{
			last_slash_pos = i;
			break;
		}
	}

	// Symlink is created across directories in a relative manner
	// We find the type of file here
	int wsource_length = wcslen(wsource);
	if (last_slash_pos != 0 && !is_absolute_pathw(wsource))
	{
		wchar_t *wsource_final = (wchar_t *)malloc(sizeof(wchar_t) * (wsource_length + last_slash_pos + 2)); // '\0','/'
		wcsncpy(wsource_final, wtarget, last_slash_pos + 1);
		wsource_final[last_slash_pos + 1] = L'\0';
		wcscat(wsource_final, wsource);
		attributes = GetFileAttributes(wsource_final);
		if (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			flags |= SYMBOLIC_LINK_FLAG_DIRECTORY;
		}
		free(wsource_final);
	}
	else
	{
		attributes = GetFileAttributes(wsource);
		if (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			flags |= SYMBOLIC_LINK_FLAG_DIRECTORY;
		}
	}

	// wsource needs to have backslashes only
	wchar_t *wsource_bs = (wchar_t *)malloc(sizeof(wchar_t) * (wsource_length + 1));
	wcscpy(wsource_bs, wsource);

	wchar_t *temp = wsource_bs;
	while (*wsource_bs != L'\0')
	{
		if (*wsource_bs == L'/')
			*wsource_bs = L'\\';
		++wsource_bs;
	}
	wsource_bs = temp;

	if (!CreateSymbolicLink(wtarget, wsource_bs, flags))
	{
		map_win32_error_to_wlibc(GetLastError());
		return -1;
	}
	free(wsource_bs);

	return 0;
}
#endif

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

	size_t total_reparse_data_length = REPARSE_DATA_BUFFER_HEADER_SIZE + 12 + 2 * u16_source.Length; // For SubstituteName, PrintName
	IO_STATUS_BLOCK I;
	PREPARSE_DATA_BUFFER reparse_data = (PREPARSE_DATA_BUFFER)malloc(total_reparse_data_length);
	memset(reparse_data, 0, total_reparse_data_length);

	reparse_data->ReparseTag = IO_REPARSE_TAG_SYMLINK;
	reparse_data->ReparseDataLength = 12 + 2 * u16_source.Length;
	reparse_data->SymbolicLinkReparseBuffer.Flags = is_absolute ? 0 : SYMLINK_FLAG_RELATIVE;
	reparse_data->SymbolicLinkReparseBuffer.SubstituteNameOffset = 0;
	reparse_data->SymbolicLinkReparseBuffer.SubstituteNameLength = u16_source.Length;
	reparse_data->SymbolicLinkReparseBuffer.PrintNameOffset = u16_source.Length;
	reparse_data->SymbolicLinkReparseBuffer.PrintNameLength = u16_source.Length;
	memcpy(reparse_data->SymbolicLinkReparseBuffer.PathBuffer, u16_source.Buffer, u16_source.Length);
	memcpy(reparse_data->SymbolicLinkReparseBuffer.PathBuffer + u16_source.Length / sizeof(WCHAR), u16_source.Buffer, u16_source.Length);
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
	VALIDATE_PATH(source, EINVAL);
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
