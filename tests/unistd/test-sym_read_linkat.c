/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <sys/stat.h>
#include <fcntl.h>
#include <test-macros.h>
#include <errno.h>
#include <Windows.h>
#include <unistd.h>

// Testing both symlinkat and readlinkat
void test_okay()
{
	int fd;
	fd = creat("CMakeFiles/t-readlink", 0700);
	close(fd);
	fd = open("CMakeFiles/", O_RDONLY);
	int status = symlinkat("t-readlink", fd, "t-readlink.sym");
	ASSERT_EQ(status, 0);
	char buf[MAX_PATH];
	ssize_t length = readlinkat(fd, "t-readlink.sym", buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, 10);
	ASSERT_STREQ(buf, "t-readlink");
	unlinkat(fd, "t-readlink.sym", 0);
	unlinkat(fd, "t-readlink", 0);
	close(fd);
}

void test_empty_path()
{
	int fd;
	fd = creat("t-readlink-empty", 0700);
	close(fd);
	int status = symlink("t-readlink-empty", "t-readlink-empty.sym");
	fd = open("t-readlink-empty.sym", O_NOFOLLOW | O_PATH);
	ASSERT_EQ(fd, 3);

	char buf[MAX_PATH];
	ssize_t length = readlinkat(fd, "", buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, 16);
	ASSERT_STREQ(buf, "t-readlink-empty");
	close(fd);

	unlink("t-readlink-empty.sym");
	unlink("t-readlink-empty");
}

int main()
{
	test_okay();
	test_empty_path();
	return 0;
}
