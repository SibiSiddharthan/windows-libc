/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <errno.h>
#include <thread.h>

#define VALIDATE_BARRIER_PTR(barrier) \
	if (barrier == NULL)              \
	{                                 \
		errno = EINVAL;               \
		return -1;                    \
	}

int wlibc_barrier_init(barrier_t *restrict barrier, const barrier_attr_t *restrict attributes, unsigned int count)
{
	VALIDATE_BARRIER_PTR(barrier);

	NTSTATUS status;

	status = RtlInitBarrier((PRTL_BARRIER)barrier, count, 0);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}

int wlibc_barrier_destroy(barrier_t *barrier)
{
	VALIDATE_BARRIER_PTR(barrier);

	// This function will not fail.
	RtlDeleteBarrier((PRTL_BARRIER)barrier);
	return 0;
}

int wlibc_barrier_wait(barrier_t *barrier)
{
	VALIDATE_BARRIER_PTR(barrier);
	return RtlBarrier((PRTL_BARRIER)barrier, 0);
}
