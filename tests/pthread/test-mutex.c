/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <pthread.h>
#include <sys/time.h>
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

void *timedlock(void *arg)
{
	pthread_mutex_t *mutex = (pthread_mutex_t *)arg;
	struct timeval current_time;
	struct timespec abstime;

	gettimeofday(&current_time, NULL);

	abstime.tv_sec = current_time.tv_sec;
	abstime.tv_nsec = (current_time.tv_usec * 1000) % 999999999;

	// Wait for 10 microseconds.
	// Don't worry about the tv_nsec overflow, it doesn't matter.
	abstime.tv_nsec += 10000;

	if (pthread_mutex_timedlock(mutex, &abstime) == 0)
	{
		++variable;
		pthread_mutex_unlock(mutex);
	}

	return NULL;
}

void *trylock(void *arg)
{
	pthread_mutex_t *mutex = (pthread_mutex_t *)arg;

	if (pthread_mutex_trylock(mutex) == 0)
	{
		++variable;
		pthread_mutex_unlock(mutex);
	}
	return NULL;
}

int test_mutex_basic()
{
	int status;
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

	status = pthread_join(thread, NULL);
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

int test_mutex_recursive()
{
	int status;
	pthread_mutex_t mutex;
	pthread_mutexattr_t attr;

	status = pthread_mutexattr_init(&attr);
	ASSERT_EQ(status, 0);

	status = pthread_mutex_init(&mutex, &attr);
	ASSERT_EQ(status, 0);

	status = pthread_mutex_lock(&mutex);
	ASSERT_EQ(status, 0);

	// Recursive lock should fail.
	errno = 0;
	status = pthread_mutex_lock(&mutex);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EDEADLK);

	status = pthread_mutex_unlock(&mutex);
	ASSERT_EQ(status, 0);

	// Not a recursive mutex, unlocking it again should fail.
	errno = 0;
	status = pthread_mutex_unlock(&mutex);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EPERM);

	status = pthread_mutex_destroy(&mutex);
	ASSERT_EQ(status, 0);

	// Do the above all over again but with a recursive mutex.
	status = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	ASSERT_EQ(status, 0);

	status = pthread_mutex_init(&mutex, &attr);
	ASSERT_EQ(status, 0);

	status = pthread_mutex_lock(&mutex);
	ASSERT_EQ(status, 0);

	// Recursive lock, should succeed.
	status = pthread_mutex_lock(&mutex);
	ASSERT_EQ(status, 0);

	status = pthread_mutex_unlock(&mutex);
	ASSERT_EQ(status, 0);

	// Second unlock should succeed.
	status = pthread_mutex_unlock(&mutex);
	ASSERT_EQ(status, 0);

	status = pthread_mutex_destroy(&mutex);
	ASSERT_EQ(status, 0);

	status = pthread_mutexattr_destroy(&attr);
	ASSERT_EQ(status, 0);

	return 0;
}

int test_mutex_timed()
{
	int status;
	pthread_t thread;
	pthread_mutex_t mutex;

	status = pthread_mutex_init(&mutex, NULL);
	ASSERT_EQ(status, 0);

	status = pthread_mutex_lock(&mutex);
	ASSERT_EQ(status, 0);

	status = pthread_create(&thread, NULL, timedlock, (void *)&mutex);
	ASSERT_EQ(status, 0);

	usleep(1000);

	status = pthread_mutex_unlock(&mutex);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread, NULL);
	ASSERT_EQ(status, 0);

	// Thread has exited now, it should have failed while trying to acquire the mutex
	// and the varaible should not be updated.
	ASSERT_EQ(variable, 0);

	status = pthread_mutex_destroy(&mutex);
	ASSERT_EQ(status, 0);

	return 0;
}

int test_mutex_try()
{
	int status;
	pthread_t thread;
	pthread_mutex_t mutex;

	status = pthread_mutex_init(&mutex, NULL);
	ASSERT_EQ(status, 0);

	status = pthread_mutex_trylock(&mutex);
	ASSERT_EQ(status, 0);

	status = pthread_create(&thread, NULL, trylock, (void *)&mutex);
	ASSERT_EQ(status, 0);

	usleep(1000);

	status = pthread_join(thread, NULL);
	ASSERT_EQ(status, 0);

	status = pthread_mutex_unlock(&mutex);
	ASSERT_EQ(status, 0);

	// Thread has exited now, it should have failed while trying to acquire the mutex
	// and the varaible should not be updated.
	ASSERT_EQ(variable, 0);

	status = pthread_mutex_destroy(&mutex);
	ASSERT_EQ(status, 0);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();
	TEST(test_mutex_basic());
	TEST(test_mutex_recursive());
	variable = 0;
	TEST(test_mutex_timed());
	TEST(test_mutex_try());
	VERIFY_RESULT_AND_EXIT();
}
