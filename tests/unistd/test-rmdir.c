/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <test-macros.h>
#include <errno.h>

void test_ENOENT()
{
	errno = 0;
	int status = rmdir("");
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOENT);
}

void test_ENOTDIR()
{
	errno = 0;
	int fd = creat("test", 0700);
	int status = rmdir("test");
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOTDIR);
	close(fd);
	unlink("test");
}

void test_ENOTEMPTY()
{
	errno = 0;
	int status = rmdir("CMakeFiles");
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOTEMPTY);
}

void test_okay()
{
	errno = 0;
	mkdir("t-rmdir", 0700);
	int status = rmdir("t-rmdir");
	ASSERT_EQ(status, 0);
	int fd = open("t-rmdir", O_RDONLY | O_EXCL);
	ASSERT_EQ(fd, -1);
}

int main()
{
	test_ENOENT();
	test_ENOTDIR();
	test_ENOTEMPTY();
	test_okay();
	return 0;
}