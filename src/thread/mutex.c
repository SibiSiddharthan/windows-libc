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

	return 0;
}

int wlibc_mutex_lock(mutex_t *mutex)
{
	VALIDATE_MUTEX(mutex);

	NTSTATUS status;

	status = NtWaitForSingleObject(mutex->handle, FALSE, NULL);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	_InterlockedExchange((volatile long *)&mutex->owner, GetCurrentThreadId());

	return 0;
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

	_InterlockedExchange((volatile long *)&mutex->owner, 0);

	return 0;
}

int wlibc_mutex_trylock(mutex_t *mutex)
{
	VALIDATE_MUTEX(mutex);

	NTSTATUS status;
	LARGE_INTEGER timeout;

	timeout.QuadPart = 0;

	status = NtWaitForSingleObject(mutex->handle, FALSE, &timeout);
	if (status != STATUS_SUCCESS && status != STATUS_TIMEOUT)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	if (status == STATUS_TIMEOUT)
	{
		errno = EBUSY;
		return -1;
	}

	// Mutex has been acquired by the thread.
	_InterlockedExchange((volatile long *)&mutex->owner, GetCurrentThreadId());

	return 0;
}

int wlibc_mutex_timedlock(mutex_t *restrict mutex, const struct timespec *restrict abstime)
{
	VALIDATE_MUTEX(mutex);
	VALIDATE_PTR(abstime);

	NTSTATUS status;
	LARGE_INTEGER timeout;

	// From utimens.c. TODO
	timeout.QuadPart = abstime->tv_sec * 10000000 + abstime->tv_nsec / 100;
	timeout.QuadPart += 116444736000000000LL;

	status = NtWaitForSingleObject(mutex->handle, FALSE, &timeout);
	if (status != STATUS_SUCCESS && status != STATUS_TIMEOUT)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	if (status == STATUS_TIMEOUT)
	{
		errno = EBUSY;
		return -1;
	}

	// Mutex has been acquired by the thread.
	_InterlockedExchange((volatile long *)&mutex->owner, GetCurrentThreadId());

	return 0;
}
