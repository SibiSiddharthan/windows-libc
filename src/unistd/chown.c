/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <internal/security.h>
#include <errno.h>
#include <unistd.h>

PISID lookup_id(int id, void *buffer)
{
	PISID sid_of_id;
	WCHAR name[128], domain_name[128];
	DWORD name_size = 128, domain_size = 128;
	SID_NAME_USE use;
	SID_IDENTIFIER_AUTHORITY nt_authority = SECURITY_NT_AUTHORITY;

	// ROOT hook.
	if (id == 0)
	{
		return adminstrators_sid;
	}

	// Search in the current computer first.
	RtlCopySid(SECURITY_SID_SIZE(SID_MAX_SUB_AUTHORITIES), (PSID)buffer, (PSID)current_computer_sid);
	sid_of_id = (PISID)buffer;
	sid_of_id->SubAuthority[sid_of_id->SubAuthorityCount] = id;
	sid_of_id->SubAuthorityCount++;

	if (LookupAccountSidW(NULL, (PSID)sid_of_id, name, &name_size, domain_name, &domain_size, &use))
	{
		return (PISID)buffer;
	}

	// Search in the 'BUILTIN' next.
	RtlInitializeSidEx((PSID)buffer, &nt_authority, 2, SECURITY_BUILTIN_DOMAIN_RID, id);
	if (LookupAccountSidW(NULL, (PSID)sid_of_id, name, &name_size, domain_name, &domain_size, &use))
	{
		return (PISID)buffer;
	}

	// Search in 'NT AUTHORITY' finally.
	RtlInitializeSidEx((PSID)buffer, &nt_authority, 1, id);
	if (LookupAccountSidW(NULL, (PSID)sid_of_id, name, &name_size, domain_name, &domain_size, &use))
	{
		return (PISID)buffer;
	}

	// Id not found.
	return NULL;
}

int do_chown(HANDLE handle, uid_t owner, gid_t group)
{
	NTSTATUS status;
	SECURITY_INFORMATION security_information = 0;
	SECURITY_DESCRIPTOR security_descriptor = {SECURITY_DESCRIPTOR_REVISION, 0, 0, NULL, NULL, NULL, NULL};
	PISID owner_sid = NULL, group_sid = NULL;
	char owner_sid_buffer[SECURITY_SID_SIZE(SID_MAX_SUB_AUTHORITIES)];
	char group_sid_buffer[SECURITY_SID_SIZE(SID_MAX_SUB_AUTHORITIES)];

	if (owner != -1)
	{
		security_information |= OWNER_SECURITY_INFORMATION;
		owner_sid = lookup_id(owner, owner_sid_buffer);
		if (owner_sid == NULL)
		{
			// Id not found.
			errno = ESRCH;
			return -1;
		}

		security_descriptor.Owner = (PSID)owner_sid;
	}
	else
	{
		security_descriptor.Control |= SE_OWNER_DEFAULTED;
	}

	if (group != -1)
	{
		security_information |= GROUP_SECURITY_INFORMATION;
		group_sid = lookup_id(group, group_sid_buffer);
		if (group_sid == NULL)
		{
			// Id not found.
			errno = ESRCH;
			return -1;
		}

		security_descriptor.Group = (PSID)group_sid;
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
	PVOID state = NULL;
	ULONG privilege = SE_TAKE_OWNERSHIP_PRIVILEGE;
	BOOLEAN acquired_privilege = FALSE;
	HANDLE handle;

	handle = just_open(dirfd, path, WRITE_OWNER, flags == AT_SYMLINK_NOFOLLOW ? FILE_OPEN_REPARSE_POINT : 0);
	if (handle == NULL)
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
			if (handle == NULL)
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

	if (owner < -1 || group < -1)
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
		PVOID state = NULL;
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
		if (handle == NULL)
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
				if (handle == NULL)
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
