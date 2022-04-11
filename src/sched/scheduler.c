/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <errno.h>
#include <sched.h>

int wlibc_sched_getscheduler(pid_t pid)
{
	NTSTATUS status;
	OBJECT_ATTRIBUTES object;
	HANDLE handle;
	CLIENT_ID client_id;
	PROCESS_PRIORITY_CLASS priority_class;
	priority_class.PriorityClass = PROCESS_PRIORITY_CLASS_UNKNOWN;

	if (pid != 0)
	{
		InitializeObjectAttributes(&object, NULL, 0, NULL, NULL);
		client_id.UniqueProcess = (HANDLE)(LONG_PTR)pid;
		client_id.UniqueThread = 0;

		status = NtOpenProcess(&handle, PROCESS_QUERY_INFORMATION, &object, &client_id);
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}

		status = NtQueryInformationProcess(handle, ProcessPriorityClass, &priority_class, sizeof(PROCESS_PRIORITY_CLASS), NULL);
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
		status = NtQueryInformationProcess(NtCurrentProcess(), ProcessPriorityClass, &priority_class, sizeof(PROCESS_PRIORITY_CLASS), NULL);
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}
	}

	return priority_class.PriorityClass;
}

int wlibc_sched_setscheduler(pid_t pid, int policy, const struct sched_param *param /*unused*/)
{

	NTSTATUS status;
	OBJECT_ATTRIBUTES object;
	HANDLE handle;
	CLIENT_ID client_id;
	PROCESS_PRIORITY_CLASS priority_class;

	// To set a process with realtime priority we need 'SeIncreaseBasePriorityPrivilege'.
	// This is will not be supported here.
	if((policy < SCHED_IDLE || policy > SCHED_SPORADIC) && policy != PROCESS_PRIORITY_CLASS_REALTIME)
	{
		errno = EINVAL;
		return -1;
	}

	priority_class.PriorityClass = policy;

	if (pid != 0)
	{
		InitializeObjectAttributes(&object, NULL, 0, NULL, NULL);
		client_id.UniqueProcess = (HANDLE)(LONG_PTR)pid;
		client_id.UniqueThread = 0;

		status = NtOpenProcess(&handle, PROCESS_SET_INFORMATION, &object, &client_id);
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}

		status = NtSetInformationProcess(handle, ProcessPriorityClass, &priority_class, sizeof(PROCESS_PRIORITY_CLASS));
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
		status = NtSetInformationProcess(NtCurrentProcess(), ProcessPriorityClass, &priority_class, sizeof(PROCESS_PRIORITY_CLASS));
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}
	}

	return 0;
}
