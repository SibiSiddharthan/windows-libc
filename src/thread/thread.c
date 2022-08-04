/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/convert.h>
#include <internal/error.h>
#include <internal/sched.h>
#include <internal/thread.h>
#include <internal/validate.h>
#include <errno.h>
#include <thread.h>

#define VALIDATE_THREAD(thread)           VALIDATE_PTR(thread, EINVAL, -1)
#define VALIDATE_THREAD_ATTR(thread_attr) VALIDATE_PTR(thread_attr, EINVAL, -1)

DWORD wlibc_thread_entry(void *arg)
{
	threadinfo *tinfo = (threadinfo *)arg;
	void *result;

	TlsSetValue(_wlibc_threadinfo_index, (void *)tinfo);

	result = tinfo->routine(tinfo->args);
	tinfo->result = result;

	// Cleanup
	execute_cleanup(tinfo);
	cleanup_tls(tinfo);
	RtlExitUserThread((NTSTATUS)(LONG_PTR)result);

	// The End.
}

int wlibc_thread_create(thread_t *thread, thread_attr_t *attributes, thread_start_t routine, void *arg)
{
	DWORD thread_id;
	HANDLE thread_handle;
	SIZE_T stacksize = 0;
	BOOLEAN should_detach = FALSE;
	BOOLEAN create_suspended = FALSE;
	threadinfo *tinfo;

	if (thread == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	if (attributes != NULL)
	{
		stacksize = attributes->stacksize;
		if (attributes->detachstate == WLIBC_THREAD_DETACHED)
		{
			should_detach = TRUE;
		}

		if (attributes->suspendstate == WLIBC_THREAD_SUSPENDED)
		{
			create_suspended = TRUE;
		}
	}

	*thread = RtlAllocateHeap(NtCurrentProcessHeap(), HEAP_ZERO_MEMORY, sizeof(threadinfo));
	tinfo = (threadinfo *)*thread;

	if (*thread == NULL)
	{
		errno = ENOMEM;
		return -1;
	}

	tinfo->routine = routine;
	tinfo->args = arg;

	thread_handle =
		CreateRemoteThreadEx(NtCurrentProcess(), NULL, stacksize, wlibc_thread_entry, (void *)tinfo, CREATE_SUSPENDED, NULL, &thread_id);
	if (thread_handle == NULL)
	{
		map_doserror_to_errno(GetLastError());
		return -1;
	}

	tinfo->handle = thread_handle;
	tinfo->id = thread_id;

	if (attributes != NULL)
	{
		if (attributes->inherit == WLIBC_THREAD_EXPLICIT_SCHED)
		{
			struct sched_param param;
			param.sched_priority = attributes->priority;

			wlibc_thread_setschedparam(tinfo, attributes->policy, &param);
		}

		if (attributes->set != NULL)
		{
			// This can be done by passing GROUP_AFFINITY in PROC_THREAD_ATTRIBUTE_LIST.
			wlibc_thread_setaffinity(tinfo, attributes->set);
		}
	}

	if (!create_suspended)
	{
		NtResumeThread(thread_handle, NULL);
	}

	if (should_detach)
	{
		NtClose(thread_handle);
		tinfo->handle = 0;
	}

	return 0;
}

// Detaching a thread doesn't really work in Windows, as we can always open
// a handle to it with the thread id. Anyway just close the handle.
int wlibc_thread_detach(thread_t thread)
{
	NTSTATUS status;
	threadinfo *tinfo = (threadinfo *)thread;

	VALIDATE_THREAD(thread);

	if (tinfo->handle != 0)
	{
		status = NtClose(tinfo->handle);
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}

		tinfo->handle = 0;
	}
	else
	{
		// Already detached thread.
		errno = EINVAL;
		return -1;
	}

	return 0;
}

