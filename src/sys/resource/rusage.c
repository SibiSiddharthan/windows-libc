/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <errno.h>
#include <string.h>
#include <sys/resource.h>

int wlibc_getrusage(int who, struct rusage *usage)
{
	NTSTATUS status;
	KERNEL_USER_TIMES user_times;
	IO_COUNTERS io_counters;
	VM_COUNTERS_EX2 vm_counters;

	if (who < RUSAGE_SELF || who > RUSAGE_CHILDREN)
	{
		errno = EINVAL;
		return -1;
	}

	memset(usage, 0, sizeof(struct rusage));

	if (who != RUSAGE_CHILDREN) // (who == RUSAGE_SELF || who == RUSAGE_THREAD)
	{

		status = NtQueryInformationProcess(NtCurrentProcess(), ProcessTimes, &user_times, sizeof(KERNEL_USER_TIMES), NULL);
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}

		usage->ru_stime.tv_sec = user_times.KernelTime.QuadPart / 10000000;
		usage->ru_stime.tv_usec = (user_times.KernelTime.QuadPart % 10000000) / 10;

		usage->ru_utime.tv_sec = user_times.UserTime.QuadPart / 10000000;
		usage->ru_utime.tv_usec = (user_times.UserTime.QuadPart % 10000000) / 10;

		status = NtQueryInformationProcess(NtCurrentProcess(), ProcessIoCounters, &io_counters, sizeof(IO_COUNTERS), NULL);
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}

		usage->ru_inblock = io_counters.WriteOperationCount;
		usage->ru_oublock = io_counters.ReadOperationCount;

		status = NtQueryInformationProcess(NtCurrentProcess(), ProcessVmCounters, &vm_counters, sizeof(VM_COUNTERS_EX2), NULL);
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}

		usage->ru_ixrss = vm_counters.SharedCommitUsage;
		usage->ru_idrss = vm_counters.PrivateWorkingSetSize;
		usage->ru_isrss = vm_counters.PrivateWorkingSetSize;

		usage->ru_maxrss = vm_counters.CountersEx.PeakWorkingSetSize;
		usage->ru_majflt = vm_counters.CountersEx.PageFaultCount;
		usage->ru_minflt = vm_counters.CountersEx.PageFaultCount;
		usage->ru_nswap = vm_counters.CountersEx.PeakPagefileUsage;
	}

	return 0;
}
