/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/registry.h>
#include <internal/security.h>
#include <LM.h>
#include <sddl.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <errno.h>
#include <wchar.h>

int USER_INFO_3_to_passwd(PUSER_INFO_3 user_info, struct passwd *pw_entry, void *buffer, size_t size);

struct passwd *common_getpwuid(uid_t uid, struct passwd *restrict pwd_entry, char *restrict buffer, size_t size)
{
	BOOL status;
	DWORD lmstatus;
	BYTE *info_buffer = NULL;
	PISID sid_of_uid;
	struct passwd *result = pwd_entry;
	char sid_buffer[SECURITY_SID_SIZE(SID_MAX_SUB_AUTHORITIES)];
	wchar_t name[128], domain_name[128];
	DWORD name_size = 128, domain_size = 128;
	SID_NAME_USE use;

	/* 
	   If uid is 0 return an administrator profile similar to BUILTIN\Adminstrators.
	   NOTE: When a program is invoked by 'Run as Administrator' it is run as the current user with an elevated token.
	   Querying the old and disabled 'Administrator' account is not correct.
	*/
	if(uid == 0)
	{
		pwd_entry->pw_name = "Administrator";
		pwd_entry->pw_passwd = NULL;
		pwd_entry->pw_uid = 0;
		pwd_entry->pw_gid = 0;
		pwd_entry->pw_gecos = "Local Administrator";
		pwd_entry->pw_dir = "C:\\Windows\\System32";
		pwd_entry->pw_shell = "C:\\Windows\\System32\\cmd.exe";

		return result;
	}

	// All the user ids have the current computer sid as their common prefix.
	// First copy the computer sid to a buffer.
	RtlCopySid(SECURITY_SID_SIZE(SID_MAX_SUB_AUTHORITIES), (PSID)sid_buffer, current_computer_sid);
	sid_of_uid = (PISID)sid_buffer;
	// Then add uid to it;
	sid_of_uid->SubAuthority[sid_of_uid->SubAuthorityCount] = uid;
	sid_of_uid->SubAuthorityCount++;

	status = LookupAccountSidW(NULL, (PSID)sid_of_uid, name, &name_size, domain_name, &domain_size, &use);
	if (status == 0)
	{
		errno = ENOENT;
		return NULL;
	}

	lmstatus = NetUserGetInfo(NULL, name, 3, &info_buffer);
	if (lmstatus != NERR_Success)
	{
		if (lmstatus == NERR_UserNotFound)
		{
			errno = ENOENT;
		}
		result = NULL;
		goto finish;
	}

	if (USER_INFO_3_to_passwd((PUSER_INFO_3)info_buffer, pwd_entry, buffer, size) == -1)
	{
		result = NULL;
		goto finish;
	}

finish:
	NetApiBufferFree(info_buffer);
	return result;
}

struct passwd *wlibc_getpwuid(uid_t uid)
{
	if (uid < 0)
	{
		errno = EINVAL;
		return NULL;
	}

	static char pwd_buffer[512];
	static struct passwd entry;

	if (common_getpwuid(uid, &entry, pwd_buffer, 512) == NULL)
	{
		return NULL;
	}

	return &entry;
}

int wlibc_getpwuid_r(uid_t uid, struct passwd *restrict pwd_entry, char *restrict buffer, size_t size, struct passwd **restrict result)
{
	if (pwd_entry == NULL || buffer == NULL || result == NULL || uid < 0)
	{
		errno = EINVAL;
		return EINVAL;
	}

	// Save errno and restore it if no error has been encountered.
	errno_t old_errno, new_errno;
	old_errno = errno;
	errno = 0;

	struct passwd *entry = common_getpwuid(uid, pwd_entry, buffer, size);
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
