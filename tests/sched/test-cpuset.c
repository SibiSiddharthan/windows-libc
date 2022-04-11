/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <sched.h>

int test_cpuset_alloc_size()
{
	ASSERT_EQ(CPU_ALLOC_SIZE(2), 16);
	ASSERT_EQ(CPU_ALLOC_SIZE(64), 16);
	ASSERT_EQ(CPU_ALLOC_SIZE(65), 24);

	return 0;
}

int test_cpuset_unary_ops()
{
	cpu_set_t *cpuset = NULL;

	cpuset = CPU_ALLOC(8);
	ASSERT_NOTNULL(cpuset);

	CPU_ZERO(cpuset);
	ASSERT_EQ(cpuset->group_mask[0], 0);

	CPU_SET(1, cpuset);
	ASSERT_EQ(CPU_ISSET(1, cpuset), 1);
	ASSERT_EQ(cpuset->group_mask[0], 2);

	CPU_CLR(2, cpuset);
	ASSERT_EQ(CPU_ISSET(1, cpuset), 1);
	ASSERT_EQ(cpuset->group_mask[0], 2);

	CPU_ZERO(cpuset);
	ASSERT_EQ(CPU_ISSET(1, cpuset), 0);
	ASSERT_EQ(cpuset->group_mask[0], 0);

	CPU_SET(1, cpuset);
	ASSERT_EQ(CPU_ISSET(1, cpuset), 1);
	ASSERT_EQ(cpuset->group_mask[0], 2);

	CPU_CLR(1, cpuset);
	ASSERT_EQ(CPU_ISSET(1, cpuset), 0);
	ASSERT_EQ(cpuset->group_mask[0], 0);

	CPU_FREE(cpuset);

	return 0;
}

int test_cpuset_binary_ops()
{
	cpu_set_t *set1 = NULL, *set2 = NULL, *set3 = NULL, *set4 = NULL;

	set1 = CPU_ALLOC(8);
	ASSERT_NOTNULL(set1);

	set2 = CPU_ALLOC(8);
	ASSERT_NOTNULL(set2);

	set3 = CPU_ALLOC(8);
	ASSERT_NOTNULL(set3);

	set4 = CPU_ALLOC(8);
	ASSERT_NOTNULL(set4);

	CPU_ZERO(set1);
	CPU_ZERO(set2);
	CPU_ZERO(set3);

	CPU_SET(0, set1);
	CPU_SET(1, set1);
	ASSERT_EQ(set1->group_mask[0], 3);

	CPU_SET(1, set2);
	CPU_SET(2, set2);
	ASSERT_EQ(set2->group_mask[0], 6);

	CPU_ZERO(set4);

	CPU_AND(set3, set1, set2);
	ASSERT_EQ(set3->group_mask[0], 2);
	CPU_SET(1, set4);
	ASSERT_EQ(CPU_EQUAL(set3, set4), 1);

	CPU_ZERO(set4);

	CPU_OR(set3, set1, set2);
	ASSERT_EQ(set3->group_mask[0], 7);
	CPU_SET(0, set4);
	CPU_SET(1, set4);
	CPU_SET(2, set4);
	ASSERT_EQ(CPU_EQUAL(set3, set4), 1);

	CPU_ZERO(set4);

	CPU_XOR(set3, set1, set2);
	ASSERT_EQ(set3->group_mask[0], 5);
	CPU_SET(0, set4);
	CPU_SET(2, set4);
	ASSERT_EQ(CPU_EQUAL(set3, set4), 1);

	CPU_FREE(set1);
	CPU_FREE(set2);
	CPU_FREE(set3);
	CPU_FREE(set4);

	return 0;
}

int test_cpuset_count()
{
	cpu_set_t *cpuset = NULL;

	cpuset = CPU_ALLOC(8);
	ASSERT_NOTNULL(cpuset);

	CPU_ZERO(cpuset);
	ASSERT_EQ(CPU_COUNT(cpuset), 0);

	CPU_SET(0, cpuset);
	ASSERT_EQ(CPU_COUNT(cpuset), 1);

	CPU_SET(1, cpuset);
	ASSERT_EQ(CPU_COUNT(cpuset), 2);

	CPU_SET(4, cpuset);
	ASSERT_EQ(CPU_COUNT(cpuset), 3);

	CPU_SET(7, cpuset);
	ASSERT_EQ(CPU_COUNT(cpuset), 4);

	// Trying to set a cpu beyond the allocated limit should fail.
	errno = 0;
	CPU_SET(8, cpuset);
	ASSERT_ERRNO(EINVAL);
	ASSERT_EQ(CPU_COUNT(cpuset), 4);

	CPU_FREE(cpuset);

	return 0;
}

int test_cpuset_large()
{
	cpu_set_t *cpuset = NULL;

	cpuset = CPU_ALLOC(128);
	ASSERT_NOTNULL(cpuset);

	CPU_ZERO(cpuset);
	ASSERT_EQ(cpuset->num_groups, 2);
	ASSERT_EQ(cpuset->group_mask[0], 0);
	ASSERT_EQ(cpuset->group_mask[1], 0);

	CPU_SET(0, cpuset);
	CPU_SET(64, cpuset);
	ASSERT_EQ(CPU_ISSET(64, cpuset), 1);
	ASSERT_EQ(cpuset->group_mask[0], 1);
	ASSERT_EQ(cpuset->group_mask[1], 1);

	CPU_CLR(64, cpuset);
	ASSERT_EQ(CPU_ISSET(64, cpuset), 0);
	ASSERT_EQ(cpuset->group_mask[0], 1);
	ASSERT_EQ(cpuset->group_mask[1], 0);

	CPU_ZERO(cpuset);
	ASSERT_EQ(cpuset->group_mask[0], 0);
	ASSERT_EQ(cpuset->group_mask[1], 0);

	CPU_SET(32, cpuset);
	ASSERT_EQ(CPU_ISSET(32, cpuset), 1);
	CPU_SET(64, cpuset);
	ASSERT_EQ(CPU_ISSET(64, cpuset), 1);
	CPU_SET(80, cpuset);
	ASSERT_EQ(CPU_ISSET(80, cpuset), 1);
	CPU_SET(100, cpuset);
	ASSERT_EQ(CPU_ISSET(100, cpuset), 1);

	ASSERT_EQ(CPU_COUNT(cpuset), 4);

	CPU_FREE(cpuset);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_cpuset_alloc_size());
	TEST(test_cpuset_unary_ops());
	TEST(test_cpuset_binary_ops());
	TEST(test_cpuset_count());
	TEST(test_cpuset_large());

	VERIFY_RESULT_AND_EXIT();
}
