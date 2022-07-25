/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/signal.h>

CRITICAL_SECTION _wlibc_signal_critical;
siginfo _wlibc_signal_table[NSIG];

void signal_init(void)
{
	RtlInitializeCriticalSection(&_wlibc_signal_critical);
	memset(_wlibc_signal_table, 0, NSIG * sizeof(siginfo));
}

void signal_cleanup(void)
{
	RtlDeleteCriticalSection(&_wlibc_signal_critical);
}

void get_siginfo(int sig, siginfo *sinfo)
{
	LOCK_SIGNAL_TABLE();
	memcpy(sinfo, &_wlibc_signal_table[sig], sizeof(siginfo));
	UNLOCK_SIGNAL_TABLE();
}

void set_siginfo(int sig, const siginfo *sinfo)
{
	LOCK_SIGNAL_TABLE();
	memcpy(&_wlibc_signal_table[sig], sinfo, sizeof(siginfo));
	UNLOCK_SIGNAL_TABLE();
}
