/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <test-macros.h>
#include <errno.h>
#include <Windows.h>

void test_okay()
{
	errno = 0;
	int fd;
	fd = creat("CMakeFiles/t-unlinkat", 0700);
	close(fd);
	fd = open("CMakeFiles/", O_RDONLY);
	int status = unlinkat(fd, "t-unlinkat", 0);
	ASSERT_EQ(status, 0);
	close(fd);
	fd = open("CMakeFiles/t-unlinkat", O_RDONLY | O_EXCL);
	ASSERT_EQ(fd, -1);
}

void test_AT_REMOVEDIR()
{
	errno = 0;
	mkdir("CMakeFiles/t-rmdirat", 0700);
	int fd = open("CMakeFiles/", O_RDONLY);
	int status = unlinkat(fd, "t-rmdirat", AT_REMOVEDIR);
	ASSERT_EQ(status, 0);
	close(fd);
	fd = open("CMakeFiles/t-rmdirat", O_RDONLY | O_EXCL);
	ASSERT_EQ(fd, -1);
}

int main()
{
	test_okay();
	test_AT_REMOVEDIR();
	return 0;
}
