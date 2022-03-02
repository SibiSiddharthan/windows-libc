/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <test-macros.h>
#include <pthread.h>
#include <sys/time.h>
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

void *try_shared(void *arg)
{
	pthread_rwlock_t *rwlock = (pthread_rwlock_t *)arg;

	if (pthread_rwlock_tryrdlock(rwlock) == 0)
	{
		if (variable == 0)
		{
			++test_variable;
		}

		pthread_rwlock_unlock(rwlock);
	}
	return NULL;
}

void *try_exclusive(void *arg)
{
	pthread_rwlock_t *rwlock = (pthread_rwlock_t *)arg;

	if (pthread_rwlock_trywrlock(rwlock) == 0)
	{
		++test_variable;
		pthread_rwlock_unlock(rwlock);
	}

	return NULL;
}

void *timed_exclusive(void *arg)
{
	pthread_rwlock_t *rwlock = (pthread_rwlock_t *)arg;
	struct timeval current_time;
	struct timespec abstime;

	gettimeofday(&current_time, NULL);

	abstime.tv_sec = current_time.tv_sec;
	abstime.tv_nsec = (current_time.tv_usec * 1000) % 999999999;

	// Wait for 10 microseconds.
	// Don't worry about the tv_nsec overflow, it doesn't matter.
	abstime.tv_nsec += 10000;

	if (pthread_rwlock_timedwrlock(rwlock, &abstime) == 0)
	{
		test_variable = 4;
		pthread_rwlock_unlock(rwlock);
	}

	return NULL;
}

int test_rwlock_shared()
{
	int status;
	pthread_t thread;
	pthread_rwlock_t rwlock;

	status = pthread_rwlock_init(&rwlock, NULL);
	ASSERT_EQ(status, 0);

	status = pthread_rwlock_rdlock(&rwlock);
	ASSERT_EQ(status, 0);

	status = pthread_create(&thread, NULL, shared, (void *)&rwlock);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread, NULL);
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

	status = pthread_join(thread, NULL);
	ASSERT_EQ(status, 0);

	// Test variable should be updated now.
	ASSERT_EQ(test_variable, 2);

	status = pthread_rwlock_destroy(&rwlock);
	ASSERT_EQ(status, 0);

	return 0;
}

int test_rwlock_try()
{
	int status;
	pthread_t thread;
	pthread_rwlock_t rwlock;

	status = pthread_rwlock_init(&rwlock, NULL);
	ASSERT_EQ(status, 0);

	// shared-shared
	status = pthread_rwlock_rdlock(&rwlock);
	ASSERT_EQ(status, 0);

	status = pthread_create(&thread, NULL, try_shared, (void *)&rwlock);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread, NULL);
	ASSERT_EQ(status, 0);

	// Test variable should be updated as two threads can simultaneously share the lock for reading.
	ASSERT_EQ(test_variable, 3);

	// shared-exclusive
	status = pthread_create(&thread, NULL, try_exclusive, (void *)&rwlock);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread, NULL);
	ASSERT_EQ(status, 0);

	// Test variable should not be updated as the lock cannot be exclusively acquired as it is
	// already acquired in a shared state.
	ASSERT_EQ(test_variable, 3);

	status = pthread_rwlock_unlock(&rwlock);
	ASSERT_EQ(status, 0);

	// exclusive-exclusive
	status = pthread_rwlock_wrlock(&rwlock);
	ASSERT_EQ(status, 0);

	status = pthread_create(&thread, NULL, try_exclusive, (void *)&rwlock);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread, NULL);
	ASSERT_EQ(status, 0);

	// Test variable should not be updated as two threads can't simultaneously hold the lock exclusively.
	ASSERT_EQ(test_variable, 3);

	// exclusive-shared
	status = pthread_create(&thread, NULL, try_shared, (void *)&rwlock);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread, NULL);
	ASSERT_EQ(status, 0);

	// Test variable should not be updated as the lock acquired exclusively can't be acquired in a shared way.
	ASSERT_EQ(test_variable, 3);

	status = pthread_rwlock_unlock(&rwlock);
	ASSERT_EQ(status, 0);

	status = pthread_rwlock_destroy(&rwlock);
	ASSERT_EQ(status, 0);

	return 0;
}

int test_rwlock_timed()
{
	int status;
	pthread_t thread;
	pthread_rwlock_t rwlock;

	status = pthread_rwlock_init(&rwlock, NULL);
	ASSERT_EQ(status, 0);

	status = pthread_rwlock_wrlock(&rwlock);
	ASSERT_EQ(status, 0);

	status = pthread_create(&thread, NULL, timed_exclusive, (void *)&rwlock);
	ASSERT_EQ(status, 0);

	// Main thread is holding an exclusive lock, variable should not be modified.
	ASSERT_EQ(test_variable, 3);

	usleep(1000);

	status = pthread_rwlock_unlock(&rwlock);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread, NULL);
	ASSERT_EQ(status, 0);

	// Test variable should be not be updated as the thread failed to acquire the lock.
	ASSERT_EQ(test_variable, 3);

	status = pthread_rwlock_destroy(&rwlock);
	ASSERT_EQ(status, 0);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();
	TEST(test_rwlock_shared());
	TEST(test_rwlock_exclusive());
	TEST(test_rwlock_try());
	TEST(test_rwlock_timed());
	VERIFY_RESULT_AND_EXIT();
}
