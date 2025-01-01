/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <signal.h>

static int global_variable = 0;

void my_sighandler(int sig)
{
	global_variable += sig;
}

int test_SA_RESETHAND()
{
	struct sigaction S;
	int status;

	S.sa_handler = my_sighandler;
	S.sa_flags = SA_RESETHAND | SA_NODEFER;
	S.sa_mask = 0;
	// Pick one with default action as ignore.
	status = sigaction(SIGCONT, &S, NULL);
	ASSERT_EQ(status, 0);

	raise(SIGCONT);
	ASSERT_EQ(global_variable, SIGCONT);
	// SIGCONT now must be noop.
	raise(SIGCONT);
	ASSERT_EQ(global_variable, SIGCONT);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	global_variable = 0;
	TEST(test_SA_RESETHAND());

	VERIFY_RESULT_AND_EXIT();
}
