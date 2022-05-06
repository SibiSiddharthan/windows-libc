/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <unistd.h>

pid_t wlibc_getpid()
{
	return (pid_t)NtCurrentProcessId();
}

pid_t wlibc_getppid()
{
	// This will not fail.
	PROCESS_BASIC_INFORMATION basic_info;
	NtQueryInformationProcess(NtCurrentProcess(), ProcessBasicInformation, &basic_info, sizeof(PROCESS_BASIC_INFORMATION), NULL);
	return (pid_t)(INT_PTR)basic_info.InheritedFromUniqueProcessId;
}

pid_t wlibc_gettid()
{
	return (pid_t)NtCurrentThreadId();
}
