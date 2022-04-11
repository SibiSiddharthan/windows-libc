/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <errno.h>
#include <sched.h>

HANDLE open_process(DWORD pid, ACCESS_MASK access);

int wlibc_sched_getscheduler(pid_t pid)
{
	int result = -1;
	NTSTATUS status;
	HANDLE handle;
	PROCESS_PRIORITY_CLASS priority_class;
	priority_class.PriorityClass = PROCESS_PRIORITY_CLASS_UNKNOWN;

	handle = open_process(pid, PROCESS_QUERY_INFORMATION);
	if (handle == 0)
	{
		// errno will be set by `open_process`.
		return -1;
	}

	status = NtQueryInformationProcess(handle, ProcessPriorityClass, &priority_class, sizeof(PROCESS_PRIORITY_CLASS), NULL);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		goto finish;
	}

	result = priority_class.PriorityClass;

finish:
	if (handle != NtCurrentProcess())
	{
		NtClose(handle);
	}
	return result;
}

int wlibc_sched_setscheduler(pid_t pid, int policy, const struct sched_param *param /*unused*/)
{
	int result = -1;
	NTSTATUS status;
	HANDLE handle;
	PROCESS_PRIORITY_CLASS priority_class;

	// To set a process with realtime priority we need 'SeIncreaseBasePriorityPrivilege'.
	// This is will not be supported here.
	if ((policy < SCHED_IDLE || policy > SCHED_SPORADIC) && policy != PROCESS_PRIORITY_CLASS_REALTIME)
	{
		errno = EINVAL;
		return -1;
	}

	priority_class.PriorityClass = policy;

	handle = open_process(pid, PROCESS_SET_INFORMATION);
	if (handle == 0)
	{
		// errno will be set by `open_process`.
		return -1;
	}

	status = NtSetInformationProcess(handle, ProcessPriorityClass, &priority_class, sizeof(PROCESS_PRIORITY_CLASS));
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
