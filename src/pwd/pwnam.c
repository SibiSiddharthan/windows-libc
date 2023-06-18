/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/validate.h>
#include <LM.h>
#include <errno.h>
#include <pwd.h>

int USER_INFO_3_to_passwd(PUSER_INFO_3 user_info, struct passwd *pw_entry, void *buffer, size_t size);

struct passwd *common_getpwnam(const char *restrict name, struct passwd *restrict pwd_entry, char *restrict buffer, size_t size)
{
	NTSTATUS ntstatus;
	DWORD status;
	BYTE *info_buffer = NULL;
	UTF8_STRING u8_name;
	UNICODE_STRING u16_name;
	struct passwd *result = pwd_entry;

	// ROOT hook
	if (stricmp(name, "root") == 0)
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

	RtlInitUTF8String(&u8_name, name);
	ntstatus = RtlUTF8StringToUnicodeString(&u16_name, &u8_name, TRUE);

	if (ntstatus != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(ntstatus);
		return NULL;
	}

	status = NetUserGetInfo(NULL, u16_name.Buffer, 3, &info_buffer);
	if (status != NERR_Success)
	{
		if (status == NERR_UserNotFound)
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
	RtlFreeUnicodeString(&u16_name);
	return result;
}

struct passwd *wlibc_getpwnam(const char *name)
{
	if (name == NULL || name[0] == '\0')
	{
		errno = EINVAL;
		return NULL;
	}

	static char pwd_buffer[512];
	static struct passwd entry;

	if (common_getpwnam(name, &entry, pwd_buffer, 512) == NULL)
	{
		return NULL;
	}

	return &entry;
}

int wlibc_getpwnam_r(const char *restrict name, struct passwd *restrict pwd_entry, char *restrict buffer, size_t size,
					 struct passwd **restrict result)
{
	VALIDATE_STRING(name, EINVAL, EINVAL);

	if (pwd_entry == NULL || buffer == NULL || result == NULL)
	{
		errno = EINVAL;
		return EINVAL;
	}

	// Save errno and restore it if no error has been encountered.
	errno_t old_errno, new_errno;
	old_errno = errno;
	errno = 0;

	struct passwd *entry = common_getpwnam(name, pwd_entry, buffer, size);
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