int wlibc_common_thread_join(thread_t thread, void **result, const struct timespec *abstime)
{
	NTSTATUS status;
	LARGE_INTEGER timeout;
	threadinfo *tinfo = (threadinfo *)thread;

	VALIDATE_THREAD(thread);

	timeout.QuadPart = 0;
	// If abstime is null                    -> infinite wait.
	// If abstime is 0(tv_sec, tv_nsec is 0) -> try wait.
	if (abstime != NULL && abstime->tv_sec != 0 && abstime->tv_nsec != 0)
	{
		timeout = timespec_to_LARGE_INTEGER(abstime);
	}

	status = NtWaitForSingleObject(tinfo->handle, FALSE, abstime == NULL ? NULL : &timeout);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		// If we are doing a try join errno should be set to EBUSY.
		if (timeout.QuadPart == 0 && errno == ETIMEDOUT)
		{
			errno = EBUSY;
		}
		return -1;
	}

	status = NtClose(tinfo->handle);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	if (result != NULL)
	{
		*result = tinfo->result;
	}

	// Finally free the thread structure.
	RtlFreeHeap(NtCurrentProcessHeap(), 0, thread);

	return 0;
}

int wlibc_thread_join(thread_t thread, void **result)
{
	return wlibc_common_thread_join(thread, result, NULL);
}

int wlibc_thread_tryjoin(thread_t thread, void **result)
{
	struct timespec timeout = {0, 0};
	return wlibc_common_thread_join(thread, result, &timeout);
}

int wlibc_thread_timedjoin(thread_t thread, void **result, const struct timespec *abstime)
{
	VALIDATE_PTR(abstime, EINVAL, -1);
	return wlibc_common_thread_join(thread, result, abstime);
}

int wlibc_thread_equal(thread_t thread_a, thread_t thread_b)
{
	return ((threadinfo *)thread_a)->id == ((threadinfo *)thread_b)->id;
}

thread_t wlibc_thread_self(void)
{
	return (thread_t)TlsGetValue(_wlibc_threadinfo_index);
}

