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

#include <internal/process.h>
#include <Windows.h>

void test_EINVAL()
{
	errno = 0;
	int status = kill(getpid(), 64);
	ASSERT_ERRNO(EINVAL);
	ASSERT_EQ(status, -1);
}

void test_okay()
{
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;

	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	memset(&pi, 0, sizeof(pi));

	BOOL result = CreateProcessA(NULL, "kill-helper", NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
	add_child(pi.dwProcessId, pi.hProcess);

	kill(pi.dwProcessId, SIGTERM);
	int wstatus;
	waitpid(pi.dwProcessId, &wstatus, 0);
	ASSERT_EQ(wstatus, 3);
}

int main()
{
	test_EINVAL();
	test_okay();
	return 0;
}
