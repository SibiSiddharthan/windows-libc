/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <tests/test.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

int test_EBADF()
{
	errno = 0;
	int status = isatty(3);
	ASSERT_EQ(status, 0);
	ASSERT_ERRNO(EBADF);
	return 0;
}

int test_ENOTTY()
{
	errno = 0;
	int status;
	int fd;
	const char *filename = "t-isatty-notty";

	fd = creat(filename, 0700);
	ASSERT_NOTEQ(fd, -1);

	status = isatty(fd);
	ASSERT_EQ(status, 0);
	ASSERT_ERRNO(ENOTTY);

	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_null()
{
	errno = 0;
	int status;
	int fd;

	fd = open("/dev/null", O_WRONLY);
	ASSERT_NOTEQ(fd, -1);

	status = isatty(fd);
	ASSERT_EQ(status, 0);
	ASSERT_ERRNO(ENOTTY);

	ASSERT_SUCCESS(close(fd));

	return 0;
}

int test_tty()
{
	errno = 0;
	int fd = open("/dev/tty", O_WRONLY);
	if (fd != -1)
	{
		int status = isatty(fd);
		ASSERT_EQ(status, 1);
		ASSERT_SUCCESS(close(fd));
	}

	return 0;
}

int test_duped()
{
	errno = 0;
	int tty_fd = open("/dev/tty", O_WRONLY);
	if (tty_fd != -1)
	{
		int dup_fd = dup(tty_fd);
		int status = isatty(dup_fd);
		ASSERT_EQ(status, 1);
		ASSERT_SUCCESS(close(tty_fd));
		ASSERT_SUCCESS(close(dup_fd));
	}

	return 0;
}

void cleanup()
{
	remove("t-isatty-notty");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_EBADF());
	TEST(test_ENOTTY());
	TEST(test_null());
	TEST(test_tty());
	TEST(test_duped());

	VERIFY_RESULT_AND_EXIT();
}
