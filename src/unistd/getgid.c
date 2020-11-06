/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
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