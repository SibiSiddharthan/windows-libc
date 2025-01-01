/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <sys/param.h>
#include <unistd.h>

int wlibc_nice(int change)
{
	NTSTATUS status;
	PROCESS_BASIC_INFORMATION basic_info;
	KPRIORITY priority;
	ULONG privilege;
	PVOID state;

	if (change == 0)
	{
		// Nop
		return 0;
	}
	else if (change > 0)
	{
		// Decreasing priority.
		status =
			NtQueryInformationProcess(NtCurrentProcess(), ProcessBasicInformation, &basic_info, sizeof(PROCESS_BASIC_INFORMATION), NULL);
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}

		// Bound priority to be greater than 0, as 0 means a idle process.
		priority = MAX(basic_info.BasePriority - change, 1);

		status = NtSetInformationProcess(NtCurrentProcess(), ProcessBasePriority, &priority, sizeof(KPRIORITY));
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}
	}
	else // (change < 0)
	{
		// Increasing priority.
		status =
			NtQueryInformationProcess(NtCurrentProcess(), ProcessBasicInformation, &basic_info, sizeof(PROCESS_BASIC_INFORMATION), NULL);
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}

		// To increase the base priority we require 'SE_INC_BASE_PRIORITY_PRIVILEGE'. If we don't possess it, fail with 'EPERM'.
		privilege = SE_INC_BASE_PRIORITY_PRIVILEGE;
		status = RtlAcquirePrivilege(&privilege, 1, 0, &state);
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}

		// Bound priority to be less than 16, as 16 means a realtime process.
		priority = MIN(basic_info.BasePriority - change, 15);

		status = NtSetInformationProcess(NtCurrentProcess(), ProcessBasePriority, &priority, sizeof(KPRIORITY));
		// If execution reaches here, it means we have acquired 'SE_INC_BASE_PRIORITY_PRIVILEGE'.
		// Release it before checking status.
		RtlReleasePrivilege(state);

		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}
	}

	return 0;
}
