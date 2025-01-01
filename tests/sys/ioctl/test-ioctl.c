/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

int test_TIOCGWINSZ()
{
	int fd;
	int status;
	struct winsize window;

	if (isatty(STDOUT_FILENO))
	{
		fd = STDOUT_FILENO;
	}
	else if (isatty(STDERR_FILENO))
	{
		fd = STDERR_FILENO;
	}
	else
	{
		// Cannot test this.
		return 0;
	}

	status = ioctl(fd, TIOCGWINSZ, &window);
	ASSERT_EQ(status, 0);

	printf("Window size: %hu columns %hu rows\n", window.ws_col, window.ws_row);

	return 0;
}

int test_FION()
{
	int status;
	int bytes;
	int fd[2];
	ssize_t count;
	char buffer[1024] = {0};

	ASSERT_SUCCESS(pipe(fd));

	count = write(fd[1], buffer, 1024);
	ASSERT_EQ(count, 1024);

	status = ioctl(fd[0], FIONREAD, &bytes);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(bytes, 1024);

	status = ioctl(fd[1], FIONWRITE, &bytes);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(bytes, 1024);

	ASSERT_SUCCESS(close(fd[0]));
	ASSERT_SUCCESS(close(fd[1]));

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_TIOCGWINSZ());
	TEST(test_FION());

	VERIFY_RESULT_AND_EXIT();
}
