/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <signal-ext.h>
#include <test-macros.h>
#include <errno.h>

static int global_variable = 0;

void my_sighandler_1(int sig)
{
	global_variable += sig;
}

void my_sighandler_2(int sig)
{
	global_variable -= sig;
}

void test_EFAULT()
{
	int result;
	result = sigemptyset(NULL);
	ASSERT_EQ(result, -1);
	ASSERT_ERRNO(EFAULT);
	result = sigfillset(NULL);
	ASSERT_EQ(result, -1);
	ASSERT_ERRNO(EFAULT);
	result = sigaddset(NULL, SIGINT);
	ASSERT_EQ(result, -1);
	ASSERT_ERRNO(EFAULT);
	result = sigdelset(NULL, SIGINT);
	ASSERT_EQ(result, -1);
	ASSERT_ERRNO(EFAULT);
	result = sigismember(NULL, SIGINT);
	ASSERT_EQ(result, -1);
	ASSERT_ERRNO(EFAULT);
	result = sigpending(NULL);
	ASSERT_EQ(result, -1);
	ASSERT_ERRNO(EFAULT);
	result = sigprocmask(SIG_SETMASK, NULL, NULL);
	ASSERT_EQ(result, -1);
	ASSERT_ERRNO(EFAULT);
}

void test_EINVAL()
{
	errno = 0;
	int result;
	sigset_t s;

	result = sigemptyset(&s);
	ASSERT_EQ(result, 0);
	ASSERT_ERRNO(0);

	result = sigaddset(&s, SIGKILL);
	ASSERT_EQ(result, -1);
	ASSERT_ERRNO(EINVAL);
	result = sigaddset(&s, SIGSTOP);
	ASSERT_EQ(result, -1);
	ASSERT_ERRNO(EINVAL);

	result = sigprocmask(3, &s, NULL);
	ASSERT_EQ(result, -1);
	ASSERT_ERRNO(EINVAL);
}

void test_sigprocmask_1()
{
	global_variable = 0;
	int result;
	signal(SIGABRT, my_sighandler_1);
	sigset_t new, old, pending;
	result = sigemptyset(&new);
	ASSERT_EQ(result, 0);

	// Add signal mask
	result = sigaddset(&new, SIGABRT);
	ASSERT_EQ(result, 0);
	result = sigprocmask(SIG_BLOCK, &new, &old);
	ASSERT_EQ(result, 0);
	ASSERT_EQ(old, 0);

	raise(SIGABRT);
	ASSERT_EQ(global_variable, 0);

	// Remove signal mask
	result = sigprocmask(SIG_UNBLOCK, &new, &old);
	ASSERT_EQ(result, 0);
	ASSERT_EQ(old, 1u << SIGABRT);
	ASSERT_EQ(global_variable, SIGABRT);
}

void test_sigprocmask_2()
{
	global_variable = 0;
	signal(SIGUSR1, my_sighandler_1);
	signal(SIGUSR2, my_sighandler_2);
	sigset_t new, old, pending;
	sigemptyset(&new);

	// Add signal mask
	sigaddset(&new, SIGUSR1);
	sigprocmask(SIG_BLOCK, &new, NULL);
	raise(SIGUSR1);
	ASSERT_EQ(global_variable, 0);
	sigpending(&pending);
	ASSERT_EQ(pending, 1u << SIGUSR1);

	sigaddset(&new, SIGUSR2);
	sigprocmask(SIG_BLOCK, &new, NULL);
	raise(SIGUSR2);
	ASSERT_EQ(global_variable, 0);
	sigpending(&pending);
	ASSERT_EQ(pending, ((1u << SIGUSR1) | (1u << SIGUSR2)));

	// Remove signal mask
	new = 0;
	sigprocmask(SIG_SETMASK, &new, &old);
	ASSERT_EQ(old, ((1u << SIGUSR1) | (1u << SIGUSR2)));
	ASSERT_EQ(global_variable, SIGUSR1 - SIGUSR2);
	sigpending(&pending);
	ASSERT_EQ(pending, 0);
}

int main()
{
	test_EFAULT();
	test_EINVAL();
	test_sigprocmask_1();
	test_sigprocmask_2();
	return 0;
}
