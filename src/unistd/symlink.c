/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <internal/path.h>
#include <internal/security.h>
#include <unistd.h>

// Symlinking to root '/' requires special handling.
int symlink_root(int dirfd, const char *restrict target, mode_t mode)
{
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	HANDLE target_handle;
	OBJECT_ATTRIBUTES object;
	WCHAR root_drive[7]; // "\??\C:\"
	UNICODE_STRING *u16_nttarget = NULL;
	PSECURITY_DESCRIPTOR security_descriptor = NULL;

	u16_nttarget = get_absolute_ntpath(dirfd, target);
	if (u16_nttarget == NULL)
	{
		// errno will be set by `get_absolute_ntpath`.
		return -1;
	}

	// Efficient Loading.
	*(ULONGLONG *)&root_drive[0] = *(LONGLONG *)L"\\??\\";
	root_drive[4] = NtCurrentPeb()->ProcessParameters->CurrentDirectory.DosPath.Buffer[0];
	*(ULONG *)&root_drive[5] = *(ULONG *)L":\\";

	security_descriptor = (PSECURITY_DESCRIPTOR)get_security_descriptor(mode & 0777, 1);
	InitializeObjectAttributes(&object, u16_nttarget, OBJ_CASE_INSENSITIVE, NULL, security_descriptor);

	status = NtCreateFile(&target_handle, FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | SYNCHRONIZE, &object, &io, NULL, 0,
						  FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_CREATE, FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, u16_nttarget);

	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	USHORT total_reparse_data_length =
		REPARSE_DATA_BUFFER_HEADER_SIZE + 12 + 6 + 14; // 6 -> "C:\" (PrintName), 14 -> "\??\C:\" (SubstituteName).
	PREPARSE_DATA_BUFFER reparse_data =
		(PREPARSE_DATA_BUFFER)RtlAllocateHeap(NtCurrentProcessHeap(), HEAP_ZERO_MEMORY, total_reparse_data_length);

	if (reparse_data == NULL)
	{
		errno = ENOMEM;
		return -1;
	}

	reparse_data->ReparseTag = IO_REPARSE_TAG_SYMLINK;
	reparse_data->ReparseDataLength = total_reparse_data_length - REPARSE_DATA_BUFFER_HEADER_SIZE;
	reparse_data->SymbolicLinkReparseBuffer.Flags = 0;
	reparse_data->SymbolicLinkReparseBuffer.PrintNameOffset = 0;
	reparse_data->SymbolicLinkReparseBuffer.PrintNameLength = 6;
	reparse_data->SymbolicLinkReparseBuffer.SubstituteNameOffset = 6;
	reparse_data->SymbolicLinkReparseBuffer.SubstituteNameLength = 14;

	// copy PrintName
	memcpy(reparse_data->SymbolicLinkReparseBuffer.PathBuffer, &root_drive[4], 6);
	// copy SubstituteName
	memcpy((CHAR *)reparse_data->SymbolicLinkReparseBuffer.PathBuffer + 6, root_drive, 14);

	status =
		NtFsControlFile(target_handle, NULL, NULL, NULL, &io, FSCTL_SET_REPARSE_POINT, reparse_data, total_reparse_data_length, NULL, 0);

	RtlFreeHeap(NtCurrentProcessHeap(), 0, reparse_data);
	NtClose(target_handle);

	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}

