/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <unistd.h>

// Convert backslash to forward slash
static void convert_bs_to_fs(char *str, ssize_t length)
{
	for (ssize_t i = 0; i < length; i++)
	{
		if (str[i] == '\\')
		{
			str[i] = '/';
		}
	}
}

ssize_t do_readlink(HANDLE handle, char *restrict buffer, size_t bufsiz)
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

	PREPARSE_DATA_BUFFER reparse_buffer =
		(PREPARSE_DATA_BUFFER)RtlAllocateHeap(NtCurrentProcessHeap(), 0, MAXIMUM_REPARSE_DATA_BUFFER_SIZE);

	if (reparse_buffer == NULL)
	{
		errno = ENOMEM;
		return -1;
	}

	status =
		NtFsControlFile(handle, NULL, NULL, NULL, &I, FSCTL_GET_REPARSE_POINT, NULL, 0, reparse_buffer, MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		goto finish;
	}

	UNICODE_STRING u16_data;
	UTF8_STRING u8_data;

	u8_data.Buffer = buffer;
	u8_data.Length = 0;
	u8_data.MaximumLength = (USHORT)bufsiz;

	if (reparse_buffer->ReparseTag == IO_REPARSE_TAG_SYMLINK)
	{
		if (reparse_buffer->SymbolicLinkReparseBuffer.PrintNameLength != 0)
		{
			u16_data.Buffer = (WCHAR *)((CHAR *)(reparse_buffer->SymbolicLinkReparseBuffer.PathBuffer) +
										reparse_buffer->SymbolicLinkReparseBuffer.PrintNameOffset);
			u16_data.Length = reparse_buffer->SymbolicLinkReparseBuffer.PrintNameLength;
			u16_data.MaximumLength = reparse_buffer->SymbolicLinkReparseBuffer.PrintNameLength;
		}
		else if (reparse_buffer->SymbolicLinkReparseBuffer.SubstituteNameLength != 0)
		{
			u16_data.Buffer = (WCHAR *)((CHAR *)(reparse_buffer->SymbolicLinkReparseBuffer.PathBuffer) +
										reparse_buffer->SymbolicLinkReparseBuffer.SubstituteNameOffset);
			u16_data.Length = reparse_buffer->SymbolicLinkReparseBuffer.SubstituteNameLength;
			u16_data.MaximumLength = reparse_buffer->SymbolicLinkReparseBuffer.SubstituteNameLength;
		}
		else
		{
			// This should not happen.
			errno = EBADF;
			goto finish;
		}
	}

	if (reparse_buffer->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT)
	{
		if (reparse_buffer->MountPointReparseBuffer.PrintNameLength != 0)
		{
			u16_data.Buffer = (WCHAR *)((CHAR *)(reparse_buffer->MountPointReparseBuffer.PathBuffer) +
										reparse_buffer->MountPointReparseBuffer.PrintNameOffset);
			u16_data.Length = reparse_buffer->MountPointReparseBuffer.PrintNameLength;
			u16_data.MaximumLength = reparse_buffer->MountPointReparseBuffer.PrintNameLength;
		}
		else if (reparse_buffer->MountPointReparseBuffer.SubstituteNameLength != 0)
		{
			u16_data.Buffer = (WCHAR *)((CHAR *)(reparse_buffer->MountPointReparseBuffer.PathBuffer) +
										reparse_buffer->MountPointReparseBuffer.SubstituteNameOffset);
			u16_data.Length = reparse_buffer->MountPointReparseBuffer.SubstituteNameLength;
			u16_data.MaximumLength = reparse_buffer->MountPointReparseBuffer.SubstituteNameLength;
		}
		else
		{
			// This should not happen.
			errno = EBADF;
			goto finish;
		}
	}

	status = RtlUnicodeStringToUTF8String(&u8_data, &u16_data, FALSE);
	if (status != STATUS_SUCCESS)
	{
		if (status == STATUS_BUFFER_TOO_SMALL)
		{
			// Newer versions of `RtlUnicodeStringToUTF8String` don't truncate if the the buffer is too small.
			// In this case let the routine allocate a temporary buffer and copy the contents to the result.
			status = RtlUnicodeStringToUTF8String(&u8_data, &u16_data, TRUE);
			if (status != STATUS_SUCCESS)
			{
				map_ntstatus_to_errno(status);
				goto finish;
			}

			// Copy the contents to the resultant buffer(truncate upto bufsiz).
			memcpy(buffer, u8_data.Buffer, bufsiz);
			RtlFreeUTF8String(&u8_data);
			result = bufsiz;
		}
	}
	else
	{
		result = u8_data.Length;
	}

	if (result != -1)
	{
		convert_bs_to_fs(buffer, result);
	}

finish:
	RtlFreeHeap(NtCurrentProcessHeap(), 0, reparse_buffer);
	return result;
}

ssize_t common_readlink(int dirfd, const char *restrict path, char *restrict buffer, size_t bufsiz)
{
	HANDLE handle = just_open(dirfd, path, FILE_READ_ATTRIBUTES, FILE_OPEN_REPARSE_POINT);
	if (handle == NULL)
	{
		// errno wil be set by just_open
		return -1;
	}

	ssize_t result = do_readlink(handle, buffer, bufsiz);
	NtClose(handle);

	return result;
}

ssize_t wlibc_common_readlink(int dirfd, const char *restrict path, char *restrict buffer, size_t bufsiz)
{
	if (buffer == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	if (path == NULL || path[0] == '\0')
	{
		if (validate_fd(dirfd) && (get_fd_flags(dirfd) & (O_NOFOLLOW | O_PATH)) == (O_NOFOLLOW | O_PATH))
		{
			return do_readlink(get_fd_handle(dirfd), buffer, bufsiz);
		}

		errno = ENOENT;
		return -1;
	}

	VALIDATE_DIRFD(dirfd);

	return common_readlink(dirfd, path, buffer, bufsiz);
}
