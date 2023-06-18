/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/spawn.h>
#include <tests/test.h>
#include <signal.h>
#include <spawn.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

pid_t create_process_simple()
{
	int status;
	pid_t pid;
	const char *program = "child-helper.exe";
	char *argv[] = {(char *)program, NULL};

	status = posix_spawn(&pid, program, NULL, NULL, argv, NULL);

	if (status != 0)
	{
		return -1;
	}

	return pid;
}

pid_t create_process(int arg1, int arg2) // mode, argument
{
	int status;
	pid_t pid;
	const char *program = "child-helper.exe";
	char arg1buf[16] = {0};
	char arg2buf[16] = {0};
	char *argv[] = {(char *)program, arg1buf, arg2buf, NULL};

	itoa(arg1, arg1buf, 10);
	itoa(arg2, arg2buf, 10);

	status = posix_spawn(&pid, program, NULL, NULL, argv, NULL);

	if (status != 0)
	{
		return -1;
	}

	return pid;
}

int test_internal()
{
	create_process_simple();
	ASSERT_EQ(_wlibc_child_process_count, 1);
	ASSERT_EQ(_wlibc_process_table_size, 4);
	create_process_simple();
	ASSERT_EQ(_wlibc_child_process_count, 2);
	create_process_simple();
	ASSERT_EQ(_wlibc_child_process_count, 3);
	create_process_simple();
	ASSERT_EQ(_wlibc_child_process_count, 4);

	// Make sure the process table grows properly
	create_process_simple();
	ASSERT_EQ(_wlibc_child_process_count, 5);
	ASSERT_EQ(_wlibc_process_table_size, 8);
	create_process_simple();
	ASSERT_EQ(_wlibc_child_process_count, 6);

	// Wait for the processes
	waitpid(0, NULL, 0);
	ASSERT_EQ(_wlibc_child_process_count, 5);
	waitpid(0, NULL, 0);
	ASSERT_EQ(_wlibc_child_process_count, 4);
	waitpid(0, NULL, 0);
	ASSERT_EQ(_wlibc_child_process_count, 3);
	waitpid(0, NULL, 0);
	ASSERT_EQ(_wlibc_child_process_count, 2);
	waitpid(0, NULL, 0);
	ASSERT_EQ(_wlibc_child_process_count, 1);

	// Add more children
	create_process_simple();
	ASSERT_EQ(_wlibc_child_process_count, 2);
	create_process_simple();
	ASSERT_EQ(_wlibc_child_process_count, 3);
	create_process_simple();
	ASSERT_EQ(_wlibc_child_process_count, 4);
	create_process_simple();
	ASSERT_EQ(_wlibc_child_process_count, 5);
	ASSERT_EQ(_wlibc_process_table_size, 8);

	// Wait for all the processes
	waitpid(0, NULL, 0);
	ASSERT_EQ(_wlibc_child_process_count, 4);
	waitpid(0, NULL, 0);
	ASSERT_EQ(_wlibc_child_process_count, 3);
	waitpid(0, NULL, 0);
	ASSERT_EQ(_wlibc_child_process_count, 2);
	waitpid(0, NULL, 0);
	ASSERT_EQ(_wlibc_child_process_count, 1);
	waitpid(0, NULL, 0);
	ASSERT_EQ(_wlibc_child_process_count, 0);

	return 0;
}

int test_ECHILD()
{
	pid_t pid = waitpid(1, NULL, 0);
	ASSERT_EQ(pid, -1);
	ASSERT_ERRNO(ECHILD);
	return 0;
}

int test_waitpid()
{
	int wstatus;
	pid_t child, result;

	child = create_process(1, 20); // child will sleep for 20ms
	result = waitpid(child, &wstatus, 0);
	ASSERT_EQ(result, child);
	ASSERT_EQ(wstatus, 0); // child will exit normally

	return 0;
}

int test_waitpid_WNOHANG()
{
	int wstatus;
	pid_t child, result;

	child = create_process(1, 100);
	result = waitpid(child, &wstatus, WNOHANG);
	ASSERT_EQ(result, 0);
	ASSERT_EQ(wstatus, -1);

	result = waitpid(child, &wstatus, 0);
	ASSERT_EQ(result, child);
	ASSERT_EQ(wstatus, 0);

	return 0;
}

int test_signal_exit()
{
	pid_t child, result;
	int wstatus;

	child = create_process(2, SIGHUP); // child will raise SIGHUP
	result = waitpid(child, &wstatus, 0);
	ASSERT_EQ(result, child);
	ASSERT_EQ(WIFSIGNALED(wstatus), 1);
	ASSERT_EQ(WIFEXITED(wstatus), 0);

	child = create_process(2, SIGABRT);
	result = waitpid(child, &wstatus, 0);
	ASSERT_EQ(result, child);
	ASSERT_EQ(WIFSIGNALED(wstatus), 1);
	ASSERT_EQ(WIFEXITED(wstatus), 0);

	child = create_process(2, SIGCONT);
	result = waitpid(child, &wstatus, 0);
	ASSERT_EQ(result, child);
	ASSERT_EQ(WIFSIGNALED(wstatus), 0);
	ASSERT_EQ(WIFEXITED(wstatus), 1);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_internal());
	TEST(test_ECHILD());
	TEST(test_waitpid());
	TEST(test_waitpid_WNOHANG());
	TEST(test_signal_exit());

	VERIFY_RESULT_AND_EXIT();
}
