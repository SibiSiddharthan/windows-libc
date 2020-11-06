/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <unistd.h>
#include <signal.h>
#include <Windows.h>
#include <errno.h>
#include <wlibc_errors.h>

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

	if (!TerminateProcess(process, 128 + sig))
	{
		map_win32_error_to_wlibc(GetLastError());
		return -1;
	}

	return 0;
}