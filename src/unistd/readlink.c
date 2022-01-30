/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <stdlib.h>
#include <unistd.h>

// Convert backslash to forward slash
void convert_bs_to_fs(char *str, int length)
{
	for (int i = 0; i < length; i++)
	{
		if (str[i] == '\\')
		{
			str[i] = '/';
		}
	}
}

ssize_t do_readlink(HANDLE handle, char *restrict buf, size_t bufsiz)
{
	ssize_t result = -1;
	NTSTATUS status;
	IO_STATUS_BLOCK I;
	FILE_ATTRIBUTE_TAG_INFORMATION INFO;

	status = NtQueryInformationFile(handle, &I, &INFO, sizeof(FILE_ATTRIBUTE_TAG_INFORMATION), FileAttributeTagInformation);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	if ((INFO.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) == 0)
	{
		errno = EINVAL;
		return -1;
	}

	// Only support these two
	if (INFO.ReparseTag != IO_REPARSE_TAG_SYMLINK && INFO.ReparseTag != IO_REPARSE_TAG_MOUNT_POINT)
	{
		errno = EINVAL;
		return -1;
	}

	PREPARSE_DATA_BUFFER reparse_buffer = (PREPARSE_DATA_BUFFER)malloc(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
	status =
		NtFsControlFile(handle, NULL, NULL, NULL, &I, FSCTL_GET_REPARSE_POINT, NULL, 0, reparse_buffer, MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		free(reparse_buffer);
		return -1;
	}

	if (reparse_buffer->ReparseTag == IO_REPARSE_TAG_SYMLINK)
	{
		if (reparse_buffer->SymbolicLinkReparseBuffer.PrintNameLength != 0)
		{
			wchar_t *data = (wchar_t *)malloc(reparse_buffer->SymbolicLinkReparseBuffer.PrintNameLength + 2);
			UNICODE_STRING u16_data;
			UTF8_STRING u8_data;
			memcpy(data,
				   reparse_buffer->SymbolicLinkReparseBuffer.PathBuffer +
					   reparse_buffer->SymbolicLinkReparseBuffer.PrintNameOffset / sizeof(WCHAR),
				   reparse_buffer->SymbolicLinkReparseBuffer.PrintNameLength);
			data[reparse_buffer->SymbolicLinkReparseBuffer.PrintNameLength / sizeof(WCHAR)] = L'\0';
			RtlInitUnicodeString(&u16_data, data);
			RtlUnicodeStringToUTF8String(&u8_data, &u16_data, TRUE);
			free(data);
			memcpy(buf, u8_data.Buffer, u8_data.Length <= bufsiz ? u8_data.Length : bufsiz);
			result = u8_data.Length <= bufsiz ? u8_data.Length : bufsiz;
			RtlFreeUTF8String(&u8_data);
		}
		else if (reparse_buffer->SymbolicLinkReparseBuffer.SubstituteNameLength != 0)
		{
			wchar_t *data = (wchar_t *)malloc(reparse_buffer->SymbolicLinkReparseBuffer.SubstituteNameLength + 2);
			UNICODE_STRING u16_data;
			UTF8_STRING u8_data;
			memcpy(data,
				   reparse_buffer->SymbolicLinkReparseBuffer.PathBuffer +
					   reparse_buffer->SymbolicLinkReparseBuffer.SubstituteNameOffset / sizeof(WCHAR),
				   reparse_buffer->SymbolicLinkReparseBuffer.SubstituteNameLength);
			data[reparse_buffer->SymbolicLinkReparseBuffer.SubstituteNameLength / sizeof(WCHAR)] = L'\0';
			RtlInitUnicodeString(&u16_data, data);
			RtlUnicodeStringToUTF8String(&u8_data, &u16_data, TRUE);
			free(data);
			memcpy(buf, u8_data.Buffer, u8_data.Length <= bufsiz ? u8_data.Length : bufsiz);
			result = u8_data.Length <= bufsiz ? u8_data.Length : bufsiz;
			RtlFreeUTF8String(&u8_data);
		}
		else
		{
			// This should not happen
			errno = EBADF;
		}
	}

	if (reparse_buffer->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT)
	{
		if (reparse_buffer->MountPointReparseBuffer.PrintNameLength != 0)
		{
			wchar_t *data = (wchar_t *)malloc(reparse_buffer->MountPointReparseBuffer.SubstituteNameLength + 2);
			UNICODE_STRING u16_data;
			UTF8_STRING u8_data;
			memcpy(data,
				   reparse_buffer->MountPointReparseBuffer.PathBuffer +
					   reparse_buffer->MountPointReparseBuffer.SubstituteNameOffset / sizeof(WCHAR),
				   reparse_buffer->MountPointReparseBuffer.SubstituteNameLength);
			data[reparse_buffer->MountPointReparseBuffer.SubstituteNameLength / sizeof(WCHAR)] = L'\0';
			RtlInitUnicodeString(&u16_data, data);
			RtlUnicodeStringToUTF8String(&u8_data, &u16_data, TRUE);
			free(data);
			memcpy(buf, u8_data.Buffer, u8_data.Length <= bufsiz ? u8_data.Length : bufsiz);
			result = u8_data.Length <= bufsiz ? u8_data.Length : bufsiz;
			RtlFreeUTF8String(&u8_data);
		}
		else if (reparse_buffer->MountPointReparseBuffer.SubstituteNameLength != 0)
		{
			wchar_t *data = (wchar_t *)malloc(reparse_buffer->MountPointReparseBuffer.SubstituteNameLength + 2);
			UNICODE_STRING u16_data;
			UTF8_STRING u8_data;
			memcpy(data,
				   reparse_buffer->MountPointReparseBuffer.PathBuffer +
					   reparse_buffer->MountPointReparseBuffer.SubstituteNameOffset / sizeof(WCHAR),
				   reparse_buffer->MountPointReparseBuffer.SubstituteNameLength);
			data[reparse_buffer->MountPointReparseBuffer.SubstituteNameLength / sizeof(WCHAR)] = L'\0';
			RtlInitUnicodeString(&u16_data, data);
			RtlUnicodeStringToUTF8String(&u8_data, &u16_data, TRUE);
			free(data);
			memcpy(buf, u8_data.Buffer, u8_data.Length <= bufsiz ? u8_data.Length : bufsiz);
			result = u8_data.Length <= bufsiz ? u8_data.Length : bufsiz;
			RtlFreeUTF8String(&u8_data);
		}

		else
		{
			// This should not happen
			errno = EBADF;
		}
	}

	free(reparse_buffer);
	if (result != -1)
	{
		convert_bs_to_fs(buf, result);
	}
	return result;
}

ssize_t common_readlink(int dirfd, const char *restrict path, char *restrict buf, size_t bufsiz)
{
	HANDLE handle = just_open(dirfd, path, FILE_READ_ATTRIBUTES, FILE_OPEN_REPARSE_POINT);
	if (handle == INVALID_HANDLE_VALUE)
	{
		// errno wil be set by just_open
		return -1;
	}

	int result = do_readlink(handle, buf, bufsiz);
	NtClose(handle);

	return result;
}

ssize_t wlibc_common_readlink(int dirfd, const char *restrict path, char *restrict buf, size_t bufsiz)
{
	if (buf == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	if (path == NULL || path[0] == '\0')
	{
		if (validate_fd(dirfd) && (get_fd_flags(dirfd) & (O_NOFOLLOW | O_PATH)) == (O_NOFOLLOW | O_PATH))
		{
			return do_readlink(get_fd_handle(dirfd), buf, bufsiz);
		}

		errno = ENOENT;
		return -1;
	}

	VALIDATE_DIRFD(dirfd);

	return common_readlink(dirfd, path, buf, bufsiz);
}
