/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
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
