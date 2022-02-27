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

#define VALIDATE_BARRIER(barrier)           VALIDATE_PTR(barrier)
#define VALIDATE_BARRIER_ATTR(barrier_attr) VALIDATE_PTR(barrier_attr)

// NOTE: As of Windows 10 21H2 ntdll.lib does not export the Rtl barrier functions. They are present in ntdllp.lib.
// The Github hosted runner does not have ntdllp.lib yet. For the time being use the kernel32.lib functions.
// Once ntdll.lib exports these functions we can switch over to using the Rtl functions.

int wlibc_barrier_init(barrier_t *restrict barrier, const barrier_attr_t *restrict attributes, unsigned int count)
{
	VALIDATE_BARRIER(barrier);

#if 0
	NTSTATUS status;

	status = RtlInitBarrier((PRTL_BARRIER)barrier, count, 0);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}
#endif
	BOOL status;

	status = InitializeSynchronizationBarrier((PRTL_BARRIER)barrier, count, 0);
	if (!status)
	{
		map_doserror_to_errno(GetLastError());
		return -1;
	}

	return 0;
}

int wlibc_barrier_destroy(barrier_t *barrier)
{
	VALIDATE_BARRIER(barrier);

	// This function will not fail.
	// RtlDeleteBarrier((PRTL_BARRIER)barrier);
	DeleteSynchronizationBarrier((PRTL_BARRIER)barrier);
	return 0;
}

int wlibc_barrier_wait(barrier_t *barrier)
{
	VALIDATE_BARRIER(barrier);
	// return RtlBarrier((PRTL_BARRIER)barrier, 0);
	return EnterSynchronizationBarrier((PRTL_BARRIER)barrier, 0);
}

int pthread_barrierattr_init(barrier_attr_t *attributes)
{
	VALIDATE_BARRIER_ATTR(attributes);
	attributes->shared = WLIBC_PROCESS_PRIVATE;
	return 0;
}

int pthread_barrierattr_getpshared(const barrier_attr_t *restrict attributes, int *restrict pshared)
{
	VALIDATE_BARRIER_ATTR(attributes);
	VALIDATE_PTR(pshared);
	*pshared = attributes->shared;
	return 0;
}

int pthread_barrierattr_setpshared(barrier_attr_t *attributes, int pshared)
{
	// nop
	// Synchronization barriers cannot be shared across processes in Windows.
	VALIDATE_BARRIER_ATTR(attributes);
	if (pshared != WLIBC_PROCESS_PRIVATE && pshared != WLIBC_PROCESS_SHARED)
	{
		errno = EINVAL;
		return -1;
	}

	return 0;
}
