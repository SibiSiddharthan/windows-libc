/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <errno.h>
#include <thread.h>

#define VALIDATE_RWLOCK_PTR(rwlock) \
	if (rwlock == NULL)             \
	{                               \
		errno = EINVAL;             \
		return -1;                  \
	}

int wlibc_rwlock_init(rwlock_t *restrict rwlock, const rwlock_attr_t *restrict attributes)
{
	VALIDATE_RWLOCK_PTR(rwlock);
	rwlock->ptr = 0;
	return 0;
}

int wlibc_rwlock_rdlock(rwlock_t *rwlock)
{
	VALIDATE_RWLOCK_PTR(rwlock)
	RtlAcquireSRWLockShared((PRTL_SRWLOCK)rwlock);
	return 0;
}

int wlibc_rwlock_tryrdlock(rwlock_t *rwlock)
{
	VALIDATE_RWLOCK_PTR(rwlock);
	return RtlTryAcquireSRWLockShared((PRTL_SRWLOCK)rwlock);
}

int wlibc_rwlock_timedrdlock(rwlock_t *restrict rwlock, const struct timespec *restrict abstime)
{
	VALIDATE_RWLOCK_PTR(rwlock);
	// TODO
	return 0;
}

int wlibc_rwlock_wrlock(rwlock_t *rwlock)
{
	VALIDATE_RWLOCK_PTR(rwlock);
	RtlAcquireSRWLockExclusive((PRTL_SRWLOCK)rwlock);
	return 0;
}

int wlibc_rwlock_trywrlock(rwlock_t *rwlock)
{
	VALIDATE_RWLOCK_PTR(rwlock);
	return RtlTryAcquireSRWLockExclusive((PRTL_SRWLOCK)rwlock);
}

int wlibc_rwlock_timedwrlock(rwlock_t *restrict rwlock, const struct timespec *restrict abstime)
{
	VALIDATE_RWLOCK_PTR(rwlock);
	// TODO
	return 0;
}

int wlibc_rwlock_unlock(rwlock_t *rwlock)
{
	VALIDATE_RWLOCK_PTR(rwlock);

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
