/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <sys/wait.h>
#include <process_internal.h>
#include <errno.h>
#include <wlibc_errors.h>
#include <stdlib.h>
#include <Windows.h>
#include <signal_internal.h>
#include <stdbool.h>

pid_t wlibc_waitpid_implementation(pid_t pid, int *wstatus, int options)
{
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

	if (pid == -1 || pid == 0) // Same thing for us (wait for all children)
	{
		DWORD child_count = get_child_process_count();
		HANDLE *child_handles = (HANDLE *)malloc(sizeof(HANDLE) * child_count);

		EnterCriticalSection(&_wlibc_process_critical);
		for (int i = 0; i < child_count; i++)
		{
			child_handles[i] = _wlibc_process_table[i].process_handle;
		}
		LeaveCriticalSection(&_wlibc_process_critical);

		DWORD wait_result = WaitForMultipleObjects(child_count, child_handles, FALSE, (options & WNOHANG) ? 0 : INFINITE);
		if (wait_result == WAIT_FAILED)
		{
			map_win32_error_to_wlibc(GetLastError());
			free(child_handles);
			return -1;
		}
		else if (wait_result >= WAIT_OBJECT_0 &&
				 wait_result < WAIT_OBJECT_0 + child_count) // return code - WAIT_OBJECT_0 = array index of child_handles
		{
			DWORD child_exit_code;
			if (!GetExitCodeProcess(child_handles[wait_result - WAIT_OBJECT_0], &child_exit_code))
			{
				map_win32_error_to_wlibc(GetLastError());
				free(child_handles);
				return -1;
			}

			if (wstatus)
			{
				*wstatus = child_exit_code;
			}
			delete_child(child_handles[wait_result - WAIT_OBJECT_0]);
			free(child_handles);
			return pid;
		}
		else if (wait_result == WAIT_TIMEOUT) // WNOHANG
		{
			if (wstatus)
			{
				*wstatus = -1;
			}
			free(child_handles);
			return 0;
		}
	}

	if (pid > 0)
	{
		if (is_child(pid))
		{
			HANDLE child_handle = get_child_handle(pid);
			DWORD wait_result = WaitForSingleObject(child_handle, (options & WNOHANG) ? 0 : INFINITE);
			if (wait_result == WAIT_FAILED)
			{
				map_win32_error_to_wlibc(GetLastError());
				return -1;
			}
			else if (wait_result == WAIT_OBJECT_0) // child has exited
			{
				DWORD child_exit_code;
				if (!GetExitCodeProcess(child_handle, &child_exit_code))
				{
					map_win32_error_to_wlibc(GetLastError());
					return -1;
				}

				if (wstatus)
				{
					*wstatus = child_exit_code;
				}
				delete_child(child_handle);
				return pid;
			}
			else if (wait_result == WAIT_TIMEOUT) // WNOHANG
			{
				if (wstatus)
				{
					*wstatus = -1;
				}
				return 0;
			}
		}

		else
		{
			errno = ECHILD;
			return -1;
		}
	}

	// Should be unreachable
	return -1;
}

pid_t wlibc_waitpid(pid_t pid, int *wstatus, int options)
{
	pid_t _pid = wlibc_waitpid_implementation(pid, wstatus, options);
	if (_pid > 0) // Not an error and when options is WNOHANG
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
	return _pid;
}
