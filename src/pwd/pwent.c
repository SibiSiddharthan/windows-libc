/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/registry.h>
#include <LM.h>
#include <sddl.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <wchar.h>
#include <stdio.h>

static int pw_index = 0;
static int pw_started = 0;
static BYTE *enum_buffer = NULL;

int USER_INFO_3_to_passwd(PUSER_INFO_3 user_info, struct passwd *pw_entry, void *buffer, size_t size)
{
	NTSTATUS ntstatus;
	BOOL status;
	UNICODE_STRING u16_name, u16_gecos, u16_homedir;
	UTF8_STRING u8_name, u8_gecos, u8_homedir;
	DWORD domain_size = 128, sid_size = SECURITY_SID_SIZE(SID_MAX_SUB_AUTHORITIES);
	SID_NAME_USE use;
	size_t string_sid_length, homedir_size;
	char sid_buffer[SECURITY_SID_SIZE(SID_MAX_SUB_AUTHORITIES)];
	wchar_t *string_sid = NULL;
	wchar_t *registry_key = NULL;
	wchar_t *real_homedir = NULL;
	wchar_t domain_name[128]; // Unused

	// name
	RtlInitUnicodeString(&u16_name, user_info->usri3_name);
	u8_name.MaximumLength = size;
	u8_name.Buffer = (char *)buffer;

	ntstatus = RtlUnicodeStringToUTF8String(&u8_name, &u16_name, FALSE);
	if (ntstatus != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(ntstatus);
		return -1;
	}

	u8_name.MaximumLength = u8_name.Length + 1;

	// gecos
	if (size <= u8_name.MaximumLength)
	{
		errno = ERANGE;
		return -1;
	}

	RtlInitUnicodeString(&u16_gecos, user_info->usri3_comment);
	u8_gecos.MaximumLength = size - u8_name.MaximumLength;
	u8_gecos.Buffer = (char *)buffer + u8_name.MaximumLength;

	ntstatus = RtlUnicodeStringToUTF8String(&u8_gecos, &u16_gecos, FALSE);
	if (ntstatus != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(ntstatus);
		return -1;
	}
	u8_gecos.MaximumLength = u8_gecos.Length + 1;

	if (size <= u8_name.MaximumLength + u8_gecos.MaximumLength)
	{
		errno = ERANGE;
		return -1;
	}

	pw_entry->pw_name = u8_name.Buffer;
	pw_entry->pw_passwd = NULL;
	pw_entry->pw_uid = user_info->usri3_user_id;
	pw_entry->pw_gid = user_info->usri3_primary_group_id;
	pw_entry->pw_gecos = u8_gecos.Buffer;
	pw_entry->pw_dir = NULL;

	status = LookupAccountNameW(NULL, u16_name.Buffer, sid_buffer, &sid_size, domain_name, &domain_size, &use);
	if (status != 0)
	{
		status = ConvertSidToStringSidW(sid_buffer, &string_sid);
		if (status != 0)
		{
			string_sid_length = wcslen(string_sid);
			// "\\Registry\\User\\sid\\Volatile Environment\0"
			registry_key = (wchar_t *)malloc((15 + string_sid_length + 20 + 2) * sizeof(wchar_t));
			memcpy(registry_key, L"\\Registry\\User\\", 15 * sizeof(wchar_t));
			memcpy(registry_key + 15, string_sid, string_sid_length * sizeof(wchar_t));
			registry_key[15 + string_sid_length] = L'\\';
			memcpy(registry_key + 15 + string_sid_length + 1, L"Volatile Environment\0", 21 * sizeof(wchar_t));

			real_homedir = get_registry_value(registry_key, L"USERPROFILE", &homedir_size);

			LocalFree(string_sid);
			free(registry_key);

			if (real_homedir != NULL)
			{
				u16_homedir.Buffer = real_homedir;
				u16_homedir.Length = homedir_size;
				u16_homedir.MaximumLength = homedir_size;

				u8_homedir.MaximumLength = size - u8_name.MaximumLength + u8_gecos.MaximumLength;
				u8_homedir.Buffer = (char *)buffer + u8_name.MaximumLength + u8_gecos.MaximumLength;
				ntstatus = RtlUnicodeStringToUTF8String(&u8_homedir, &u16_homedir, FALSE);
				free(real_homedir);

				if (ntstatus != STATUS_SUCCESS)
				{
					map_ntstatus_to_errno(ntstatus);
					return -1;
				}

				pw_entry->pw_dir = u8_homedir.Buffer;
			}
		}
	}
	pw_entry->pw_shell = "C:\\Windows\\System32\\cmd.exe";

	return 0;
}

struct passwd *common_getpwent(struct passwd *restrict pwd_entry, char *restrict buffer, size_t size)
{
	static DWORD entries_read = 0, total_entries = 0;
	DWORD status;
	PUSER_INFO_3 user_info;

	// Starting to enumerate
	if (pw_started == 0)
	{
		// We don't need to resume as the we are fetching all records.
		status = NetUserEnum(NULL, 3, FILTER_NORMAL_ACCOUNT, &enum_buffer, MAX_PREFERRED_LENGTH, &entries_read, &total_entries, NULL);
		if (status == NERR_Success)
		{
			pw_started = 1;
			pw_index = 0;
		}
	}

	if (pw_index >= entries_read)
	{
		return NULL;
	}

	user_info = (PUSER_INFO_3)enum_buffer;

	if (USER_INFO_3_to_passwd(&user_info[pw_index], pwd_entry, buffer, size) == -1)
	{
		// errno will be set by 'USER_INFO_3_to_passwd'
		return NULL;
	}

	++pw_index;
	return pwd_entry;
}

struct passwd *wlibc_getpwent()
{
	static char pwd_buffer[512];
	static struct passwd entry;

	if (common_getpwent(&entry, pwd_buffer, 512) == NULL)
	{
		return NULL;
	}

	return &entry;
}

void wlibc_endpwent()
{
	// Free the static buffer and reset the state.
	if (pw_started == 1)
	{
		NetApiBufferFree(enum_buffer);
	}
	pw_started = 0;
	pw_index = 0;
}

void wlibc_setpwent()
{
	pw_index = 0;
}

int wlibc_getpwent_r(struct passwd *restrict pwd_entry, char *restrict buffer, size_t size, struct passwd **restrict result)
{
	if (pwd_entry == NULL || buffer == NULL || result == NULL || size < 0)
	{
		errno = EINVAL;
		return -1;
	}

	// Save errno and restore it if no error has been encountered.
	errno_t old_errno, new_errno;
	old_errno = errno;
	errno = 0;

	struct passwd *entry = common_getpwent(pwd_entry, buffer, size);
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
