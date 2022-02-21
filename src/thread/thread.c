/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <errno.h>
#include <thread.h>

int wlibc_thread_create(thread_t *thread, thread_attr_t *attributes, thread_start_t routine, void *arg)
{
	DWORD thread_id;
	HANDLE thread_handle;

	if (thread == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	thread_handle =
		CreateRemoteThreadEx(NtCurrentProcess(), NULL, 0, (LPTHREAD_START_ROUTINE)routine, arg, CREATE_SUSPENDED, NULL, &thread_id);
	if (thread_handle == NULL)
	{
		map_doserror_to_errno(GetLastError());
		return -1;
	}

	thread->handle = thread_handle;
	thread->id = thread_id;

	NtResumeThread(thread_handle, NULL);

	return 0;
}

// Detaching a thread doesn't really work in Windows, as we can always open
// a handle to it with the thread id. Anyway just close the handle.
int wlibc_thread_detach(thread_t thread)
{
	NTSTATUS status;

	status = NtClose(thread.handle);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}

int wlibc_common_thread_join(thread_t thread, void **result, const struct timespec *abstime)
{
	NTSTATUS status;
	LARGE_INTEGER timeout;
	THREAD_BASIC_INFORMATION basic_info;

	timeout.QuadPart = 0;
	// If abstime is null                    -> infinite wait.
	// If abstime is 0(tv_sec, tv_nsec is 0) -> try wait.
	if (abstime != NULL && abstime->tv_sec != 0 && abstime->tv_nsec != 0)
	{
		// From utimens.c. TODO
		timeout.QuadPart = abstime->tv_sec * 10000000 + abstime->tv_nsec / 100;
		timeout.QuadPart += 116444736000000000LL;
	}

	status = NtWaitForSingleObject(thread.handle, FALSE, abstime == NULL ? NULL : &timeout);
	if (status != STATUS_SUCCESS && status != STATUS_TIMEOUT)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	if (status == STATUS_TIMEOUT)
	{
		errno = EBUSY;
		return -1;
	}

	status = NtQueryInformationThread(thread.handle, ThreadBasicInformation, &basic_info, sizeof(basic_info), NULL);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	status = NtClose(thread.handle);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	if (result != NULL)
	{
		*result = (void *)(intptr_t)basic_info.ExitStatus;
	}

	return 0;
}

int wlibc_thread_join(thread_t thread, void **result)
{
	return wlibc_common_thread_join(thread, result, NULL);
}

int wlibc_thread_tryjoin(thread_t thread, void **result)
{
	struct timespec timeout = {0, 0};
	return wlibc_common_thread_join(thread, result, &timeout);
}

int wlibc_thread_timedjoin(thread_t thread, void **result, const struct timespec *abstime)
{
	return wlibc_common_thread_join(thread, result, abstime);
}

int wlibc_thread_equal(thread_t thread_a, thread_t thread_b)
{
	return thread_a.id == thread_b.id;
}

thread_t wlibc_thread_self(void)
{
	thread_t thread;

	thread.handle = NtCurrentThread();
	thread.id = GetCurrentThreadId();

	return thread;
}

int wlibc_thread_sleep(const struct timespec *duration, struct timespec *remaining)
{
	NTSTATUS status;
	LARGE_INTEGER interval;

	if (duration == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	// Measure in 100 nanosecond intervals.
	// Negative means relative measurement.
	interval.QuadPart = -1 * (duration->tv_sec * 10000000 + duration->tv_nsec / 100);

	status = NtDelayExecution(TRUE, &interval);
	if (status != STATUS_SUCCESS)
	{
		// TODO check alerted case
		map_ntstatus_to_errno(status);
		return -1;
	}

	// TODO Alerts
	return 0;
}

int wlibc_thread_yield(void)
{
	NtYieldExecution();
	return 0;
}
