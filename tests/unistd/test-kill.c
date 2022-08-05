/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <errno.h>
#include <signal.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

int test_EINVAL()
{
	errno = 0;
	int status = kill(getpid(), 64);
	ASSERT_ERRNO(EINVAL);
	ASSERT_EQ(status, -1);
	return 0;
}

int test_kill()
{
	int status, wstatus = 0;
	pid_t pid;
	char *argv[] = {"kill-helper", NULL};

	status = posix_spawn(&pid, "kill-helper", NULL, NULL, argv, NULL);
	ASSERT_EQ(status, 0);

	status = kill(pid, SIGTERM);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, &wstatus, 0);
	ASSERT_EQ(status, pid);
	ASSERT_EQ(wstatus, 128 + SIGTERM);

	return 0;
}

int test_suspend()
{
	int status, wstatus = 0;
	pid_t pid;
	char *argv[] = {"kill-helper", NULL};

	status = posix_spawn(&pid, "kill-helper", NULL, NULL, argv, NULL);
	ASSERT_EQ(status, 0);

	// This should suspend the process.
	status = kill(pid, SIGSTOP);
	ASSERT_EQ(status, 0);

	usleep(1000);

	status = waitpid(pid, &wstatus, WNOHANG);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(wstatus, -1);

	// Now kill the process.
	status = kill(pid, SIGTERM);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, &wstatus, 0);
	ASSERT_EQ(status, pid);
	ASSERT_EQ(wstatus, 128 + SIGTERM);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_EINVAL());
	TEST(test_kill());
	TEST(test_suspend());

	VERIFY_RESULT_AND_EXIT();
}
