/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <test-macros.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

void test_EBADF()
{
	errno = 0;
	int status = isatty(3);
	ASSERT_EQ(status, 0);
	ASSERT_ERRNO(EBADF);
}

void test_ENOTTY()
{
	errno = 0;
	int fd = creat("t-isatty", 0700);
	int status = isatty(fd);
	ASSERT_EQ(status, 0);
	ASSERT_ERRNO(ENOTTY);
	close(fd);
	unlink("t-isatty");
}

void test_null()
{
	errno = 0;
	int fd = open("/dev/null", O_WRONLY);
	int status = isatty(fd);
	ASSERT_EQ(status, 0);
	ASSERT_ERRNO(ENOTTY);
	close(fd);
}

void test_tty()
{
	errno = 0;
	int fd = open("/dev/tty", O_WRONLY);
	if (fd != -1)
	{
		int status = isatty(fd);
		ASSERT_EQ(status, 1);
		close(fd);
	}
}

void test_duped()
{
	errno = 0;
	int tty_fd = open("/dev/tty", O_WRONLY);
	if (tty_fd != -1)
	{
		int dup_fd = dup(tty_fd);
		int status = isatty(dup_fd);
		ASSERT_EQ(status, 1);
		close(tty_fd);
		close(dup_fd);
	}
}

int main()
{
	test_EBADF();
	test_ENOTTY();
	test_null();
	test_tty();
	test_duped();
	return 0;
}
