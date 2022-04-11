/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <sched.h>

int wlibc_sched_yield(void)
{
	NtYieldExecution();
	return 0;
}
