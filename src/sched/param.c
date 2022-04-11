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

int wlibc_sched_getparam(pid_t pid, struct sched_param *param)
{
	int result = -1;
	NTSTATUS status;
	HANDLE handle;
	PROCESS_BASIC_INFORMATION basic_info;
	PROCESS_PRIORITY_CLASS priority_class;

	if (param == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	handle = open_process(pid, PROCESS_QUERY_INFORMATION);
	if (handle == 0)
	{
		// errno will be set by `open_process`.
		return -1;
	}

	status = NtQueryInformationProcess(handle, ProcessBasicInformation, &basic_info, sizeof(PROCESS_BASIC_INFORMATION), NULL);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		goto finish;
	}

	status = NtQueryInformationProcess(handle, ProcessPriorityClass, &priority_class, sizeof(PROCESS_PRIORITY_CLASS), NULL);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		goto finish;
	}

	switch (priority_class.PriorityClass)
	{
	case PROCESS_PRIORITY_CLASS_IDLE:
		param->sched_priority = basic_info.BasePriority - 4;
		break;
	case PROCESS_PRIORITY_CLASS_NORMAL:
		param->sched_priority = basic_info.BasePriority - 8;
		break;
	case PROCESS_PRIORITY_CLASS_HIGH:
		param->sched_priority = basic_info.BasePriority - 13;
		break;
	case PROCESS_PRIORITY_CLASS_BELOW_NORMAL:
		param->sched_priority = basic_info.BasePriority - 6;
		break;
	case PROCESS_PRIORITY_CLASS_ABOVE_NORMAL:
		param->sched_priority = basic_info.BasePriority - 10;
		break;
	default:
		param->sched_priority = 0;
	}

	// Bound the priorities.
	if (param->sched_priority < -2)
	{
		param->sched_priority = -2;
	}

	if (param->sched_priority > 2)
	{
		param->sched_priority = 2;
	}

	result = 0;

finish:
	if (handle != NtCurrentProcess())
	{
		NtClose(handle);
	}
	return result;
}

int wlibc_sched_setparam(pid_t pid, const struct sched_param *param)
{
	int result = -1;
	NTSTATUS status;
	HANDLE handle;
	KPRIORITY base_priority;
	PROCESS_BASIC_INFORMATION basic_info;
	PROCESS_PRIORITY_CLASS priority_class;

	if (param == NULL || param->sched_priority < -2 || param->sched_priority > 2)
	{
		errno = EINVAL;
		return -1;
	}

	handle = open_process(pid, PROCESS_QUERY_INFORMATION | PROCESS_SET_INFORMATION);
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

	switch (priority_class.PriorityClass)
	{
	case PROCESS_PRIORITY_CLASS_IDLE:
		base_priority = 4;
		break;
	case PROCESS_PRIORITY_CLASS_NORMAL:
		base_priority = 8;
		break;
	case PROCESS_PRIORITY_CLASS_HIGH:
		base_priority = 13;
		break;
	case PROCESS_PRIORITY_CLASS_BELOW_NORMAL:
		base_priority = 6;
		break;
	case PROCESS_PRIORITY_CLASS_ABOVE_NORMAL:
		base_priority = 10;
		break;
	default:
		// Should be unreachable.
		goto finish;
	}

	status = NtQueryInformationProcess(handle, ProcessBasicInformation, &basic_info, sizeof(PROCESS_BASIC_INFORMATION), NULL);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		goto finish;
	}

	if (base_priority + param->sched_priority == basic_info.BasePriority)
	{
		// No need to increase or decrease in this case.
		result = 0;
		goto finish;
	}

	if (base_priority + param->sched_priority > basic_info.BasePriority)
	{
		// Increasing priority.
		ULONG priority_increase = (ULONG)(base_priority + param->sched_priority - basic_info.BasePriority);

		status = NtSetInformationProcess(handle, ProcessRaisePriority, &priority_increase, sizeof(ULONG));
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			goto finish;
		}

		result = 0;
		goto finish;
	}

	if (base_priority + param->sched_priority < basic_info.BasePriority)
	{
		// Decreasing priority.
		base_priority += param->sched_priority;

		status = NtSetInformationProcess(handle, ProcessBasePriority, &base_priority, sizeof(KPRIORITY));
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			goto finish;
		}

		result = 0;
		goto finish;
	}

finish:
	if (handle != NtCurrentProcess())
	{
		NtClose(handle);
	}
	return result;
}
