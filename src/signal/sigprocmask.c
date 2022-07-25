/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/signal.h>
#include <internal/thread.h>
#include <internal/validate.h>
#include <signal.h>
#include <errno.h>

int wlibc_sigemptyset(sigset_t *set)
{
	VALIDATE_SIGSET(set);

	*set = 0;
	return 0;
}

int wlibc_sigfillset(sigset_t *set)
{
	VALIDATE_SIGSET(set);

	*set = ((2u << (NSIG - 1)) - 1);
	return 0;
}

int wlibc_sigaddset(sigset_t *set, int sig)
{
	VALIDATE_SIGSET(set);
	VALIDATE_SIGNAL(sig);

	*set |= 1u << sig;
	return 0;
}

int wlibc_sigdelset(sigset_t *set, int sig)
{
	VALIDATE_SIGSET(set);
	VALIDATE_SIGNAL(sig);

	*set &= ~(1u << sig);
	return 0;
}

int wlibc_sigismember(sigset_t *set, int sig)
{
	VALIDATE_SIGSET(set);
	VALIDATE_SIGNAL(sig);

	if (*set & (1u << sig))
	{
		return 1;
	}

	return 0;
}

int wlibc_sigpending(sigset_t *set)
{
	threadinfo *tinfo = (threadinfo *)TlsGetValue(_wlibc_threadinfo_index);

	VALIDATE_SIGSET(set);
	*set = tinfo->pending;

	return 0;
}

int wlibc_sigprocmask(int how, const sigset_t *newset, sigset_t *oldset)
{
	threadinfo *tinfo = (threadinfo *)TlsGetValue(_wlibc_threadinfo_index);
	sigset_t unblocked_signals = tinfo->sigmask;
	sigset_t pending_signals = tinfo->pending;

	if (how != SIG_BLOCK && how != SIG_UNBLOCK && how != SIG_SETMASK)
	{
		errno = EINVAL;
		return -1;
	}

	if (oldset)
	{
		*oldset = tinfo->sigmask;
	}

	if (newset == NULL)
	{
		return 0;
	}

	if ((*newset > (sigset_t)((2u << (NSIG - 1)) - 1)) || (*newset & (1u << SIGKILL)) || (*newset & (1u << SIGSTOP)))
	{
		errno = EINVAL;
		return -1;
	}

	switch (how)
	{
	case SIG_BLOCK:
		tinfo->sigmask |= (*newset);
		break;
	case SIG_UNBLOCK:
		tinfo->sigmask &= ~(*newset);
		break;
	case SIG_SETMASK:
		tinfo->sigmask = (*newset);
		break;
	}

	// Raise any unblocked signals.
	unblocked_signals &= ~tinfo->sigmask;
	pending_signals &= unblocked_signals;

	if (pending_signals)
	{
		for (int i = 1; i < NSIG - 1; i++)
		{
			if (pending_signals & (1u << i))
			{
				tinfo->pending &= ~(1u << i);
				wlibc_raise(i);
			}
		}
	}

	return 0;
}
