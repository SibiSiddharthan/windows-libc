/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/sched.h>
#include <errno.h>
#include <sched.h>

HANDLE open_process(DWORD pid, ACCESS_MASK access);
int adjust_priority_of_process(HANDLE process, KPRIORITY change);

int wlibc_sched_getscheduler(pid_t pid)
{
	int result = -1;
	NTSTATUS status;
	HANDLE handle;
	__declspec(align(4)) PROCESS_PRIORITY_CLASS priority_class;

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

int wlibc_sched_setscheduler(pid_t pid, int policy, const struct sched_param *param)
{
	int result = -1;
	NTSTATUS status;
	HANDLE handle;
	__declspec(align(4)) PROCESS_PRIORITY_CLASS priority_class;

	// To set a process with realtime priority we need 'SeIncreaseBasePriorityPrivilege'.
	// This is will not be supported here.
	VALIDATE_SCHED_POLICY(policy, EINVAL, -1);

	if (param)
	{
		VALIDATE_SCHED_PRIORITY(param->sched_priority, EINVAL, -1);
	}

	priority_class.PriorityClass = (UCHAR)policy;

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

	if (param && param->sched_priority != 0)
	{
		result = adjust_priority_of_process(handle, param->sched_priority);
	}

	result = 0;

finish:
	if (handle != NtCurrentProcess())
	{
		NtClose(handle);
	}
	return result;
}
