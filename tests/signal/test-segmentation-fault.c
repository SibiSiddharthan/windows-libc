/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <signal.h>
#include <stdlib.h>

void segv_handler(int sig)
{
	exit(sig - SIGSEGV);
}

void test_SIGSEGV()
{
	signal(SIGSEGV, segv_handler);

	// Cause a segmentation fault.
	volatile int *p = NULL;
	*p = 0;
}

int main()
{
	// If this works correctly the exit code of the process should be 0.
	test_SIGSEGV();
	exit(1);
}