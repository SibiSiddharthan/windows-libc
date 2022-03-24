/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/thread.h>
#include <internal/validate.h>
#include <errno.h>
#include <stdlib.h>
#include <thread.h>

#define VALIDATE_THREAD_ATTR(thread_attr) VALIDATE_PTR(thread_attr, EINVAL, -1)

DWORD wlibc_thread_entry(void *arg)
{
	threadinfo *tinfo = (threadinfo *)arg;
	void *result;

	TlsSetValue(_wlibc_threadinfo_index, (void *)tinfo);

	result = tinfo->routine(tinfo->args);
	tinfo->result = result;

	// Cleanup
	execute_cleanup(tinfo);
	cleanup_tls(tinfo);
	RtlExitUserThread((NTSTATUS)(LONG_PTR)result);
	// Unreachable
	return 0;
}

int wlibc_thread_create(thread_t *thread, thread_attr_t *attributes, thread_start_t routine, void *arg)
{
	DWORD thread_id;
	HANDLE thread_handle;
	SIZE_T stacksize = 0;
	BOOLEAN should_detach = FALSE;
	threadinfo *tinfo;

	if (thread == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	if (attributes != NULL)
	{
		stacksize = attributes->stacksize;
		if (attributes->state == WLIBC_THREAD_DETACHED)
		{
			should_detach = TRUE;
		}
	}

	*thread = malloc(sizeof(threadinfo));
	tinfo = (threadinfo *)*thread;
	memset(tinfo, 0, sizeof(threadinfo));

	tinfo->routine = routine;
	tinfo->args = arg;

	thread_handle =
		CreateRemoteThreadEx(NtCurrentProcess(), NULL, stacksize, wlibc_thread_entry, (void *)tinfo, CREATE_SUSPENDED, NULL, &thread_id);
	if (thread_handle == NULL)
	{
		map_doserror_to_errno(GetLastError());
		return -1;
	}

	tinfo->handle = thread_handle;
	tinfo->id = thread_id;

	NtResumeThread(thread_handle, NULL);

	if (should_detach)
	{
		NtClose(thread_handle);
		tinfo->handle = 0;
	}

	return 0;
}

// Detaching a thread doesn't really work in Windows, as we can always open
// a handle to it with the thread id. Anyway just close the handle.
int wlibc_thread_detach(thread_t thread)
{
	NTSTATUS status;
	threadinfo *tinfo = (threadinfo *)thread;

	if (tinfo->handle != 0)
	{
		status = NtClose(tinfo->handle);
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}

		tinfo->handle = 0;
	}
	else
	{
		// Already detached thread.
		errno = EINVAL;
		return -1;
	}

	return 0;
}

int wlibc_common_thread_join(thread_t thread, void **result, const struct timespec *abstime)
{
	NTSTATUS status;
	LARGE_INTEGER timeout;
	threadinfo *tinfo = (threadinfo *)thread;

	timeout.QuadPart = 0;
	// If abstime is null                    -> infinite wait.
	// If abstime is 0(tv_sec, tv_nsec is 0) -> try wait.
	if (abstime != NULL && abstime->tv_sec != 0 && abstime->tv_nsec != 0)
	{
		// From utimens.c. TODO
		timeout.QuadPart = abstime->tv_sec * 10000000 + abstime->tv_nsec / 100;
		timeout.QuadPart += 116444736000000000LL;
	}

	status = NtWaitForSingleObject(tinfo->handle, FALSE, abstime == NULL ? NULL : &timeout);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		// If we are doing a try join errno should be set to EBUSY.
		if (timeout.QuadPart == 0 && errno == ETIMEDOUT)
		{
			errno = EBUSY;
		}
		return -1;
	}

	status = NtClose(tinfo->handle);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	if (result != NULL)
	{
		*result = tinfo->result;
	}

	// Finally free the thread structure.
	free(thread);

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
	VALIDATE_PTR(abstime, EINVAL, -1);
	return wlibc_common_thread_join(thread, result, abstime);
}

int wlibc_thread_equal(thread_t thread_a, thread_t thread_b)
{
	return ((threadinfo *)thread_a)->id == ((threadinfo *)thread_b)->id;
}

