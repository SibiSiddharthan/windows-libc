/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <fcntl.h>
#include <test-macros.h>
#include <errno.h>

void test_ENOENT()
{
	int status = unlink("junk");
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOENT);
}

void test_okay()
{
	int fd = creat("t-unlink", 0700);
	close(fd);
	int status = unlink("t-unlink");
	ASSERT_EQ(status, 0);
	fd = open("t-unlink", O_RDONLY | O_EXCL);
	ASSERT_EQ(fd, -1);
}

void test_READONLY()
{
	int fd = creat("t-unlink", S_IREAD);
	close(fd);
	int status = unlink("t-unlink");
	ASSERT_EQ(status, 0);
	fd = open("t-unlink", O_RDONLY | O_EXCL);
	ASSERT_EQ(fd, -1);
}

int main()
{
	test_ENOENT();
	test_okay();
	test_READONLY();
	return 0;
}
