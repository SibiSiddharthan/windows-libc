/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <unistd.h>

int wlibc_getdtablesize()
{
	return 8192; // From rlimit.c
}
