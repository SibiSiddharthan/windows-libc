/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <errno.h>
#include <thread.h>

#define VALIDATE_MUTEX_PTR(mutex) \
	if (mutex == NULL)            \
	{                             \
		errno = EINVAL;           \
		return -1;                \
	}

int wlibc_mutex_init(mutex_t *mutex, const mutex_attr_t *attributes)
{
	VALIDATE_MUTEX_PTR(mutex);

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
	VALIDATE_MUTEX_PTR(mutex);

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
	VALIDATE_MUTEX_PTR(mutex);

	NTSTATUS status;

	status = NtWaitForSingleObject(mutex->handle, FALSE, NULL);

	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	mutex->owner = GetCurrentThreadId();

	return 0;
}

int wlibc_mutex_unlock(mutex_t *mutex)
{
	VALIDATE_MUTEX_PTR(mutex);

	NTSTATUS status;

	status = NtReleaseMutant(mutex->handle, NULL);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	mutex->owner = 0;

	return 0;
}

int wlibc_mutex_trylock(mutex_t *mutex)
{
	VALIDATE_MUTEX_PTR(mutex);

	// TODO
	return 0;
}

int wlibc_mutex_timedlock(mutex_t *restrict mutex, const struct timespec *restrict abstime)
{
	VALIDATE_MUTEX_PTR(mutex);

	// TODO
	return 0;
}
