/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/signal.h>
#include <thread.h>

RTL_CRITICAL_SECTION _wlibc_signal_critical;
siginfo _wlibc_signal_table[NSIG];

static void console_raise(int sig)
{
	siginfo sinfo;
	thread_t thread;

	get_siginfo(sig, &sinfo);

	if (sinfo.action == SIG_IGN)
	{
		return;
	}

	if (sinfo.action == SIG_DFL)
	{
		NtTerminateProcess(NtCurrentProcess(), 128 + sig);
		return;
	}

	// The console handler routines are executed by a separate thread.
	// In a static build we have no way of intializng the threads TLS structure.
	// If the user has given a specific funtions for these signals, create a thread and execute them.
	// This way we can initialize the thread's TLS structure appropriately.
	if (wlibc_thread_create(&thread, NULL, (thread_start_t)sinfo.action, (void *)(intptr_t)sig) == 0)
	{
		wlibc_thread_join(thread, NULL);
	}
}

static BOOL console_handler(DWORD ctrl)
{
	if (ctrl == CTRL_C_EVENT)
	{
		console_raise(SIGINT);
		return TRUE;
	}
	else if (ctrl == CTRL_BREAK_EVENT)
	{
		console_raise(SIGBREAK);
		return TRUE;
	}

	return FALSE;
}

void signal_init(void)
{
	RtlInitializeCriticalSection(&_wlibc_signal_critical);
	memset(_wlibc_signal_table, 0, NSIG * sizeof(siginfo));

	// Initialize the console control signal handler.
	SetConsoleCtrlHandler(console_handler, TRUE);
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