int wlibc_thread_sleep(const struct timespec *duration, struct timespec *remaining)
{
	NTSTATUS status;
	LARGE_INTEGER interval;

	if (duration == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	// Measure in 100 nanosecond intervals.
	// Negative means relative measurement.
	interval.QuadPart = -1 * (duration->tv_sec * 10000000 + duration->tv_nsec / 100);

	status = NtDelayExecution(TRUE, &interval);
	if (status != STATUS_SUCCESS)
	{
		// TODO check alerted case
		map_ntstatus_to_errno(status);
		return -1;
	}

	if (remaining)
	{
	}

	// TODO Alerts
	return 0;
}

void wlibc_thread_exit(void *retval)
{
	threadinfo *tinfo = TlsGetValue(_wlibc_threadinfo_index);

	// We use this field to determine the result. The parameter for `RtlExitUserThread` is just for backup.
	tinfo->result = retval;

	// Cleanup
	execute_cleanup(tinfo);
	cleanup_tls(tinfo);

	// The exit code of thread will be truncated to 32bits.
	RtlExitUserThread((NTSTATUS)(LONG_PTR)retval);
}

int wlibc_thread_setcancelstate(int state, int *oldstate)
{
	if (state != WLIBC_THREAD_CANCEL_ENABLE && state != WLIBC_THREAD_CANCEL_DISABLE)
	{
		errno = EINVAL;
		return -1;
	}

	threadinfo *tinfo = TlsGetValue(_wlibc_threadinfo_index);

	if (oldstate)
	{
		*oldstate = tinfo->cancelstate;
	}

	tinfo->cancelstate = state;

	return 0;
}

int wlibc_thread_setcanceltype(int type, int *oldtype)
{
	if (type != WLIBC_THREAD_CANCEL_ASYNCHRONOUS && type != WLIBC_THREAD_CANCEL_DEFERRED)
	{
		errno = EINVAL;
		return -1;
	}

	threadinfo *tinfo = TlsGetValue(_wlibc_threadinfo_index);

	if (oldtype)
	{
		*oldtype = tinfo->canceltype;
	}

	tinfo->cancelstate = type;

	return 0;
}

static void execute_thread_cancellation(threadinfo *tinfo)
{
	if (tinfo->cancelstate == WLIBC_THREAD_CANCEL_DISABLE)
	{
		// Cancellations for this thread are disabled, just return.
		return;
	}

	execute_cleanup(tinfo);
	cleanup_tls(tinfo);
	tinfo->result = WLIBC_THREAD_CANCELED;
	RtlExitUserThread((NTSTATUS)(LONG_PTR)WLIBC_THREAD_CANCELED);
}

static void cancel_apc(void *arg1, void *arg2, void *arg3)
{
	UNREFERENCED_PARAMETER(arg2);
	UNREFERENCED_PARAMETER(arg3);

	execute_thread_cancellation(arg1);
}

int wlibc_thread_cancel(thread_t thread)
{
	NTSTATUS status;
	threadinfo *tinfo = (threadinfo *)thread;

	VALIDATE_THREAD(thread);

	// NOTE: The apc we queue will prempt execution of the thread.
	status = NtQueueApcThreadEx(tinfo->handle, QUEUE_USER_SPECIAL_APC, cancel_apc, (void *)tinfo, NULL, NULL);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}

void wlibc_thread_testcancel(void)
{
	threadinfo *tinfo = TlsGetValue(_wlibc_threadinfo_index);
	execute_thread_cancellation(tinfo);
}

static void kill_apc(void *arg1, void *arg2, void *arg3)
{
	UNREFERENCED_PARAMETER(arg2);
	UNREFERENCED_PARAMETER(arg3);

	wlibc_raise((int)(intptr_t)arg1);
}

int wlibc_thread_kill(thread_t thread, int sig)
{
	NTSTATUS status;
	threadinfo *tinfo = (threadinfo *)thread;

	VALIDATE_THREAD(thread);

	status = NtQueueApcThreadEx(tinfo->handle, QUEUE_USER_SPECIAL_APC, kill_apc, (void *)(intptr_t)sig, NULL, NULL);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}

void wlibc_thread_cleanup_push(cleanup_t routine, void *arg)
{
	threadinfo *tinfo = TlsGetValue(_wlibc_threadinfo_index);

	if (tinfo->cleanup_slots_allocated == 0)
	{
		tinfo->cleanup_entries = (cleanup_entry *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(cleanup_entry) * 8);
		if (tinfo->cleanup_entries == NULL)
		{
			errno = ENOMEM;
			return;
		}

		tinfo->cleanup_slots_allocated = 8;
	}

	// Double the cleanup routine list
	if (tinfo->cleanup_slots_allocated == tinfo->cleanup_slots_used)
	{
		void *temp = (cleanup_entry *)RtlReAllocateHeap(NtCurrentProcessHeap(), 0, tinfo->cleanup_entries,
														sizeof(cleanup_entry) * tinfo->cleanup_slots_allocated * 2);

		if (temp == NULL)
		{
			errno = ENOMEM;
			return;
		}

		tinfo->cleanup_entries = temp;
		tinfo->cleanup_slots_allocated *= 2;
	}

	tinfo->cleanup_entries[tinfo->cleanup_slots_used].routine = routine;
	tinfo->cleanup_entries[tinfo->cleanup_slots_used].arg = arg;
	++tinfo->cleanup_slots_used;
}

void wlibc_thread_cleanup_pop(int execute)
{
	threadinfo *tinfo = TlsGetValue(_wlibc_threadinfo_index);

	if (tinfo->cleanup_slots_used == 0)
	{
		// No cleanup functions registered or all of them have been executed.
		return;
	}

	if (execute)
	{
		// Only execute the cleanup function if it is non NULL.
		if (tinfo->cleanup_entries[tinfo->cleanup_slots_used - 1].routine != NULL)
		{
			tinfo->cleanup_entries[tinfo->cleanup_slots_used - 1].routine(tinfo->cleanup_entries[tinfo->cleanup_slots_used - 1].arg);
		}
	}

	--tinfo->cleanup_slots_used;
}

int wlibc_threadattr_init(thread_attr_t *attributes)
{
	attributes->detachstate = WLIBC_THREAD_JOINABLE;
	attributes->suspendstate = WLIBC_THREAD_RUNNING;
	attributes->inherit = WLIBC_THREAD_INHERIT_SCHED;
	attributes->policy = SCHED_RR;
	attributes->priority = 0;
	attributes->stacksize = 0;
	attributes->set = NULL;
	return 0;
}

int wlibc_threadattr_getdetachstate(const thread_attr_t *attributes, int *detachstate)
{
	VALIDATE_THREAD_ATTR(attributes);
	VALIDATE_PTR(detachstate, EINVAL, -1);
	*detachstate = attributes->detachstate;
	return 0;
}

int wlibc_threadattr_setdetachstate(thread_attr_t *attributes, int detachstate)
{
	VALIDATE_THREAD_ATTR(attributes);
	if (detachstate != WLIBC_THREAD_JOINABLE && detachstate != WLIBC_THREAD_DETACHED)
	{
		errno = EINVAL;
		return -1;
	}

	attributes->detachstate = detachstate;
	return 0;
}

int wlibc_threadattr_getsuspendstate(const thread_attr_t *attributes, int *suspendstate)
{
	VALIDATE_THREAD_ATTR(attributes);
	VALIDATE_PTR(suspendstate, EINVAL, -1);
	*suspendstate = attributes->suspendstate;
	return 0;
}

int wlibc_threadattr_setsuspendstate(thread_attr_t *attributes, int suspendstate)
{
	VALIDATE_THREAD_ATTR(attributes);
	if (suspendstate != WLIBC_THREAD_RUNNING && suspendstate != WLIBC_THREAD_SUSPENDED)
	{
		errno = EINVAL;
		return -1;
	}

	attributes->suspendstate = suspendstate;
	return 0;
}

int wlibc_threadattr_getstacksize(const thread_attr_t *restrict attributes, size_t *restrict stacksize)
{
	VALIDATE_THREAD_ATTR(attributes);
	VALIDATE_PTR(stacksize, EINVAL, -1);
	*stacksize = attributes->stacksize;
	return 0;
}

int wlibc_threadattr_setstacksize(thread_attr_t *attributes, size_t stacksize)
{
	VALIDATE_THREAD_ATTR(attributes);
	attributes->stacksize = stacksize;
	return 0;
}

int wlibc_threadattr_getscope(const thread_attr_t *restrict attributes, int *restrict scope)
{
	VALIDATE_THREAD_ATTR(attributes);
	VALIDATE_PTR(scope, EINVAL, -1);
	*scope = WLIBC_THREAD_SCOPE_SYSTEM;
	return 0;
}

int wlibc_threadattr_setscope(thread_attr_t *attributes, int scope)
{
	VALIDATE_THREAD_ATTR(attributes);

	// We only support the system scope. Just validate input and return.
	if (scope != WLIBC_THREAD_SCOPE_SYSTEM && scope != WLIBC_THREAD_SCOPE_PROCESS)
	{
		errno = EINVAL;
		return -1;
	}

	return 0;
}

int wlibc_threadattr_getinheritsched(thread_attr_t *restrict attributes, int *restrict inherit)
{
	VALIDATE_THREAD_ATTR(attributes);
	VALIDATE_PTR(inherit, EINVAL, -1);
	*inherit = attributes->inherit;
	return 0;
}

int wlibc_threadattr_setinheritsched(thread_attr_t *attributes, int inherit)
{
	VALIDATE_THREAD_ATTR(attributes);

	if (inherit != WLIBC_THREAD_INHERIT_SCHED && inherit != WLIBC_THREAD_EXPLICIT_SCHED)
	{
		errno = EINVAL;
		return -1;
	}

	attributes->inherit = inherit;
	return 0;
}

int wlibc_threadattr_getschedparam(const thread_attr_t *restrict attributes, struct sched_param *restrict param)
{
	VALIDATE_THREAD_ATTR(attributes);
	VALIDATE_PTR(param, EINVAL, -1);
	param->sched_priority = attributes->priority;
	return 0;
}

int wlibc_threadattr_setschedparam(thread_attr_t *restrict attributes, const struct sched_param *restrict param)
{
	VALIDATE_THREAD_ATTR(attributes);
	VALIDATE_PTR(param, EINVAL, -1);

	if (param->sched_priority < SCHED_MIN_PRIORITY || param->sched_priority > SCHED_MAX_PRIORITY)
	{
		errno = EINVAL;
		return -1;
	}

	attributes->priority = param->sched_priority;
	return 0;
}

int wlibc_threadattr_getschedpolicy(const thread_attr_t *restrict attributes, int *restrict policy)
{
	VALIDATE_THREAD_ATTR(attributes);
	VALIDATE_PTR(policy, EINVAL, -1);
	*policy = attributes->policy;
	return 0;
}

int wlibc_threadattr_setschedpolicy(thread_attr_t *attributes, int policy)
{
	VALIDATE_THREAD_ATTR(attributes);
	VALIDATE_SCHED_POLICY(policy, EINVAL, -1);

	attributes->policy = policy;
	return 0;
}

int wlibc_threadattr_getaffinity(const thread_attr_t *attributes, cpu_set_t *restrict cpuset)
{
	VALIDATE_THREAD_ATTR(attributes);
	VALIDATE_PTR(cpuset, EINVAL, -1);

	if (attributes->set == NULL)
	{
		// No specific affinity.
		memset(cpuset->group_mask, 0, cpuset->num_groups * sizeof(unsigned long long));
	}
	else
	{
		if (attributes->set->num_cpus != cpuset->num_cpus)
		{
			errno = ERANGE;
			return -1;
		}

		memcpy(cpuset->group_mask, attributes->set->group_mask, attributes->set->num_groups * sizeof(unsigned long long));
	}

	return 0;
}

int wlibc_threadattr_setaffinity(thread_attr_t *attributes, const cpu_set_t *restrict cpuset)
{
	VALIDATE_THREAD_ATTR(attributes);
	VALIDATE_PTR(cpuset, EINVAL, -1);

	if (cpuset->num_cpus <= 0)
	{
		errno = EINVAL;
		return -1;
	}

	// Discard qualifiers.
	attributes->set = (cpu_set_t *)cpuset;

	return 0;
}

int wlibc_thread_resume(thread_t thread)
{
	NTSTATUS status;
	threadinfo *tinfo = (threadinfo *)thread;

	VALIDATE_THREAD(thread);

	status = NtResumeThread(tinfo->handle, NULL);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}

int wlibc_thread_suspend(thread_t thread)
{
	NTSTATUS status;
	threadinfo *tinfo = (threadinfo *)thread;

	VALIDATE_THREAD(thread);

	status = NtSuspendThread(tinfo->handle, NULL);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}

int wlibc_thread_getconcurrency(void)
{
	NTSTATUS status;
	LOGICAL_PROCESSOR_RELATIONSHIP relationship = RelationGroup;
	ULONG length;
	CHAR buffer[128];
	BYTE count = 0;

	status = NtQuerySystemInformationEx(SystemLogicalProcessorAndGroupInformation, &relationship, sizeof(LOGICAL_PROCESSOR_RELATIONSHIP),
										buffer, sizeof(buffer), &length);
	if (status != STATUS_SUCCESS)
	{
		// CHECK For larger multiprocessor systems.
		map_ntstatus_to_errno(status);
		return 0;
	}

	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX processor_info = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)buffer;

	for (WORD i = 0; i < processor_info->Group.ActiveGroupCount; ++i)
	{
		count += processor_info->Group.GroupInfo[i].ActiveProcessorCount;
	}

	return count;
}

