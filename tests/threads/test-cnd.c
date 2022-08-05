/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <threads.h>
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
	cnd_t *cond;
	mtx_t *mutex;
} locking;

int single(void *arg)
{
	locking *locks = (locking *)arg;

	mtx_lock(locks->mutex);
	cnd_wait(locks->cond, locks->mutex);

	signal_variable = 1;

	mtx_unlock(locks->mutex);

	return 0;
}

int all_1(void *arg)
{
	locking *locks = (locking *)arg;

	mtx_lock(locks->mutex);
	cnd_wait(locks->cond, locks->mutex);

	broadcast_variable_1 = 1;

	mtx_unlock(locks->mutex);

	return 0;
}

int all_2(void *arg)
{
	locking *locks = (locking *)arg;

	mtx_lock(locks->mutex);
	cnd_wait(locks->cond, locks->mutex);

	broadcast_variable_2 = 1;

	mtx_unlock(locks->mutex);

	return 0;
}

int timed(void *arg)
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

	mtx_lock(locks->mutex);
	if (cnd_timedwait(locks->cond, locks->mutex, &abstime) == 0)
	{
		signal_variable = 1;
		mtx_unlock(locks->mutex);
	}

	return 0;
}

int test_cnd_signal()
{
	int status;
	thrd_t thread;
	cnd_t cond;
	mtx_t mutex;

	status = mtx_init(&mutex, mtx_plain);
	ASSERT_EQ(status, 0);

	status = cnd_init(&cond);
	ASSERT_EQ(status, 0);

	locking args = {&cond, &mutex};

	status = thrd_create(&thread, single, &args);
	ASSERT_EQ(status, 0);

	// Main thread has not signalled the condition variable yet, variable should not be modified.
	ASSERT_EQ(signal_variable, 0);

	usleep(1000);

	// Condition variable still not signalled yet.
	ASSERT_EQ(signal_variable, 0);

	status = cnd_signal(&cond);
	ASSERT_EQ(status, 0);

	status = thrd_join(thread, NULL);
	ASSERT_EQ(status, 0);

	// Variable should be updated now as the condition variable has been signalled.
	ASSERT_EQ(signal_variable, 1);

	cnd_destroy(&cond);
	mtx_destroy(&mutex);

	return 0;
}

int test_cnd_broadcast()
{
	int status;
	thrd_t thread_1, thread_2;
	cnd_t cond;
	mtx_t mutex_1, mutex_2;

	status = mtx_init(&mutex_1, mtx_plain);
	ASSERT_EQ(status, 0);

	status = mtx_init(&mutex_2, mtx_plain);
	ASSERT_EQ(status, 0);

	status = cnd_init(&cond);
	ASSERT_EQ(status, 0);

	locking args_1 = {&cond, &mutex_1};

	status = thrd_create(&thread_1, all_1, &args_1);
	ASSERT_EQ(status, 0);

	locking args_2 = {&cond, &mutex_2};

	status = thrd_create(&thread_2, all_2, &args_2);
	ASSERT_EQ(status, 0);

	// Main thread has not signalled the condition variable yet, variables should not be modified.
	ASSERT_EQ(broadcast_variable_1, 0);
	ASSERT_EQ(broadcast_variable_2, 0);

	usleep(1000);

	// Condition variable still not signalled yet.
	ASSERT_EQ(broadcast_variable_1, 0);
	ASSERT_EQ(broadcast_variable_2, 0);

	status = cnd_broadcast(&cond);
	ASSERT_EQ(status, 0);

	status = thrd_join(thread_1, NULL);
	ASSERT_EQ(status, 0);

	status = thrd_join(thread_2, NULL);
	ASSERT_EQ(status, 0);

	// Variables should be updated now as the condition variable has been signalled.
	ASSERT_EQ(broadcast_variable_1, 1);
	ASSERT_EQ(broadcast_variable_2, 1);

	cnd_destroy(&cond);
	mtx_destroy(&mutex_1);
	mtx_destroy(&mutex_2);

	return 0;
}

int test_cnd_timed()
{
	int status;
	thrd_t thread;
	cnd_t cond;
	mtx_t mutex;

	status = mtx_init(&mutex, mtx_plain);
	ASSERT_EQ(status, 0);

	status = cnd_init(&cond);
	ASSERT_EQ(status, 0);

	locking args = {&cond, &mutex};

	status = thrd_create(&thread, timed, &args);
	ASSERT_EQ(status, 0);

	// Main thread has not signalled the condition variable yet, variable should not be modified.
	ASSERT_EQ(signal_variable, 0);

	usleep(1000);

	status = cnd_signal(&cond);
	ASSERT_EQ(status, 0);

	status = thrd_join(thread, NULL);
	ASSERT_EQ(status, 0);

	// Variable should not be updated as the wait for the signaling of the condition variable has timedout.
	ASSERT_EQ(signal_variable, 0);

	cnd_destroy(&cond);
	mtx_destroy(&mutex);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();
	int concurrency = wlibc_thread_getconcurrency();

	TEST(test_cnd_signal());

	// In the CI where each runner is allocated 2 cpus, this test hangs when ASAN is enabled.
	if (!(asan_build && concurrency <= 2))
	{
		TEST(test_cnd_broadcast());
	}

	signal_variable = 0;
	TEST(test_cnd_timed());

	VERIFY_RESULT_AND_EXIT();
}