thread_t wlibc_thread_self(void)
{
	return (thread_t)TlsGetValue(_wlibc_threadinfo_index);
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

void wlibc_thread_exit(void *retval)
{
	threadinfo *tinfo = TlsGetValue(_wlibc_threadinfo_index);

	// We use this field to determine the result. The parameter for `RtlExitUserThread` is just for backup.
	tinfo->result = retval;

	// Cleanup
	execute_cleanup(tinfo);
	cleanup_tls(tinfo);

	// The exit code of thread will be truncated to 32bits.
	RtlExitUserThread((NTSTATUS)(LONG_PTR)retval);
}

int wlibc_thread_setcancelstate(int state, int *oldstate)
{
	if (state != WLIBC_THREAD_CANCEL_ENABLE && state != WLIBC_THREAD_CANCEL_DISABLE)
	{
		errno = EINVAL;
		return -1;
	}

	threadinfo *tinfo = TlsGetValue(_wlibc_threadinfo_index);

	if (oldstate)
	{
		*oldstate = tinfo->cancelstate;
	}

	tinfo->cancelstate = state;

	return 0;
}

int wlibc_thread_setcanceltype(int type, int *oldtype)
{
	if (type != WLIBC_THREAD_CANCEL_ASYNCHRONOUS && type != WLIBC_THREAD_CANCEL_DEFERRED)
	{
		errno = EINVAL;
		return -1;
	}

	threadinfo *tinfo = TlsGetValue(_wlibc_threadinfo_index);

	if (oldtype)
	{
		*oldtype = tinfo->canceltype;
	}

	tinfo->cancelstate = type;

	return 0;
}

#if 0
static void execute_thread_cancellation(threadinfo *tinfo)
{
	if (tinfo->cancelstate == WLIBC_THREAD_CANCEL_DISABLE)
	{
		// Cancellations for this thread are disabled, just return.
		return;
	}

	execute_cleanup(tinfo);
	cleanup_tls(tinfo);
	tinfo->result = WLIBC_THREAD_CANCELED;
	RtlExitUserThread((NTSTATUS)(LONG_PTR)WLIBC_THREAD_CANCELED);
}

static void cancel_apc(void *arg1, void *arg2, void *arg3)
{
	execute_thread_cancellation(arg1);
}
#endif

WLIBC_API int wlibc_thread_cancel(thread_t thread)
{
	threadinfo *tinfo = (threadinfo *)thread;

	if (tinfo->cancelstate == WLIBC_THREAD_CANCEL_DISABLE)
	{
		// Cancellations for this thread are disabled.
		return -1;
	}

	tinfo->result = WLIBC_THREAD_CANCELED;
	// TODO The thread is not cleaned up when cancelled.
	// In Windows 11 there is NtQueueApcThreadEx2 where we can presumably queue special APCs
	// that prempt execution.
	NtTerminateThread(tinfo->handle, (NTSTATUS)(LONG_PTR)WLIBC_THREAD_CANCELED);

	return 0;
}

WLIBC_API void wlibc_thread_testcancel(void)
{
	threadinfo *tinfo = TlsGetValue(_wlibc_threadinfo_index);

	if (tinfo->cancelstate == WLIBC_THREAD_CANCEL_DISABLE)
	{
		// Cancellations for this thread are disabled, just return.
		return;
	}

	execute_cleanup(tinfo);
	cleanup_tls(tinfo);
	tinfo->result = WLIBC_THREAD_CANCELED;
	RtlExitUserThread((NTSTATUS)(LONG_PTR)WLIBC_THREAD_CANCELED);
}

void wlibc_thread_cleanup_push(cleanup_t routine, void *arg)
{
	threadinfo *tinfo = TlsGetValue(_wlibc_threadinfo_index);

	if (tinfo->cleanup_slots_allocated == 0)
	{
		tinfo->cleanup_entries = (cleanup_entry *)malloc(sizeof(cleanup_entry) * 8);
		tinfo->cleanup_slots_allocated = 8;
	}

	// Double the cleanup routine list
	if (tinfo->cleanup_slots_allocated == tinfo->cleanup_slots_used)
	{
		cleanup_entry *temp = (cleanup_entry *)malloc(sizeof(cleanup_entry) * tinfo->cleanup_slots_allocated * 2);
		memcpy(temp, tinfo->cleanup_entries, sizeof(cleanup_entry) * tinfo->cleanup_slots_allocated);
		free(tinfo->cleanup_entries);
		tinfo->cleanup_entries = temp;
		tinfo->cleanup_slots_allocated *= 2;
	}

	tinfo->cleanup_entries[tinfo->cleanup_slots_used].routine = routine;
	tinfo->cleanup_entries[tinfo->cleanup_slots_used].arg = arg;
	++tinfo->cleanup_slots_used;
}

void wlibc_thread_cleanup_pop(int execute)
{
	threadinfo *tinfo = TlsGetValue(_wlibc_threadinfo_index);

	if (tinfo->cleanup_slots_used == 0)
	{
		// No cleanup functions registered or all of them have been executed.
		return;
	}

	if (execute)
	{
		// Only execute the cleanup function if it is non NULL.
		if (tinfo->cleanup_entries[tinfo->cleanup_slots_used - 1].routine != NULL)
		{
			tinfo->cleanup_entries[tinfo->cleanup_slots_used - 1].routine(tinfo->cleanup_entries[tinfo->cleanup_slots_used - 1].arg);
		}
	}

	--tinfo->cleanup_slots_used;
}

int wlibc_threadattr_init(thread_attr_t *attributes)
{
	attributes->stacksize = 0;
	attributes->state = WLIBC_THREAD_JOINABLE;
	return 0;
}

int wlibc_threadattr_getdetachstate(const thread_attr_t *attributes, int *detachstate)
{
	VALIDATE_THREAD_ATTR(attributes);
	VALIDATE_PTR(detachstate, EINVAL, -1);
	*detachstate = attributes->state;
	return 0;
}

int wlibc_threadattr_setdetachstate(thread_attr_t *attributes, int detachstate)
{
	VALIDATE_THREAD_ATTR(attributes);
	if (detachstate != WLIBC_THREAD_JOINABLE && detachstate != WLIBC_THREAD_DETACHED)
	{
		errno = EINVAL;
		return -1;
	}

	attributes->state = detachstate;
	return 0;
}

int wlibc_threadattr_getstacksize(const thread_attr_t *restrict attributes, size_t *restrict stacksize)
{
	VALIDATE_THREAD_ATTR(attributes);
	VALIDATE_PTR(stacksize, EINVAL, -1);
	*stacksize = attributes->stacksize;
	return 0;
}

int wlibc_threadattr_setstacksize(thread_attr_t *attributes, size_t stacksize)
{
	VALIDATE_THREAD_ATTR(attributes);
	attributes->stacksize = stacksize;
	return 0;
}
