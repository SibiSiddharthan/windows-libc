/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/validate.h>
#include <errno.h>
#include <thread.h>
#include <intrin.h>

#define VALIDATE_RWLOCK(rwlock)           \
	VALIDATE_PTR(rwlock, EINVAL, -1)      \
	if (rwlock->lock == RWLOCK_DESTROYED) \
	{                                     \
		errno = EINVAL;                   \
		return -1;                        \
	}

#define VALIDATE_RWLOCK_ATTR(rwlock_attr) VALIDATE_PTR(rwlock_attr, EINVAL, -1)

#define RWLOCK_NOT_LOCKED     0
#define RWLOCK_SHARED_LOCK    1
#define RWLOCK_EXCLUSIVE_LOCK 2
#define RWLOCK_DESTROYED      -1

int wlibc_rwlock_init(rwlock_t *restrict rwlock, const rwlock_attr_t *restrict attributes)
{
	VALIDATE_PTR(rwlock, EINVAL, -1);
	rwlock->ptr = 0;
	rwlock->lock = RWLOCK_NOT_LOCKED;
	return 0;
}

int wlibc_rwlock_destroy(rwlock_t *rwlock)
{
	VALIDATE_RWLOCK(rwlock)
	rwlock->lock = RWLOCK_DESTROYED;
	return 0;
}

int wlibc_rwlock_rdlock(rwlock_t *rwlock)
{
	VALIDATE_RWLOCK(rwlock)
	RtlAcquireSRWLockShared((PRTL_SRWLOCK)&rwlock->ptr);
	rwlock->lock = RWLOCK_SHARED_LOCK;
	return 0;
}

int wlibc_rwlock_tryrdlock(rwlock_t *rwlock)
{
	VALIDATE_RWLOCK(rwlock);
	if (RtlTryAcquireSRWLockShared((PRTL_SRWLOCK)&rwlock->ptr))
	{
		rwlock->lock = RWLOCK_SHARED_LOCK;
		return 0;
	}

	errno = EBUSY;
	return -1;
}

int wlibc_rwlock_timedrdlock(rwlock_t *restrict rwlock, const struct timespec *restrict abstime)
{
	VALIDATE_RWLOCK(rwlock);
	VALIDATE_PTR(abstime, EINVAL, -1);

	LARGE_INTEGER duetime, current;

	// From utimens.c. TODO
	duetime.QuadPart = abstime->tv_sec * 10000000 + abstime->tv_nsec / 100;
	duetime.QuadPart += 116444736000000000LL;

	// Perform a gentle spin till we acquire the lock.
	do
	{
		if (RtlTryAcquireSRWLockShared((PRTL_SRWLOCK)&rwlock->ptr))
		{
			// Acquired the lock.
			rwlock->lock = RWLOCK_SHARED_LOCK;
			return 0;
		}
		_mm_pause();

		GetSystemTimeAsFileTime((LPFILETIME)&current);
	} while (current.QuadPart <= duetime.QuadPart);

	// Unable to acquire lock in the given time.
	errno = ETIMEDOUT;
	return -1;
}

int wlibc_rwlock_wrlock(rwlock_t *rwlock)
{
	VALIDATE_RWLOCK(rwlock);
	RtlAcquireSRWLockExclusive((PRTL_SRWLOCK)&rwlock->ptr);
	rwlock->lock = RWLOCK_EXCLUSIVE_LOCK;
	return 0;
}

int wlibc_rwlock_trywrlock(rwlock_t *rwlock)
{
	VALIDATE_RWLOCK(rwlock);
	if (RtlTryAcquireSRWLockExclusive((PRTL_SRWLOCK)&rwlock->ptr))
	{
		rwlock->lock = RWLOCK_EXCLUSIVE_LOCK;
		return 0;
	}

	errno = EBUSY;
	return -1;
}

int wlibc_rwlock_timedwrlock(rwlock_t *restrict rwlock, const struct timespec *restrict abstime)
{
	VALIDATE_RWLOCK(rwlock);
	VALIDATE_PTR(abstime, EINVAL, -1);

	LARGE_INTEGER duetime, current;

	// From utimens.c. TODO
	duetime.QuadPart = abstime->tv_sec * 10000000 + abstime->tv_nsec / 100;
	duetime.QuadPart += 116444736000000000LL;

	// Perform a gentle spin till we acquire the lock.
	do
	{
		if (RtlTryAcquireSRWLockExclusive((PRTL_SRWLOCK)&rwlock->ptr))
		{
			// Acquired the lock.
			rwlock->lock = RWLOCK_EXCLUSIVE_LOCK;
			return 0;
		}
		_mm_pause();

		GetSystemTimeAsFileTime((LPFILETIME)&current);
	} while (current.QuadPart <= duetime.QuadPart);

	// Unable to acquire lock in the given time.
	errno = ETIMEDOUT;
	return -1;
}

int wlibc_rwlock_unlock(rwlock_t *rwlock)
{
	VALIDATE_RWLOCK(rwlock);

	// Releasing a not owned SRW lock segfaults. Perform a free check beforehand.
	if (_InterlockedCompareExchange64((volatile LONG64 *)&rwlock->ptr, 0, 0) == 0)
	{
		errno = EPERM;
		rwlock->lock = RWLOCK_NOT_LOCKED;
		return -1;
	}

	switch (rwlock->lock)
	{
	case RWLOCK_SHARED_LOCK:
		RtlReleaseSRWLockShared((PRTL_SRWLOCK)&rwlock->ptr);
		break;
	case RWLOCK_EXCLUSIVE_LOCK:
		RtlReleaseSRWLockExclusive((PRTL_SRWLOCK)&rwlock->ptr);
		break;
	default:
		errno = EPERM;
		return -1;
	}

	// Set the lock status to not locked if ptr becomes 0.
	if (_InterlockedCompareExchange64((volatile LONG64 *)&rwlock->ptr, 0, 0) == 0)
	{
		rwlock->lock = RWLOCK_NOT_LOCKED;
	}
	return 0;
}

int wlibc_rwlockattr_init(rwlock_attr_t *attributes)
{
	VALIDATE_RWLOCK_ATTR(attributes);
	attributes->shared = WLIBC_PROCESS_PRIVATE;
	return 0;
}

int wlibc_rwlockattr_getpshared(const rwlock_attr_t *restrict attributes, int *restrict pshared)
{
	VALIDATE_RWLOCK_ATTR(attributes);
	VALIDATE_PTR(pshared, EINVAL, -1);
	*pshared = attributes->shared;
	return 0;
}

int wlibc_rwlockattr_setpshared(rwlock_attr_t *attributes, int pshared)
{
	// nop
	// Slim Reader-Writer locks cannot be shared across processes in Windows.
	VALIDATE_RWLOCK_ATTR(attributes);
	if (pshared != WLIBC_PROCESS_PRIVATE && pshared != WLIBC_PROCESS_SHARED)
	{
		errno = EINVAL;
		return -1;
	}

	return 0;
}
