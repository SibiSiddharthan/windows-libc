/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/signal.h>
#include <internal/thread.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>

int wlibc_raise(int sig)
{
	threadinfo *tinfo = (threadinfo *)TlsGetValue(_wlibc_threadinfo_index);
	sigset_t blocked_signals = tinfo->sigmask;
	siginfo sinfo;
	sigset_t oldmask = tinfo->sigmask;

	VALIDATE_SIGNAL(sig);

	// If the signal is blocked ignore it.
	if (blocked_signals & (1u << sig))
	{
		tinfo->pending |= (1u << sig);
		return 0;
	}

	get_siginfo(sig, &sinfo);

	// If the action for the specified signal is set to SIG_IGN just return.
	if (sinfo.action == SIG_IGN)
	{
		return 0;
	}

	// Block signals according to sa_mask given during setup of this signal.
	tinfo->sigmask |= sinfo.mask;

	// Block this signal as well if SA_NODEFER is not given.
	if ((sinfo.flags & SA_NODEFER) == 0)
	{
		tinfo->sigmask |= 1u << sig;
	}

	// Restore the default signal handler.
	if (sinfo.flags & SA_RESETHAND)
	{
		EXCLUSIVE_LOCK_SIGNAL_TABLE();
		_wlibc_signal_table[sig].action = SIG_DFL;
		EXCLUSIVE_UNLOCK_SIGNAL_TABLE();
	}

	if (sinfo.action == SIG_DFL)
	{
		switch (sig)
		{
		// Ignore
		case SIGCHLD:
		case SIGURG:
		case SIGWINCH:
			break;

		// Terminate
		case SIGHUP:
		case SIGINT:
		case SIGKILL:
		case SIGUSR1:
		case SIGUSR2:
		case SIGPIPE:
		case SIGALRM:
		case SIGTERM:
		case SIGSTKFLT:
		case SIGVTALRM:
		case SIGPROF:
		case SIGPOLL:
		case SIGPWR:
			NtTerminateProcess(NtCurrentProcess(), 128 + sig);
			break;

		// Core dump (Abort)
		case SIGQUIT:
		case SIGILL:
		case SIGTRAP:
		case SIGABRT:
		case SIGBUS:
		case SIGFPE:
		case SIGSEGV:
		case SIGXCPU:
		case SIGXFSZ:
		case SIGSYS:
			NtTerminateProcess(NtCurrentProcess(), 128 + sig);
			break;

		// Continue
		case SIGCONT:
			// Ignore this SIGCONT. This only makes sense if some other process is giving us this signal.
			break;

		// Stop
		case SIGTTIN:
		case SIGTTOU:
		case SIGTSTP:
		case SIGSTOP:
			NtSuspendProcess(NtCurrentProcess());
			break;
		}
	}
	else
	{
		// Execute the action.
		sinfo.action(sig);
	}

	// Reset the signal mask of the thread.
	tinfo->sigmask = oldmask;

	return 0;
}
