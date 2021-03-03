/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <signal_internal.h>

CRITICAL_SECTION _wlibc_signal_critical;
_crt_signal_t _wlibc_signal_table[NSIG];
sigset_t _wlibc_blocked_signals = 0;
sigset_t _wlibc_pending_signals = 0;

void signal_init()
{
	InitializeCriticalSection(&_wlibc_signal_critical);

	// Initialize our default signal handlers, rest will be handled by msvcrt
	_wlibc_signal_table[SIGHUP] = SIG_DFL;
	_wlibc_signal_table[SIGQUIT] = SIG_DFL;
	_wlibc_signal_table[SIGTRAP] = SIG_DFL;
	_wlibc_signal_table[SIGBUS] = SIG_DFL;
	_wlibc_signal_table[SIGKILL] = SIG_DFL;
	_wlibc_signal_table[SIGUSR1] = SIG_DFL;
	_wlibc_signal_table[SIGUSR2] = SIG_DFL;
	_wlibc_signal_table[SIGPIPE] = SIG_DFL;
	_wlibc_signal_table[SIGALRM] = SIG_DFL;
	_wlibc_signal_table[SIGSTKFLT] = SIG_IGN;
	_wlibc_signal_table[SIGCHLD] = SIG_IGN;
	_wlibc_signal_table[SIGCONT] = SIG_IGN;
	_wlibc_signal_table[SIGSTOP] = SIG_DFL;
	_wlibc_signal_table[SIGTSTP] = SIG_IGN;
}

void signal_cleanup()
{
	DeleteCriticalSection(&_wlibc_signal_critical);
}

_crt_signal_t get_action(int signum)
{
	_crt_signal_t old_action;
	EnterCriticalSection(&_wlibc_signal_critical);
	old_action = _wlibc_signal_table[signum];
	LeaveCriticalSection(&_wlibc_signal_critical);
	return old_action;
}

_crt_signal_t set_action(int signum, _crt_signal_t action)
{
	_crt_signal_t old_action;
	EnterCriticalSection(&_wlibc_signal_critical);
	old_action = _wlibc_signal_table[signum];
	_wlibc_signal_table[signum] = action;
	LeaveCriticalSection(&_wlibc_signal_critical);
	return old_action;
}

void add_pending_signals(int sig)
{
	EnterCriticalSection(&_wlibc_signal_critical);
	_wlibc_pending_signals |= 1u << sig;
	LeaveCriticalSection(&_wlibc_signal_critical);
}

sigset_t get_pending_signals()
{
	sigset_t result;
	EnterCriticalSection(&_wlibc_signal_critical);
	result = _wlibc_pending_signals;
	LeaveCriticalSection(&_wlibc_signal_critical);
	return result;
}
