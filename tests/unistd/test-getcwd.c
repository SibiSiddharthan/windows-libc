/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
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
