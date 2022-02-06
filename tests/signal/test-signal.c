/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <test-macros.h>
#include <signal.h>

static int global_variable = 0;

void handler_1(int sig)
{
	++global_variable;
}

void handler_2(int sig)
{
	--global_variable;
}

int test_SIGKILL()
{
	// signal handler for SIGKILL can't be overridden
	_crt_signal_t old_handler = signal(SIGKILL, handler_1);
	ASSERT_ERRNO(EINVAL);
	ASSERT_EQ(old_handler, SIG_ERR);
	return 0;
}

int test_custom_signals()
{
	_crt_signal_t old_handler;

	old_handler = signal(SIGHUP, handler_1);
	ASSERT_EQ(old_handler, SIG_DFL);
	raise(SIGHUP);
	ASSERT_EQ(global_variable, 1);

	old_handler = signal(SIGPIPE, handler_2);
	ASSERT_EQ(old_handler, SIG_DFL);
	raise(SIGPIPE);
	ASSERT_EQ(global_variable, 0);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_SIGKILL());
	TEST(test_custom_signals());

	VERIFY_RESULT_AND_EXIT();
}
