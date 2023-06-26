/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>

int signal_count = 0;
int notification_count = 0;

void signal_handler(int sig)
{
	signal_count += sig;
}

void notification_handler(union sigval arg)
{
	notification_count += arg.sival_int;
}

int test_timer_none()
{
	int status;
	timer_t timer;
	struct sigevent event;
	struct itimerspec new_value, old_value;

	event.sigev_notify = SIGEV_NONE;

	status = timer_create(CLOCK_REALTIME, &event, &timer);
	ASSERT_EQ(status, 0);

	new_value.it_interval.tv_sec = 0;
	new_value.it_interval.tv_nsec = 0;
	new_value.it_value.tv_sec = 0;
	new_value.it_value.tv_nsec = 500000; // 500us

	status = timer_settime(timer, 0, &new_value, &old_value);
	ASSERT_EQ(status, 0);

	ASSERT_EQ(old_value.it_interval.tv_sec, 0);
	ASSERT_EQ(old_value.it_interval.tv_nsec, 0);
	ASSERT_EQ(old_value.it_value.tv_sec, 0);
	ASSERT_EQ(old_value.it_value.tv_nsec, 0);

	status = timer_delete(timer);
	ASSERT_EQ(status, 0);

	return 0;
}

int test_timer_signal()
{
	int status;
	timer_t timer;
	struct sigevent event;
	struct itimerspec new_value;

	signal_count = 0;

	event.sigev_notify = SIGEV_SIGNAL;
	event.sigev_signo = SIGUSR1;

	signal(SIGUSR1, signal_handler);

	status = timer_create(CLOCK_REALTIME, &event, &timer);
	ASSERT_EQ(status, 0);

	new_value.it_interval.tv_sec = 0;
	new_value.it_interval.tv_nsec = 0;
	new_value.it_value.tv_sec = 0;
	new_value.it_value.tv_nsec = 500000; // 500us

	status = timer_settime(timer, 0, &new_value, NULL);
	ASSERT_EQ(status, 0);

	usleep(1000); // 1ms

	status = timer_delete(timer);
	ASSERT_EQ(status, 0);

	ASSERT_EQ(signal_count, SIGUSR1);

	return 0;
}

int test_timer_thread()
{
	int status;
	timer_t timer;
	struct sigevent event;
	struct itimerspec new_value, old_value;

	event.sigev_notify = SIGEV_THREAD;
	event.sigev_value.sival_int = 1;
	event.sigev_notify_function = notification_handler;
	event.sigev_notify_attributes = NULL;

	status = timer_create(CLOCK_REALTIME, &event, &timer);
	ASSERT_EQ(status, 0);

	new_value.it_interval.tv_sec = 0;
	new_value.it_interval.tv_nsec = 200000; // 200us
	new_value.it_value.tv_sec = 0;
	new_value.it_value.tv_nsec = 500000;    // 500us

	status = timer_settime(timer, 0, &new_value, NULL);
	ASSERT_EQ(status, 0);

	usleep(1000); // 1ms

	// Cancel the timer.
	new_value.it_value.tv_nsec = 0;
	status = timer_settime(timer, 0, &new_value, &old_value);
	ASSERT_EQ(status, 0);

	ASSERT_EQ(old_value.it_interval.tv_sec, 0);
	ASSERT_EQ(old_value.it_interval.tv_nsec, 200000);
	ASSERT_EQ(old_value.it_value.tv_sec, 0);
	ASSERT_EQ(old_value.it_value.tv_nsec, 0);

	status = timer_delete(timer);
	ASSERT_EQ(status, 0);

	// Timer should have fired atleast 3 times.
	ASSERT_GTEQ(notification_count, 3);

	return 0;
}

int test_timer_absolute()
{
	int status;
	timer_t timer;
	struct sigevent event;
	struct itimerspec new_value;

	signal_count = 0;

	event.sigev_notify = SIGEV_SIGNAL;
	event.sigev_signo = SIGUSR2;

	signal(SIGUSR2, signal_handler);

	status = timer_create(CLOCK_REALTIME, &event, &timer);
	ASSERT_EQ(status, 0);

	status = clock_gettime(CLOCK_REALTIME, &new_value.it_value);
	ASSERT_EQ(status, 0);

	// Increate the time given by the clock by 1ms.
	new_value.it_interval.tv_sec = 0;
	new_value.it_interval.tv_nsec = 0;
	new_value.it_value.tv_nsec += 1000000; // 1ms

	status = timer_settime(timer, TIMER_ABSTIME, &new_value, NULL);
	ASSERT_EQ(status, 0);

	usleep(2000); // 2ms

	status = timer_delete(timer);
	ASSERT_EQ(status, 0);

	ASSERT_EQ(signal_count, SIGUSR2);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_timer_none());
	TEST(test_timer_signal());
	TEST(test_timer_thread());
	TEST(test_timer_absolute());

	VERIFY_RESULT_AND_EXIT()
}
