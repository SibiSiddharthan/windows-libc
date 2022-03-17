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

int prepare_input(const char *filename, const char *content)
{
	int fd;
	size_t length;
	ssize_t result;

	fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0700);

	length = strlen(content);
	result = write(fd, content, (int)length);

	ASSERT_EQ(result, length);
	ASSERT_SUCCESS(close(fd));

	return 0;
}

int check_output(const char *filename, const char *content)
{
	int fd;
	size_t length;
	ssize_t result;
	char buf[32];

	fd = open(filename, O_RDONLY);

	length = strlen(content);
	result = read(fd, buf, 32);

	ASSERT_EQ(result, length);
	ASSERT_MEMEQ(buf, content, (int)length);
	ASSERT_SUCCESS(close(fd));

	return 0;
}

int test_spawn_basic()
{
	int status;
	pid_t pid;
	posix_spawn_file_actions_t actions;

	const char *program = "basic.exe";
	const char *program_noext = "basic";
	const char *input = "input.txt";
	const char *output = "output.txt";
	const char *content = "Hello World";
	char *const argv[] = {"basic.exe", NULL};

	// Prepare the input.
	ASSERT_SUCCESS(prepare_input(input, content));

	status = posix_spawn_file_actions_init(&actions);
	ASSERT_EQ(status, 0);

	status = posix_spawn_file_actions_addopen(&actions, STDIN_FILENO, input, O_RDONLY, 0700);
	ASSERT_EQ(status, 0);

	status = posix_spawn_file_actions_addopen(&actions, STDOUT_FILENO, output, O_CREAT | O_WRONLY, 0700);
	ASSERT_EQ(status, 0);

	status = posix_spawn(&pid, program, &actions, NULL, argv, NULL);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, NULL, 0);
	ASSERT_EQ(status, pid);

	// Check output
	ASSERT_SUCCESS(check_output(output, content));
	ASSERT_SUCCESS(remove(output));

	// Spawn the process again but this time give a program with no extension.
	status = posix_spawn(&pid, program_noext, &actions, NULL, argv, NULL);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, NULL, 0);
	ASSERT_EQ(status, pid);

	// Check output
	ASSERT_SUCCESS(check_output(output, content));
	ASSERT_SUCCESS(remove(output));

	status = posix_spawn_file_actions_destroy(&actions);
	ASSERT_EQ(status, 0);

	ASSERT_SUCCESS(remove(input));

	return 0;
}

int test_spawn_pipe()
{
	int status;
	int input_fds[2], output_fds[2];
	size_t length;
	ssize_t result;
	char buf[32];
	pid_t pid;
	posix_spawn_file_actions_t actions;

	const char *program = "basic.exe";
	const char *content = "Hello World!!!";
	char *const argv[] = {"basic.exe", NULL};

	length = strlen(content);

	status = pipe(input_fds);
	ASSERT_EQ(status, 0);
	status = pipe(output_fds);
	ASSERT_EQ(status, 0);

	status = posix_spawn_file_actions_init(&actions);
	ASSERT_EQ(status, 0);

	status = posix_spawn_file_actions_adddup2(&actions, input_fds[0], STDIN_FILENO);
	ASSERT_EQ(status, 0);

	status = posix_spawn_file_actions_adddup2(&actions, output_fds[1], STDOUT_FILENO);
	ASSERT_EQ(status, 0);

	status = posix_spawn(&pid, program, &actions, NULL, argv, NULL);
	ASSERT_EQ(status, 0);

	// We don't need these anymore
	ASSERT_SUCCESS(close(input_fds[0]));
	ASSERT_SUCCESS(close(output_fds[1]));

	result = write(input_fds[1], content, length);
	ASSERT_EQ(result, length);
	ASSERT_SUCCESS(close(input_fds[1]));

	status = waitpid(pid, NULL, 0);
	ASSERT_EQ(status, pid);

	// Check output
	result = read(output_fds[0], buf, 32);
	ASSERT_EQ(result, length);
	ASSERT_MEMEQ(buf, content, (int)length);
	ASSERT_SUCCESS(close(output_fds[0]));

	status = posix_spawn_file_actions_destroy(&actions);
	ASSERT_EQ(status, 0);

	return 0;
}

void cleanup()
{
	remove("input.txt");
	remove("output.txt");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_spawn_basic());
	TEST(test_spawn_pipe());

	VERIFY_RESULT_AND_EXIT();
}
