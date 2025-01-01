/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/signal.h>
#include <internal/validate.h>
#include <signal.h>
#include <errno.h>

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

	if (new_action == NULL)
	{
		return 0;
	}

	if (sig == SIGKILL || sig == SIGSTOP)
	{
		errno = EINVAL;
		return -1;
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
