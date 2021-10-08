/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <internal/nt.h>
#include <internal/error.h>

int wlibc_common_sleep(long long nanoseconds_100)
{
	NTSTATUS status;
	LARGE_INTEGER interval;
	interval.QuadPart = -1 * nanoseconds_100; // relative
	status = NtDelayExecution(TRUE, &interval);
	// TODO see what to do in case of failure
	//if(status < 0)
	//{
	//	map_ntstatus_to_errno(status);
	//}

	return 0;
}
