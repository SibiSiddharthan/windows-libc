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

#define VALIDATE_COND(cond)           VALIDATE_PTR(cond)
#define VALIDATE_COND_ATTR(cond_attr) VALIDATE_PTR(cond_attr)

#define VALIDATE_COND_AND_MUTEX_PTRS(cond, mutex) \
	if (cond == NULL || mutex == NULL)            \
	{                                             \
		errno = EINVAL;                           \
		return -1;                                \
	}

int wlibc_cond_init(cond_t *restrict cond, const cond_attr_t *restrict attributes)
{
	VALIDATE_COND(cond);
	cond->ptr = 0;
	return 0;
}

int wlibc_cond_signal(cond_t *cond)
{
	VALIDATE_COND(cond);
	RtlWakeConditionVariable((PRTL_CONDITION_VARIABLE)cond);
	return 0;
}

int wlibc_cond_broadcast(cond_t *cond)
{
	VALIDATE_COND(cond);
	RtlWakeAllConditionVariable((PRTL_CONDITION_VARIABLE)cond);
	return 0;
}

int wlibc_cond_wait(cond_t *restrict cond, mutex_t *restrict mutex)
{
	VALIDATE_COND_AND_MUTEX_PTRS(cond, mutex);
	// TODO
	return 0;
}

int wlibc_cond_timedwait(cond_t *restrict cond, mutex_t *restrict mutex, const struct timespec *restrict abstime)
{
	VALIDATE_COND_AND_MUTEX_PTRS(cond, mutex);
	// TODO
	return 0;
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
	VALIDATE_PTR(pshared);
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
