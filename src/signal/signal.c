/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/signal.h>
#include <signal.h>
#include <errno.h>

int do_sigaction(int sig, const struct sigaction *new_action, struct sigaction *old_action);

signal_t wlibc_signal(int sig, signal_t handler)
{
	int status;
	struct sigaction new_action, old_action;

	if (sig <= 0 || sig >= NSIG)
	{
		errno = EINVAL;
		return SIG_ERR;
	}

	if (handler == SIG_ERR)
	{
		errno = EINVAL;
		return SIG_ERR;
	}

	if (handler == SIG_GET)
	{
		// This will not fail.
		do_sigaction(sig, NULL, &old_action);
		return old_action.sa_handler;
	}

	new_action.sa_handler = handler;
	new_action.sa_mask = 1u << sig; // Always add the signal to the mask.
	new_action.sa_flags = 0;

	status = do_sigaction(sig, &new_action, &old_action);

	if (status != 0)
	{
		return SIG_ERR;
	}

	return old_action.sa_handler;

}
