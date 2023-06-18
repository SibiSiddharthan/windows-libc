/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/fcntl.h>
#include <internal/error.h>
#include <dlfcn.h>
#include <errno.h>

void *wlibc_dlopen(const char *filename, int flags)
{
	NTSTATUS status;
	UTF8_STRING u8_image;
	UNICODE_STRING u16_image;
	HANDLE handle;

	VALIDATE_PATH(filename, ENOENT, NULL);
	UNREFERENCED_PARAMETER(flags);

	RtlInitUTF8String(&u8_image, filename);
	status = RtlUTF8StringToUnicodeString(&u16_image, &u8_image, TRUE);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return NULL;
	}

	status = LdrLoadDll(NULL, NULL, &u16_image, &handle);
	if (status != STATUS_SUCCESS)
	{
		_wlibc_last_dlfcn_error = status;
		handle = NULL;
	}

	RtlFreeUnicodeString(&u16_image);
	return handle;
}
