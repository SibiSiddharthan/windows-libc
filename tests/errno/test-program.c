/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <errno.h>

int test_program()
{
	printf("Full program name  : %s\n", program_invocation_name);
	printf("Short program name : %s\n", program_invocation_short_name);

	// We can only reliably test for the short name.
	ASSERT_STREQ(program_invocation_short_name, "test-program.exe");

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();
	TEST(test_program());
	VERIFY_RESULT_AND_EXIT();
}
