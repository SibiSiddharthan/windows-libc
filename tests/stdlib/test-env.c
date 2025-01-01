/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <stdlib.h>

int test_okay()
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

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();
	TEST(test_okay());
	VERIFY_RESULT_AND_EXIT();
}
