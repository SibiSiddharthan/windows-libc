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

uid_t wlibc_getuid()
{
	return 0;
}

uid_t wlibc_geteuid()
{
	return 0;
}