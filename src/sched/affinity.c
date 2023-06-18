/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <errno.h>
#include <sched.h>

HANDLE open_process(DWORD pid, ACCESS_MASK access);

int wlibc_sched_getaffinity(pid_t pid, cpu_set_t *cpuset)
{
	int result = -1;
	NTSTATUS status;
	HANDLE handle;
	LONGLONG mask;
	// TODO: Scale this beyond 64 cores.

	if (cpuset == NULL || cpuset->num_groups == 0)
	{
		errno = EINVAL;
		return -1;
	}

	handle = open_process(pid, PROCESS_QUERY_LIMITED_INFORMATION);
	if (handle == 0)
	{
		// errno will be set by `open_process`.
		return -1;
	}

	status = NtQueryInformationProcess(NtCurrentProcess(), ProcessDefaultCpuSetsInformation, &mask, sizeof(LONGLONG), NULL);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		goto finish;
	}

	cpuset->group_mask[0] = mask;

	result = 0;

finish:
	if (handle != NtCurrentProcess())
	{
		NtClose(handle);
	}
	return result;
}

int wlibc_sched_setaffinity(pid_t pid, const cpu_set_t *cpuset)
{
	int result = -1;
	NTSTATUS status;
	HANDLE handle;
	LONGLONG mask;
	// TODO: Scale this beyond 64 cores.

	if (cpuset == NULL || cpuset->num_groups == 0)
	{
		errno = EINVAL;
		return -1;
	}

	// Fill only the information of the first group.
	mask = cpuset->group_mask[0];

	handle = open_process(pid, PROCESS_SET_LIMITED_INFORMATION);
	if (handle == 0)
	{
		// errno will be set by `open_process`.
		return -1;
	}

	status = NtSetInformationProcess(handle, ProcessDefaultCpuSetsInformation, &mask, sizeof(LONGLONG));
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		goto finish;
	}

	result = 0;

finish:
	if (handle != NtCurrentProcess())
	{
		NtClose(handle);
	}
	return result;
}
