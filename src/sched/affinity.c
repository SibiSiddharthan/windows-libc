/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <errno.h>
#include <sched.h>

int wlibc_sched_getaffinity(pid_t pid, size_t cpusetsize /*unused*/, cpu_set_t *cpuset)
{
	int result = -1;
	NTSTATUS status;
	OBJECT_ATTRIBUTES object;
	HANDLE handle = 0;
	CLIENT_ID client_id;
	LONGLONG mask;
	// TODO: Scale this beyond 64 cores.

	if (cpuset == NULL || cpuset->num_groups == 0)
	{
		errno = EINVAL;
		return -1;
	}

	if (pid != 0)
	{
		InitializeObjectAttributes(&object, NULL, 0, NULL, NULL);
		client_id.UniqueProcess = (HANDLE)(LONG_PTR)pid;
		client_id.UniqueThread = 0;

		status = NtOpenProcess(&handle, PROCESS_QUERY_LIMITED_INFORMATION, &object, &client_id);
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}
	}
	else // pid == 0 -> Current process
	{
		handle = NtCurrentProcess();
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
	if (handle != NtCurrentProcess() && handle != 0)
	{
		NtClose(handle);
	}
	return result;
}

int wlibc_sched_setaffinity(pid_t pid, size_t cpusetsize /*unused*/, const cpu_set_t *cpuset)
{
	NTSTATUS status;
	OBJECT_ATTRIBUTES object;
	HANDLE handle;
	CLIENT_ID client_id;
	LONGLONG mask;
	// TODO: Scale this beyond 64 cores.

	if (cpuset == NULL || cpuset->num_groups == 0)
	{
		errno = EINVAL;
		return -1;
	}

	// Fill only the information of the first group.
	mask = cpuset->group_mask[0];

	if (pid != 0)
	{
		InitializeObjectAttributes(&object, NULL, 0, NULL, NULL);
		client_id.UniqueProcess = (HANDLE)(LONG_PTR)pid;
		client_id.UniqueThread = 0;

		status = NtOpenProcess(&handle, PROCESS_SET_LIMITED_INFORMATION, &object, &client_id);
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}

		status = NtSetInformationProcess(handle, ProcessDefaultCpuSetsInformation, &mask, sizeof(LONGLONG));
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			NtClose(handle);
			return -1;
		}

		NtClose(handle);
	}
	else // pid == 0 -> Current process
	{
		status = NtSetInformationProcess(NtCurrentProcess(), ProcessDefaultCpuSetsInformation, &mask, sizeof(LONGLONG));
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}
	}

	return 0;
}
