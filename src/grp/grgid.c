/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/security.h>
#include <LM.h>
#include <grp.h>
#include <string.h>

int LOCALGROUP_INFO_0_to_group(PLOCALGROUP_INFO_0 group_info, struct group *grp_entry, void *buffer, size_t size);
int GROUP_INFO_2_to_group(PGROUP_INFO_2 group_info, struct group *grp_entry, void *buffer, size_t size);

struct group *common_getgrgid(gid_t gid, struct group *restrict grp_entry, char *restrict buffer, size_t size)
{
	BOOL status;
	DWORD lmstatus;
	BYTE *local_info_buffer = NULL;
	BYTE *global_info_buffer = NULL;
	struct group *result = grp_entry;
	wchar_t name[128], domain_name[128];
	DWORD name_size = 128, domain_size = 128;
	SID_NAME_USE use;
	SID_IDENTIFIER_AUTHORITY nt_authority = SECURITY_NT_AUTHORITY;
	char sid_computer_buffer[SECURITY_SID_SIZE(SID_MAX_SUB_AUTHORITIES)];
	char sid_builtin_buffer[SECURITY_SID_SIZE(2)];
	PISID sid_of_gid;

	// Set these field first.
	grp_entry->gr_passwd = NULL;
	grp_entry->gr_mem = &(grp_entry->gr_passwd);

	// First check whether they name are part of 'NT AUTHORITY'
	if (gid < 500)
	{
		switch (gid)
		{
		case 1:
			grp_entry->gr_name = "DIALUP";
			grp_entry->gr_gid = 1;
			return result;
		case 2:
			grp_entry->gr_name = "NETWORK";
			grp_entry->gr_gid = 2;
			return result;
		case 3:
			grp_entry->gr_name = "BATCH";
			grp_entry->gr_gid = 3;
			return result;
		case 4:
			grp_entry->gr_name = "INTERACTIVE";
			grp_entry->gr_gid = 4;
			return result;
		case 6:
			grp_entry->gr_name = "SERVICE";
			grp_entry->gr_gid = 6;
			return result;
		case 7:
			grp_entry->gr_name = "ANONYMOUS LOGON";
			grp_entry->gr_gid = 7;
			return result;
		case 8:
			grp_entry->gr_name = "PROXY";
			grp_entry->gr_gid = 8;
			return result;
		case 9:
			grp_entry->gr_name = "ENTERPRISE DOMAIN CONTROLLERS";
			grp_entry->gr_gid = 9;
			return result;
		case 10:
			grp_entry->gr_name = "SELF";
			grp_entry->gr_gid = 10;
			return result;
		case 11:
			grp_entry->gr_name = "Authenticated Users";
			grp_entry->gr_gid = 11;
			return result;
		case 12:
			grp_entry->gr_name = "RESTRICTED";
			grp_entry->gr_gid = 12;
			return result;
		case 13:
			grp_entry->gr_name = "TERMINAL SERVER USER";
			grp_entry->gr_gid = 13;
			return result;
		case 14:
			grp_entry->gr_name = "REMOTE INTERACTIVE LOGON";
			grp_entry->gr_gid = 14;
			return result;
		case 15:
			grp_entry->gr_name = "This Organization";
			grp_entry->gr_gid = 15;
			return result;
		case 17:
			grp_entry->gr_name = "IUSR";
			grp_entry->gr_gid = 17;
			return result;
		case 18:
			grp_entry->gr_name = "SYSTEM";
			grp_entry->gr_gid = 18;
			return result;
		case 19:
			grp_entry->gr_name = "LOCAL SERVICE";
			grp_entry->gr_gid = 19;
			return result;
		case 20:
			grp_entry->gr_name = "NETWORK SERVICE";
			grp_entry->gr_gid = 20;
			return result;
		}
	}

	// Check BUILTIN first
	RtlInitializeSidEx((PSID)sid_builtin_buffer, &nt_authority, 2, SECURITY_BUILTIN_DOMAIN_RID, gid);
	status = LookupAccountSidW(NULL, (PSID)sid_builtin_buffer, name, &name_size, domain_name, &domain_size, &use);
	if (status == 0)
	{
		// Then the computer
		RtlCopySid(SECURITY_SID_SIZE(SID_MAX_SUB_AUTHORITIES), (PSID)sid_computer_buffer, (PSID)current_computer_sid);
		sid_of_gid = (PISID)sid_computer_buffer;
		sid_of_gid->SubAuthority[sid_of_gid->SubAuthorityCount] = gid;
		sid_of_gid->SubAuthorityCount++;

		status = LookupAccountSidW(NULL, (PSID)sid_of_gid, name, &name_size, domain_name, &domain_size, &use);
		if (status == 0)
		{
			errno = ENOENT;
			return NULL;
		}
	}

	// Finally fetch the entry similar to getgrnam
	lmstatus = NetLocalGroupGetInfo(NULL, name, 0, &local_info_buffer);
	if (lmstatus != NERR_Success)
	{
		lmstatus = NetGroupGetInfo(NULL, name, 2, &global_info_buffer);
		if (lmstatus != NERR_Success)
		{
			if (lmstatus == NERR_GroupNotFound)
			{
				errno = ENOENT;
			}
			result = NULL;
			goto finish_global;
		}

		if (GROUP_INFO_2_to_group((PGROUP_INFO_2)global_info_buffer, grp_entry, buffer, size) == -1)
		{
			result = NULL;
			goto finish_global;
		}
	}
	else
	{
		if (LOCALGROUP_INFO_0_to_group((PLOCALGROUP_INFO_0)local_info_buffer, grp_entry, buffer, size) == -1)
		{
			result = NULL;
			goto finish;
		}
	}

finish:
	NetApiBufferFree(local_info_buffer);
	return result;

finish_global:
	NetApiBufferFree(global_info_buffer);
	goto finish;
}

struct group *wlibc_getgrgid(gid_t gid)
{
	if (gid < 0)
	{
		errno = EINVAL;
		return NULL;
	}

	static char grp_buffer[1024];
	static struct group entry;

	if (common_getgrgid(gid, &entry, grp_buffer, 1024) == NULL)
	{
		return NULL;
	}

	return &entry;
}

int wlibc_getgrgid_r(gid_t gid, struct group *restrict grp_entry, char *restrict buffer, size_t size, struct group **restrict result)
{
	if (gid < 0)
	{
		errno = EINVAL;
		return EINVAL;
	}

	if (grp_entry == NULL || buffer == NULL || result == NULL || size < 0)
	{
		errno = EINVAL;
		return EINVAL;
	}

	// Save errno and restore it if no error has been encountered.
	errno_t old_errno, new_errno;
	old_errno = errno;
	errno = 0;

	struct group *entry = common_getgrgid(gid, grp_entry, buffer, size);
	*result = entry;

	if (entry == NULL)
	{
		new_errno = errno;
		if (new_errno != 0)
		{
			return new_errno;
		}
	}

	errno = old_errno;
	return 0;
}
