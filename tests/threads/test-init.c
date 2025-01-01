/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <threads.h>

static int test_variable = 0;

void init_function(void)
{
	++test_variable;
}

int thrd_function(void *once)
{
	call_once((once_flag *)once, init_function);
	return 1;
}

int test_init()
{
	int status;
	int result;
	thrd_t thread_1, thread_2;
	once_flag once = ONCE_FLAG_INIT;

	status = thrd_create(&thread_1, thrd_function, (void *)&once);
	ASSERT_EQ(status, 0);

	status = thrd_create(&thread_2, thrd_function, (void *)&once);
	ASSERT_EQ(status, 0);

	status = thrd_join(thread_1, &result);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(result, 1);

	status = thrd_join(thread_2, &result);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(result, 1);

	ASSERT_EQ(test_variable, 1);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();
	TEST(test_init());
	VERIFY_RESULT_AND_EXIT();
}
