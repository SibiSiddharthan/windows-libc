/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>

/* Unsupported functions
   Return 0 for now.
   Later we can use SIDs for determining them. TODO
*/

uid_t wlibc_getuid()
{
	return 0;
}

uid_t wlibc_geteuid()
{
	return 0;
}
