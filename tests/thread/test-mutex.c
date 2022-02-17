/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <test-macros.h>
#include <pthread.h>
#include <unistd.h>

static int variable = 0;

void *lock(void *arg)
{
	pthread_mutex_t *mutex = (pthread_mutex_t *)arg;

	pthread_mutex_lock(mutex);
	++variable;
	pthread_mutex_unlock(mutex);
	return NULL;
}

int test_mutex_basic()
{
	int status;
	void *result;
	pthread_t thread;
	pthread_mutex_t mutex;

	status = pthread_mutex_init(&mutex, NULL);
	ASSERT_EQ(status, 0);

	status = pthread_mutex_lock(&mutex);
	ASSERT_EQ(status, 0);

	status = pthread_create(&thread, NULL, lock, (void *)&mutex);
	ASSERT_EQ(status, 0);

	// Main thread is holding the mutex, variable should not be modified.
	ASSERT_EQ(variable, 0);

	usleep(1000);

	// Main thread is still holding the mutex.
	ASSERT_EQ(variable, 0);

	status = pthread_mutex_unlock(&mutex);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread, &result);
	ASSERT_EQ(status, 0);

	// Thread has exited now, variable must have been updated.
	ASSERT_EQ(variable, 1);

	status = pthread_mutex_destroy(&mutex);
	ASSERT_EQ(status, 0);

	// Destroying an already destroyed mutex should fail.
	status = pthread_mutex_destroy(&mutex);
	ASSERT_EQ(status, -1);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();
	TEST(test_mutex_basic());
	VERIFY_RESULT_AND_EXIT();
}
