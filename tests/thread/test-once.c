/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <test-macros.h>
#include <pthread.h>
#include <threads.h>

static int test_variable = 0;

void init_function(void)
{
	++test_variable;
}

void *pthread_function(void *once)
{
	pthread_once((pthread_once_t *)once, init_function);
	return NULL;
}

int thrd_function(void *once)
{
	call_once((once_flag *)once, init_function);
	return 1;
}

int test_once_posix()
{
	int status;
	void *result;
	pthread_t t1, t2;
	pthread_once_t once = PTHREAD_ONCE_INIT;

	status = pthread_create(&t1, NULL, pthread_function, (void *)&once);
	ASSERT_EQ(status, 0);

	status = pthread_create(&t2, NULL, pthread_function, (void *)&once);
	ASSERT_EQ(status, 0);

	status = pthread_join(t1, &result);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(result, NULL);

	status = pthread_join(t2, &result);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(result, NULL);

	ASSERT_EQ(test_variable, 1);

	return 0;
}

int test_once_c11()
{
	int status;
	int result;
	thrd_t t1, t2;
	once_flag once = ONCE_FLAG_INIT;

	status = thrd_create(&t1, thrd_function, (void *)&once);
	ASSERT_EQ(status, 0);

	status = thrd_create(&t2, thrd_function, (void *)&once);
	ASSERT_EQ(status, 0);

	status = thrd_join(t1, &result);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(result, 1);

	status = thrd_join(t2, &result);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(result, 1);

	ASSERT_EQ(test_variable, 2);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();
	TEST(test_once_posix());
	TEST(test_once_c11());
	VERIFY_RESULT_AND_EXIT();
}
