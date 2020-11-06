/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <sys/stat.h>
#include <unistd.h>
#include <test-macros.h>
#include <errno.h>
#include <fcntl.h>

void test_ENOENT()
{
	int status = chmod("", S_IREAD);
	ASSERT_ERRNO(ENOENT);
	ASSERT_EQ(status, -1);
}

void test_READONLY()
{
	int fd = creat("t-chmod", 0700);
	close(fd);
	int status = chmod("t-chmod", S_IREAD);
	ASSERT_EQ(status, 0);
	fd = open("t-chmod", O_WRONLY | O_EXCL);
	ssize_t length = write(fd, "hello", 5);
	ASSERT_EQ(length, -1);
	unlink("t-chmod");
}

int main()
{
	test_ENOENT();
	test_READONLY();
	return 0;
}
