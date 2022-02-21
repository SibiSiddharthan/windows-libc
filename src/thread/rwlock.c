/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <errno.h>
#include <thread.h>

#define VALIDATE_PTR(ptr) \
	if (ptr == NULL)      \
	{                     \
		errno = EINVAL;   \
		return -1;        \
	}

#define VALIDATE_RWLOCK(rwlock)           VALIDATE_PTR(rwlock)
#define VALIDATE_RWLOCK_ATTR(rwlock_attr) VALIDATE_PTR(rwlock_attr)

int wlibc_rwlock_init(rwlock_t *restrict rwlock, const rwlock_attr_t *restrict attributes)
{
	VALIDATE_RWLOCK(rwlock);
	rwlock->ptr = 0;
	return 0;
}

int wlibc_rwlock_rdlock(rwlock_t *rwlock)
{
	VALIDATE_RWLOCK(rwlock)
	RtlAcquireSRWLockShared((PRTL_SRWLOCK)rwlock);
	return 0;
}

int wlibc_rwlock_tryrdlock(rwlock_t *rwlock)
{
	VALIDATE_RWLOCK(rwlock);
	return RtlTryAcquireSRWLockShared((PRTL_SRWLOCK)rwlock);
}

int wlibc_rwlock_timedrdlock(rwlock_t *restrict rwlock, const struct timespec *restrict abstime)
{
	VALIDATE_RWLOCK(rwlock);
	// TODO
	return 0;
}

int wlibc_rwlock_wrlock(rwlock_t *rwlock)
{
	VALIDATE_RWLOCK(rwlock);
	RtlAcquireSRWLockExclusive((PRTL_SRWLOCK)rwlock);
	return 0;
}

int wlibc_rwlock_trywrlock(rwlock_t *rwlock)
{
	VALIDATE_RWLOCK(rwlock);
	return RtlTryAcquireSRWLockExclusive((PRTL_SRWLOCK)rwlock);
}

int wlibc_rwlock_timedwrlock(rwlock_t *restrict rwlock, const struct timespec *restrict abstime)
{
	VALIDATE_RWLOCK(rwlock);
	// TODO
	return 0;
}

int wlibc_rwlock_unlock(rwlock_t *rwlock)
{
	VALIDATE_RWLOCK(rwlock);

	// CHECK
	// Exclusive locks set the 0th bit to high.
	if (rwlock->ptr == 1)
	{
		RtlReleaseSRWLockExclusive((PRTL_SRWLOCK)rwlock);
	}
	else
	{
		RtlReleaseSRWLockShared((PRTL_SRWLOCK)rwlock);
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
	VALIDATE_PTR(pshared);
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
