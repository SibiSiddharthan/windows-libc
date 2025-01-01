/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <pthread.h>

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

int test_once()
{
	int status;
	void *result;
	pthread_t thread_1, thread_2;
	pthread_once_t once = PTHREAD_ONCE_INIT;

	status = pthread_create(&thread_1, NULL, pthread_function, (void *)&once);
	ASSERT_EQ(status, 0);

	status = pthread_create(&thread_2, NULL, pthread_function, (void *)&once);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread_1, &result);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(result, NULL);

	status = pthread_join(thread_2, &result);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(result, NULL);

	ASSERT_EQ(test_variable, 1);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();
	TEST(test_once());
	VERIFY_RESULT_AND_EXIT();
}