int common_symlink(const char *restrict source, int dirfd, const char *restrict target, mode_t mode)
{
	int result = -1;
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	HANDLE target_handle;
	UNICODE_STRING *u16_nttarget = NULL, *u16_ntsource = NULL;
	OBJECT_ATTRIBUTES object;
	PSECURITY_DESCRIPTOR security_descriptor = NULL;

	if (IS_ROOT_PATH(source))
	{
		return symlink_root(dirfd, target, mode);
	}

	u16_nttarget = get_absolute_ntpath(dirfd, target);
	if (u16_nttarget == NULL)
	{
		// errno will be set by `get_absolute_ntpath`.
		return -1;
	}

	char *normalized_source = NULL;
	size_t target_length = strlen(target);
	size_t source_length = strlen(source);
	bool is_absolute = false;

	if (IS_ABSOLUTE_PATH(source))
	{
		is_absolute = true;
	}

	// Try opening the source
	if (!is_absolute)
	{
		normalized_source = RtlAllocateHeap(NtCurrentProcessHeap(), 0, target_length + source_length + 5); // '/../' + NULL
		if (normalized_source == NULL)
		{
			errno = ENOMEM;
			goto finish;
		}

		// This is an easy way of checking whether the link text if it exists is a directory or a file.
		memcpy(normalized_source, target, target_length + 1); // Including NULL
		if (target[target_length - 1] != '/' && target[target_length - 1] != '\\')
		{
			strcat(normalized_source, "/");
		}
		strcat(normalized_source, "../");
		strcat(normalized_source, source);

		u16_ntsource = get_absolute_ntpath(dirfd, normalized_source);
		RtlFreeHeap(NtCurrentProcessHeap(), 0, normalized_source);
	}
	else
	{
		// Source is an absolute path. Just use it as is.
		u16_ntsource = get_absolute_ntpath(dirfd, source);
	}

	if (u16_ntsource == NULL)
	{
		// Bad path
		// errno will be set by 'get_absolute_ntpath'.
		goto finish;
	}

	ULONG options = FILE_NON_DIRECTORY_FILE;
	HANDLE source_handle = just_open2(u16_ntsource, FILE_READ_ATTRIBUTES, FILE_OPEN_REPARSE_POINT | FILE_NON_DIRECTORY_FILE);

	if (source_handle == NULL)
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
	else // file exists and is a normal file, close the handle.
	{
		NtClose(source_handle);
	}

	security_descriptor = (PSECURITY_DESCRIPTOR)get_security_descriptor(mode & 0777, options == FILE_DIRECTORY_FILE ? 1 : 0);
	InitializeObjectAttributes(&object, u16_nttarget, OBJ_CASE_INSENSITIVE, NULL, security_descriptor);

	// Open synchronously as NtFsControlFile might return STATUS_PENDING.
	status = NtCreateFile(&target_handle, FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | SYNCHRONIZE, &object, &io, NULL, 0,
						  FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_CREATE, options | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	// Set the reparse data
	UTF8_STRING u8_source;
	UNICODE_STRING u16_source;

	u8_source.Length = (USHORT)source_length;
	u8_source.MaximumLength = (USHORT)(source_length + 1);
	u8_source.Buffer = (PCHAR)source;

	status = RtlUTF8StringToUnicodeString(&u16_source, &u8_source, TRUE);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		goto finish;
	}

	// Handle cygwin paths
	if (u16_source.Buffer[0] == L'/' && isalpha((char)u16_source.Buffer[1]))
	{
		// Change /c to C:.
		u16_source.Buffer[0] = towupper(u16_source.Buffer[1]);
		u16_source.Buffer[1] = L':';
	}

	// Convert all forward slashes to back slashes.
	for (int i = 0; u16_source.Buffer[i] != L'\0'; i++)
	{
		if (u16_source.Buffer[i] == L'/')
		{
			u16_source.Buffer[i] = L'\\';
		}
	}

	if (is_absolute)
	{
		// Convert drive letter to uppercase.
		u16_source.Buffer[0] = towupper(u16_source.Buffer[0]);

		// Handle root drive.
		if (u16_source.MaximumLength == 3 * sizeof(WCHAR)) // "C:\"
		{
			// If suppose the source reads "C:". We need to append a trailing slash
			// so that it gets intepreted as directory.
			// Since `RtlUTF8StringToUnicodeString` appends a terminating NULL, use those
			// 2 bytes to insert a '\'.
			u16_source.Buffer[2] = L'\\';
			u16_source.Length += sizeof(WCHAR);
		}
	}

	USHORT total_reparse_data_length =
		REPARSE_DATA_BUFFER_HEADER_SIZE + 12 + 2 * u16_source.Length + (is_absolute ? 8 : 0); // For SubstituteName, PrintName
	PREPARSE_DATA_BUFFER reparse_data =
		(PREPARSE_DATA_BUFFER)RtlAllocateHeap(NtCurrentProcessHeap(), HEAP_ZERO_MEMORY, total_reparse_data_length);

	if (reparse_data == NULL)
	{
		errno = ENOMEM;
		goto finish;
	}

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

	status =
		NtFsControlFile(target_handle, NULL, NULL, NULL, &io, FSCTL_SET_REPARSE_POINT, reparse_data, total_reparse_data_length, NULL, 0);

	RtlFreeHeap(NtCurrentProcessHeap(), 0, reparse_data);
	NtClose(target_handle);

	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		goto finish;
	}

	result = 0;

finish:
	RtlFreeHeap(NtCurrentProcessHeap(), 0, u16_nttarget);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, u16_ntsource);
	return result;
}

int wlibc_common_symlink(const char *restrict source, int dirfd, const char *restrict target, mode_t mode)
{
	VALIDATE_PATH(source, EINVAL, -1);
	VALIDATE_PATH_AND_DIRFD(target, dirfd);

	return common_symlink(source, dirfd, target, mode);
}
