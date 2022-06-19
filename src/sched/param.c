/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/sched.h>
#include <internal/validate.h>
#include <errno.h>
#include <sched.h>
#include <sys/param.h>

HANDLE open_process(DWORD pid, ACCESS_MASK access);

static KPRIORITY get_base_priority_of_class(UCHAR priority_class)
{
	switch (priority_class)
	{
	case PROCESS_PRIORITY_CLASS_IDLE:
		return 4;
	case PROCESS_PRIORITY_CLASS_NORMAL:
		return 8;
	case PROCESS_PRIORITY_CLASS_HIGH:
		return 13;
	case PROCESS_PRIORITY_CLASS_REALTIME:
		// This should not be possible with this API, just list it here.
		return 16;
	case PROCESS_PRIORITY_CLASS_BELOW_NORMAL:
		return 6;
	case PROCESS_PRIORITY_CLASS_ABOVE_NORMAL:
		return 10;
	default: // Should be unreachable.
		return 0;
	}
}

int adjust_priority_of_process(HANDLE process, KPRIORITY new_priority)
{
	NTSTATUS status;
	KPRIORITY new_base_priority;
	KPRIORITY priority_of_class;
	PROCESS_BASIC_INFORMATION basic_info;
	PROCESS_PRIORITY_CLASS priority_class;

	status = NtQueryInformationProcess(process, ProcessPriorityClass, &priority_class, sizeof(PROCESS_PRIORITY_CLASS), NULL);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	priority_of_class = get_base_priority_of_class(priority_class.PriorityClass);

	status = NtQueryInformationProcess(process, ProcessBasicInformation, &basic_info, sizeof(PROCESS_BASIC_INFORMATION), NULL);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	if (priority_of_class + new_priority == basic_info.BasePriority)
	{
		// No need to increase or decrease in this case.
		return 0;
	}

	if (priority_of_class + new_priority > basic_info.BasePriority)
	{
		// Increasing priority.
		ULONG privilege = SE_INC_BASE_PRIORITY_PRIVILEGE;
		PVOID state;

		status = RtlAcquirePrivilege(&privilege, 1, 0, &state);
		if (status == STATUS_PRIVILEGE_NOT_HELD)
		{
			// We don't have 'SeIncreaseBasePriorityPrivilege'. Fallback to using 'ProcessRaisePriority'
			// to boost the priority of all the threads. This is fine here because we have bounded
			// the priorities between -2 and 2. (Like `SetThreadPriority`).
			ULONG priority_increase = (ULONG)(priority_of_class + new_priority - basic_info.BasePriority);

			status = NtSetInformationProcess(process, ProcessRaisePriority, &priority_increase, sizeof(ULONG));
			if (status != STATUS_SUCCESS)
			{
				map_ntstatus_to_errno(status);
				return -1;
			}
		}
		else // (status == STATUS_SUCCESS)
		{
			new_base_priority = priority_of_class + new_priority;

			status = NtSetInformationProcess(process, ProcessBasePriority, &new_base_priority, sizeof(KPRIORITY));
			RtlReleasePrivilege(state);

			if (status != STATUS_SUCCESS)
			{
				map_ntstatus_to_errno(status);
				return -1;
			}
		}
	}

	if (priority_of_class + new_priority < basic_info.BasePriority)
	{
		// Decreasing priority.
		new_base_priority = priority_of_class + new_priority;

		status = NtSetInformationProcess(process, ProcessBasePriority, &new_base_priority, sizeof(KPRIORITY));
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}
	}

	return 0;
}

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

	param->sched_priority = basic_info.BasePriority - get_base_priority_of_class(priority_class.PriorityClass);

	// Bound the priorities.
	param->sched_priority = MIN(MAX(param->sched_priority, SCHED_MIN_PRIORITY), SCHED_MAX_PRIORITY);

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
	HANDLE handle;

	VALIDATE_PTR(param, EINVAL, -1);
	VALIDATE_SCHED_PRIORITY(param->sched_priority, EINVAL, -1);

	handle = open_process(pid, PROCESS_QUERY_INFORMATION | PROCESS_SET_INFORMATION);
	if (handle == 0)
	{
		// errno will be set by `open_process`.
		return -1;
	}

	int result = adjust_priority_of_process(handle, param->sched_priority);

	if (handle != NtCurrentProcess())
	{
		NtClose(handle);
	}

	return result;
}
