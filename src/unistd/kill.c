/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <signal.h>
#include <Windows.h>
#include <errno.h>
#include <internal/error.h>

int wlibc_kill(pid_t pid, int sig)
{
	HANDLE process = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
	if (process == NULL)
	{
		map_win32_error_to_wlibc(GetLastError());
		return -1;
	}

	if(sig < 0 || sig > NSIG)
	{
		errno = EINVAL;
		return -1;
	}

	if (!TerminateProcess(process, 3)) // All signals exit with status code 3. Maintain compatibility.
	{
		map_win32_error_to_wlibc(GetLastError());
		return -1;
	}

	return 0;
}
