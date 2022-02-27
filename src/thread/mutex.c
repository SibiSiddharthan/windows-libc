/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <errno.h>
#include <thread.h>

#define VALIDATE_PTR(ptr) \
	if (ptr == NULL)      \
	{                     \
		errno = EINVAL;   \
		return -1;        \
	}

#define VALIDATE_MUTEX(mutex)           VALIDATE_PTR(mutex)
#define VALIDATE_MUTEX_ATTR(mutex_attr) VALIDATE_PTR(mutex_attr)

int wlibc_mutex_init(mutex_t *mutex, const mutex_attr_t *attributes)
{
	VALIDATE_MUTEX(mutex);
	if (attributes != NULL)
	{
		if (attributes->shared != WLIBC_PROCESS_PRIVATE && attributes->shared != WLIBC_PROCESS_SHARED)
		{
			errno = EINVAL;
			return -1;
		}
		if (attributes->type < 0 || attributes->type > (WLIBC_MUTEX_NORMAL | WLIBC_MUTEX_RECURSIVE | WLIBC_MUTEX_TIMED))
		{
			errno = EINVAL;
			return -1;
		}
	}

	NTSTATUS status;
	HANDLE handle;

	status = NtCreateMutant(&handle, MUTANT_ALL_ACCESS, NULL, FALSE);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	mutex->handle = handle;
	mutex->owner = 0;
	mutex->count = 0;

	if (attributes == NULL)
	{
		// This is the default mutex type.
		mutex->type = WLIBC_MUTEX_TIMED;
	}
	else
	{
		mutex->type = attributes->type;
	}

	return 0;
}

int wlibc_mutex_destroy(mutex_t *mutex)
{
	VALIDATE_MUTEX(mutex);

	NTSTATUS status;

	status = NtClose(mutex->handle);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	mutex->handle = 0;
	mutex->owner = 0;
	mutex->count = 0;

	return 0;
}

int wlibc_mutex_common_lock(mutex_t *restrict mutex, const struct timespec *restrict abstime)
{
	NTSTATUS status;
	LARGE_INTEGER timeout;

	// First check if we are trying to acquire a non-recursive mutex recursively.
	if ((mutex->type & WLIBC_MUTEX_RECURSIVE) != WLIBC_MUTEX_RECURSIVE)
	{
		DWORD thread_id = GetCurrentThreadId();

		if (_InterlockedCompareExchange((volatile long *)&mutex->owner, thread_id, thread_id) == (LONG)thread_id)
		{
			// Trying to acquired a recursive mutex lock when the mutex is supposed to be locked onlt one once.
			// Set errno to 'EDEADLK' (would deadlock) an return.
			errno = EDEADLK;
			return -1;
		}
	}

	// Mutex does not support timed waits for locks.
	if ((mutex->type & WLIBC_MUTEX_TIMED) == 0)
	{
		abstime = NULL;
	}
	else
	{
		timeout.QuadPart = 0;
		// If abstime is null                    -> infinite wait for lock.
		// If abstime is 0(tv_sec, tv_nsec is 0) -> try lock.
		if (abstime != NULL && abstime->tv_sec != 0 && abstime->tv_nsec != 0)
		{
			// From utimens.c. TODO
			timeout.QuadPart = abstime->tv_sec * 10000000 + abstime->tv_nsec / 100;
			timeout.QuadPart += 116444736000000000LL;
		}
	}

	status = NtWaitForSingleObject(mutex->handle, FALSE, abstime == NULL ? NULL : &timeout);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	_InterlockedExchange((volatile long *)&mutex->owner, GetCurrentThreadId());
	_InterlockedIncrement((volatile long *)&mutex->count);

	return 0;
}

int wlibc_mutex_lock(mutex_t *mutex)
{
	VALIDATE_MUTEX(mutex);
	return wlibc_mutex_common_lock(mutex, NULL);
}

int wlibc_mutex_trylock(mutex_t *mutex)
{
	VALIDATE_MUTEX(mutex);

	struct timespec timeout = {0, 0};
	return wlibc_mutex_common_lock(mutex, &timeout);
}

int wlibc_mutex_timedlock(mutex_t *restrict mutex, const struct timespec *restrict abstime)
{
	VALIDATE_MUTEX(mutex);
	VALIDATE_PTR(abstime);

	return wlibc_mutex_common_lock(mutex, abstime);
}

int wlibc_mutex_unlock(mutex_t *mutex)
{
	VALIDATE_MUTEX(mutex);

	NTSTATUS status;

	status = NtReleaseMutant(mutex->handle, NULL);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	// If the lock count reaches zero, mutex has no owner.
	if (_InterlockedDecrement((volatile long *)&mutex->count) == 0)
	{
		_InterlockedExchange((volatile long *)&mutex->owner, 0);
	}

	return 0;
}

int wlibc_mutexattr_init(mutex_attr_t *attributes)
{
	VALIDATE_MUTEX_ATTR(attributes);
	attributes->shared = WLIBC_PROCESS_PRIVATE;
	attributes->type = WLIBC_MUTEX_TIMED;
	return 0;
}

int wlibc_mutexattr_getpshared(const mutex_attr_t *restrict attributes, int *restrict pshared)
{
	VALIDATE_MUTEX_ATTR(attributes);
	VALIDATE_PTR(pshared);

	*pshared = attributes->shared;
	return 0;
}

int wlibc_mutexattr_setpshared(mutex_attr_t *attributes, int pshared)
{
	VALIDATE_MUTEX_ATTR(attributes);
	if (pshared != WLIBC_PROCESS_PRIVATE && pshared != WLIBC_PROCESS_SHARED)
	{
		errno = EINVAL;
		return -1;
	}

	// Mutexes can be shared between processes on Windows, to do so the mutex needs to be named.
	// Since there is no posix equivalent of naming a mutex, this flags does little to nothing.
	attributes->shared = pshared;
	return 0;
}

int wlibc_mutexattr_gettype(const mutex_attr_t *restrict attributes, int *restrict type)
{
	VALIDATE_MUTEX_ATTR(attributes);
	VALIDATE_PTR(type);

	*type = attributes->type;
	return 0;
}

int wlibc_mutexattr_settype(mutex_attr_t *attributes, int type)
{
	VALIDATE_MUTEX_ATTR(attributes);
	if (type < 0 || type > (WLIBC_MUTEX_NORMAL | WLIBC_MUTEX_RECURSIVE | WLIBC_MUTEX_TIMED))
	{
		errno = EINVAL;
		return -1;
	}

	attributes->type = type;
	return 0;
}
