/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <unistd.h>

int test_pipe()
{
	int status;
	int pipefd[2];
	ssize_t result;
	char buf[32];
	const char *content = "Hello World";

	status = pipe(pipefd);
	ASSERT_EQ(status, 0);

	ASSERT_NOTEQ(pipefd[0], -1);
	ASSERT_NOTEQ(pipefd[1], -1);

	result = write(pipefd[1], content, 12);
	ASSERT_EQ(result, 12);

	result = read(pipefd[0], buf, 32);
	ASSERT_EQ(result, 12);
	ASSERT_MEMEQ(buf, content, 12);

	// Try to do the same operation but switch the ends. It should fail.
	errno = 0;
	result = write(pipefd[0], content, 12);
	ASSERT_EQ(result, -1);
	ASSERT_ERRNO(EACCES);

	errno = 0;
	result = read(pipefd[1], buf, 32);
	ASSERT_EQ(result, -1);
	ASSERT_ERRNO(EACCES);

	ASSERT_SUCCESS(close(pipefd[0]));
	ASSERT_SUCCESS(close(pipefd[1]));

	return 0;
}

int test_pipe2_nonblocking()
{
	int status;
	int pipefd[2];
	ssize_t result;
	char buf[32];
	const char *content = "Hello World";

	status = pipe2(pipefd, O_NONBLOCK);
	ASSERT_EQ(status, 0);

	ASSERT_NOTEQ(pipefd[0], -1);
	ASSERT_NOTEQ(pipefd[1], -1);

	// Normal writing and reading should work as normal.
	result = write(pipefd[1], content, 12);
	ASSERT_EQ(result, 12);

	result = read(pipefd[0], buf, 32);
	ASSERT_EQ(result, 12);
	ASSERT_MEMEQ(buf, content, 12);

	// Pipe buffer should be empty now.
	// Attempting a second read should not block since we have specified O_NONBLOCK.
	result = read(pipefd[0], buf, 32);
	ASSERT_EQ(result, 0);

	ASSERT_SUCCESS(close(pipefd[0]));
	ASSERT_SUCCESS(close(pipefd[1]));

	return 0;
}

int test_named_pipe()
{
	int status;
	int pipefd[2];
	ssize_t result;
	char buf[32];
	const char *content = "Hello World";
	const char *pipe_name = "\\\\.\\pipe\\mypipe";

	status = named_pipe(pipe_name, pipefd);
	ASSERT_EQ(status, 0);

	ASSERT_NOTEQ(pipefd[0], -1);
	ASSERT_NOTEQ(pipefd[1], -1);

	result = write(pipefd[1], content, 12);
	ASSERT_EQ(result, 12);

	result = read(pipefd[0], buf, 32);
	ASSERT_EQ(result, 12);
	ASSERT_MEMEQ(buf, content, 12);

	// Try to do the same operation but switch the ends. It should fail.
	errno = 0;
	result = write(pipefd[0], content, 12);
	ASSERT_EQ(result, -1);
	ASSERT_ERRNO(EACCES);

	errno = 0;
	result = read(pipefd[1], buf, 32);
	ASSERT_EQ(result, -1);
	ASSERT_ERRNO(EACCES);

	ASSERT_SUCCESS(close(pipefd[0]));
	ASSERT_SUCCESS(close(pipefd[1]));

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();
	TEST(test_pipe());
	TEST(test_pipe2_nonblocking());
	TEST(test_named_pipe());
	VERIFY_RESULT_AND_EXIT();
}
