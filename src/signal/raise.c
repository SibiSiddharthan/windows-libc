/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/signal.h>
#include <internal/thread.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>

#undef raise
// MSVC raise
extern int __cdecl raise(int sig);

static void reset_handler(int sig)
{
	EnterCriticalSection(&_wlibc_signal_critical);
	switch (sig)
	{
	case SIGHUP:
	case SIGQUIT:
	case SIGTRAP:
	case SIGBUS:
	case SIGKILL:
	case SIGUSR1:
	case SIGUSR2:
	case SIGPIPE:
	case SIGALRM:
	case SIGSTOP:
		_wlibc_signal_table[sig] = SIG_DFL;
		break;
	case SIGSTKFLT:
	case SIGCHLD:
	case SIGCONT:
	case SIGTSTP:
		_wlibc_signal_table[sig] = SIG_IGN;
		break;
	}
	LeaveCriticalSection(&_wlibc_signal_critical);
}

static int wlibc_raise_SIGALRM()
{
	// TODO
	return 0;
}

static int wlibc_raise_internal(int sig)
{
	threadinfo *tinfo = (threadinfo *)TlsGetValue(_wlibc_threadinfo_index);
	_crt_signal_t action = get_action(sig);

	if (action == SIG_DFL)
	{
		_exit(3);
	}
	else if (action == SIG_IGN || action == SIG_ACK)
	{
		return 0;
	}
	else
	{
		EnterCriticalSection(&_wlibc_signal_critical);

		// Block signals according to sa_mask given during setup of this signal.
		tinfo->sigmask |= _wlibc_signal_mask[sig];
		// Block this signal next time if SA_NODEFER is not given.
		if ((_wlibc_signal_flags[sig] & SA_NODEFER) == 0)
		{
			tinfo->sigmask |= 1u << sig;
		}

		// Restore the default signal handler
		if (_wlibc_signal_flags[sig] & SA_RESETHAND)
		{
			reset_handler(sig);
		}
		LeaveCriticalSection(&_wlibc_signal_critical);

		// Execute the action
		(*action)(sig);

		return 0;
	}
}

int wlibc_raise(int sig)
{
	threadinfo *tinfo = (threadinfo *)TlsGetValue(_wlibc_threadinfo_index);
	sigset_t blocked_signals = tinfo->sigmask;

	if (blocked_signals & (1u << sig))
	{
		tinfo->pending |= (1u << sig);
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
	// Always exit the process
	case SIGSTOP:
	case SIGKILL:
		_exit(3);
	default:
		errno = EINVAL;
		return -1;
	}
}
