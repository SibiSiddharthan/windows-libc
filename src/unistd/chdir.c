/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <internal/path.h>
#include <stdlib.h>
#include <unistd.h>

int common_chdir(const wchar_t *wname)
{
	// Changing the 'DosPath' in RTL_USER_PROCESS_PARAMETERS does not work.
	// Just call 'SetCurrentDirectoryW' and call it a day.
	// Luckily doing this also updates 'DosPath', so all is well.
	if (!SetCurrentDirectoryW(wname))
	{
		map_doserror_to_errno(GetLastError());
		return -1;
	}

	return 0;
}

int wlibc_chdir(const char *name)
{
	VALIDATE_PATH(name, ENOENT, -1);

	NTSTATUS status;
	UTF8_STRING u8_name;
	UNICODE_STRING u16_name;

	RtlInitUTF8String(&u8_name, name);
	status = RtlUTF8StringToUnicodeString(&u16_name, &u8_name, TRUE);

	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	int result = common_chdir(u16_name.Buffer);

	RtlFreeUnicodeString(&u16_name);
	return result;
}

int wlibc_fchdir(int fd)
{
	if (get_fd_type(fd) != DIRECTORY_HANDLE)
	{
		errno = ENOTDIR;
		return -1;
	}

	UNICODE_STRING *dirpath = get_fd_dospath(fd);
	if (dirpath == NULL)
	{
		// errno will be set by `get_fd_dospath`.
		return -1;
	}

	int result = common_chdir(dirpath->Buffer);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, dirpath);

	return result;
}
