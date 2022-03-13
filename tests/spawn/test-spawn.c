/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <spawn.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int test_spawn_basic()
{
	int status;
	int result;
	int fd;
	pid_t pid;
	posix_spawn_file_actions_t actions;
	char rbuf[32];
	const char *program = "basic.exe";
	const char *input = "input.txt";
	const char *output = "output.txt";
	char *const argv[] = {"basic.exe", NULL};

	// Prepare the input.
	fd = open(input, O_CREAT | O_WRONLY | O_TRUNC, 0700);
	result = write(fd, "Hello World", 11);
	ASSERT_EQ(result, 11);
	ASSERT_SUCCESS(close(fd));

	status = posix_spawn_file_actions_init(&actions);
	ASSERT_EQ(status, 0);

	status = posix_spawn_file_actions_addopen(&actions, STDIN_FILENO, input, O_RDONLY, 0700);
	ASSERT_EQ(status, 0);

	status = posix_spawn_file_actions_addopen(&actions, STDOUT_FILENO, output, O_CREAT | O_WRONLY, 0700);
	ASSERT_EQ(status, 0);

	status = posix_spawn(&pid, program, &actions, NULL, argv, environ);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, NULL, 0);
	ASSERT_EQ(status, pid);

	status = posix_spawn_file_actions_destroy(&actions);
	ASSERT_EQ(status, 0);

	// Check output
	fd = open(input, O_RDONLY);
	result = read(fd, rbuf, 32);
	ASSERT_EQ(result, 11);
	ASSERT_MEMEQ(rbuf, "Hello World", 11);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(remove(input));
	ASSERT_SUCCESS(remove(output));

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();
	TEST(test_spawn_basic());
	VERIFY_RESULT_AND_EXIT();
}
