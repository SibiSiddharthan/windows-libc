/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/process.h>
#include <test-macros.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

pid_t create_process_simple()
{
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;

	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	memset(&pi, 0, sizeof(pi));

	BOOL result = CreateProcessA(NULL, "child-helper", NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
	if (result == 0)
	{
		return -1;
	}

	add_child(pi.dwProcessId, pi.hProcess);
	return pi.dwProcessId;
}

pid_t create_process(int arg1, int arg2) // mode, argument
{
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;

	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	memset(&pi, 0, sizeof(pi));

	char cmd[128] = "\0";
	char buf[16] = "\0";
	strcat(cmd, "child-helper");
	strcat(cmd, " ");
	itoa(arg1, buf, 10);
	strcat(cmd, buf);
	strcat(cmd, " ");
	itoa(arg2, buf, 10);
	strcat(cmd, buf);

	BOOL result = CreateProcessA(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
	if (result == 0)
	{
		return -1;
	}

	add_child(pi.dwProcessId, pi.hProcess);
	return pi.dwProcessId;
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

	child = create_process(1, 200); // child will sleep for 200ms
	result = waitpid(child, &wstatus, 0);
	ASSERT_EQ(result, child);
	ASSERT_EQ(wstatus, 0); // child will exit normally

	return 0;
}

int test_waitpid_WNOHANG()
{
	int wstatus;
	pid_t child, result;

	child = create_process(1, 200);
	result = waitpid(child, &wstatus, WNOHANG);
	ASSERT_EQ(result, 0);

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

	VERIFY_RESULT_AND_EXIT();
}
