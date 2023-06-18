/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <tests/test.h>
#include <sched.h>
#include <stdbool.h>
#include <unistd.h>

static bool have_increase_base_priority_privilege = false;

static void check_for_increase_base_priority_privilege()
{
	NTSTATUS status;
	ULONG privilege = SE_INC_BASE_PRIORITY_PRIVILEGE;
	PVOID state;

	status = RtlAcquirePrivilege(&privilege, 1, 0, &state);
	if (status == STATUS_SUCCESS)
	{
		have_increase_base_priority_privilege = true;
		RtlReleasePrivilege(state);
	}
}

int test_affinity()
{
	int status;
	cpu_set_t *cpuset, *result;
	pid_t pid;

	cpuset = CPU_ALLOC(2);
	result = CPU_ALLOC(2);

	CPU_ZERO(cpuset);
	// We can only set affinity to core 0, as this will always exist.
	CPU_SET(0, cpuset);

	// Setting affinity for the current process.
	pid = getpid();

	status = sched_getaffinity(pid, 16, result);
	ASSERT_EQ(status, 0);

	// Process should have no default affinity.
	ASSERT_EQ(CPU_ISSET(0, result), 0);
	ASSERT_EQ(CPU_ISSET(1, result), 0);

	status = sched_setaffinity(pid, 16, cpuset);
	ASSERT_EQ(status, 0);

	// pid = 0, means current process.
	status = sched_getaffinity(0, 16, result);
	ASSERT_EQ(status, 0);
	// Check to see whether setting affinity works.
	ASSERT_EQ(CPU_ISSET(0, result), 1);
	ASSERT_EQ(CPU_ISSET(1, result), 0);

	CPU_FREE(cpuset);
	CPU_FREE(result);

	return 0;
}

static int test_each_policy(pid_t pid, int policy)
{
	int status;
	struct sched_param param, result;

	for (int i = sched_get_priority_min(policy); i <= sched_get_priority_max(policy); ++i)
	{
		param.sched_priority = i;

		status = sched_setparam(pid, &param);
		ASSERT_EQ(status, 0);

		status = sched_getparam(pid, &result);
		ASSERT_EQ(status, 0);

		if (have_increase_base_priority_privilege)
		{
			// Only check this assertion if we have 'SeIncreaseBasePriorityPrivilege'.
			ASSERT_EQ(result.sched_priority, param.sched_priority);
		}
	}

	return 0;
}

int test_sched()
{
	int status;
	int policy;
	pid_t pid;
	struct sched_param param;

	pid = getpid();

	// SCHED_IDLE
	status = sched_setscheduler(0, SCHED_IDLE, NULL);
	ASSERT_EQ(status, 0);

	policy = sched_getscheduler(pid);
	ASSERT_EQ(policy, SCHED_IDLE);

	ASSERT_SUCCESS(test_each_policy(pid, SCHED_IDLE));

	// SCHED_RR
	param.sched_priority = 0;
	status = sched_setscheduler(0, SCHED_RR, &param);
	ASSERT_EQ(status, 0);

	policy = sched_getscheduler(pid);
	ASSERT_EQ(policy, SCHED_RR);

	ASSERT_SUCCESS(test_each_policy(pid, SCHED_RR));

	// SCHED_FIFO
	param.sched_priority = -1;
	status = sched_setscheduler(0, SCHED_FIFO, &param);
	ASSERT_EQ(status, 0);

	policy = sched_getscheduler(pid);
	ASSERT_EQ(policy, SCHED_FIFO);

	ASSERT_SUCCESS(test_each_policy(pid, SCHED_FIFO));

	// SCHED_BATCH
	param.sched_priority = 1;
	status = sched_setscheduler(0, SCHED_BATCH, &param);
	ASSERT_EQ(status, 0);

	policy = sched_getscheduler(pid);
	ASSERT_EQ(policy, SCHED_BATCH);

	ASSERT_SUCCESS(test_each_policy(pid, SCHED_BATCH));

	// SCHED_SPORADIC
	status = sched_setscheduler(0, SCHED_SPORADIC, NULL);
	ASSERT_EQ(status, 0);

	policy = sched_getscheduler(pid);
	ASSERT_EQ(policy, SCHED_SPORADIC);

	ASSERT_SUCCESS(test_each_policy(pid, SCHED_SPORADIC));

	return 0;
}

int test_error()
{
	int status;
	int policy;
	struct sched_param param, result;

	param.sched_priority = 0;
	status = sched_setscheduler(0, SCHED_RR, &param);
	ASSERT_EQ(status, 0);

	policy = sched_getscheduler(0);
	ASSERT_EQ(policy, SCHED_RR);

	status = sched_getparam(0, &result);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(result.sched_priority, param.sched_priority);

	// Attempting to set priority higher than allowed.
	errno = 0;
	param.sched_priority = SCHED_MAX_PRIORITY + 1;
	status = sched_setscheduler(0, SCHED_FIFO, &param);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EINVAL);

	policy = sched_getscheduler(0);
	ASSERT_EQ(policy, SCHED_RR);

	status = sched_getparam(0, &result);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(result.sched_priority, 0);

	errno = 0;
	status = sched_setparam(0, &param);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EINVAL);

	status = sched_getparam(0, &result);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(result.sched_priority, 0);

	// Attempting to set priority lower than allowed.
	errno = 0;
	param.sched_priority = SCHED_MIN_PRIORITY - 1;
	status = sched_setscheduler(0, SCHED_IDLE, &param);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EINVAL);

	policy = sched_getscheduler(0);
	ASSERT_EQ(policy, SCHED_RR);

	status = sched_getparam(0, &result);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(result.sched_priority, 0);

	errno = 0;
	status = sched_setparam(0, &param);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EINVAL);

	status = sched_getparam(0, &result);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(result.sched_priority, 0);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();
	check_for_increase_base_priority_privilege();

	TEST(test_affinity());
	TEST(test_sched());
	TEST(test_error());

	VERIFY_RESULT_AND_EXIT();
}
