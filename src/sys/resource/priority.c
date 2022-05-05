/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <errno.h>
#include <sys/resource.h>

// From sched/open.c
HANDLE open_process(DWORD pid, ACCESS_MASK access);

int wlibc_getpriority(int which, id_t who)
{
	int result = -1;
	NTSTATUS status;
	HANDLE process;
	PROCESS_BASIC_INFORMATION basic_info;

	if (which < PRIO_PROCESS || which > PRIO_USER)
	{
		errno = EINVAL;
		return -1;
	}

	if (who < 0)
	{
		errno = EINVAL;
		return -1;
	}

	if (which == PRIO_USER)
	{
		// Unsupported.
		return 0;
	}

	// TODO process groups.
	process = open_process(who, PROCESS_QUERY_INFORMATION);
	if (process == NULL)
	{
		// errno will be set by `open_process`.
		return -1;
	}

	status = NtQueryInformationProcess(process, ProcessBasicInformation, &basic_info, sizeof(PROCESS_BASIC_INFORMATION), NULL);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		goto finish;
	}

	result = 16 - basic_info.BasePriority;

finish:
	if (process != NtCurrentProcess())
	{
		NtClose(process);
	}

	return result;
}

int wlibc_setpriority(int which, id_t who, int priority /*actually nice*/)
{
	int result = -1;
	NTSTATUS status;
	HANDLE process;
	PROCESS_BASIC_INFORMATION basic_info;
	KPRIORITY new_base_priority = 16 - priority;

	if (which < PRIO_PROCESS || which > PRIO_USER)
	{
		errno = EINVAL;
		return -1;
	}

	if (priority < PRIO_MIN || priority > PRIO_MAX)
	{
		errno = EINVAL;
		return -1;
	}

	if (who < 0)
	{
		errno = EINVAL;
		return -1;
	}

	if (which == PRIO_USER)
	{
		// Unsupported.
		return 0;
	}

	if(new_base_priority > 15)
	{
		// Restrict the priority to prevent realtime scheduling.
		new_base_priority = 15;
	}

	// TODO process groups.
	process = open_process(who, PROCESS_QUERY_INFORMATION | PROCESS_SET_INFORMATION);
	if (process == 0)
	{
		// errno will be set by `open_process`.
		return -1;
	}

	status = NtQueryInformationProcess(process, ProcessBasicInformation, &basic_info, sizeof(PROCESS_BASIC_INFORMATION), NULL);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		goto finish;
	}

	if (basic_info.BasePriority == new_base_priority)
	{
		// Same priority, nop.
		result = 0;
		goto finish;
	}
	else if (basic_info.BasePriority > new_base_priority)
	{
		// Lowering base priority.
		status = NtSetInformationProcess(process, ProcessBasePriority, &new_base_priority, sizeof(KPRIORITY));
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			goto finish;
		}
	}
	else
	{
		// Increasing base priority.
		ULONG privilege = SE_INC_BASE_PRIORITY_PRIVILEGE;
		PVOID state;

		status = RtlAcquirePrivilege(&privilege, 1, 0, &state);
		if (status == STATUS_PRIVILEGE_NOT_HELD)
		{
			// We don't have 'SeIncreaseBasePriorityPrivilege'. Fallback to using 'ProcessRaisePriority'
			// to boost the priority of all the threads.
			ULONG priority_increase = (ULONG)(new_base_priority - basic_info.BasePriority);

			status = NtSetInformationProcess(process, ProcessRaisePriority, &priority_increase, sizeof(ULONG));
			if (status != STATUS_SUCCESS)
			{
				map_ntstatus_to_errno(status);
				goto finish;
			}
		}
		else // (status == STATUS_SUCCESS)
		{
			status = NtSetInformationProcess(process, ProcessBasePriority, &new_base_priority, sizeof(KPRIORITY));
			RtlReleasePrivilege(state);

			if (status != STATUS_SUCCESS)
			{
				map_ntstatus_to_errno(status);
				goto finish;
			}
		}
	}

	result = 0;

finish:
	if (process != NtCurrentProcess())
	{
		NtClose(process);
	}

	return result;
}
