/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/security.h>
#include <unistd.h>

gid_t wlibc_getgid()
{
	return current_gid;
}

int wlibc_setgid(gid_t gid)
{
	// Nop
	UNREFERENCED_PARAMETER(gid);
	return 0;
}
