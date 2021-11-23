/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <sys/xattr.h>
#include <sys/types.h>
#include <internal/fcntl.h>
#include <internal/error.h>
#include <stdlib.h>
#include <errno.h>
#include <wchar.h>

ssize_t do_getxattr(HANDLE handle, const char *restrict name, void *restrict value, size_t size)
{
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	PFILE_GET_EA_INFORMATION name_info;
	PFILE_FULL_EA_INFORMATION ea_info;

	int length;
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
	name_info->EaNameLength = length;
	memcpy(name_info->EaName, name, length);

	query_buffer = (char *)malloc(query_buffer_size);

	status = NtQueryEaFile(handle, &io, query_buffer, query_buffer_size, FALSE, ea_name_buffer, 260, NULL, FALSE);
	if (status != STATUS_SUCCESS)
	{
		if (status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL)
		{
			query_buffer_size = 65536; // Use this big of a buffer, fail if this also fails
			query_buffer = (char *)realloc(query_buffer, query_buffer_size);
			status = NtQueryEaFile(handle, &io, query_buffer, query_buffer_size, FALSE, ea_name_buffer, 260, NULL, FALSE);
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
	free(query_buffer);
	return result;
}

ssize_t common_getxattr(int fd, const char *restrict path, const char *restrict name, void *restrict value, size_t size, int flags)
{
	wchar_t *u16_ntpath = get_absolute_ntpath(fd, path);
	if (u16_ntpath == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	HANDLE handle = just_open(u16_ntpath, FILE_READ_EA, 0, FILE_OPEN, flags == AT_SYMLINK_NOFOLLOW ? FILE_OPEN_REPARSE_POINT : 0);
	free(u16_ntpath);
	if (handle == INVALID_HANDLE_VALUE)
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

	VALIDATE_PATH(name, EINVAL, -1); // Need to have a different macro for this

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
		enum handle_type type = get_fd_type(fd);
		if (type != FILE_HANDLE && type != DIRECTORY_HANDLE)
		{
			errno = EBADF;
			return -1;
		}

		HANDLE handle = get_fd_handle(fd);
		return do_getxattr(handle, name, value, size);
	}
}
