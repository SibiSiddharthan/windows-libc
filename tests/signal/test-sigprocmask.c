/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <signal.h>

static int global_variable = 0;

void my_sighandler_1(int sig)
{
	global_variable += sig;
}

void my_sighandler_2(int sig)
{
	global_variable -= sig;
}

int test_EINVAL_1()
{
	int result;

	result = sigemptyset(NULL);
	ASSERT_EQ(result, -1);
	ASSERT_ERRNO(EINVAL);
	result = sigfillset(NULL);
	ASSERT_EQ(result, -1);
	ASSERT_ERRNO(EINVAL);
	result = sigaddset(NULL, SIGINT);
	ASSERT_EQ(result, -1);
	ASSERT_ERRNO(EINVAL);
	result = sigdelset(NULL, SIGINT);
	ASSERT_EQ(result, -1);
	ASSERT_ERRNO(EINVAL);
	result = sigismember(NULL, SIGINT);
	ASSERT_EQ(result, -1);
	ASSERT_ERRNO(EINVAL);
	result = sigpending(NULL);
	ASSERT_EQ(result, -1);
	ASSERT_ERRNO(EINVAL);

	return 0;
}

int test_EINVAL_2()
{
	errno = 0;
	int result;
	sigset_t s;

	result = sigemptyset(&s);
	ASSERT_EQ(result, 0);
	ASSERT_ERRNO(0);

	result = sigaddset(&s, NSIG);
	ASSERT_EQ(result, -1);
	ASSERT_ERRNO(EINVAL);
	result = sigaddset(&s, 0);
	ASSERT_EQ(result, -1);
	ASSERT_ERRNO(EINVAL);

	result = sigprocmask(3, &s, NULL);
	ASSERT_EQ(result, -1);
	ASSERT_ERRNO(EINVAL);

	return 0;
}

int test_sigprocmask_1()
{
	global_variable = 0;
	int result;
	sigset_t new, old;

	result = sigemptyset(&new);
	ASSERT_EQ(result, 0);

	signal(SIGABRT, my_sighandler_1);

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

	// Query signal mask
	result = sigprocmask(SIG_UNBLOCK, NULL, &old);
	ASSERT_EQ(result, 0);
	ASSERT_EQ(old, 0);

	return 0;
}

int test_sigprocmask_2()
{
	global_variable = 0;
	int result;
	sigset_t new, old, pending;

	result = sigemptyset(&new);
	ASSERT_EQ(result, 0);

	signal(SIGUSR1, my_sighandler_1);
	signal(SIGUSR2, my_sighandler_2);

	// Add signal mask
	result = sigaddset(&new, SIGUSR1);
	ASSERT_EQ(result, 0);
	result = sigprocmask(SIG_BLOCK, &new, NULL);
	ASSERT_EQ(result, 0);

	raise(SIGUSR1);
	ASSERT_EQ(global_variable, 0);

	result = sigpending(&pending);
	ASSERT_EQ(result, 0);
	ASSERT_EQ(pending, 1u << SIGUSR1);

	result = sigaddset(&new, SIGUSR2);
	ASSERT_EQ(result, 0);
	result = sigprocmask(SIG_BLOCK, &new, NULL);
	ASSERT_EQ(result, 0);

	raise(SIGUSR2);
	ASSERT_EQ(global_variable, 0);

	result = sigpending(&pending);
	ASSERT_EQ(result, 0);
	ASSERT_EQ(pending, ((1u << SIGUSR1) | (1u << SIGUSR2)));

	// Remove signal mask
	new = 0;
	result = sigprocmask(SIG_SETMASK, &new, &old);
	ASSERT_EQ(result, 0);
	ASSERT_EQ(old, ((1u << SIGUSR1) | (1u << SIGUSR2)));
	ASSERT_EQ(global_variable, SIGUSR1 - SIGUSR2);
	result = sigpending(&pending);
	ASSERT_EQ(result, 0);
	ASSERT_EQ(pending, 0);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_EINVAL_1());
	TEST(test_EINVAL_2());
	TEST(test_sigprocmask_1());
	TEST(test_sigprocmask_2());

	VERIFY_RESULT_AND_EXIT();
}
