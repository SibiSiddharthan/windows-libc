/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <errno.h>
#include <thread.h>

#define VALIDATE_COND_PTR(cond) \
	if (cond == NULL)           \
	{                           \
		errno = EINVAL;         \
		return -1;              \
	}

#define VALIDATE_COND_AND_MUTEX_PTRS(cond, mutex) \
	if (cond == NULL || mutex == NULL)            \
	{                                             \
		errno = EINVAL;                           \
		return -1;                                \
	}

int wlibc_cond_init(cond_t *restrict cond, const cond_attr_t *restrict attributes)
{
	VALIDATE_COND_PTR(cond);
	cond->ptr = 0;
	return 0;
}

int wlibc_cond_signal(cond_t *cond)
{
	VALIDATE_COND_PTR(cond);
	RtlWakeConditionVariable((PRTL_CONDITION_VARIABLE)cond);
	return 0;
}

int wlibc_cond_broadcast(cond_t *cond)
{
	VALIDATE_COND_PTR(cond);
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
