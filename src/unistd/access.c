/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <internal/security.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

// From stat.c
mode_t get_permissions(ACCESS_MASK access);

int have_required_access(mode_t given_access, mode_t mode)
{
	mode_t required_access = 0;
	if (mode & X_OK)
	{
		required_access |= S_IEXEC;
	}
	if (mode & W_OK)
	{
		required_access |= S_IWRITE;
	}
	if (mode & R_OK)
	{
		required_access |= S_IREAD;
	}

	// given_access should be a superset of required_access
	if ((required_access & ~given_access) != 0)
	{
		return -1;
	}
	return 0;
}

// TODO AT_EACCESS
int do_access(HANDLE handle, int mode, int flags)
{
	NTSTATUS status;
	IO_STATUS_BLOCK I;
	FILE_STAT_INFORMATION stat_info;
	ULONG length;
	char security_buffer[512];

	status = NtQuerySecurityObject(handle, DACL_SECURITY_INFORMATION, security_buffer, 512, &length);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	mode_t final_access = 0, allowed_access = 0, denied_access = 0;

	PISECURITY_DESCRIPTOR_RELATIVE security_descriptor = (PISECURITY_DESCRIPTOR_RELATIVE)security_buffer;
	PACL acl = (PACL)(security_buffer + security_descriptor->Dacl);
	size_t acl_read = 0;
	bool user_ace_present = false;
	// Iterate through the ACLs
	// Order should be (NT AUTHORITY\SYSTEM), (BUILTIN\Administrators), Current User ,(BUILTIN\Users), Everyone
	for (int i = 0; i < acl->AceCount; ++i)
	{
		PISID sid = NULL;
		PACE_HEADER ace_header = (PACE_HEADER)((char *)acl + sizeof(ACL) + acl_read);

		// Only support allowed and denied ACEs
		// Both ACCESS_ALLOWED_ACE and PACCESS_DENIED_ACE have ACE_HEADER at the start.
		// Type casting of pointers here will work.
		if (ace_header->AceType == ACCESS_ALLOWED_ACE_TYPE)
		{
			PACCESS_ALLOWED_ACE allowed_ace = (PACCESS_ALLOWED_ACE)ace_header;
			sid = (PISID) & (allowed_ace->SidStart);
			if (RtlEqualSid(sid, current_user_sid))
			{
				user_ace_present = true;
				allowed_access |= get_permissions(allowed_ace->Mask);
			}
			else
			{

				// Not user SID, ignore
			}
		}
		else if (ace_header->AceType == ACCESS_DENIED_ACE_TYPE)
		{
			PACCESS_DENIED_ACE denied_ace = (PACCESS_DENIED_ACE)ace_header;
			sid = (PISID) & (denied_ace->SidStart);
			if (RtlEqualSid(sid, current_user_sid))
			{
				user_ace_present = true;
				denied_access |= get_permissions(denied_ace->Mask);
			}
			else
			{
				// Not user SID, ignore
			}
		}
		else
		{
			// Unsupported ACE type
		}
		acl_read += ace_header->AceSize;
	}

	final_access = allowed_access & ~denied_access;

	if (!user_ace_present)
	{
		// Fallback to 'EffectiveAccess' field of FILE_STAT_INFORMATION
		status = NtQueryInformationFile(handle, &I, &stat_info, sizeof(FILE_STAT_INFORMATION), FileStatInformation);
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}

		final_access = stat_info.EffectiveAccess;
	}

	return have_required_access(final_access, mode);
}

int common_access(int dirfd, const char *path, int mode, int flags)
{
	int result = -1;
	wchar_t *u16_ntpath = get_absolute_ntpath(dirfd, path);
	if (u16_ntpath == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	HANDLE handle = just_open(u16_ntpath, FILE_READ_ATTRIBUTES | READ_CONTROL, 0, FILE_OPEN,
							  (flags & AT_SYMLINK_NOFOLLOW) == AT_SYMLINK_NOFOLLOW ? FILE_OPEN_REPARSE_POINT : 0);
	free(u16_ntpath);
	if (handle == INVALID_HANDLE_VALUE)
	{
		// errno wil be set by just_open
		return -1;
	}

	if (mode == F_OK)
	{
		// Avoid doing any expensive checks if we are checking whether the file exists or not only.
		result = 0;
	}
	else
	{
		result = do_access(handle, mode, flags & AT_EACCESS);
	}

	NtClose(handle);
	return result;
}

int wlibc_common_access(int dirfd, const char *path, int mode, int flags)
{
	if ((flags & ~(AT_EACCESS | AT_SYMLINK_NOFOLLOW)) != 0)
	{
		errno = EINVAL;
		return -1;
	}

	if (mode < F_OK || mode > (F_OK | R_OK | W_OK | X_OK))
	{
		errno = EINVAL;
		return -1;
	}

	VALIDATE_PATH_AND_DIRFD(path, dirfd);
	return common_access(dirfd, path, mode, flags);
}
