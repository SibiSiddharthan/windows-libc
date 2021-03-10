/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <test-macros.h>
#include <errno.h>
#include <signal-ext.h>
#include <sys/wait.h>

#include <process_internal.h>
#include <Windows.h>

pid_t create_process_simple()
{
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;

	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	memset(&pi, 0, sizeof(pi));

	BOOL result = CreateProcessA(NULL, "child-helper", NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
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
	add_child(pi.dwProcessId, pi.hProcess);

	return pi.dwProcessId;
}

void test_internal()
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
}

void test_ECHILD()
{
	pid_t pid = waitpid(1, NULL, 0);
	ASSERT_EQ(pid, -1);
	ASSERT_ERRNO(ECHILD);
}

void test_waitpid()
{
	pid_t pid = create_process(1, 200); // child will sleep for 200ms
	int wstatus;
	pid_t result = waitpid(pid, &wstatus, 0);
	ASSERT_EQ(result, pid);
	ASSERT_EQ(wstatus, 0); // child will exit normally
}

void test_waitpid_WNOHANG()
{
	pid_t pid = create_process(1, 200);
	int wstatus;
	pid_t result = waitpid(pid, &wstatus, WNOHANG);
	ASSERT_EQ(result, 0);

	result = waitpid(pid, &wstatus, 0);
	ASSERT_EQ(result, pid);
	ASSERT_EQ(wstatus, 0);
}

void test_signal_exit()
{
	pid_t pid;
	int wstatus;

	pid = create_process(2, SIGHUP); // child will raise SIGHUP
	waitpid(pid, &wstatus, 0);
	ASSERT_EQ(WIFSIGNALED(wstatus), 1);
	ASSERT_EQ(WIFEXITED(wstatus), 0);

	pid = create_process(2, SIGABRT);
	waitpid(pid, &wstatus, 0);
	ASSERT_EQ(WIFSIGNALED(wstatus), 1);
	ASSERT_EQ(WIFEXITED(wstatus), 0);

	pid = create_process(2, SIGCONT);
	waitpid(pid, &wstatus, 0);
	ASSERT_EQ(WIFSIGNALED(wstatus), 0);
	ASSERT_EQ(WIFEXITED(wstatus), 1);

}

int main()
{
	test_internal();
	test_ECHILD();
	test_waitpid();
	test_waitpid_WNOHANG();
	return 0;
}
