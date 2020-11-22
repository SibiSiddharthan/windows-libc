/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <test-macros.h>
#include <errno.h>
#include <fcntl_internal.h>

void test_dupfd()
{
	int old_fd = STDOUT_FILENO;
	int new_fd = fcntl(old_fd, F_DUPFD, 0);
	ASSERT_EQ(new_fd, 3);
	close(new_fd);
}

void test_flags()
{
	int fd = creat("t-fcntl", 0700);

	int flags = fcntl(fd, F_GETFL);
	ASSERT_EQ(flags, (O_WRONLY | O_CREAT | O_TRUNC));

	fcntl(fd, F_SETFL, O_APPEND | O_RDWR);
	flags = fcntl(fd, F_GETFL);
	ASSERT_EQ(flags, (O_WRONLY | O_CREAT | O_TRUNC | O_APPEND));

	close(fd);
	unlink("t-fcntl");
}

int main()
{
	test_dupfd();
	test_flags();
	return 0;
}
