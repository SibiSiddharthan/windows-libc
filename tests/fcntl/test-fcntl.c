/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/fcntl.h>
#include <tests/test.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int test_dupfd()
{
	int old_fd = STDOUT_FILENO;
	int new_fd = fcntl(old_fd, F_DUPFD, 0);
	ASSERT_EQ(new_fd, 3);
	ASSERT_SUCCESS(close(new_fd));

	return 0;
}

int test_flags()
{
	int fd = creat("t-fcntl", 0700);

	int flags = fcntl(fd, F_GETFL);
	ASSERT_EQ(flags, (O_WRONLY | O_CREAT | O_TRUNC));

	fcntl(fd, F_SETFL, O_APPEND | O_RDWR);
	flags = fcntl(fd, F_GETFL);
	ASSERT_EQ(flags, (O_WRONLY | O_CREAT | O_TRUNC | O_APPEND));

	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(unlink("t-fcntl"));

	return 0;
}

void cleanup()
{
	remove("t-fcntl");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_dupfd());
	TEST(test_flags());

	VERIFY_RESULT_AND_EXIT();
}
