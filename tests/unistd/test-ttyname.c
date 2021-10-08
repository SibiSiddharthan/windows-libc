/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <fcntl.h>
#include <test-macros.h>
#include <errno.h>

void test_EBADF()
{
	char *name = ttyname(3);
	ASSERT_ERRNO(EBADF);
	ASSERT_NULL(name);
}

void test_ENOTTY()
{
	int fd = creat("t-ttyname", 0700);
	char *name = ttyname(fd);
	ASSERT_ERRNO(ENOTTY);
	ASSERT_NULL(name);
	close(fd);
	unlink("t-ttyname");
}

void test_ERANGE()
{
	char name[4];
	int result = ttyname_r(0, name, 4);
	ASSERT_ERRNO(ERANGE);
	ASSERT_EQ(result, ERANGE);
}

void test_okay()
{
	fprintf(stderr, "%s\n", ttyname(0));
}

int main()
{
	test_EBADF();
	test_ENOTTY();
	// Only run these if stdin is a console device
	if (isatty(0))
	{
		test_ERANGE();
		test_okay();
	}
	return 0;
}
