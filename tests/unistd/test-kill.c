/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <test-macros.h>
#include <errno.h>
#include <signal.h>

void test_EINVAL()
{
	errno = 0;
	int status = kill(getpid(), 32);
	ASSERT_ERRNO(EINVAL);
	ASSERT_EQ(status, -1);
}

void test_bad_process()
{
	errno = 0;
	int status = kill(-1, SIGTERM);
	ASSERT_ERRNO(EINVAL);
	ASSERT_EQ(status, -1);
}

void test_okay()
{
	kill(getpid(), SIGTERM);
}

int main()
{
	test_EINVAL();
	test_bad_process();
	test_okay();
	return 0;
}