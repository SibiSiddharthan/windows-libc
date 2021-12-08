/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <LM.h>
#include <grp.h>

static int gr_index = 0;
static int gr_started = 0;
static BYTE *local_enum_buffer = NULL;
static BYTE *global_enum_buffer = NULL;

int LOCALGROUP_INFO_0_to_group(PLOCALGROUP_INFO_0 group_info, struct group *grp_entry, void *buffer, size_t size)
{
	NTSTATUS ntstatus;
	BOOL status;
	DWORD lmstatus;
	UNICODE_STRING u16_name, u16_member;
	UTF8_STRING u8_name, u8_member;
	DWORD domain_size = 128, sid_size = SECURITY_SID_SIZE(SID_MAX_SUB_AUTHORITIES);
	DWORD member_entries_read, member_entries_total;
	BYTE *local_member_buffer;
	PLOCALGROUP_MEMBERS_INFO_1 member_info;
	SID_NAME_USE use;
	size_t buffer_used = 0;
	char sid_buffer[SECURITY_SID_SIZE(SID_MAX_SUB_AUTHORITIES)];
	wchar_t domain_name[128]; // Unused

	// Default values
	grp_entry->gr_gid = -1;
	grp_entry->gr_mem = &grp_entry->gr_passwd;

	// name
	RtlInitUnicodeString(&u16_name, group_info->lgrpi0_name);
	u8_name.MaximumLength = size;
	u8_name.Buffer = (char *)buffer;

	ntstatus = RtlUnicodeStringToUTF8String(&u8_name, &u16_name, FALSE);
	if (ntstatus != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(ntstatus);
		return -1;
	}

	u8_name.MaximumLength = u8_name.Length + 1;
	buffer_used += u8_name.MaximumLength;
	grp_entry->gr_name = u8_name.Buffer;

	// gid
	status = LookupAccountNameW(NULL, u16_name.Buffer, sid_buffer, &sid_size, domain_name, &domain_size, &use);
	if (status != 0)
	{
		// Group id is last SubAuthority
		grp_entry->gr_gid = ((PISID)sid_buffer)->SubAuthority[((PISID)sid_buffer)->SubAuthorityCount - 1];
	}

	// members
	lmstatus = NetLocalGroupGetMembers(NULL, u16_name.Buffer, 1, &local_member_buffer, MAX_PREFERRED_LENGTH, &member_entries_read,
									   &member_entries_total, NULL);
	if (lmstatus == NERR_Success)
	{
		if (member_entries_read > 0)
		{
			// Reserve space in the buffer for the pointers we are about to store
			grp_entry->gr_mem = (char **)((char *)buffer + buffer_used);
			buffer_used += (member_entries_read + 1) * sizeof(void *); // NULL sentinel
			if (size <= buffer_used)
			{
				errno = ERANGE;
				goto fail;
			}

			grp_entry->gr_mem[member_entries_read] = NULL;

			member_info = (PLOCALGROUP_MEMBERS_INFO_1)local_member_buffer;
			for (int i = 0; i < member_entries_read; ++i)
			{
				RtlInitUnicodeString(&u16_member, member_info[i].lgrmi1_name);
				u8_member.MaximumLength = size - buffer_used;
				u8_member.Buffer = (char *)buffer + buffer_used;

				if (u8_member.MaximumLength == 0)
				{
					errno = ERANGE;
					goto fail;
				}

				ntstatus = RtlUnicodeStringToUTF8String(&u8_member, &u16_member, FALSE);
				if (ntstatus != STATUS_SUCCESS)
				{
					map_ntstatus_to_errno(ntstatus);
					goto fail;
				}

				grp_entry->gr_mem[i] = u8_member.Buffer;
				buffer_used += u8_member.Length + 1;
			}
		}
	}

	NetApiBufferFree(local_member_buffer);
	return 0;
fail:
	NetApiBufferFree(local_member_buffer);
	return -1;
}

