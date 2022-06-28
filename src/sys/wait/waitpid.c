/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/signal.h>
#include <internal/spawn.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/wait.h>

pid_t wait_child(pid_t pid, int *wstatus, int options)
{
	NTSTATUS status;
	PROCESS_BASIC_INFORMATION basic_info;
	LARGE_INTEGER timeout = {0};
	processinfo pinfo;

	get_processinfo(pid, &pinfo);

	if (pinfo.handle == INVALID_HANDLE_VALUE)
	{
		errno = ECHILD;
		return -1;
	}

	status = NtWaitForSingleObject(pinfo.handle, FALSE, (options & WNOHANG) ? &timeout : NULL);
	if (status != STATUS_SUCCESS)
	{
		if (status == STATUS_TIMEOUT) // WNOHANG
		{
			if (wstatus)
			{
				*wstatus = -1;
			}

			return 0;
		}

		map_ntstatus_to_errno(status);
		return -1;
	}

	if (wstatus)
	{
		status = NtQueryInformationProcess(pinfo.handle, ProcessBasicInformation, &basic_info, sizeof(PROCESS_BASIC_INFORMATION), NULL);
		if (status == STATUS_SUCCESS)
		{
			*wstatus = (int)basic_info.ExitStatus;
		}
	}

	delete_child(pinfo.id);

	return pid;
}

pid_t wait_all_children(int *wstatus, int options)
{
	NTSTATUS status;
	PROCESS_BASIC_INFORMATION basic_info;
	LARGE_INTEGER timeout = {0};
	HANDLE child_handles[MAXIMUM_WAIT_OBJECTS] = {0};
	DWORD child_pids[MAXIMUM_WAIT_OBJECTS] = {0};
	ULONG count = 0;

	// We can only wait for 64(MAXIMUM_WAIT_OBJECTS) children simultaneously at a time. Ignore the rest.
	SHARED_LOCK_PROCESS_TABLE();
	for (size_t i = 0; i < _wlibc_process_table_size; ++i)
	{
		if (_wlibc_process_table[i].handle != INVALID_HANDLE_VALUE)
		{
			child_handles[count] = _wlibc_process_table[i].handle;
			child_pids[count] = _wlibc_process_table[i].id;
			++count;
		}
	}
	SHARED_UNLOCK_PROCESS_TABLE();

	status = NtWaitForMultipleObjects(count, child_handles, WaitAny, FALSE, (options & WNOHANG) ? &timeout : NULL);

	if (status == STATUS_TIMEOUT) // WNOHANG
	{
		if (wstatus)
		{
			*wstatus = -1;
		}

		return 0;
	}

	if (status < STATUS_WAIT_0 && status > STATUS_WAIT_63)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	// After this point status will be between 0 (STATUS_WAIT_0) and 63 (STATUS_WAIT_63).
	if (wstatus)
	{
		status =
			NtQueryInformationProcess(child_handles[status], ProcessBasicInformation, &basic_info, sizeof(PROCESS_BASIC_INFORMATION), NULL);
		if (status == STATUS_SUCCESS)
		{
			*wstatus = (int)basic_info.ExitStatus;
		}
	}

	delete_child(child_pids[status]);

	return child_pids[status];
}

pid_t wlibc_waitpid(pid_t pid, int *wstatus, int options)
{
	pid_t result = -1;

	if (pid < -1)
	{
		errno = ENOTSUP;
		return -1;
	}

	if (options > 3)
	{
		errno = EINVAL;
		return -1;
	}

	if (pid > 0)
	{
		result = wait_child(pid, wstatus, options);
	}
	else if (pid == -1 || pid == 0) // Same thing for us (wait for all children)
	{
		result = wait_all_children(wstatus, options);
	}

	if (result > 0) // Not an error and when options is WNOHANG
	{
		bool should_raise_SIGCHLD = true;

		EnterCriticalSection(&_wlibc_signal_critical);
		if (!(_wlibc_signal_flags[SIGCHLD] & SA_NOCLDSTOP))
		{
			should_raise_SIGCHLD = false;
		}
		LeaveCriticalSection(&_wlibc_signal_critical);

		if (should_raise_SIGCHLD)
		{
			wlibc_raise(SIGCHLD);
		}
	}
	return result;
}
