/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/time.h>
#include <unistd.h>

static int signal_variable = 0;
static int broadcast_variable_1 = 0;
static int broadcast_variable_2 = 0;

#ifndef WLIBC_ASAN_BUILD
static bool asan_build = false;
#else
static bool asan_build = true;
#endif

typedef struct _locking
{
	pthread_cond_t *cond;
	pthread_mutex_t *mutex;
} locking;

void *single(void *arg)
{
	locking *locks = (locking *)arg;

	pthread_mutex_lock(locks->mutex);
	pthread_cond_wait(locks->cond, locks->mutex);

	signal_variable = 1;

	pthread_mutex_unlock(locks->mutex);

	return NULL;
}

void *all_1(void *arg)
{
	locking *locks = (locking *)arg;

	pthread_mutex_lock(locks->mutex);
	pthread_cond_wait(locks->cond, locks->mutex);

	broadcast_variable_1 = 1;

	pthread_mutex_unlock(locks->mutex);

	return NULL;
}

void *all_2(void *arg)
{
	locking *locks = (locking *)arg;

	pthread_mutex_lock(locks->mutex);
	pthread_cond_wait(locks->cond, locks->mutex);

	broadcast_variable_2 = 1;

	pthread_mutex_unlock(locks->mutex);

	return NULL;
}

void *timed(void *arg)
{
	locking *locks = (locking *)arg;
	struct timeval current_time;
	struct timespec abstime;

	gettimeofday(&current_time, NULL);

	abstime.tv_sec = current_time.tv_sec;
	abstime.tv_nsec = (current_time.tv_usec * 1000) % 999999999;

	// Wait for 10 microseconds.
	// Don't worry about the tv_nsec overflow, it doesn't matter.
	abstime.tv_nsec += 10000;

	pthread_mutex_lock(locks->mutex);
	if (pthread_cond_timedwait(locks->cond, locks->mutex, &abstime) == 0)
	{
		signal_variable = 1;
		pthread_mutex_unlock(locks->mutex);
	}

	return NULL;
}

int test_cond_signal()
{
	int status;
	pthread_t thread;
	pthread_cond_t cond;
	pthread_mutex_t mutex;

	status = pthread_mutex_init(&mutex, NULL);
	ASSERT_EQ(status, 0);

	status = pthread_cond_init(&cond, NULL);
	ASSERT_EQ(status, 0);

	locking args = {&cond, &mutex};

	status = pthread_create(&thread, NULL, single, &args);
	ASSERT_EQ(status, 0);

	// Main thread has not signalled the condition variable yet, variable should not be modified.
	ASSERT_EQ(signal_variable, 0);

	usleep(1000);

	// Condition variable still not signalled yet.
	ASSERT_EQ(signal_variable, 0);

	status = pthread_cond_signal(&cond);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread, NULL);
	ASSERT_EQ(status, 0);

	// Variable should be updated now as the condition variable has been signalled.
	ASSERT_EQ(signal_variable, 1);

	status = pthread_cond_destroy(&cond);
	ASSERT_EQ(status, 0);

	status = pthread_mutex_destroy(&mutex);
	ASSERT_EQ(status, 0);

	return 0;
}

int test_cond_broadcast()
{
	int status;
	pthread_t thread_1, thread_2;
	pthread_cond_t cond;
	pthread_mutex_t mutex_1, mutex_2;

	status = pthread_mutex_init(&mutex_1, NULL);
	ASSERT_EQ(status, 0);

	status = pthread_mutex_init(&mutex_2, NULL);
	ASSERT_EQ(status, 0);

	status = pthread_cond_init(&cond, NULL);
	ASSERT_EQ(status, 0);

	locking args_1 = {&cond, &mutex_1};

	status = pthread_create(&thread_1, NULL, all_1, &args_1);
	ASSERT_EQ(status, 0);

	locking args_2 = {&cond, &mutex_2};

	status = pthread_create(&thread_2, NULL, all_2, &args_2);
	ASSERT_EQ(status, 0);

	// Main thread has not signalled the condition variable yet, variables should not be modified.
	ASSERT_EQ(broadcast_variable_1, 0);
	ASSERT_EQ(broadcast_variable_2, 0);

	usleep(1000);

	// Condition variable still not signalled yet.
	ASSERT_EQ(broadcast_variable_1, 0);
	ASSERT_EQ(broadcast_variable_2, 0);

	status = pthread_cond_broadcast(&cond);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread_1, NULL);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread_2, NULL);
	ASSERT_EQ(status, 0);

	// Variables should be updated now as the condition variable has been signalled.
	ASSERT_EQ(broadcast_variable_1, 1);
	ASSERT_EQ(broadcast_variable_2, 1);

	status = pthread_cond_destroy(&cond);
	ASSERT_EQ(status, 0);

	status = pthread_mutex_destroy(&mutex_1);
	ASSERT_EQ(status, 0);

	status = pthread_mutex_destroy(&mutex_2);
	ASSERT_EQ(status, 0);

	return 0;
}

int test_cond_timed()
{
	int status;
	pthread_t thread;
	pthread_cond_t cond;
	pthread_mutex_t mutex;

	status = pthread_mutex_init(&mutex, NULL);
	ASSERT_EQ(status, 0);

	status = pthread_cond_init(&cond, NULL);
	ASSERT_EQ(status, 0);

	locking args = {&cond, &mutex};

	status = pthread_create(&thread, NULL, timed, &args);
	ASSERT_EQ(status, 0);

	// Main thread has not signalled the condition variable yet, variable should not be modified.
	ASSERT_EQ(signal_variable, 0);

	usleep(1000);

	status = pthread_cond_signal(&cond);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread, NULL);
	ASSERT_EQ(status, 0);

	// Variable should not be updated as the wait for the signaling of the condition variable has timedout.
	ASSERT_EQ(signal_variable, 0);

	status = pthread_cond_destroy(&cond);
	ASSERT_EQ(status, 0);

	status = pthread_mutex_destroy(&mutex);
	ASSERT_EQ(status, 0);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();
	int concurrency = pthread_getconcurrency();

	TEST(test_cond_signal());

	// In the CI where each runner is allocated 2 cpus, this test hangs when ASAN is enabled.
	if (!(asan_build && concurrency <= 2))
	{
		TEST(test_cond_broadcast());
	}

	signal_variable = 0;
	TEST(test_cond_timed());
	VERIFY_RESULT_AND_EXIT();
}
