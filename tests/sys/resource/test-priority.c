/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <sys/resource.h>

int test_priority()
{
	int status;

	// As we increasing the nice value of the process, we don't need any special privileges to do so.
	for (int i = 1; i <= PRIO_MAX; ++i)
	{
		status = setpriority(PRIO_PROCESS, 0, i);
		ASSERT_EQ(status, 0);

		status = getpriority(PRIO_PROCESS, 0);
		// The status might be negative, make sure errno is 0.
		ASSERT_ERRNO(0);
		ASSERT_EQ(status, i);
	}

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();
	TEST(test_priority());
	VERIFY_RESULT_AND_EXIT();
}
