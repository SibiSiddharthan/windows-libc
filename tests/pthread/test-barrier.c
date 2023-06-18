/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <pthread.h>
#include <unistd.h>

static int variable_1 = 0;
static int variable_2 = 0;

void *wait1(void *arg)
{
	pthread_barrier_t *barrier = (pthread_barrier_t *)arg;

	pthread_barrier_wait(barrier);
	++variable_1;
	return NULL;
}

void *wait2(void *arg)
{
	pthread_barrier_t *barrier = (pthread_barrier_t *)arg;

	pthread_barrier_wait(barrier);
	++variable_2;
	return NULL;
}

int test_barrier()
{
	int status;
	void *result;
	pthread_t thread_1, thread_2;
	pthread_barrier_t barrier;

	status = pthread_barrier_init(&barrier, NULL, 3);
	ASSERT_EQ(status, 0);

	status = pthread_create(&thread_1, NULL, wait1, (void *)&barrier);
	ASSERT_EQ(status, 0);

	status = pthread_create(&thread_2, NULL, wait2, (void *)&barrier);
	ASSERT_EQ(status, 0);

	// Main thread needs to enter the barrier.
	ASSERT_EQ(variable_1, 0);
	ASSERT_EQ(variable_2, 0);

	usleep(1000);

	// Main thread still needs to enter the barrier.
	ASSERT_EQ(variable_1, 0);
	ASSERT_EQ(variable_2, 0);

	// Main thread enters the barrier now. The waiting threads should resume now.
	status = pthread_barrier_wait(&barrier);
	// Since this is the last thread that enters the barrier, the return value should be 'PTHREAD_BARRIER_SERIAL_THREAD'.
	ASSERT_EQ(status, PTHREAD_BARRIER_SERIAL_THREAD);

	status = pthread_join(thread_1, &result);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread_2, &result);
	ASSERT_EQ(status, 0);

	// Thread has exited now, variable must have been updated.
	ASSERT_EQ(variable_1, 1);
	ASSERT_EQ(variable_2, 1);

	status = pthread_barrier_destroy(&barrier);
	ASSERT_EQ(status, 0);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();
	TEST(test_barrier());
	VERIFY_RESULT_AND_EXIT();
}
