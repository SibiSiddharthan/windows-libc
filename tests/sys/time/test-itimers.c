/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>

int SIGALRM_count = 0;
int SIGVTALRM_count = 0;
int SIGPROF_count = 0;

#pragma warning(push)
#pragma warning(disable : 4100) // Unused parameter

void SIGALRM_handler(int sig WLIBC_UNUSED)
{
	++SIGALRM_count;
}

void SIGVTALRM_handler(int sig WLIBC_UNUSED)
{
	++SIGVTALRM_count;
}

void SIGPROF_handler(int sig WLIBC_UNUSED)
{
	++SIGPROF_count;
}

#pragma warning(pop)

void init()
{
	signal(SIGALRM, SIGALRM_handler);
	signal(SIGVTALRM, SIGVTALRM_handler);
	signal(SIGPROF, SIGPROF_handler);
}

int test_itimer_real()
{
	int status;
	struct itimerval new_value, old_value;

	new_value.it_interval.tv_sec = 0;
	new_value.it_interval.tv_usec = 0;
	new_value.it_value.tv_sec = 0;
	new_value.it_value.tv_usec = 500; // 500us

	status = setitimer(ITIMER_REAL, &new_value, &old_value);
	ASSERT_EQ(status, 0);

	ASSERT_EQ(old_value.it_interval.tv_sec, 0);
	ASSERT_EQ(old_value.it_interval.tv_usec, 0);
	ASSERT_EQ(old_value.it_value.tv_sec, 0);
	ASSERT_EQ(old_value.it_value.tv_usec, 0);

	usleep(1000); // 1ms

	// Cancel the timer.
	new_value.it_value.tv_usec = 0;
	status = setitimer(ITIMER_REAL, &new_value, &old_value);
	ASSERT_EQ(status, 0);

	ASSERT_EQ(old_value.it_interval.tv_sec, 0);
	ASSERT_EQ(old_value.it_interval.tv_usec, 0);
	ASSERT_EQ(old_value.it_value.tv_sec, 0);
	ASSERT_EQ(old_value.it_value.tv_usec, 0);

	// Timer should have fired only once.
	ASSERT_EQ(SIGALRM_count, 1);

	return 0;
}

int test_itimer_virtual()
{
	int status;
	struct itimerval new_value, old_value;

	new_value.it_interval.tv_sec = 0;
	new_value.it_interval.tv_usec = 200; // 200us
	new_value.it_value.tv_sec = 0;
	new_value.it_value.tv_usec = 500;    // 500us

	status = setitimer(ITIMER_VIRTUAL, &new_value, &old_value);
	ASSERT_EQ(status, 0);

	ASSERT_EQ(old_value.it_interval.tv_sec, 0);
	ASSERT_EQ(old_value.it_interval.tv_usec, 0);
	ASSERT_EQ(old_value.it_value.tv_sec, 0);
	ASSERT_EQ(old_value.it_value.tv_usec, 0);

	usleep(1000); // 1ms

	// Cancel the timer.
	new_value.it_value.tv_usec = 0;
	status = setitimer(ITIMER_VIRTUAL, &new_value, &old_value);
	ASSERT_EQ(status, 0);

	ASSERT_EQ(old_value.it_interval.tv_sec, 0);
	ASSERT_EQ(old_value.it_interval.tv_usec, 200);

	// Timer should have fired atleast once.
	ASSERT_GTEQ(SIGVTALRM_count, 1);

	return 0;
}

int test_itimer_prof()
{
	int status;
	struct itimerval new_value, old_value;

	status = getitimer(ITIMER_PROF, &old_value);
	ASSERT_EQ(status, 0);

	ASSERT_EQ(old_value.it_interval.tv_sec, 0);
	ASSERT_EQ(old_value.it_interval.tv_usec, 0);
	ASSERT_EQ(old_value.it_value.tv_sec, 0);
	ASSERT_EQ(old_value.it_value.tv_usec, 0);

	new_value.it_interval.tv_sec = 0;
	new_value.it_interval.tv_usec = 400; // 100us
	new_value.it_value.tv_sec = 0;
	new_value.it_value.tv_usec = 500;    // 500us

	status = setitimer(ITIMER_PROF, &new_value, NULL);
	ASSERT_EQ(status, 0);

	usleep(1000); // 1ms

	// Cancel the timer.
	new_value.it_value.tv_usec = 0;
	status = setitimer(ITIMER_PROF, &new_value, &old_value);
	ASSERT_EQ(status, 0);

	ASSERT_EQ(old_value.it_interval.tv_sec, 0);
	ASSERT_EQ(old_value.it_interval.tv_usec, 400);

	// Timer should have fired atleast once.
	ASSERT_GTEQ(SIGPROF_count, 1);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	init();
	TEST(test_itimer_real());
	TEST(test_itimer_virtual());
	TEST(test_itimer_prof());

	VERIFY_RESULT_AND_EXIT()
}
