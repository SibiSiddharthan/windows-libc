/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdio-hooks.h>
#include <test-macros.h>

void test_std_streams()
{
	int fd;
	fd = fileno(stdin);
	ASSERT_EQ(fd,0);
	fd = fileno(stdout);
	ASSERT_EQ(fd,1);
	fd = fileno(stderr);
	ASSERT_EQ(fd,2);
}

int main()
{
	// For files opened by fopen, we test fileno in test-fopen.c
	test_std_streams();
	return 0;
}
