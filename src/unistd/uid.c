/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/security.h>
#include <unistd.h>

uid_t wlibc_getuid()
{
	return current_uid;
}

int wlibc_setuid(uid_t uid)
{
	// Nop
	UNREFERENCED_PARAMETER(uid);
	return 0;
}
