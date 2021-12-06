/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <signal.h>
#include <internal/signal.h>
#include <stdbool.h>
#include <errno.h>

bool validate_signal(int sig)
{
	if (sig >= NSIG || sig == SIGKILL || sig == SIGSTOP)
	{
		errno = EINVAL;
		return false;
	}
	return true;
}

int wlibc_sigemptyset(sigset_t *set)
{
	if (!set)
	{
		errno = EFAULT;
		return -1;
	}

	*set = 0;
	return 0;
}

int wlibc_sigfillset(sigset_t *set)
{
	if (!set)
	{
		errno = EFAULT;
		return -1;
	}

	*set = ((2u << (NSIG - 1)) - 1);
	return 0;
}

int wlibc_sigaddset(sigset_t *set, int sig)
{
	if (!set)
	{
		errno = EFAULT;
		return -1;
	}

	if (!validate_signal(sig))
	{
		return -1;
	}
	*set |= 1u << sig;
	return 0;
}

int wlibc_sigdelset(sigset_t *set, int sig)
{
	if (!set)
	{
		errno = EFAULT;
		return -1;
	}

	if (!validate_signal(sig))
	{
		return -1;
	}
	*set &= ~(1u << sig);
	return 0;
}

int wlibc_sigismember(sigset_t *set, int sig)
{
	if (!set)
	{
		errno = EFAULT;
		return -1;
	}

	if (!validate_signal(sig))
	{
		return -1;
	}

	if (*set & (1u << sig))
	{
		return 1;
	}

	return 0;
}

int wlibc_sigpending(sigset_t *set)
{
	if (!set)
	{
		errno = EFAULT;
		return -1;
	}

	*set = get_pending_signals();
	return 0;
}

int wlibc_sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
	if (!set)
	{
		errno = EFAULT;
		return -1;
	}

	if (how < 0 || how > 2)
	{
		errno = EINVAL;
		return -1;
	}

	if (*set > ((2u << (NSIG - 1)) - 1) || *set & (1u << SIGKILL) || *set & (1u << SIGSTOP))
	{
		errno = EINVAL;
		return -1;
	}

	EnterCriticalSection(&_wlibc_signal_critical);

	if (oldset)
	{
		*oldset = _wlibc_blocked_signals;
	}

	switch (how)
	{
	case SIG_BLOCK:
		_wlibc_blocked_signals |= (*set);
		break;
	case SIG_UNBLOCK:
		_wlibc_blocked_signals &= ~(*set);
		break;
	case SIG_SETMASK:
		_wlibc_blocked_signals = (*set);
		break;
	}

	LeaveCriticalSection(&_wlibc_signal_critical);

	// Raise any pending signals
	sigset_t pending_signals = get_pending_signals();
	if (pending_signals)
	{
		for (int i = 1; i < NSIG - 1; i++) // Don't check for SIGABRT_COMPAT -> 22
		{
			if (pending_signals & (1u << i))
			{
				remove_pending_signals(i);
				raise(i);
			}
		}
	}

	return 0;
}
