/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/acl.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <internal/security.h>
#include <errno.h>
#include <sys/acl.h>

static acl_t get_acl(HANDLE handle)
{
	NTSTATUS status;
	PVOID buffer = NULL;
	ULONG buffer_size = 512, length_required = 0;
	WORD acl_read = 0;
	PACL acl = NULL;
	acl_t result = NULL;

	// Start with 512 bytes, grow as required.
	buffer = RtlAllocateHeap(NtCurrentProcessHeap(), 0, buffer_size);
	if (buffer == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}

	status = NtQuerySecurityObject(handle, DACL_SECURITY_INFORMATION, buffer, buffer_size, &length_required);

	if (status != STATUS_SUCCESS)
	{
		if (status == STATUS_BUFFER_TOO_SMALL) // CHECK
		{
			buffer = RtlReAllocateHeap(NtCurrentProcessHeap(), 0, buffer, length_required);
			if (buffer == NULL)
			{
				errno = ENOMEM;
				return NULL;
			}

			status = NtQuerySecurityObject(handle, DACL_SECURITY_INFORMATION, buffer, buffer_size, &length_required);
			if (status != STATUS_SUCCESS)
			{
				RtlFreeHeap(NtCurrentProcessHeap(), 0, buffer);
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

	// Get the DACL information.
	acl = (PACL)((CHAR *)buffer + ((PISECURITY_DESCRIPTOR_RELATIVE)buffer)->Dacl);
	result = wlibc_acl_init(acl->AceCount);

	// For each of the supported ACE types copy the contents.
	// We make the SIDs here of maximum length. This makes memory management
	// a lot easier for the entire ACL API.
	for (WORD i = 0; i < acl->AceCount; ++i)
	{
		PACE_HEADER ace_header = (PACE_HEADER)((char *)acl + sizeof(ACL) + acl_read);
		if (ace_header->AceType == ACCESS_ALLOWED_ACE_TYPE || ace_header->AceType == ACCESS_DENIED_ACE_TYPE)
		{
			// Copy ACE_HEADER (4 bytes) and ACCESS_MASK (4 bytes).
			*(LONGLONG *)&(result->entries[result->count]) = *(LONGLONG *)ace_header;

			// Set the size of the ace to the maximum allowed one.
			result->entries[result->count].size = sizeof(struct _wlibc_acl_entry_t);

			// Copy SID.
			RtlCopySid(SECURITY_SID_SIZE(SID_MAX_SUB_AUTHORITIES), &(result->entries[result->count].sid),
					   (CHAR *)ace_header + sizeof(ACE_HEADER) + sizeof(ACCESS_MASK));

			// Increment count.
			++result->count;
		}
		acl_read += ace_header->AceSize;
	}

	RtlFreeHeap(NtCurrentProcessHeap(), 0, buffer);

	return result;
}

static int set_acl(HANDLE handle, acl_t acl)
{
	NTSTATUS status;
	SECURITY_DESCRIPTOR security_descriptor = {
		SECURITY_DESCRIPTOR_REVISION, 0, SE_OWNER_DEFAULTED | SE_GROUP_DEFAULTED | SE_DACL_PRESENT, NULL, NULL, NULL, (PACL)acl};

	status = NtSetSecurityObject(handle, DACL_SECURITY_INFORMATION, &security_descriptor);

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
		if (handle == NULL)
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
		if (handle == NULL)
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
		if (handle == NULL)
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
