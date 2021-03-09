/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <signal-ext.h>
#include <test-macros.h>
#include <errno.h>

static int global_variable = 0;

void my_sighandler(int sig)
{
	global_variable += sig;
}

void test_SA_NODEFER()
{
	global_variable = 0;

	struct sigaction S;
	S.sa_handler = my_sighandler;
	S.sa_flags = 0;
	S.sa_mask = 0;
	sigaction(SIGHUP, &S, NULL);

	raise(SIGHUP);
	ASSERT_EQ(global_variable, SIGHUP);
	// SIGHUP now must be blocked
	raise(SIGHUP);
	ASSERT_EQ(global_variable, SIGHUP);

	S.sa_flags = SA_NODEFER;
	sigaction(SIGHUP, &S, NULL);

	// unblock SIGHUP
	sigset_t s;
	sigemptyset(&s);
	sigprocmask(SIG_SETMASK, &s, NULL);
	// pending SIGHUP is raised here
	ASSERT_EQ(global_variable, SIGHUP + SIGHUP);

	raise(SIGHUP);
	ASSERT_EQ(global_variable, SIGHUP + SIGHUP + SIGHUP);
}

void test_SA_RESETHAND()
{
	global_variable = 0;

	struct sigaction S;
	S.sa_handler = my_sighandler;
	S.sa_flags = SA_RESETHAND | SA_NODEFER;
	S.sa_mask = 0;
	// Pick one with SIG_IGN
	sigaction(SIGCONT, &S, NULL);

	raise(SIGCONT);
	ASSERT_EQ(global_variable, SIGCONT);
	// SIGCONT now must be noop
	raise(SIGCONT);
	ASSERT_EQ(global_variable, SIGCONT);
}

int main()
{
	test_SA_NODEFER();
	test_SA_RESETHAND();
	return 0;
}
