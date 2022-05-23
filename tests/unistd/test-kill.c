/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <tests/test.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

#include <internal/spawn.h>
#include <Windows.h>

int test_EINVAL()
{
	errno = 0;
	int status = kill(getpid(), 64);
	ASSERT_ERRNO(EINVAL);
	ASSERT_EQ(status, -1);
	return 0;
}

int test_okay()
{
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	int status, wait_status;
	pid_t child_pid;

	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	memset(&pi, 0, sizeof(pi));

	CreateProcessA(NULL, "kill-helper", NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
	add_child(pi.dwProcessId, pi.hProcess);

	status = kill(pi.dwProcessId, SIGTERM);
	ASSERT_EQ(status, 0);

	child_pid = waitpid(pi.dwProcessId, &wait_status, 0);
	ASSERT_EQ(child_pid, pi.dwProcessId);
	ASSERT_EQ(wait_status, 3);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_EINVAL());
	TEST(test_okay());

	VERIFY_RESULT_AND_EXIT();
}
