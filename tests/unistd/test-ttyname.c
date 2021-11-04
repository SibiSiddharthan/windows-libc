/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <fcntl.h>
#include <test-macros.h>
#include <errno.h>

int test_EBADF()
{
	char *name = ttyname(3);
	ASSERT_ERRNO(EBADF);
	ASSERT_NULL(name);
	return 0;
}

int test_ENOTTY()
{
	int fd = creat("t-ttyname", 0700);
	char *name = ttyname(fd);
	ASSERT_ERRNO(ENOTTY);
	ASSERT_NULL(name);
	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(unlink("t-ttyname"));
	return 0;
}

int test_ERANGE()
{
	char name[4];
	int result = ttyname_r(0, name, 4);
	ASSERT_ERRNO(ERANGE);
	ASSERT_EQ(result, ERANGE);
	return 0;
}

int test_okay()
{
	fprintf(stderr, "%s\n", ttyname(0));
	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_EBADF());
	TEST(test_ENOTTY());
	// Only run these if stdin is a console device
	if (isatty(0))
	{
		TEST(test_ERANGE());
		TEST(test_okay());
	}

	VERIFY_RESULT_AND_EXIT();
}
