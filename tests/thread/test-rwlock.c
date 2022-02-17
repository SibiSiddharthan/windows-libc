/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <test-macros.h>
#include <pthread.h>
#include <unistd.h>

static int variable = 0;
static int test_variable = 0;

void *shared(void *arg)
{
	pthread_rwlock_t *rwlock = (pthread_rwlock_t *)arg;

	pthread_rwlock_rdlock(rwlock);

	if (variable == 0)
	{
		test_variable = 1;
	}

	pthread_rwlock_unlock(rwlock);

	return NULL;
}

void *exclusive(void *arg)
{
	pthread_rwlock_t *rwlock = (pthread_rwlock_t *)arg;

	pthread_rwlock_wrlock(rwlock);

	test_variable = 2;

	pthread_rwlock_unlock(rwlock);

	return NULL;
}

int test_rwlock_shared()
{
	int status;
	void *result;
	pthread_t thread;
	pthread_rwlock_t rwlock;

	status = pthread_rwlock_init(&rwlock, NULL);
	ASSERT_EQ(status, 0);

	status = pthread_rwlock_rdlock(&rwlock);
	ASSERT_EQ(status, 0);

	status = pthread_create(&thread, NULL, shared, (void *)&rwlock);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread, &result);
	ASSERT_EQ(status, 0);

	// Test variable should be updated.
	ASSERT_EQ(test_variable, 1);

	status = pthread_rwlock_unlock(&rwlock);
	ASSERT_EQ(status, 0);

	status = pthread_rwlock_destroy(&rwlock);
	ASSERT_EQ(status, 0);

	return 0;
}

int test_rwlock_exclusive()
{
	int status;
	void *result;
	pthread_t thread;
	pthread_rwlock_t rwlock;

	status = pthread_rwlock_init(&rwlock, NULL);
	ASSERT_EQ(status, 0);

	status = pthread_rwlock_wrlock(&rwlock);
	ASSERT_EQ(status, 0);

	status = pthread_create(&thread, NULL, exclusive, (void *)&rwlock);
	ASSERT_EQ(status, 0);

	// Main thread is holding an exclusive lock, variable should not be modified.
	ASSERT_EQ(test_variable, 1);

	usleep(1000);

	// Main thread is still holding the lock.
	ASSERT_EQ(test_variable, 1);

	status = pthread_rwlock_unlock(&rwlock);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread, &result);
	ASSERT_EQ(status, 0);

	// Test variable should be updated now.
	ASSERT_EQ(test_variable, 2);

	status = pthread_rwlock_destroy(&rwlock);
	ASSERT_EQ(status, 0);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();
	TEST(test_rwlock_shared());
	TEST(test_rwlock_exclusive());
	VERIFY_RESULT_AND_EXIT();
}
