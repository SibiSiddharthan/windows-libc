/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <stdlib.h>
#include <stdlib-ext.h>
#include <test-macros.h>

void test_okay()
{
	setenv("t-env", "good", 0);
	char *env = getenv("t-env");
	ASSERT_STREQ(env, "good");

	setenv("t-env", "bad", 0); // Should not overwrite
	env = getenv("t-env");
	ASSERT_STREQ(env, "good");

	setenv("t-env", "best", 1);
	env = getenv("t-env");
	ASSERT_STREQ(env, "best");

	unsetenv("t-env");
	env = getenv("t-env");
	ASSERT_NULL(env);
}

int main()
{
	test_okay();
	return 0;
}
