/*
   Copyright (c) 2020 Sibi Siddharthan

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
	int fd = open("CMakeFiles/", O_RDONLY | O_EXCL | O_PATH);
	int status = isatty(fd);
	ASSERT_EQ(status, 0);
	ASSERT_ERRNO(EBADF);
	close(fd);
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

void test_okay()
{
	errno = 0;
	// Dont test with stdout and stderr as they are piped to a file by CTest
	int status = isatty(0);
	ASSERT_EQ(status, 1);
}

void test_duped()
{
	int fd = dup(0);
	ASSERT_EQ(fd, 3);
	int status = isatty(fd);
	ASSERT_EQ(status, 1);
	close(fd);
}

int main()
{
	test_EBADF();
	test_ENOTTY();
	test_okay();
	test_duped();
	return 0;
}
