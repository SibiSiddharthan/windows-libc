/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <LM.h>
#include <grp.h>
#include <string.h>

int LOCALGROUP_INFO_0_to_group(PLOCALGROUP_INFO_0 group_info, struct group *grp_entry, void *buffer, size_t size);
int GROUP_INFO_2_to_group(PGROUP_INFO_2 group_info, struct group *grp_entry, void *buffer, size_t size);

struct group *common_getgrnam(const char *restrict name, struct group *restrict grp_entry, char *restrict buffer, size_t size)
{
	DWORD status;
	BYTE *local_info_buffer = NULL;
	BYTE *global_info_buffer = NULL;
	UTF8_STRING u8_name;
	UNICODE_STRING u16_name;
	struct group *result = grp_entry;

	// Set these field first.
	grp_entry->gr_passwd = NULL;
	grp_entry->gr_mem = &(grp_entry->gr_passwd);

	// First check whether they name are part of 'NT AUTHORITY'
	if (stricmp(name, "DIALUP") == 0)
	{
		grp_entry->gr_name = "DIALUP";
		grp_entry->gr_gid = 1;
		return result;
	}
	if (stricmp(name, "NETWORK") == 0)
	{
		grp_entry->gr_name = "NETWORK";
		grp_entry->gr_gid = 2;
		return result;
	}
	if (stricmp(name, "BATCH") == 0)
	{
		grp_entry->gr_name = "BATCH";
		grp_entry->gr_gid = 3;
		return result;
	}
	if (stricmp(name, "INTERACTIVE") == 0)
	{
		grp_entry->gr_name = "INTERACTIVE";
		grp_entry->gr_gid = 4;
		return result;
	}
	if (stricmp(name, "SERVICE") == 0)
	{
		grp_entry->gr_name = "SERVICE";
		grp_entry->gr_gid = 6;
		return result;
	}
	if (stricmp(name, "ANONYMOUS LOGON") == 0)
	{
		grp_entry->gr_name = "ANONYMOUS LOGON";
		grp_entry->gr_gid = 7;
		return result;
	}
	if (stricmp(name, "PROXY") == 0)
	{
		grp_entry->gr_name = "PROXY";
		grp_entry->gr_gid = 8;
		return result;
	}
	if (stricmp(name, "ENTERPRISE DOMAIN CONTROLLERS") == 0)
	{
		grp_entry->gr_name = "ENTERPRISE DOMAIN CONTROLLERS";
		grp_entry->gr_gid = 9;
		return result;
	}
	if (stricmp(name, "SELF") == 0)
	{
		grp_entry->gr_name = "SELF";
		grp_entry->gr_gid = 10;
		return result;
	}
	if (stricmp(name, "Authenticated Users") == 0)
	{
		grp_entry->gr_name = "Authenticated Users";
		grp_entry->gr_gid = 11;
		return result;
	}
	if (stricmp(name, "RESTRICTED") == 0)
	{
		grp_entry->gr_name = "RESTRICTED";
		grp_entry->gr_gid = 12;
		return result;
	}
	if (stricmp(name, "TERMINAL SERVER USER") == 0)
	{
		grp_entry->gr_name = "TERMINAL SERVER USER";
		grp_entry->gr_gid = 13;
		return result;
	}
	if (stricmp(name, "REMOTE INTERACTIVE LOGON") == 0)
	{
		grp_entry->gr_name = "REMOTE INTERACTIVE LOGON";
		grp_entry->gr_gid = 14;
		return result;
	}
	if (stricmp(name, "This Organization") == 0)
	{
		grp_entry->gr_name = "This Organization";
		grp_entry->gr_gid = 15;
		return result;
	}
	if (stricmp(name, "IUSR") == 0)
	{
		grp_entry->gr_name = "IUSR";
		grp_entry->gr_gid = 17;
		return result;
	}
	if (stricmp(name, "SYSTEM") == 0)
	{
		grp_entry->gr_name = "SYSTEM";
		grp_entry->gr_gid = 18;
		return result;
	}
	if (stricmp(name, "LOCAL SERVICE") == 0)
	{
		grp_entry->gr_name = "LOCAL SERVICE";
		grp_entry->gr_gid = 19;
		return result;
	}
	if (stricmp(name, "NETWORK SERVICE") == 0)
	{
		grp_entry->gr_name = "NETWORK SERVICE";
		grp_entry->gr_gid = 20;
		return result;
	}

	RtlInitUTF8String(&u8_name, name);
	RtlUTF8StringToUnicodeString(&u16_name, &u8_name, TRUE);

	status = NetLocalGroupGetInfo(NULL, u16_name.Buffer, 0, &local_info_buffer);
	if (status != NERR_Success)
	{
		status = NetGroupGetInfo(NULL, u16_name.Buffer, 2, &global_info_buffer);
		if (status != NERR_Success)
		{
			if (status == NERR_GroupNotFound)
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
	RtlFreeUnicodeString(&u16_name);
	return result;

finish_global:
	NetApiBufferFree(global_info_buffer);
	goto finish;
}

struct group *wlibc_getgrnam(const char *name)
{
	static char grp_buffer[1024];
	static struct group entry;

	if (common_getgrnam(name, &entry, grp_buffer, 1024) == NULL)
	{
		return NULL;
	}

	return &entry;
}

int wlibc_getgrnam_r(const char *restrict name, struct group *restrict grp_entry, char *restrict buffer, size_t size,
					 struct group **restrict result)
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

	struct group *entry = common_getgrnam(name, grp_entry, buffer, size);
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
