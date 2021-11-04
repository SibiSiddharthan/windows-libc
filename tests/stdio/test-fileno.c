/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdio.h>
#include <test-macros.h>

int test_std_streams()
{
	int fd;
	fd = fileno(stdin);
	ASSERT_EQ(fd, 0);
	fd = fileno(stdout);
	ASSERT_EQ(fd, 1);
	fd = fileno(stderr);
	ASSERT_EQ(fd, 2);
	return 0;
}

int main()
{
	// For files opened by fopen, we test fileno in test-fopen.c
	INITIAILIZE_TESTS();
	TEST(test_std_streams());
	VERIFY_RESULT_AND_EXIT();
}
