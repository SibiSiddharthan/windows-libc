/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <internal/security.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/acl.h>

static acl_t get_acl(HANDLE handle)
{
	NTSTATUS status;
	PVOID buffer = NULL;
	ULONG buffer_size = 512, length_required = 0;

	// Start with 512 bytes, grow as required.
	buffer = malloc(buffer_size);

	status = NtQuerySecurityObject(handle, DACL_SECURITY_INFORMATION, buffer, buffer_size, &length_required);

	if (status != STATUS_SUCCESS)
	{
		// Free the buffer first.
		free(buffer);
		if (status == STATUS_BUFFER_TOO_SMALL) // CHECK
		{
			buffer = malloc(length_required);

			status = NtQuerySecurityObject(handle, DACL_SECURITY_INFORMATION, buffer, buffer_size, &length_required);
			if (status != STATUS_SUCCESS)
			{
				free(buffer);
				map_ntstatus_to_errno(status);
				return NULL;
			}
		}
		else
		{
			map_ntstatus_to_errno(status);
			return NULL;
		}
	}

	return buffer;
}

static int set_acl(HANDLE handle, acl_t acl)
{
	NTSTATUS status;

	status = NtSetSecurityObject(handle, DACL_SECURITY_INFORMATION, (PSECURITY_DESCRIPTOR)acl);

	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}

acl_t wlibc_acl_get(int fd, const char *path, int flags)
{
	if (flags != 0 && flags != AT_EMPTY_PATH && flags != AT_SYMLINK_NOFOLLOW)
	{
		errno = EINVAL;
		return NULL;
	}

	if (flags == AT_EMPTY_PATH)
	{
		fdinfo info;
		get_fdinfo(fd, &info);

		if (info.type != FILE_HANDLE && info.type != DIRECTORY_HANDLE)
		{
			errno = EBADF;
			return NULL;
		}

		return get_acl(info.handle);
	}
	else
	{
		VALIDATE_PATH(path, ENOENT, NULL);

		HANDLE handle = just_open(AT_FDCWD, path, READ_CONTROL, (flags == AT_SYMLINK_NOFOLLOW ? FILE_OPEN_REPARSE_POINT : 0));
		if (handle == INVALID_HANDLE_VALUE)
		{
			// errno wil be set by just_open
			return NULL;
		}

		acl_t result;

		result = get_acl(handle);
		NtClose(handle);

		return result;
	}
}

int wlibc_acl_set(int fd, const char *path, acl_t acl, int flags)
{
	if (flags != 0 && flags != AT_EMPTY_PATH && flags != AT_SYMLINK_NOFOLLOW)
	{
		errno = EINVAL;
		return -1;
	}

	if (flags != 0 && flags != AT_EMPTY_PATH && flags != AT_SYMLINK_NOFOLLOW)
	{
		errno = EINVAL;
		return -1;
	}

	if (flags == AT_EMPTY_PATH)
	{
		fdinfo info;
		HANDLE handle;
		int result;

		get_fdinfo(fd, &info);

		if (info.type != FILE_HANDLE && info.type != DIRECTORY_HANDLE)
		{
			errno = EBADF;
			return -1;
		}

		handle = just_reopen(info.handle, WRITE_DAC, 0);
		if (handle == INVALID_HANDLE_VALUE)
		{
			// errno wil be set by `just_reopen`.
			return -1;
		}

		result = set_acl(handle, acl);
		NtClose(handle);

		return result;
	}
	else
	{
		VALIDATE_PATH(path, ENOENT, -1);

		HANDLE handle = just_open(AT_FDCWD, path, WRITE_DAC, (flags == AT_SYMLINK_NOFOLLOW ? FILE_OPEN_REPARSE_POINT : 0));
		if (handle == INVALID_HANDLE_VALUE)
		{
			// errno wil be set by `just_open`.
			return -1;
		}

		int result;

		result = set_acl(handle, acl);
		NtClose(handle);

		return result;
	}
}
