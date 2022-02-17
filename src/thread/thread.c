/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <thread.h>

int wlibc_thread_create(thread_t *thread, thread_attr_t *attributes, thread_start_t routine, void *arg)
{
	DWORD thread_id;
	HANDLE thread_handle;

	thread_handle =
		CreateRemoteThreadEx(NtCurrentProcess(), NULL, 0, (LPTHREAD_START_ROUTINE)routine, arg, CREATE_SUSPENDED, NULL, &thread_id);
	if (thread_handle == NULL)
	{
		map_doserror_to_errno(GetLastError());
		return -1;
	}

	thread->handle = thread_handle;
	thread->id = thread_id;

	NtResumeThread(thread_handle, NULL);

	return 0;
}

// Detaching a thread doesn't really work in Windows, as we can always open
// a handle to it with the thread id. Anyway just close the handle.
int wlibc_thread_detach(thread_t thread)
{
	NTSTATUS status;

	status = NtClose(thread.handle);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}

int wlibc_thread_join(thread_t thread, void **result)
{
	NTSTATUS status;
	THREAD_BASIC_INFORMATION basic_info;

	status = NtWaitForSingleObject(thread.handle, FALSE, NULL);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	status = NtQueryInformationThread(thread.handle, ThreadBasicInformation, &basic_info, sizeof(basic_info), NULL);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	status = NtClose(thread.handle);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	*result = (void *)basic_info.ExitStatus;

	return 0;
}

int wlibc_thread_equal(thread_t thread_a, thread_t thread_b)
{
	return thread_a.id == thread_b.id;
}

thread_t wlibc_thread_self(void)
{
	thread_t thread;

	thread.handle = NtCurrentThread();
	thread.id = GetCurrentThreadId();

	return thread;
}

int wlibc_thread_sleep(const struct timespec *duration, struct timespec *remaining)
{
	NTSTATUS status;
	LARGE_INTEGER interval;

	// TODO time conversions
	status = NtDelayExecution(TRUE, &interval);
	if (status != STATUS_SUCCESS)
	{
		// TODO check alerted case
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}

int wlibc_thread_yield(void)
{
	NtYieldExecution();
	return 0;
}
