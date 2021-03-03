/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <signal-ext.h>
#include <signal_internal.h>
#include <errno.h>

#undef signal
static _crt_signal_t wlibc_signal_internal(int sig, _crt_signal_t handler)
{
	_crt_signal_t old_action = set_action(sig, handler);
	return old_action;
}

_crt_signal_t wlibc_signal(int sig, _crt_signal_t handler)
{
	switch (sig)
	{
	case SIGALRM:
	case SIGHUP:
	case SIGQUIT:
	case SIGTRAP:
	case SIGPIPE:
	case SIGCHLD:
	case SIGSTOP:
	case SIGCONT:
		return wlibc_signal_internal(sig, handler);
	case SIGINT:
	case SIGILL:
	case SIGFPE:
	case SIGSEGV:
	case SIGTERM:
	case SIGBREAK:
	case SIGABRT:
		return signal(sig, handler); // call msvcrt raise
	default:
		errno = EINVAL;
		return SIG_ERR;
	}
}