int wlibc_thread_setconcurrency(int level)
{
	// Nop
	if (level < 0)
	{
		errno = EINVAL;
		return -1;
	}

	return 0;
}

int wlibc_threadid(thread_t thread, pid_t *id)
{
	VALIDATE_THREAD(thread);

	threadinfo *tinfo = (threadinfo *)thread;
	*id = (pid_t)tinfo->id;

	return 0;
}

int wlibc_thread_getschedparam(thread_t thread, int *restrict policy, struct sched_param *restrict param)
{
	NTSTATUS status;
	KPRIORITY priority;
	THREAD_BASIC_INFORMATION basic_info;
	threadinfo *tinfo = (threadinfo *)thread;

	VALIDATE_THREAD(thread);

	status = NtQueryInformationThread(tinfo->handle, ThreadBasicInformation, &basic_info, sizeof(THREAD_BASIC_INFORMATION), NULL);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	priority = basic_info.Priority;

	if (priority <= 4) // SCHED_IDLE
	{
		*policy = SCHED_IDLE;
		param->sched_priority = priority - 4;
	}
	else if (priority <= 6) // SCHED_BATCH
	{
		*policy = SCHED_BATCH;
		param->sched_priority = priority - 6;
	}
	else if (priority <= 8) // SCHED_RR
	{
		*policy = SCHED_RR;
		param->sched_priority = priority - 8;
	}
	else if (priority <= 10) // SCHED_SPORADIC
	{
		*policy = SCHED_SPORADIC;
		param->sched_priority = priority - 10;
	}
	else
	{
		*policy = SCHED_FIFO;
		param->sched_priority = priority - 13;
	}

	// Bound the maximum value of sched_priority. This should only happen
	// if the thread is running with realtime priority.
	if (param->sched_priority > 2)
	{
		param->sched_priority = 2;
	}

	// In this case reduce the policy class and adjust priority accordingly.
	if (param->sched_priority < -2)
	{
		if (*policy == SCHED_IDLE)
		{
			param->sched_priority = -2;
		}
		else if (*policy == SCHED_BATCH)
		{
			*policy = SCHED_IDLE;
			param->sched_priority = priority - 4;
		}
		else if (*policy == SCHED_RR)
		{
			*policy = SCHED_BATCH;
			param->sched_priority = priority - 6;
		}
		else if (*policy == SCHED_SPORADIC)
		{
			*policy = SCHED_RR;
			param->sched_priority = priority - 8;
		}
		else if (*policy == SCHED_FIFO)
		{
			*policy = SCHED_SPORADIC;
			param->sched_priority = priority - 10;
		}
	}

	return 0;
}

