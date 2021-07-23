/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <signal-ext.h>
#include <internal/signal.h>
#include <errno.h>
#include <stdbool.h>

#undef signal

// implemented is sigprocmask.c
bool validate_signal(int sig);

int wlibc_sigaction(int sig, const struct sigaction *new_action, struct sigaction *old_action)
{
	if (!new_action)
	{
		errno = EFAULT;
		return -1;
	}

	if (!validate_signal(sig))
	{
		return -1;
	}

	_crt_signal_t old_handler;

	old_handler = wlibc_signal(sig,new_action->sa_handler);
	
	if(old_handler == SIG_ERR)
	{
		return -1;
	}

	EnterCriticalSection(&_wlibc_signal_critical);

	if(old_action)
	{
		old_action->sa_flags = _wlibc_signal_flags[sig];
		old_action->sa_mask = _wlibc_signal_mask[sig];
		old_action->sa_handler = old_handler;
	}

	_wlibc_signal_mask[sig] = new_action->sa_mask;
	_wlibc_signal_flags[sig] = new_action->sa_flags;
	
	LeaveCriticalSection(&_wlibc_signal_critical);

	return 0;
}
