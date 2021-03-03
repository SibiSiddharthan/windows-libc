/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <signal-ext.h>
#include <test-macros.h>
#include <errno.h>

static int global_variable = 0;

void handler_1(int sig)
{
	++global_variable;
}

void handler_2(int sig)
{
	--global_variable;
}

void test_SIGKILL()
{
	_crt_signal_t result = signal(SIGKILL, handler_1);
	ASSERT_ERRNO(EINVAL);
	ASSERT_EQ(result, SIG_ERR);
}

void test_our_signals()
{
	signal(SIGHUP, handler_1);
	raise(SIGHUP);
	ASSERT_EQ(global_variable, 1);
	signal(SIGPIPE, handler_2);
	raise(SIGPIPE);
	ASSERT_EQ(global_variable, 0);
}

int main()
{
	test_our_signals();
	test_SIGKILL();
}
