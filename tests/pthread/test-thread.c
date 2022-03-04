/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <test-macros.h>
#include <pthread.h>

typedef struct _args_struct
{
	int a, b;
} args_struct;

void *func(void *arg)
{
	args_struct *args = (args_struct *)arg;
	return (void *)(intptr_t)(args->a + args->b);
}

void *empty(void *arg)
{
	return NULL;
}

int test_thread_basic()
{
	int status;
	void *result;
	args_struct args;
	pthread_t thread;

	args.a = 5;
	args.b = 10;

	status = pthread_create(&thread, NULL, func, (void *)&args);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread, &result);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(result, 15);

	return 0;
}

int test_join_detach()
{
	int status;
	pthread_t thread;

	status = pthread_create(&thread, NULL, empty, NULL);
	ASSERT_EQ(status, 0);

	status = pthread_detach(thread);
	ASSERT_EQ(status, 0);

	// Joining a detached thread should fail.
	status = pthread_join(thread, NULL);
	ASSERT_EQ(status, -1);

	// Repeated detach should also fail.
	status = pthread_detach(thread);
	ASSERT_EQ(status, -1);

	return 0;
}

int test_attributes()
{
	int status;
	pthread_t thread;
	pthread_attr_t attr;

	status = pthread_attr_init(&attr);
	ASSERT_EQ(status, 0);

	status = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	ASSERT_EQ(status, 0);
	status = pthread_attr_setstacksize(&attr, 16384); // 16KB
	ASSERT_EQ(status, 0);

	status = pthread_create(&thread, &attr, empty, NULL);
	ASSERT_EQ(status, 0);

	// Already detached thread, detaching again should fail.
	status = pthread_detach(thread);
	ASSERT_EQ(status, -1);

	status = pthread_attr_destroy(&attr);
	ASSERT_EQ(status, 0);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();
	TEST(test_thread_basic());
	TEST(test_join_detach());
	TEST(test_attributes());
	VERIFY_RESULT_AND_EXIT();
}
