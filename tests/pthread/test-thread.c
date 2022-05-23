/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

size_t test_variable = 0;

typedef struct _args_struct
{
	int a, b;
} args_struct;

void *func(void *arg)
{
	args_struct *args = (args_struct *)arg;
	return (void *)(intptr_t)(args->a + args->b);
}

void *empty(void *arg WLIBC_UNUSED)
{
	return NULL;
}

void *join(void *arg WLIBC_UNUSED)
{
	// Sleep for 10 milliseconds.
	usleep(10000);
	return NULL;
}

void *bigexit(void *arg WLIBC_UNUSED)
{
	pthread_exit((void *)0xffffffffffffffff);
}

void cleanup_routine(void *arg)
{
	test_variable += (intptr_t)arg;
}

void *cleanup_push(void *arg)
{
	pthread_cleanup_push(cleanup_routine, arg);
	return NULL;
}

void *cleanup_push_pop_noexecute(void *arg)
{
	pthread_cleanup_push(cleanup_routine, arg);
	pthread_cleanup_pop(0); // Pop the function but don't execute it.
	return NULL;
}

void *cleanup_push_pop_execute(void *arg)
{
	pthread_cleanup_push(cleanup_routine, arg);
	pthread_cleanup_pop(1); // Pop the function and execute it.
	return NULL;
}

void *bigcleanup(void *arg)
{
	// Push more the 8 routines (10 routines pushed)
	pthread_cleanup_push(cleanup_routine, arg);
	pthread_cleanup_push(cleanup_routine, arg);
	pthread_cleanup_push(cleanup_routine, arg);
	pthread_cleanup_push(cleanup_routine, arg);
	pthread_cleanup_push(cleanup_routine, arg);
	pthread_cleanup_push(cleanup_routine, arg);
	pthread_cleanup_push(cleanup_routine, arg);
	pthread_cleanup_push(cleanup_routine, arg);
	pthread_cleanup_push(cleanup_routine, arg);
	pthread_cleanup_push(cleanup_routine, arg);
	// Pop three of them
	pthread_cleanup_pop(1);
	pthread_cleanup_pop(0);
	pthread_cleanup_pop(0);
	return NULL;
}

void *cancel(void *arg WLIBC_UNUSED)
{
	while (1)
		;
	return NULL;
}

void *cancel_with_cleanup(void *arg)
{
	pthread_cleanup_push(cleanup_routine, arg);
	while (1)
		;
	return NULL;
}

void *testcancel(void *arg)
{
	pthread_cleanup_push(cleanup_routine, arg);
	pthread_testcancel();
	return NULL;
}

void *testcancel_disabled(void *arg WLIBC_UNUSED)
{
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	pthread_testcancel();
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

int test_timedjoin()
{
	int status;
	pthread_t thread;
	struct timeval current_time;
	struct timespec abstime;

	status = pthread_create(&thread, NULL, join, NULL);
	ASSERT_EQ(status, 0);

	errno = 0;
	status = pthread_tryjoin(thread, NULL);
	ASSERT_EQ(status, -1);
	ASSERT_EQ(errno, EBUSY);

	gettimeofday(&current_time, NULL);

	abstime.tv_sec = current_time.tv_sec;
	abstime.tv_nsec = (current_time.tv_usec * 1000) % 999999999;
	// Wait for 10 microseconds
	abstime.tv_nsec += 10000;

	errno = 0;
	status = pthread_timedjoin(thread, NULL, &abstime);
	ASSERT_EQ(status, -1);
	ASSERT_EQ(errno, ETIMEDOUT);

	gettimeofday(&current_time, NULL);

	// Wait for 1 second. This should be sufficient for the thread to exit.
	abstime.tv_sec = current_time.tv_sec;
	++abstime.tv_sec;

	status = pthread_timedjoin(thread, NULL, &abstime);
	ASSERT_EQ(status, 0);

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

int test_64bit_return()
{
	int status;
	void *result;
	args_struct args;
	pthread_t thread;

	args.a = 0;
	args.b = -1;

	status = pthread_create(&thread, NULL, func, (void *)&args);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread, &result);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(result, 0xffffffffffffffff);

	return 0;
}

int test_64bit_exit()
{
	int status;
	void *result;
	pthread_t thread;

	status = pthread_create(&thread, NULL, bigexit, NULL);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread, &result);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(result, 0xffffffffffffffff);

	return 0;
}

int test_cleanup()
{
	int status;
	pthread_t thread;

	ASSERT_EQ(test_variable, 0);

	status = pthread_create(&thread, NULL, cleanup_push, (void *)(intptr_t)1);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread, NULL);
	ASSERT_EQ(status, 0);

	ASSERT_EQ(test_variable, 1);

	status = pthread_create(&thread, NULL, cleanup_push_pop_noexecute, (void *)(intptr_t)2);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread, NULL);
	ASSERT_EQ(status, 0);

	ASSERT_EQ(test_variable, 1);

	status = pthread_create(&thread, NULL, cleanup_push_pop_execute, (void *)(intptr_t)3);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread, NULL);
	ASSERT_EQ(status, 0);

	ASSERT_EQ(test_variable, 4);

	test_variable = 0;

	status = pthread_create(&thread, NULL, bigcleanup, (void *)(intptr_t)5);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread, NULL);
	ASSERT_EQ(status, 0);

	// 8 of them should be executed.
	ASSERT_EQ(test_variable, 40);

	return 0;
}

int test_cancel()
{
	int status;
	void *result;
	pthread_t thread;

	ASSERT_EQ(test_variable, 0);

	status = pthread_create(&thread, NULL, cancel, (void *)(intptr_t)1);
	ASSERT_EQ(status, 0);

	// Give some time for the created thread to run.
	usleep(1000);

	status = pthread_cancel(thread);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread, &result);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(result, PTHREAD_CANCELED);

	ASSERT_EQ(test_variable, 0);

	status = pthread_create(&thread, NULL, cancel_with_cleanup, (void *)(intptr_t)2);
	ASSERT_EQ(status, 0);

	// Give some time for the created thread to run.
	usleep(1000);

	status = pthread_cancel(thread);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread, &result);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(result, PTHREAD_CANCELED);

	ASSERT_EQ(test_variable, 2);

	status = pthread_create(&thread, NULL, testcancel, (void *)(intptr_t)3);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread, &result);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(result, PTHREAD_CANCELED);

	ASSERT_EQ(test_variable, 5);

	status = pthread_create(&thread, NULL, testcancel_disabled, (void *)(intptr_t)4);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread, &result);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(result, NULL);

	ASSERT_EQ(test_variable, 5);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_thread_basic());
	TEST(test_join_detach());
	TEST(test_timedjoin());
	TEST(test_attributes());
	TEST(test_64bit_return());
	TEST(test_64bit_exit());
	TEST(test_cleanup());
	test_variable = 0;
	TEST(test_cancel());

	VERIFY_RESULT_AND_EXIT();
}