int wlibc_thread_setschedparam(thread_t thread, int policy, const struct sched_param *param)
{
	NTSTATUS status;
	KPRIORITY priority = SCHED_RR;
	threadinfo *tinfo = (threadinfo *)thread;

	VALIDATE_THREAD(thread);
	VALIDATE_PTR(param, EINVAL, -1);
	VALIDATE_SCHED_PRIORITY(param->sched_priority, EINVAL, -1);
	VALIDATE_SCHED_POLICY(policy, EINVAL, -1);

	// Base priority of policies.
	switch (policy)
	{
	case SCHED_IDLE:
		priority = 4;
		break;
	case SCHED_RR:
		priority = 8;
		break;
	case SCHED_FIFO:
		priority = 13;
		break;
	case SCHED_BATCH:
		priority = 6;
		break;
	case SCHED_SPORADIC:
		priority = 10;
		break;
	}

	priority += param->sched_priority;

	// This sets the absolute priority of the thread.
	status = NtSetInformationThread(tinfo->handle, ThreadPriority, &priority, sizeof(KPRIORITY));
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}

int wlibc_thread_getschedpriority(thread_t thread, int *priority)
{
	int status;
	int policy;
	struct sched_param param;

	VALIDATE_THREAD(thread);
	VALIDATE_PTR(priority, EINVAL, -1);

	status = wlibc_thread_getschedparam(thread, &policy, &param);

	*priority = param.sched_priority;

	return status;
}

