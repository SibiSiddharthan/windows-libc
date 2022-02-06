/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <test-macros.h>
#include <signal.h>

static int global_variable = 0;

void my_sighandler(int sig)
{
	global_variable += sig;
}

int test_SA_NODEFER()
{
	global_variable = 0;
	int status;
	struct sigaction S;
	sigset_t s;

	S.sa_handler = my_sighandler;
	S.sa_flags = 0;
	S.sa_mask = 0;
	status = sigaction(SIGHUP, &S, NULL);
	ASSERT_EQ(status, 0);

	raise(SIGHUP);
	ASSERT_EQ(global_variable, SIGHUP);
	// SIGHUP now must be blocked
	raise(SIGHUP);
	ASSERT_EQ(global_variable, SIGHUP);

	S.sa_flags = SA_NODEFER;
	status = sigaction(SIGHUP, &S, NULL);
	ASSERT_EQ(status, 0);

	// unblock SIGHUP
	status = sigemptyset(&s);
	ASSERT_EQ(status, 0);
	status = sigprocmask(SIG_SETMASK, &s, NULL);
	ASSERT_EQ(status, 0);
	// pending SIGHUP is raised here
	ASSERT_EQ(global_variable, SIGHUP + SIGHUP);

	raise(SIGHUP);
	ASSERT_EQ(global_variable, SIGHUP + SIGHUP + SIGHUP);

	return 0;
}

int test_SA_RESETHAND()
{
	global_variable = 0;
	struct sigaction S;
	int status;

	S.sa_handler = my_sighandler;
	S.sa_flags = SA_RESETHAND | SA_NODEFER;
	S.sa_mask = 0;
	// Pick one with SIG_IGN
	status = sigaction(SIGCONT, &S, NULL);
	ASSERT_EQ(status, 0);

	raise(SIGCONT);
	ASSERT_EQ(global_variable, SIGCONT);
	// SIGCONT now must be noop
	raise(SIGCONT);
	ASSERT_EQ(global_variable, SIGCONT);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_SA_NODEFER());
	TEST(test_SA_RESETHAND());

	VERIFY_RESULT_AND_EXIT();
}
