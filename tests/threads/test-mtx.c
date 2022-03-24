/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <threads.h>
#include <sys/time.h>
#include <unistd.h>

static int variable = 0;

int lock(void *arg)
{
	mtx_t *mutex = (mtx_t *)arg;

	mtx_lock(mutex);
	++variable;
	mtx_unlock(mutex);
	return 0;
}

int timedlock(void *arg)
{
	mtx_t *mutex = (mtx_t *)arg;
	struct timeval current_time;
	struct timespec abstime;

	gettimeofday(&current_time, NULL);

	abstime.tv_sec = current_time.tv_sec;
	abstime.tv_nsec = (current_time.tv_usec * 1000) % 999999999;

	// Wait for 10 microseconds.
	// Don't worry about the tv_nsec overflow, it doesn't matter.
	abstime.tv_nsec += 10000;

	if (mtx_timedlock(mutex, &abstime) == 0)
	{
		++variable;
		mtx_unlock(mutex);
	}

	return 0;
}

int trylock(void *arg)
{
	mtx_t *mutex = (mtx_t *)arg;

	if (mtx_trylock(mutex) == 0)
	{
		++variable;
		mtx_unlock(mutex);
	}
	return 0;
}

int test_mutex_basic()
{
	int status;
	thrd_t thread;
	mtx_t mutex;

	status = mtx_init(&mutex, mtx_plain);
	ASSERT_EQ(status, 0);

	status = mtx_lock(&mutex);
	ASSERT_EQ(status, 0);

	status = thrd_create(&thread, lock, (void *)&mutex);
	ASSERT_EQ(status, 0);

	// Main thread is holding the mutex, variable should not be modified.
	ASSERT_EQ(variable, 0);

	usleep(1000);

	// Main thread is still holding the mutex.
	ASSERT_EQ(variable, 0);

	status = mtx_unlock(&mutex);
	ASSERT_EQ(status, 0);

	status = thrd_join(thread, NULL);
	ASSERT_EQ(status, 0);

	// Thread has exited now, variable must have been updated.
	ASSERT_EQ(variable, 1);

	mtx_destroy(&mutex);

	return 0;
}

int test_mutex_recursive()
{
	int status;
	mtx_t mutex;

	status = mtx_init(&mutex, mtx_plain);
	ASSERT_EQ(status, 0);

	status = mtx_lock(&mutex);
	ASSERT_EQ(status, 0);

	// Recursive lock should fail.
	errno = 0;
	status = mtx_lock(&mutex);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EDEADLK);

	status = mtx_unlock(&mutex);
	ASSERT_EQ(status, 0);

	// Not a recursive mutex, unlocking it again should fail.
	errno = 0;
	status = mtx_unlock(&mutex);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EPERM);

	mtx_destroy(&mutex);

	// Do the above all over again but with a recursive mutex.

	status = mtx_init(&mutex, mtx_recursive);
	ASSERT_EQ(status, 0);

	status = mtx_lock(&mutex);
	ASSERT_EQ(status, 0);

	// Recursive lock, should succeed.
	status = mtx_lock(&mutex);
	ASSERT_EQ(status, 0);

	status = mtx_unlock(&mutex);
	ASSERT_EQ(status, 0);

	// Second unlock should succeed.
	status = mtx_unlock(&mutex);
	ASSERT_EQ(status, 0);

	mtx_destroy(&mutex);

	return 0;
}

int test_mutex_timed()
{
	int status;
	thrd_t thread;
	mtx_t mutex;

	status = mtx_init(&mutex, mtx_timed);
	ASSERT_EQ(status, 0);

	status = mtx_lock(&mutex);
	ASSERT_EQ(status, 0);

	status = thrd_create(&thread, timedlock, (void *)&mutex);
	ASSERT_EQ(status, 0);

	usleep(1000);

	status = mtx_unlock(&mutex);
	ASSERT_EQ(status, 0);

	status = thrd_join(thread, NULL);
	ASSERT_EQ(status, 0);

	// Thread has exited now, it should have failed while trying to acquire the mutex
	// and the varaible should not be updated.
	ASSERT_EQ(variable, 0);

	mtx_destroy(&mutex);

	return 0;
}

int test_mutex_try()
{
	int status;
	thrd_t thread;
	mtx_t mutex;

	status = mtx_init(&mutex, mtx_timed);
	ASSERT_EQ(status, 0);

	status = mtx_trylock(&mutex);
	ASSERT_EQ(status, 0);

	status = thrd_create(&thread, trylock, (void *)&mutex);
	ASSERT_EQ(status, 0);

	usleep(1000);

	status = thrd_join(thread, NULL);
	ASSERT_EQ(status, 0);

	status = mtx_unlock(&mutex);
	ASSERT_EQ(status, 0);

	// Thread has exited now, it should have failed while trying to acquire the mutex
	// and the varaible should not be updated.
	ASSERT_EQ(variable, 0);

	mtx_destroy(&mutex);

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
