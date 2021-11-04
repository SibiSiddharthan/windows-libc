/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <sys/stat.h>
#include <unistd.h>
#include <test-macros.h>
#include <errno.h>
#include <fcntl.h>

int test_ENOENT()
{
	int status = chmod("", S_IREAD);
	ASSERT_ERRNO(ENOENT);
	ASSERT_EQ(status, -1);
	return 0;
}

int test_READONLY()
{
	int status;
	int fd;
	ssize_t length;
	const char *filename = "t-chmod";

	fd = creat(filename, 0700);
	ASSERT_SUCCESS(close(fd));

	status = chmod(filename, S_IREAD);
	ASSERT_EQ(status, 0);

	fd = open(filename, O_WRONLY | O_EXCL);
	ASSERT_EQ(fd, -1);

	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

void cleanup()
{
	remove("t-chmod");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_ENOENT());
	TEST(test_READONLY());

	VERIFY_RESULT_AND_EXIT();
}
