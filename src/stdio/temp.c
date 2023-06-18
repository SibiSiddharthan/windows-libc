/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#define _CRT_RAND_S

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/registry.h>
#include <internal/security.h>
#include <sddl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wchar.h>

#define MAX_TRIES 16

static void append_random_string(char *str)
{
	char buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	unsigned int rn;
	rand_s(&rn);
	_ultoa_s(rn, buffer, 8, 36);
	strcat(str, buffer);
}

char *wlibc_tmpdir(void)
{
	static char tmpdir[L_tmpnam];
	static int tmpdir_initialized = 0;
	size_t string_sid_length, localdir_size;
	wchar_t *string_sid = NULL;
	wchar_t *registry_key = NULL;
	wchar_t *localdir = NULL;
	NTSTATUS status;
	UNICODE_STRING u16_localdir;
	UTF8_STRING u8_localdir;

	if (tmpdir_initialized == 0)
	{
		// This convertion will not fail.
		ConvertSidToStringSidW((PSID)current_user_sid, &string_sid);

		string_sid_length = wcslen(string_sid);
		// "\\Registry\\User\\sid\\Volatile Environment\0"
		registry_key = (wchar_t *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, (15 + string_sid_length + 20 + 2) * sizeof(wchar_t));
		if (registry_key == NULL)
		{
			errno = ENOMEM;
			return NULL;
		}

		memcpy(registry_key, L"\\Registry\\User\\", 15 * sizeof(wchar_t));
		memcpy(registry_key + 15, string_sid, string_sid_length * sizeof(wchar_t));
		registry_key[15 + string_sid_length] = L'\\';
		memcpy(registry_key + 15 + string_sid_length + 1, L"Volatile Environment\0", 21 * sizeof(wchar_t));

		localdir = get_registry_value(registry_key, L"LOCALAPPDATA", &localdir_size);

		RtlFreeHeap(NtCurrentProcessHeap(), 0, registry_key);
		LocalFree(string_sid);

		if (localdir != NULL)
		{
			u16_localdir.Buffer = localdir;
			u16_localdir.Length = (USHORT)localdir_size;
			u16_localdir.MaximumLength = (USHORT)localdir_size;

			u8_localdir.Buffer = tmpdir;
			u8_localdir.MaximumLength = L_tmpnam;
			status = RtlUnicodeStringToUTF8String(&u8_localdir, &u16_localdir, FALSE);
			RtlFreeHeap(NtCurrentProcessHeap(), 0, localdir);

			if (status != STATUS_SUCCESS)
			{
				map_ntstatus_to_errno(status);
				return NULL;
			}

			// The directory will be of the form "C:\Users\XXXXX\AppData\Local\Temp".
			// In Windows the user name for the home directory will be truncated to 5 characters.(Don't ask me why.)
			// As a result there will be plenty of space left in the buffer.
			strcat(tmpdir, "\\Temp\\");
			tmpdir_initialized = 1;
		}
	}

	return tmpdir;
}

FILE *wlibc_tmpfile(void)
{
	int tries = 0;
	errno_t old_errno;
	char filename[L_tmpnam];
	FILE *stream = NULL;

	old_errno = errno;
	strcpy(filename, wlibc_tmpdir());

	do
	{
		// Keep trying till we succeed.
		append_random_string(filename);

		// 'x' means open with O_EXCL (GNU Extension)
		// 'D' means delete file after use (MSVC Extension)
		stream = wlibc_fopen(filename, "w+Dx");

		++tries;

	} while (stream == NULL && tries < MAX_TRIES);

	errno = old_errno;

	return stream;
}

char *wlibc_tempnam(const char *restrict dir, const char *restrict prefix)
{
	int tries = 0;
	int status;
	errno_t old_errno;
	size_t dir_length = 0, prefix_length = 0;
	// Unlike other functions tempname is malloc'ed. Users are expected to free it.
	char *tempname = NULL;

	old_errno = errno;

	/*
	   POSIX directory preference
	   1. Environment variable 'TMPDIR'
	   2. dir argument if not NULL
	   3. P_tmpdir

	   WLIBC directory preference
	   1. dir argument if not NULL
	   2. C:\Users\XXXXX\Appdata\Local\Temp (Same as P_tmpdir)
	*/

	if (prefix != NULL)
	{
		prefix_length = strlen(prefix);
	}

	if (dir != NULL)
	{
		// It does not matter whether dir is relative or absolute, as all the open functions will handle them appropriately.
		dir_length = strlen(dir);
		tempname = (char *)malloc(dir_length + prefix_length + 10); // NULL + slash if needed + random string
		if (tempname == NULL)
		{
			errno = ENOMEM;
			return NULL;
		}

		strcpy(tempname, dir);

		if (dir[dir_length - 1] != '/' && dir[dir_length - 1] != '\\')
		{
			tempname[dir_length] = '/';
			tempname[dir_length + 1] = '\0';
		}
	}
	else
	{
		dir_length = strlen(wlibc_tmpdir());
		tempname = (char *)malloc(dir_length + prefix_length + 10);
		if (tempname == NULL)
		{
			errno = ENOMEM;
			return NULL;
		}

		strcpy(tempname, wlibc_tmpdir());
	}

	if (prefix != NULL)
	{
		strcat(tempname, prefix);
	}

	while (tries < MAX_TRIES)
	{
		// Keep trying till we succeed.
		append_random_string(tempname);

		// Just make sure the file does not exist
		status = wlibc_common_access(AT_FDCWD, tempname, F_OK, AT_SYMLINK_NOFOLLOW);

		if (status == -1 && errno == ENOENT)
		{
			// Restore old errno.
			errno = old_errno;
			break;
		}

		++tries;
	}

	return tempname;
}

char *wlibc_tmpnam(const char *name)
{
	int tries = 0;
	static char tempname[L_tmpnam];
	int status;
	errno_t old_errno;

	old_errno = errno;
	strcpy(tempname, wlibc_tmpdir());

	if (name == NULL)
	{
		while (tries < MAX_TRIES)
		{
			// Keep trying till we succeed.
			append_random_string(tempname);

			// Just make sure the file does not exist
			status = wlibc_common_access(AT_FDCWD, tempname, F_OK, AT_SYMLINK_NOFOLLOW);

			if (status == -1 && errno == ENOENT)
			{
				// Restore old errno.
				errno = old_errno;
				break;
			}

			++tries;
		}
	}
	else
	{
		strcpy(tempname, wlibc_tmpdir());
		strcat(tempname, name);

		status = wlibc_common_access(AT_FDCWD, tempname, F_OK, AT_SYMLINK_NOFOLLOW);

		if (status == 0)
		{
			return NULL;
		}
	}

	return tempname;
}
