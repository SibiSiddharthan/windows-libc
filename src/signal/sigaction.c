/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/signal.h>
#include <internal/validate.h>
#include <signal.h>
#include <errno.h>

BOOL console_handler(DWORD ctrl)
{
	int status = 0;

	if (ctrl == CTRL_C_EVENT)
	{
		status = wlibc_raise(SIGINT);
	}
	else if (ctrl == CTRL_BREAK_EVENT)
	{
		status = wlibc_raise(SIGBREAK);
	}

	// Only return false if the execution of any of the signal handlers fail.
	if (status != 0)
	{
		return FALSE;
	}

	return TRUE;
}

int do_sigaction(int sig, const struct sigaction *new_action, struct sigaction *old_action)
{

	int status = 0;
	siginfo sinfo;

	if (old_action)
	{
		get_siginfo(sig, &sinfo);

		old_action->sa_handler = sinfo.action;
		old_action->sa_flags = sinfo.flags;
		old_action->sa_mask = sinfo.mask;
	}

	if (sig == SIGKILL || sig == SIGSTOP)
	{
		errno = EINVAL;
		return -1;
	}

	if (new_action == NULL)
	{
		return 0;
	}

	// Console interrupts require special handling.
	if (sig == SIGINT || sig == SIGTSTP)
	{
		if (SetConsoleCtrlHandler(console_handler, TRUE) == 0)
		{
			errno = EINVAL;
			status = -1;
		}
	}

	// Update the signal handling information in the signal table.
	sinfo.action = new_action->sa_handler;
	sinfo.flags = new_action->sa_flags;
	sinfo.mask = new_action->sa_mask;

	set_siginfo(sig, &sinfo);

	return status;
}

int wlibc_sigaction(int sig, const struct sigaction *new_action, struct sigaction *old_action)
{
	VALIDATE_SIGNAL(sig);

	return do_sigaction(sig, new_action, old_action);
}
