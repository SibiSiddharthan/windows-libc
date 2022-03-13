/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/random.h>

int test_getrandom()
{
	ssize_t result;
	char buffer[512];

	result = getrandom(buffer, 512, 0);
	ASSERT_EQ(result, 512);

	// if length is odd last byte is not written
	result = getrandom(buffer, 511, 0);
	ASSERT_EQ(result, 510);

	result = getrandom(buffer, 510, 0);
	ASSERT_EQ(result, 510);

	result = getrandom(buffer, 509, 0);
	ASSERT_EQ(result, 508);

	result = getrandom(buffer, 508, 0);
	ASSERT_EQ(result, 508);

	result = getrandom(buffer, 507, 0);
	ASSERT_EQ(result, 506);

	result = getrandom(buffer, 506, 0);
	ASSERT_EQ(result, 506);

	result = getrandom(buffer, 505, 0);
	ASSERT_EQ(result, 504);

	return 0;
}

int test_getentropy()
{
	ssize_t result;
	char buffer[512];

	result = getentropy(buffer, 512);
	ASSERT_EQ(result, 512);

	// if length is odd last byte is not written
	result = getentropy(buffer, 511);
	ASSERT_EQ(result, 510);

	return 0;
}

void SIGILL_handler(int signum)
{
	printf("rdrand, rdseed intrinsics not supported skipping test\n");
	exit(0);
}

int main()
{
	INITIAILIZE_TESTS();

	// In CI this test sometimes fails with 'Illegal Instruction'.
	// Handle this exception and skip this test if it fails.
	signal(SIGILL, SIGILL_handler);

	TEST(test_getrandom());
	TEST(test_getentropy());

	VERIFY_RESULT_AND_EXIT();
}