int wlibc_thread_setschedpriority(thread_t thread, int priority)
{
	NTSTATUS status;
	KPRIORITY change = priority;
	threadinfo *tinfo = (threadinfo *)thread;

	VALIDATE_THREAD(thread);

	if (priority > SCHED_MAX_PRIORITY || priority < SCHED_MIN_PRIORITY)
	{
		errno = EINVAL;
		return -1;
	}

	// This will change the priority of the thread according to 'new_priority = old_priority + change'.
	status = NtSetInformationThread(tinfo->handle, ThreadChangePriority, &change, sizeof(KPRIORITY));
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}

int wlibc_thread_getname(thread_t thread, char *buffer, size_t length)
{
	NTSTATUS status;
	CHAR u16_buffer[80]; // 16 bytes UNICODE_STRING, 64 bytes name.
	UNICODE_STRING *u16_name = (UNICODE_STRING *)u16_buffer;
	UTF8_STRING u8_name;
	threadinfo *tinfo = (threadinfo *)thread;

	VALIDATE_THREAD(thread);
	VALIDATE_PTR(buffer, EINVAL, -1);

	if (length > 65536) // Limit of UNICODE_STRING.
	{
		errno = ERANGE;
		return -1;
	}

	status = NtQueryInformationThread(tinfo->handle, ThreadNameInformation, u16_buffer, sizeof(u16_buffer), NULL);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	u8_name.Length = 0;
	u8_name.MaximumLength = (USHORT)length;
	u8_name.Buffer = buffer;

	status = RtlUnicodeStringToUTF8String(&u8_name, u16_name, FALSE);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}

