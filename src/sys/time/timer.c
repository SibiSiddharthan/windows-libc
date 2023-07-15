/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/thread.h>
#include <internal/timer.h>
#include <internal/validate.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <thread.h>

#define VALIDATE_TIMER(timer) VALIDATE_PTR(timer, EINVAL, -1)

#define VALIDATE_CLOCK(id)                                 \
	{                                                      \
		if (id != CLOCK_REALTIME && id != CLOCK_MONOTONIC) \
		{                                                  \
			errno = EINVAL;                                \
			return -1;                                     \
		}                                                  \
	}

int wlibc_timer_create(clockid_t id, struct sigevent *restrict event, timer_t *restrict timer)
{
	NTSTATUS status;
	timerinfo *tinfo;
	thread_attr_t *attributes = NULL;

	VALIDATE_CLOCK(id);
	VALIDATE_PTR(event, EINVAL, -1);
	VALIDATE_PTR(timer, EINVAL, -1);

	if (event->sigev_notify < SIGEV_NONE || event->sigev_notify > SIGEV_THREAD)
	{
		errno = EINVAL;
		return -1;
	}

	*timer = RtlAllocateHeap(NtCurrentProcessHeap(), HEAP_ZERO_MEMORY, sizeof(timerinfo));
	tinfo = (timerinfo *)*timer;

	if (*timer == NULL)
	{
		errno = ENOMEM;
		return -1;
	}

	status = NtCreateTimer2(&(tinfo->handle), NULL, NULL, TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		goto fail;
	}

	// If the notification method is SIGEV_THREAD, make use of the passed in thread attributes.
	if (event->sigev_notify == SIGEV_THREAD)
	{
		attributes = event->sigev_notify_attributes;
	}

	status = (NTSTATUS)wlibc_thread_create(&(tinfo->thread), attributes, timer_proc, tinfo);
	if (status != 0)
	{
		goto fail;
	}

	memcpy(&(tinfo->event), event, sizeof(struct sigevent));

	return 0;

fail:
	RtlFreeHeap(NtCurrentProcessHeap(), 0, *timer);
	return -1;
}

int wlibc_timer_delete(timer_t timer)
{
	NTSTATUS status;
	threadinfo *thread;
	timerinfo *tinfo = (timerinfo *)timer;

	VALIDATE_TIMER(timer);
	thread = (threadinfo *)tinfo->thread;

	status = NtClose(tinfo->handle);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	// Kill the timer thread. Just use NtTerminateThread directly.
	status = NtTerminateThread(thread->handle, (NTSTATUS)(LONG_PTR)WLIBC_THREAD_CANCELED);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	RtlFreeHeap(NtCurrentProcessHeap(), 0, thread);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, timer);

	return 0;
}

int wlibc_timer_getoverrun(timer_t timer)
{
	// Just validate and return 0.
	VALIDATE_TIMER(timer);

	return 0;
}

int wlibc_timer_gettime(timer_t timer, struct itimerspec *current_value)
{
	timerinfo *tinfo = (timerinfo *)timer;

	VALIDATE_TIMER(timer);

	// Just fill in the period.
	current_value->it_interval.tv_sec = tinfo->period / 10000000;
	current_value->it_interval.tv_nsec = (tinfo->period % 10000000) * 100;
	current_value->it_value.tv_sec = 0;
	current_value->it_value.tv_nsec = 0;

	return 0;
}

int wlibc_timer_settime(timer_t timer, int flags, const struct itimerspec *restrict new_value, struct itimerspec *restrict old_value)
{
	NTSTATUS status;
	LARGE_INTEGER period, due;
	T2_SET_PARAMETERS parameters = {0, 0, 0};
	timerinfo *tinfo = (timerinfo *)timer;

	VALIDATE_TIMER(timer);

	if (flags != 0 && flags != TIMER_ABSTIME)
	{
		errno = EINVAL;
		return -1;
	}

	if (old_value != NULL)
	{
		// Just fill in the period.
		old_value->it_interval.tv_sec = tinfo->period / 10000000;
		old_value->it_interval.tv_nsec = (tinfo->period % 10000000) * 100;
		old_value->it_value.tv_sec = 0;
		old_value->it_value.tv_nsec = 0;
	}

	if (new_value->it_value.tv_sec == 0 && new_value->it_value.tv_nsec == 0)
	{
		// Cancel the timer
		status = NtCancelTimer2(tinfo->handle, NULL);

		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}

		// Zero the period.
		tinfo->period = 0;
	}
	else
	{
		period.QuadPart = new_value->it_interval.tv_sec * 10000000 + new_value->it_interval.tv_nsec / 100;
		due.QuadPart = new_value->it_value.tv_sec * 10000000 + new_value->it_value.tv_nsec / 100;

		if (flags != TIMER_ABSTIME)
		{
			due.QuadPart = -1 * due.QuadPart; // relative time.
		}

		status = NtSetTimer2(tinfo->handle, &due, &period, &parameters);

		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}

		tinfo->period = period.QuadPart;
	}

	return 0;
}
