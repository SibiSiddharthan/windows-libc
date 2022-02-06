/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <dlfcn.h>

void *wlibc_dlopen(const char *filename, int flags /*unused*/)
{
	if (filename == NULL)
	{
		return NULL;
	}

	NTSTATUS status;
	UTF8_STRING u8_image;
	UNICODE_STRING u16_image;
	HANDLE handle;

	RtlInitUTF8String(&u8_image, filename);
	RtlUTF8StringToUnicodeString(&u16_image, &u8_image, TRUE);

	status = LdrLoadDll(NULL, NULL, &u16_image, &handle);
	if (status != STATUS_SUCCESS)
	{
		_wlibc_last_dlfcn_error = status;
		handle = NULL;
	}

	RtlFreeUnicodeString(&u16_image);
	return handle;
}
