/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <sys/stat.h>
#include <internal/nt.h>
#include <internal/error.h>
#include <internal/misc.h>
#include <internal/fcntl.h>
#include <errno.h>

int wlibc_common_mkdir(int dirfd, const char *path, mode_t mode)
{
	if (mode > 0777)
	{
		errno = EINVAL;
		return -1;
	}
	VALIDATE_PATH_AND_DIRFD(path, dirfd);

	wchar_t *u16_ntpath = get_absolute_ntpath(dirfd, path);
	if (path == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	IO_STATUS_BLOCK I;
	UNICODE_STRING u16_path;
	RtlInitUnicodeString(&u16_path, u16_ntpath);
	OBJECT_ATTRIBUTES object;
	InitializeObjectAttributes(&object, &u16_path, OBJ_CASE_INSENSITIVE, NULL, NULL);
	HANDLE handle;
	NTSTATUS status = NtCreateFile(&handle, FILE_READ_ATTRIBUTES, &object, &I, NULL, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_CREATE,
								   FILE_DIRECTORY_FILE, NULL, 0);
	free(u16_ntpath);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}
	NtClose(handle);

	return 0;
}
