/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_TIMER_INTERNAL_H
#define WLIBC_TIMER_INTERNAL_H

#include <internal/nt.h>
#include <signal.h>
#include <thread.h>
#include <stdbool.h>

typedef struct _timerinfo
{
	HANDLE handle;
	LONGLONG period;
	thread_t thread;
	struct sigevent event;
} timerinfo;

extern timerinfo real_itimer;
extern timerinfo virtual_itimer;
extern timerinfo prof_itimer;
extern thread_t itimer_thread;

void initialize_itimers(void);
void cleanup_itimers(void);

void *itimer_proc(void *arg);
void *timer_proc(void *arg);

#endif
