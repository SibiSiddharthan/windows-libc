/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <unistd.h>
#include <test-macros.h>
#include <errno.h>

// This is the only worthwhile test
void test_ERANGE()
{
	char buf[10];
	getcwd(buf, 10);
	ASSERT_ERRNO(ERANGE);
}

int main()
{
	test_ERANGE();
	return 0;
}