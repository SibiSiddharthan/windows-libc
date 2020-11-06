/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
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

int main()
{
	test_okay();
	return 0;
}