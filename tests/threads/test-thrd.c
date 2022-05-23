/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <threads.h>

typedef struct _args_struct
{
	int a, b;
} args_struct;

int func(void *arg)
{
	args_struct *args = (args_struct *)arg;
	return (args->a + args->b);
}

int empty(void *arg WLIBC_UNUSED)
{
	return 0;
}

int test_thread_basic()
{
	int status;
	int result;
	args_struct args;
	thrd_t thread;

	args.a = 5;
	args.b = 10;

	status = thrd_create(&thread, func, (void *)&args);
	ASSERT_EQ(status, 0);

	status = thrd_join(thread, &result);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(result, 15);

	return 0;
}

int test_join_detach()
{
	int status;
	thrd_t thread;

	status = thrd_create(&thread, empty, NULL);
	ASSERT_EQ(status, 0);

	status = thrd_detach(thread);
	ASSERT_EQ(status, 0);

	// Joining a detached thread should fail.
	status = thrd_join(thread, NULL);
	ASSERT_EQ(status, -1);

	// Repeated detach should also fail.
	status = thrd_detach(thread);
	ASSERT_EQ(status, -1);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_thread_basic());
	TEST(test_join_detach());

	VERIFY_RESULT_AND_EXIT();
}