int GROUP_INFO_2_to_group(PGROUP_INFO_2 group_info, struct group *grp_entry, void *buffer, size_t size)
{
	NTSTATUS ntstatus;
	DWORD lmstatus;
	UNICODE_STRING u16_name, u16_member;
	UTF8_STRING u8_name, u8_member;
	DWORD member_entries_read, member_entries_total;
	BYTE *global_member_buffer;
	PGROUP_USERS_INFO_0 member_info;
	size_t buffer_used = 0;

	// Default values
	grp_entry->gr_mem = &grp_entry->gr_passwd;

	// name
	RtlInitUnicodeString(&u16_name, group_info->grpi2_name);
	u8_name.MaximumLength = size;
	u8_name.Buffer = (char *)buffer;

	ntstatus = RtlUnicodeStringToUTF8String(&u8_name, &u16_name, FALSE);
	if (ntstatus != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(ntstatus);
		return -1;
	}

	u8_name.MaximumLength = u8_name.Length + 1;
	buffer_used += u8_name.MaximumLength;
	grp_entry->gr_name = u8_name.Buffer;

	// gid
	grp_entry->gr_gid = group_info->grpi2_group_id;

	// members
	lmstatus = NetGroupGetUsers(NULL, u16_name.Buffer, 0, &global_member_buffer, MAX_PREFERRED_LENGTH, &member_entries_read,
								&member_entries_total, NULL);
	if (lmstatus == NERR_Success)
	{
		if (member_entries_read > 0)
		{
			// Reserve space in the buffer for the pointers we are about to store
			grp_entry->gr_mem = (char **)((char *)buffer + buffer_used);
			buffer_used += (member_entries_read + 1) * sizeof(void *); // NULL sentinel
			if (size <= buffer_used)
			{
				errno = ERANGE;
				goto fail;
			}

			grp_entry->gr_mem[member_entries_read] = NULL;

			member_info = (PGROUP_USERS_INFO_0)global_member_buffer;
			for (int i = 0; i < member_entries_read; ++i)
			{
				RtlInitUnicodeString(&u16_member, member_info[i].grui0_name);
				u8_member.MaximumLength = size - buffer_used;
				u8_member.Buffer = (char *)buffer + buffer_used;

				if (u8_member.MaximumLength == 0)
				{
					errno = ERANGE;
					goto fail;
				}

				ntstatus = RtlUnicodeStringToUTF8String(&u8_member, &u16_member, FALSE);
				if (ntstatus != STATUS_SUCCESS)
				{
					map_ntstatus_to_errno(ntstatus);
					goto fail;
				}

				grp_entry->gr_mem[i] = u8_member.Buffer;
				buffer_used += u8_member.Length + 1;
			}
		}
	}

	NetApiBufferFree(global_member_buffer);
	return 0;
fail:
	NetApiBufferFree(global_member_buffer);
	return -1;
}

struct group *common_getgrent(struct group *restrict grp_entry, char *restrict buffer, size_t size)
{
	static DWORD nt_authority_entries = 18;
	static DWORD local_entries_read = 0, local_total_entries = 0;
	static DWORD global_entries_read = 0, global_total_entries = 0;
	DWORD total_entries = 0;
	DWORD status;
	PLOCALGROUP_INFO_0 local_group_info;
	PGROUP_INFO_2 global_group_info;

	total_entries += nt_authority_entries + local_total_entries + global_total_entries;

	if (gr_started == 0)
	{
		// First enumerate local groups, then global
		status = NetLocalGroupEnum(NULL, 0, &local_enum_buffer, MAX_PREFERRED_LENGTH, &local_entries_read, &local_total_entries, NULL);
		if (status == NERR_Success)
		{
			total_entries += local_total_entries;
			gr_started = 1;
			gr_index = 0;
		}

		status = NetGroupEnum(NULL, 2, &global_enum_buffer, MAX_PREFERRED_LENGTH, &global_entries_read, &global_total_entries, NULL);
		if (status == NERR_Success)
		{
			total_entries += global_total_entries;
		}
	}

	if (gr_index >= total_entries)
	{
		return NULL;
	}

	// Set this field first.
	grp_entry->gr_passwd = NULL;
	// HACK: point the members array to password. This way we don't have to allocate storage for
	// groups with no members like the 'NT AUTHORITY' ones.
	grp_entry->gr_mem = &(grp_entry->gr_passwd);

