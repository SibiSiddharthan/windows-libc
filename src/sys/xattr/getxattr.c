/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <internal/validate.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/xattr.h>

ssize_t do_getxattr(HANDLE handle, const char *restrict name, void *restrict value, size_t size)
{
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	PFILE_GET_EA_INFORMATION name_info;
	PFILE_FULL_EA_INFORMATION ea_info;

	size_t length;
	char ea_name_buffer[260];
	char *query_buffer = NULL;
	size_t query_buffer_size = 1024; // start with 1024 bytes, grow on failure
	ssize_t result = -1;

	length = strlen(name);
	if (length > 254) // max UCHAR - 1
	{
		errno = E2BIG;
		goto finish;
	}

	memset(ea_name_buffer, 0, 260);

	name_info = (PFILE_GET_EA_INFORMATION)ea_name_buffer;
	name_info->NextEntryOffset = 0;
	name_info->EaNameLength = (UCHAR)length;
	memcpy(name_info->EaName, name, length);

	query_buffer = (char *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, query_buffer_size);
	if (query_buffer == NULL)
	{
		errno = ENOMEM;
		return -1;
	}

	status = NtQueryEaFile(handle, &io, query_buffer, (ULONG)query_buffer_size, FALSE, ea_name_buffer, 260, NULL, FALSE);
	if (status != STATUS_SUCCESS)
	{
		if (status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL)
		{
			query_buffer_size = 65536; // Use this big of a buffer, fail if this also fails
			query_buffer = (char *)RtlReAllocateHeap(NtCurrentProcessHeap(), 0, query_buffer, query_buffer_size);

			if (query_buffer == NULL)
			{
				errno = ENOMEM;
				goto finish;
			}

			status = NtQueryEaFile(handle, &io, query_buffer, (ULONG)query_buffer_size, FALSE, ea_name_buffer, 260, NULL, FALSE);
			if (status != STATUS_SUCCESS)
			{
				map_ntstatus_to_errno(status);
				goto finish;
			}
		}
		else
		{
			map_ntstatus_to_errno(status);
			goto finish;
		}
	}

	ea_info = (PFILE_FULL_EA_INFORMATION)query_buffer;

	if (ea_info->EaValueLength == 0) // attribute not present
	{
		errno = ENODATA;
		goto finish;
	}

	if (size == 0)
	{
		result = ea_info->EaValueLength;
		goto finish;
	}

	if (ea_info->EaValueLength > size)
	{
		errno = ERANGE;
		goto finish;
	}

	memcpy(value, ea_info->EaName + ea_info->EaNameLength + 1, ea_info->EaValueLength);
	result = ea_info->EaValueLength;

finish:
	RtlFreeHeap(NtCurrentProcessHeap(), 0, query_buffer);
	return result;
}

ssize_t common_getxattr(int fd, const char *restrict path, const char *restrict name, void *restrict value, size_t size, int flags)
{
	HANDLE handle = just_open(fd, path, FILE_READ_EA | SYNCHRONIZE,
							  FILE_SYNCHRONOUS_IO_ALERT | (flags == AT_SYMLINK_NOFOLLOW ? FILE_OPEN_REPARSE_POINT : 0));
	if (handle == NULL)
	{
		// errno wil be set by just_open
		return -1;
	}

	ssize_t result = do_getxattr(handle, name, value, size);
	NtClose(handle);
	return result;
}

ssize_t wlibc_common_getxattr(int fd, const char *restrict path, const char *restrict name, void *restrict value, size_t size, int flags)
{
	if (flags != 0 && flags != AT_EMPTY_PATH && flags != AT_SYMLINK_NOFOLLOW)
	{
		errno = EINVAL;
		return -1;
	}

	VALIDATE_STRING(name, EINVAL, -1);

	if (size != 0)
	{
		if (value == NULL)
		{
			errno = EINVAL;
			return -1;
		}
	}

	if (flags != AT_EMPTY_PATH)
	{
		VALIDATE_PATH(path, ENOENT, -1);
		return common_getxattr(fd, path, name, value, size, flags);
	}
	else
	{
		fdinfo info;
		get_fdinfo(fd, &info);

		if (info.type != FILE_HANDLE && info.type != DIRECTORY_HANDLE)
		{
			errno = EBADF;
			return -1;
		}

		return do_getxattr(info.handle, name, value, size);
	}
}
