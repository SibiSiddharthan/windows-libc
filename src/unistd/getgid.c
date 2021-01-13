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

gid_t wlibc_getgid()
{
	return 0;
}

gid_t wlibc_getegid()
{
	return 0;
}