int wlibc_thread_setname(thread_t thread, const char *name)
{
	NTSTATUS status;
	UTF8_STRING u8_name;
	UNICODE_STRING u16_name;
	threadinfo *tinfo = (threadinfo *)thread;

	VALIDATE_THREAD(thread);

	if (name != NULL)
	{
		RtlInitUTF8String(&u8_name, name);

		if (u8_name.MaximumLength > 32)
		{
			errno = E2BIG;
			return -1;
		}

		status = RtlUTF8StringToUnicodeString(&u16_name, &u8_name, TRUE);
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}
	}
	else
	{
		u16_name.Length = 0;
		u16_name.MaximumLength = 0;
		u16_name.Buffer = NULL;
	}

	status = NtSetInformationThread(tinfo->handle, ThreadNameInformation, &u16_name, sizeof(UNICODE_STRING));

	if (name != NULL)
	{
		RtlFreeUnicodeString(&u16_name);
	}

	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}

int wlibc_thread_getaffinity(thread_t thread, cpu_set_t *cpuset)
{
	NTSTATUS status;
	LONGLONG mask;
	threadinfo *tinfo = (threadinfo *)thread;
	// TODO: Scale this beyond 64 cores.

	VALIDATE_THREAD(thread);

	if (cpuset == NULL || cpuset->num_groups == 0)
	{
		errno = EINVAL;
		return -1;
	}

	status = NtQueryInformationThread(tinfo->handle, ThreadSelectedCpuSets, &mask, sizeof(LONGLONG), NULL);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	cpuset->group_mask[0] = mask;

	return 0;
}

int wlibc_thread_setaffinity(thread_t thread, const cpu_set_t *cpuset)
{
	NTSTATUS status;
	LONGLONG mask;
	threadinfo *tinfo = (threadinfo *)thread;
	// TODO: Scale this beyond 64 cores.

	VALIDATE_THREAD(thread);

	if (cpuset == NULL || cpuset->num_groups == 0)
	{
		errno = EINVAL;
		return -1;
	}

	mask = cpuset->group_mask[0];

	status = NtSetInformationThread(tinfo->handle, ThreadSelectedCpuSets, &mask, sizeof(LONGLONG));
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}
