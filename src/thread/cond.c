/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/convert.h>
#include <internal/error.h>
#include <internal/validate.h>
#include <errno.h>
#include <intrin.h>
#include <thread.h>

#define VALIDATE_COND(cond)        \
	VALIDATE_PTR(cond, EINVAL, -1) \
	if (cond->ptr == NULL)         \
	{                              \
		errno = EINVAL;            \
		return -1;                 \
	}

#define VALIDATE_MUTEX(mutex)       \
	VALIDATE_PTR(mutex, EINVAL, -1) \
	if (mutex->handle == 0)         \
	{                               \
		errno = EINVAL;             \
		return -1;                  \
	}

#define VALIDATE_COND_ATTR(cond_attr) VALIDATE_PTR(cond_attr, EINVAL, -1)

// Spin till we get a lock on the queue.
#define LOCK_QUEUE(lock)                                                   \
	while (_InterlockedCompareExchange((volatile LONG *)&lock, 1, 0) != 0) \
	{                                                                      \
		_mm_pause();                                                       \
	}

#define UNLOCK_QUEUE(lock) _InterlockedCompareExchange((volatile LONG *)&lock, 0, 1);

int wlibc_cond_init(cond_t *restrict cond, const cond_attr_t *restrict attributes)
{
	VALIDATE_PTR(cond, EINVAL, -1);
	UNREFERENCED_PARAMETER(attributes);

	// start with a size of 64.
	cond->queue_size = 64;
	cond->queue_begin = 0;
	cond->queue_end = 0;
	cond->waiting_threads = 0;
	cond->lock = 0;

	// Create an array to store 64 thread ids.
	cond->ptr = (DWORD *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, 64 * sizeof(DWORD));
	if (cond->ptr == NULL)
	{
		errno = ENOMEM;
		return -1;
	}

	return 0;
}

int wlibc_cond_destroy(cond_t *restrict cond)
{
	VALIDATE_COND(cond);

	RtlFreeHeap(NtCurrentProcessHeap(), 0, cond->ptr);
	cond->ptr = NULL;
	cond->queue_size = 0;
	cond->queue_begin = 0;
	cond->queue_end = 0;
	cond->waiting_threads = 0;

	return 0;
}

int wlibc_cond_signal(cond_t *cond)
{
	VALIDATE_COND(cond);

	NTSTATUS status;
	DWORD thread_id = 0;

	LOCK_QUEUE(cond->lock);

	// Queue is now ours.
	if (cond->waiting_threads > 0)
	{
		thread_id = cond->ptr[cond->queue_begin];
		cond->ptr[cond->queue_begin] = 0;

		if (cond->queue_begin + 1 == cond->queue_end)
		{
			// There was just one element in the queue reset it.
			cond->queue_begin = 0;
			cond->queue_end = 0;
		}

		--cond->waiting_threads;
	}

	UNLOCK_QUEUE(cond->lock);

	if (thread_id != 0)
	{
		status = NtAlertThreadByThreadId((HANDLE)(SIZE_T)thread_id);
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}
	}

	return 0;
}

int wlibc_cond_broadcast(cond_t *cond)
{
	VALIDATE_COND(cond);

	DWORD thread_id = 0;

	// We are holding the lock here for a long time here. Is this really necessary?.
	LOCK_QUEUE(cond->lock);

	// Traverse the queue multiple times (atmost 2) to get all the waiting threads.
	while (cond->waiting_threads != 0)
	{
		thread_id = cond->ptr[cond->queue_begin];
		cond->ptr[cond->queue_begin] = 0;

		++cond->queue_begin;
		--cond->waiting_threads;

		// End of queues memory, go back to the start.
		if (cond->queue_begin == cond->queue_size)
		{
			cond->queue_begin = 0;
		}

		if (thread_id != 0)
		{
			// Ignore errors here.
			NtAlertThreadByThreadId((HANDLE)(SIZE_T)thread_id);
		}
	}

	// No more waiting threads, reset the queue and return.
	cond->queue_begin = 0;
	cond->queue_end = 0;

	UNLOCK_QUEUE(cond->lock);

	return 0;
}

int wlibc_cond_common_wait(cond_t *restrict cond, mutex_t *restrict mutex, const struct timespec *restrict abstime)
{
	NTSTATUS status;
	LARGE_INTEGER timeout;
	DWORD thread_id = NtCurrentThreadId();

	// First release the mutex.
	if (wlibc_mutex_unlock(mutex) == -1)
	{
		return -1;
	}

	// Do the wait.
	if (abstime != NULL)
	{
		timeout = timespec_to_LARGE_INTEGER(abstime);
	}

	// Insert self in the queue.
	int index;

	LOCK_QUEUE(cond->lock);

	if (cond->queue_end == cond->queue_size)
	{
		// Temporary hack to go back to the beginning. This will be fixed later. FIXME
		cond->queue_end = 0;
	}

	index = cond->queue_end;

#if 0
	// TODO
	// Double the queue buffer if all slots are filled
	if (cond->queue_end == cond->queue_size && cond->ptr[0] != 0)
	{
		// Double the queue.
		DWORD *temp = (DWORD *)malloc(sizeof(DWORD) * cond->queue_size * 2);


		cond->queue_size *= 2;
	}
#endif

	cond->ptr[cond->queue_end++] = thread_id;
	++cond->waiting_threads;

	UNLOCK_QUEUE(cond->lock);

	// The first argument to this function is pretty much pointless as it does not alert the thread on change automatically.
	status = NtWaitForAlertByThreadId(NULL, abstime == NULL ? NULL : &timeout);
	if (status != STATUS_SUCCESS)
	{
		if (status == STATUS_TIMEOUT)
		{
			// Remove self from the queue.
			LOCK_QUEUE(cond->lock);
			cond->ptr[index] = 0;
			--cond->waiting_threads;
			UNLOCK_QUEUE(cond->lock);
		}

		map_ntstatus_to_errno(status);
		return -1;
	}

	// Finally require the mutex.
	if (wlibc_mutex_lock(mutex) == -1)
	{
		return -1;
	}

	return 0;
}

int wlibc_cond_wait(cond_t *restrict cond, mutex_t *restrict mutex)
{
	VALIDATE_COND(cond);
	VALIDATE_MUTEX(mutex);
	return wlibc_cond_common_wait(cond, mutex, NULL);
}

int wlibc_cond_timedwait(cond_t *restrict cond, mutex_t *restrict mutex, const struct timespec *restrict abstime)
{
	VALIDATE_COND(cond);
	VALIDATE_MUTEX(mutex);
	VALIDATE_PTR(abstime, EINVAL, -1);
	return wlibc_cond_common_wait(cond, mutex, abstime);
}

// Attributes
int wlibc_condattr_init(cond_attr_t *attributes)
{
	VALIDATE_COND_ATTR(attributes);
	attributes->shared = WLIBC_PROCESS_PRIVATE;
	return 0;
}

int wlibc_condattr_getpshared(const cond_attr_t *restrict attributes, int *restrict pshared)
{
	VALIDATE_COND_ATTR(attributes);
	VALIDATE_PTR(pshared, EINVAL, -1);
	*pshared = attributes->shared;
	return 0;
}

int wlibc_condattr_setpshared(cond_attr_t *attributes, int pshared)
{
	// nop
	// Condition variables cannot be shared across processes in Windows.
	VALIDATE_COND_ATTR(attributes);
	if (pshared != WLIBC_PROCESS_PRIVATE && pshared != WLIBC_PROCESS_SHARED)
	{
		errno = EINVAL;
		return -1;
	}

	return 0;
}
