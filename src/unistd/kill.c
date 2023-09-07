/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

// From sched/open.c
HANDLE open_process(DWORD pid, ACCESS_MASK access);

int wlibc_kill(pid_t pid, int sig)
{
	NTSTATUS status;
	HANDLE process;

	if (sig < 0 || sig > NSIG)
	{
		errno = EINVAL;
		return -1;
	}

	// If pid is us, then call raise directly.
	if (pid == (pid_t)NtCurrentProcessId())
	{
		return wlibc_raise(sig);
	}

	process = open_process(pid, PROCESS_TERMINATE | PROCESS_SUSPEND_RESUME);
	if (process == NULL)
	{
		// errno will be set by `open_process`.
		return -1;
	}

	// If sigstop is given suspend the process.
	if (sig == SIGSTOP)
	{
		status = NtSuspendProcess(process);
		NtClose(process);

		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}

		return 0;
	}

	// If sigcont is given resume the process.
	if (sig == SIGCONT)
	{
		status = NtResumeProcess(process);
		NtClose(process);

		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}

		return 0;
	}

	status = NtTerminateProcess(process, 128 + sig);
	NtClose(process);

	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}
