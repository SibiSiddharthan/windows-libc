/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <signal.h>
#include <unistd.h>

int count = 0;

void SIGALRM_handler(int sig)
{
	count += sig;
}

int test_alarm()
{
	int status;

	status = alarm(1);
	ASSERT_EQ(status, 0);

	usleep(1500000); // 1.5 seconds

	ASSERT_EQ(count, SIGALRM);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	signal(SIGALRM, SIGALRM_handler);
	TEST(test_alarm());

	VERIFY_RESULT_AND_EXIT()
}
