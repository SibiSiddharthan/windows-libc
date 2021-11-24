/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/security.h>
#include <unistd.h>

uid_t wlibc_getuid()
{
	return current_uid;
}