	// Return the NT AUTHORITY groups first
	switch (gr_index)
	{
	case 0:
		grp_entry->gr_name = "DAILUP";
		grp_entry->gr_gid = 1;
		break;
	case 1:
		grp_entry->gr_name = "NETWORK";
		grp_entry->gr_gid = 2;
		break;
	case 2:
		grp_entry->gr_name = "BATCH";
		grp_entry->gr_gid = 3;
		break;
	case 3:
		grp_entry->gr_name = "INTERACTIVE";
		grp_entry->gr_gid = 4;
		break;
	case 4:
		grp_entry->gr_name = "SERVICE";
		grp_entry->gr_gid = 6;
		break;
	case 5:
		grp_entry->gr_name = "ANONYMOUS LOGON";
		grp_entry->gr_gid = 7;
		break;
	case 6:
		grp_entry->gr_name = "PROXY";
		grp_entry->gr_gid = 8;
		break;
	case 7:
		grp_entry->gr_name = "ENTERPRISE DOMAIN CONTROLLERS";
		grp_entry->gr_gid = 9;
		break;
	case 8:
		grp_entry->gr_name = "SELF";
		grp_entry->gr_gid = 10;
		break;
	case 9:
		grp_entry->gr_name = "Authenticated Users";
		grp_entry->gr_gid = 11;
		break;
	case 10:
		grp_entry->gr_name = "RESTRICTED";
		grp_entry->gr_gid = 12;
		break;
	case 11:
		grp_entry->gr_name = "TERMINAL SERVER USER";
		grp_entry->gr_gid = 13;
		break;
	case 12:
		grp_entry->gr_name = "REMOTE INTERACTIVE LOGON";
		grp_entry->gr_gid = 14;
		break;
	case 13:
		grp_entry->gr_name = "This Organization";
		grp_entry->gr_gid = 15;
		break;
	case 14:
		grp_entry->gr_name = "IUSR";
		grp_entry->gr_gid = 17;
		break;
	case 15:
		grp_entry->gr_name = "SYSTEM";
		grp_entry->gr_gid = 18;
		break;
	case 16:
		grp_entry->gr_name = "LOCAL SERVICE";
		grp_entry->gr_gid = 19;
		break;
	case 17:
		grp_entry->gr_name = "NETWORK SERVICE";
		grp_entry->gr_gid = 20;
		break;
	}

	local_group_info = (PLOCALGROUP_INFO_0)local_enum_buffer;
	global_group_info = (PGROUP_INFO_2)global_enum_buffer;

	// Next local entries
	if (gr_index >= nt_authority_entries && gr_index < nt_authority_entries + local_total_entries)
	{
		if (LOCALGROUP_INFO_0_to_group(&local_group_info[gr_index - nt_authority_entries], grp_entry, buffer, size) == -1)
		{
			// errno will be set by 'LOCALGROUP_INFO_0_to_group'
			return NULL;
		}
	}
	// After than global entries
	else if (gr_index >= nt_authority_entries + local_total_entries)
	{
		if (GROUP_INFO_2_to_group(&global_group_info[gr_index - (nt_authority_entries + local_total_entries)], grp_entry, buffer, size) ==
			-1)
		{
			// errno will be set by 'GROUP_INFO_2_to_group'
			return NULL;
		}
	}

	++gr_index;
	return grp_entry;
}

struct group *wlibc_getgrent()
{
	static char grp_buffer[1024];
	static struct group entry;

	if (common_getgrent(&entry, grp_buffer, 1024) == NULL)
	{
		return NULL;
	}

	return &entry;
}

void wlibc_endgrent()
{
	// Free the static buffers and reset the state.
	if (gr_started == 1)
	{
		NetApiBufferFree(local_enum_buffer);
		NetApiBufferFree(global_enum_buffer);
	}
	gr_started = 0;
	gr_index = 0;
}

void wlibc_setgrent()
{
	gr_index = 0;
}

int wlibc_getgrent_r(struct group *restrict grp_entry, char *restrict buffer, size_t size, struct group **restrict result)
{
	if (grp_entry == NULL || buffer == NULL || result == NULL || size < 0)
	{
		errno = EINVAL;
		return -1;
	}

	// Save errno and restore it if no error has been encountered.
	errno_t old_errno, new_errno;
	old_errno = errno;
	errno = 0;

	struct group *entry = common_getgrent(grp_entry, buffer, size);
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
