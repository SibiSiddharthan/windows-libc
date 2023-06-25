/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/timer.h>
#include <internal/thread.h>
#include <thread.h>
#include <signal.h>
#include <stdbool.h>

timerinfo real_itimer;
timerinfo virtual_itimer;
timerinfo prof_itimer;
thread_t itimer_thread;

void *itimer_proc(void *arg);
void *timer_proc(void *arg);

void initialize_itimers(void)
{
	// Real timer
	NtCreateTimer2(&(real_itimer.handle), NULL, NULL, TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
	real_itimer.period = 0;

	// Virtual timer
	NtCreateTimer2(&(virtual_itimer.handle), NULL, NULL, TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
	virtual_itimer.period = 0;

	// Profile timer
	NtCreateTimer2(&(prof_itimer.handle), NULL, NULL, TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
	prof_itimer.period = 0;

	// Create the timer thread.
	wlibc_thread_create(&itimer_thread, NULL, itimer_proc, NULL);
}

void cleanup_itimers(void)
{
	// First kill the timer thread.
	//NtTerminateThread(((threadinfo *)itimer_thread)->handle, 0);

	// Next close the timers.
	NtClose(real_itimer.handle);
	NtClose(virtual_itimer.handle);
	NtClose(prof_itimer.handle);
	
}

void *itimer_proc(void *arg)
{
	NTSTATUS status;
	HANDLE handles[] = {real_itimer.handle, virtual_itimer.handle, prof_itimer.handle};

	UNREFERENCED_PARAMETER(arg);

	while (1)
	{
		status = NtWaitForMultipleObjects(3, handles, WaitAny, TRUE, NULL);

		switch (status)
		{
		case STATUS_WAIT_0: // real timer
			wlibc_raise(SIGALRM);
			break;
		case STATUS_WAIT_1: // virtual timer
			wlibc_raise(SIGVTALRM);
			break;
		case STATUS_WAIT_2: // prof timer
			wlibc_raise(SIGPROF);
			break;
		default:
			break;
		}
	}
}
