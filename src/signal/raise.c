/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <signal-ext.h>
#include <signal_internal.h>
#include <errno.h>

#undef raise

static int wlibc_raise_SIGALRM()
{
	// TODO
	return 0;
}

static int wlibc_raise_internal(int sig)
{
	_crt_signal_t action = get_action(sig);
	if (action == SIG_DFL)
	{
		return raise(SIGABRT);
	}
	else if (action == SIG_IGN || action == SIG_ACK)
	{
		return 0;
	}
	else
	{
		// Execute the action
		(*action)(sig);
		return 0;
	}
}

int wlibc_raise(int sig)
{
	sigset_t blocked_signals;
	EnterCriticalSection(&_wlibc_signal_critical);
	blocked_signals = _wlibc_blocked_signals;
	LeaveCriticalSection(&_wlibc_signal_critical);
	if (blocked_signals & 1u << sig)
	{
		add_pending_signals(sig);
		return 0;
	}

	switch (sig)
	{
	case SIGALRM:
		return wlibc_raise_SIGALRM();
	case SIGHUP:
	case SIGQUIT:
	case SIGTRAP:
	case SIGUSR1:
	case SIGUSR2:
	case SIGPIPE:
	case SIGSTKFLT:
	case SIGCHLD:
	case SIGCONT:
	case SIGTSTP:
		return wlibc_raise_internal(sig);
	case SIGBUS:
		return raise(SIGSEGV);
	case SIGINT:
	case SIGILL:
	case SIGFPE:
	case SIGSEGV:
	case SIGTERM:
	case SIGBREAK:
	case SIGABRT:
		return raise(sig); // call msvcrt raise
	// Always abort
	case SIGSTOP:
	case SIGKILL:
		return raise(SIGABRT);
	default:
		errno = EINVAL;
		return -1;
	}
}
