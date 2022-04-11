/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <sched.h>
#include <unistd.h>

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
	struct sched_param param;

	for (int i = sched_get_priority_min(policy); i <= sched_get_priority_max(policy); ++i)
	{
		param.sched_priority = i;

		status = sched_setparam(pid, &param);
		ASSERT_EQ(status, 0);

#if 0 // TODO
		status = sched_getparam(pid, &result);
		ASSERT_EQ(status, 0);

		ASSERT_EQ(result.sched_priority, param.sched_priority);
#endif
	}

	return 0;
}

int test_sched()
{
	int status;
	int policy;
	pid_t pid;

	pid = getpid();

	// SCHED_IDLE
	status = sched_setscheduler(0, SCHED_IDLE, NULL);
	ASSERT_EQ(status, 0);

	policy = sched_getscheduler(pid);
	ASSERT_EQ(policy, SCHED_IDLE);

	ASSERT_SUCCESS(test_each_policy(pid, SCHED_IDLE));

	// SCHED_RR
	status = sched_setscheduler(0, SCHED_RR, NULL);
	ASSERT_EQ(status, 0);

	policy = sched_getscheduler(pid);
	ASSERT_EQ(policy, SCHED_RR);

	ASSERT_SUCCESS(test_each_policy(pid, SCHED_RR));

	// SCHED_FIFO
	status = sched_setscheduler(0, SCHED_FIFO, NULL);
	ASSERT_EQ(status, 0);

	policy = sched_getscheduler(pid);
	ASSERT_EQ(policy, SCHED_FIFO);

	ASSERT_SUCCESS(test_each_policy(pid, SCHED_FIFO));

	// SCHED_BATCH
	status = sched_setscheduler(0, SCHED_BATCH, NULL);
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

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_affinity());
	TEST(test_sched());

	VERIFY_RESULT_AND_EXIT();
}
