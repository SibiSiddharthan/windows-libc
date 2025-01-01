/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/timer.h>
#include <internal/validate.h>
#include <sys/stat.h>
#include <sys/time.h>

int wlibc_getitimer(int which, struct itimerval *current_value)
{
	timerinfo *tinfo = NULL;

	VALIDATE_PTR(current_value, EINVAL, -1);

	if (which < ITIMER_REAL || which > ITIMER_PROF)
	{
		errno = EINVAL;
		return -1;
	}

	switch (which)
	{
	case ITIMER_REAL:
		tinfo = &real_itimer;
		break;
	case ITIMER_VIRTUAL:
		tinfo = &virtual_itimer;
		break;
	case ITIMER_PROF:
		tinfo = &prof_itimer;
		break;

	}

	// Just fill in the period.
	current_value->it_interval.tv_sec = tinfo->period / 10000000;
	current_value->it_interval.tv_usec = (tinfo->period % 10000000) / 10;
	current_value->it_value.tv_sec = 0;
	current_value->it_value.tv_usec = 0;

	return 0;
}

int wlibc_setitimer(int which, const struct itimerval *restrict new_value, struct itimerval *restrict old_value)
{
	NTSTATUS status;
	LARGE_INTEGER period, due;
	T2_SET_PARAMETERS parameters = {0, 0, 0};
	timerinfo *tinfo = NULL;

	VALIDATE_PTR(new_value, EINVAL, -1);

	if (which < ITIMER_REAL || which > ITIMER_PROF)
	{
		errno = EINVAL;
		return -1;
	}

	switch (which)
	{
	case ITIMER_REAL:
		tinfo = &real_itimer;
		break;
	case ITIMER_VIRTUAL:
		tinfo = &virtual_itimer;
		break;
	case ITIMER_PROF:
		tinfo = &prof_itimer;
		break;
	}

	if (old_value != NULL)
	{
		// Just fill in the period.
		old_value->it_interval.tv_sec = tinfo->period / 10000000;
		old_value->it_interval.tv_usec = (tinfo->period % 10000000) / 10;
		old_value->it_value.tv_sec = 0;
		old_value->it_value.tv_usec = 0;
	}

	if (new_value->it_value.tv_sec == 0 && new_value->it_value.tv_usec == 0)
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
		period.QuadPart = new_value->it_interval.tv_sec * 10000000 + new_value->it_interval.tv_usec * 10;
		due.QuadPart = new_value->it_value.tv_sec * 10000000 + new_value->it_value.tv_usec * 10;
		due.QuadPart = -1 * due.QuadPart; // relative time.

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
