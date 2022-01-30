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

ssize_t do_listxattr(HANDLE handle, char *restrict list, size_t size)
{
	NTSTATUS status = -1;
	IO_STATUS_BLOCK io;

	char *query_buffer = NULL;
	size_t query_buffer_size = 1024;
	ssize_t result = -1;
	size_t data_length = 0;

	query_buffer = (char *)malloc(query_buffer_size);

	while (status != STATUS_NO_MORE_EAS)
	{
		status = NtQueryEaFile(handle, &io, query_buffer, query_buffer_size, FALSE, NULL, 0, NULL, FALSE);
		if (status == STATUS_NO_EAS_ON_FILE)
		{
			// This is not an error.
			result = 0;
			goto finish;
		}

		if (status == STATUS_BUFFER_TOO_SMALL || status == STATUS_BUFFER_OVERFLOW)
		{
			query_buffer_size = 65536; // Use this big of a buffer
			query_buffer = (char *)realloc(query_buffer, query_buffer_size);
			status = NtQueryEaFile(handle, &io, query_buffer, query_buffer_size, FALSE, NULL, 0, NULL, FALSE);
			if (status != STATUS_SUCCESS)
			{
				map_ntstatus_to_errno(status);
				goto finish;
			}
		}

		size_t read_data = 0;
		PFILE_FULL_EA_INFORMATION ea_info = (PFILE_FULL_EA_INFORMATION)query_buffer;

		while (read_data != io.Information)
		{
			if (size != 0)
			{
				if (data_length + ea_info->EaNameLength + 1 > size)
				{
					errno = ERANGE;
					goto finish;
				}
				memcpy(list + data_length, ea_info->EaName, ea_info->EaNameLength + 1);
			}
			data_length += ea_info->EaNameLength + 1; // + NULL
			read_data += offsetof(FILE_FULL_EA_INFORMATION, EaName) + ea_info->EaNameLength + ea_info->EaValueLength + 1;
			// each entry is aligned to a 4 byte boundary, except the last one
			if (ea_info->NextEntryOffset != 0)
			{
				read_data += (read_data % sizeof(LONG) == 0 ? 0 : (sizeof(LONG) - read_data % sizeof(LONG)));
			}

			ea_info = (PFILE_FULL_EA_INFORMATION)((char *)ea_info + ea_info->NextEntryOffset);
		}
	}

	result = data_length;

finish:
	free(query_buffer);
	return result;
}

ssize_t common_listxattr(int fd, const char *restrict path, char *restrict list, size_t size, int flags)
{
	HANDLE handle = just_open(fd, path, FILE_READ_EA | SYNCHRONIZE,
							  FILE_SYNCHRONOUS_IO_ALERT | (flags == AT_SYMLINK_NOFOLLOW ? FILE_OPEN_REPARSE_POINT : 0));
	if (handle == INVALID_HANDLE_VALUE)
	{
		// errno wil be set by just_open
		return -1;
	}

	ssize_t result = do_listxattr(handle, list, size);
	NtClose(handle);
	return result;
}

ssize_t wlibc_common_listxattr(int fd, const char *restrict path, char *restrict list, size_t size, int flags)
{
	if (flags != 0 && flags != AT_EMPTY_PATH && flags != AT_SYMLINK_NOFOLLOW)
	{
		errno = EINVAL;
		return -1;
	}

	if (size != 0)
	{
		if (list == NULL)
		{
			errno = EINVAL;
			return -1;
		}
	}

	if (flags != AT_EMPTY_PATH)
	{
		VALIDATE_PATH(path, ENOENT, -1);
		return common_listxattr(fd, path, list, size, flags);
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
		return do_listxattr(handle, list, size);
	}
}
