/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <sys/times.h>
#include <unistd.h>

// Not really a test, just see if it works.
int test_times()
{
	clock_t clk;
	struct tms tmsbuf;

	usleep(10000);

	clk = times(&tmsbuf);
	ASSERT_NOTEQ(clk, -1);

	printf("Boot time (clock ticks)    : %ld\n", clk);
	printf("User time (clock ticks)    : %ld\n", tmsbuf.tms_utime);
	printf("Kernel time (clock ticks)  : %ld\n", tmsbuf.tms_stime);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();
	TEST(test_times());
	VERIFY_RESULT_AND_EXIT()
}
