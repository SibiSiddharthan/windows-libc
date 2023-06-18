/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <internal/path.h>
#include <internal/security.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>

int wlibc_common_mkdir(int dirfd, const char *path, mode_t mode)
{
	if (mode > 0777)
	{
		errno = EINVAL;
		return -1;
	}
	VALIDATE_PATH_AND_DIRFD(path, dirfd);

	NTSTATUS status;
	IO_STATUS_BLOCK io;
	UNICODE_STRING *u16_ntpath;
	OBJECT_ATTRIBUTES object;
	PSECURITY_DESCRIPTOR security_descriptor = (PSECURITY_DESCRIPTOR)get_security_descriptor(mode & 0777, 1);
	HANDLE handle;

	u16_ntpath = get_absolute_ntpath(dirfd, path);
	if (u16_ntpath == NULL)
	{
		// errno will be set by `get_absolute_ntpath`
		return -1;
	}

	InitializeObjectAttributes(&object, u16_ntpath, OBJ_CASE_INSENSITIVE, NULL, security_descriptor);
	status = NtCreateFile(&handle, FILE_READ_ATTRIBUTES, &object, &io, NULL, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_CREATE,
						  FILE_DIRECTORY_FILE, NULL, 0);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, u16_ntpath);

	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	NtClose(handle);

	return 0;
}
