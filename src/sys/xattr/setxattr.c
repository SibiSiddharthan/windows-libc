/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/xattr.h>

int do_setxattr(HANDLE handle, const char *restrict name, const void *restrict value, size_t size, int operation)
{
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	PFILE_GET_EA_INFORMATION name_info;
	PFILE_FULL_EA_INFORMATION ea_info;

	int length;
	char check_buffer[260];
	char query_buffer[512];
	char *insert_buffer = NULL;
	size_t size_of_insert_buffer;
	ssize_t result = -1;

	length = strlen(name);
	if (length > 254) // max UCHAR - 1
	{
		errno = E2BIG;
		goto finish;
	}

	memset(check_buffer, 0, 260);

	name_info = (PFILE_GET_EA_INFORMATION)check_buffer;
	name_info->NextEntryOffset = 0;
	name_info->EaNameLength = length;
	memcpy(name_info->EaName, name, length);

	if (operation != 0)
	{
		// Check whether the attribute exists or not
		status = NtQueryEaFile(handle, &io, query_buffer, 512, FALSE, check_buffer, 260, NULL, FALSE);
		ea_info = (PFILE_FULL_EA_INFORMATION)query_buffer;
		if (operation == XATTR_CREATE)
		{
			if (status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL ||
				(status == STATUS_SUCCESS && ea_info->EaValueLength > 0))
			{
				// attribute exists
				errno = EEXIST;
				goto finish;
			}
		}
		// Even if the file has no extended attributes doing a specified query (i.e specifying FILE_GET_EA_INFORMATION in NtQueryEaFile)
		// return a single entry with EaValueLength set to 0
		if (operation == XATTR_REPLACE || operation == XATTR_DELETE)
		{
			if (status == STATUS_SUCCESS && ea_info->EaValueLength == 0)
			{
				// attribute does not exist
				errno = ENODATA;
				goto finish;
			}
		}
	}

	size_of_insert_buffer = sizeof(FILE_FULL_EA_INFORMATION) + length + 1 + size;
	insert_buffer = (char *)malloc(size_of_insert_buffer);
	memset(insert_buffer, 0, size_of_insert_buffer);
	ea_info = (PFILE_FULL_EA_INFORMATION)insert_buffer;

	ea_info->EaNameLength = length;
	ea_info->EaValueLength = size;
	memcpy(ea_info->EaName, name, length + 1);
	if (size > 0)
	{
		memcpy(ea_info->EaName + length + 1, value, size);
	}

	status = NtSetEaFile(handle, &io, ea_info, size_of_insert_buffer);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		goto finish;
	}

	result = 0;
finish:
	free(insert_buffer);
	return result;
}

int common_setxattr(int fd, const char *restrict path, const char *restrict name, const void *restrict value, size_t size, int operation,
					int flags)
{
	HANDLE handle = just_open(fd, path, FILE_READ_EA | FILE_WRITE_EA | SYNCHRONIZE,
							  FILE_SYNCHRONOUS_IO_ALERT | (flags == AT_SYMLINK_NOFOLLOW ? FILE_OPEN_REPARSE_POINT : 0));
	if (handle == INVALID_HANDLE_VALUE)
	{
		// errno wil be set by just_open
		return -1;
	}

	ssize_t result = do_setxattr(handle, name, value, size, operation);
	NtClose(handle);
	return result;
}

int wlibc_common_setxattr(int fd, const char *restrict path, const char *restrict name, const void *restrict value, size_t size,
						  int operation, int flags)
{
	if (operation < 0 || operation > 3)
	{
		errno = EINVAL;
		return -1;
	}

	if (operation != XATTR_DELETE)
	{
		if (value == NULL || size == 0)
		{
			errno = EINVAL;
			return -1;
		}
	}
	if (operation == XATTR_DELETE)
	{
		if (value != NULL && size != 0)
		{
			errno = EINVAL;
			return -1;
		}
	}

	if (size > 65535) // max USHORT
	{
		errno = E2BIG;
		return -1;
	}

	if (flags != 0 && flags != AT_EMPTY_PATH && flags != AT_SYMLINK_NOFOLLOW)
	{
		errno = EINVAL;
		return -1;
	}

	VALIDATE_PATH(name, EINVAL, -1); // Need to have a different macro for this

	if (flags != AT_EMPTY_PATH)
	{
		VALIDATE_PATH(path, ENOENT, -1);
		return common_setxattr(fd, path, name, value, size, operation, flags);
	}
	else
	{
		handle_t type = get_fd_type(fd);
		if (type != FILE_HANDLE && type != DIRECTORY_HANDLE)
		{
			errno = EBADF;
			return -1;
		}

		HANDLE handle = get_fd_handle(fd);
		return do_setxattr(handle, name, value, size, operation);
	}
}
