/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <fcntl.h>
#include <unistd.h>
#include <test-macros.h>
#include <errno.h>

void test_okay()
{
	errno = 0;
	int fd = open("CMakeFiles/", O_RDONLY);
	int t_fd = openat(fd, "t-openat", O_CREAT | O_RDONLY, 0700);
	ASSERT_EQ(t_fd, 4);
	close(fd);
	close(t_fd);
	int status = unlink("CMakeFiles/t-openat");
	ASSERT_EQ(status, 0);
}

int main()
{
	test_okay();
	return 0;
}