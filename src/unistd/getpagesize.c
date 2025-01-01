/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <unistd.h>

int wlibc_getpagesize()
{
	// This will not fail.
	SYSTEM_BASIC_INFORMATION basic_info;
	NtQuerySystemInformation(SystemBasicInformation, &basic_info, sizeof(SYSTEM_BASIC_INFORMATION), NULL);
	return basic_info.PageSize;
}
