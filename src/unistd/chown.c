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
#include <stdbool.h>
#include <unistd.h>

PISID lookup_id(int id)
{
	// TODO
	return NULL;
}

int do_chown(HANDLE handle, uid_t owner, gid_t group)
{
	NTSTATUS status;
	SECURITY_INFORMATION security_information = 0;
	SECURITY_DESCRIPTOR security_descriptor = {SECURITY_DESCRIPTOR_REVISION, 0, 0, NULL, NULL, NULL, NULL};
	//PISID owner_sid = NULL, group_sid = NULL;
	//char owner_sid_buffer[SECURITY_SID_SIZE(SID_MAX_SUB_AUTHORITIES)];
	//char group_sid_buffer[SECURITY_SID_SIZE(SID_MAX_SUB_AUTHORITIES)];

	//owner_sid = (PISID)owner_sid_buffer;
	//group_sid = (PISID)group_sid_buffer;

	if (owner != -1)
	{
		security_information |= OWNER_SECURITY_INFORMATION;
		security_descriptor.Owner = (PSID)(owner == 0 ? adminstrators_sid : lookup_id(owner));
	}
	else
	{
		security_descriptor.Control |= SE_OWNER_DEFAULTED;
	}

	if (group != -1)
	{
		security_information |= GROUP_SECURITY_INFORMATION;
		security_descriptor.Group = (PSID)(group == 0 ? adminstrators_sid : lookup_id(group));
	}
	else
	{
		security_descriptor.Control |= SE_GROUP_DEFAULTED;
	}

	status = NtSetSecurityObject(handle, security_information, &security_descriptor);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}

int common_chown(int dirfd, const char *path, uid_t owner, gid_t group, int flags)
{
	NTSTATUS status;
	PVOID state;
	ULONG privilege = SE_TAKE_OWNERSHIP_PRIVILEGE;
	BOOLEAN acquired_privilege = FALSE;
	HANDLE handle;

	handle = just_open(dirfd, path, WRITE_OWNER, flags == AT_SYMLINK_NOFOLLOW ? FILE_OPEN_REPARSE_POINT : 0);
	if (handle == INVALID_HANDLE_VALUE)
	{
		if (errno == EACCES)
		{
			// Try again with 'SeTakeOwnershipPrivilege'.
			status = RtlAcquirePrivilege(&privilege, 1, 0, &state);
			if (status != STATUS_SUCCESS)
			{
				map_ntstatus_to_errno(status);
				return -1;
			}

			acquired_privilege = TRUE;
			errno = 0;

			handle = just_open(dirfd, path, WRITE_OWNER, flags == AT_SYMLINK_NOFOLLOW ? FILE_OPEN_REPARSE_POINT : 0);
			if (handle == INVALID_HANDLE_VALUE)
			{
				// Failed again release privilege and return.
				// errno will be set by `just_open`.
				RtlReleasePrivilege(state);
				return -1;
			}
		}
		else
		{
			return -1;
		}
	}

	int result = do_chown(handle, owner, group);

	NtClose(handle);
	if (acquired_privilege)
	{
		RtlReleasePrivilege(state);
	}

	return result;
}

int wlibc_common_chown(int dirfd, const char *path, uid_t owner, gid_t group, int flags)
{

	if (owner < 0 || group < 0)
	{
		errno = EINVAL;
		return -1;
	}

	if (flags != 0 && flags != AT_SYMLINK_NOFOLLOW && flags != AT_EMPTY_PATH)
	{
		errno = EINVAL;
		return -1;
	}

	if (flags != AT_EMPTY_PATH)
	{
		VALIDATE_PATH_AND_DIRFD(path, dirfd);
		return common_chown(dirfd, path, owner, group, flags);
	}
	else
	{
		NTSTATUS status;
		PVOID state;
		ULONG privilege = SE_TAKE_OWNERSHIP_PRIVILEGE;
		BOOLEAN acquired_privilege = FALSE;
		HANDLE handle;
		fdinfo info;

		get_fdinfo(dirfd, &info);
		if (info.type != FILE_HANDLE && info.type != DIRECTORY_HANDLE)
		{
			errno = EBADF;
			return -1;
		}

		// 'open' does not give WRITE_OWNER permission, reopen the file with 'WRITE_OWNER'.
		handle = just_reopen(info.handle, WRITE_OWNER, 0);
		if (handle == INVALID_HANDLE_VALUE)
		{
			if (errno == EACCES)
			{
				// Try again with 'SeTakeOwnershipPrivilege'.
				status = RtlAcquirePrivilege(&privilege, 1, 0, &state);
				if (status != STATUS_SUCCESS)
				{
					map_ntstatus_to_errno(status);
					return -1;
				}

				acquired_privilege = TRUE;
				errno = 0;

				handle = just_reopen(info.handle, WRITE_OWNER, 0);
				if (handle == INVALID_HANDLE_VALUE)
				{
					// Failed again release privilege and return.
					// errno will be set by `just_reopen`.
					RtlReleasePrivilege(state);
					return -1;
				}
			}
			else
			{
				return -1;
			}
		}

		int result = do_chown(handle, owner, group);
		NtClose(handle);
		if (acquired_privilege)
		{
			RtlReleasePrivilege(state);
		}

		return result;
	}
}
