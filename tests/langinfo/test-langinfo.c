/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <langinfo.h>
#include <test-macros.h>
#include <locale.h>
#include <internal/langinfo.h>

int test_langinfo()
{
	// Check whether subsequent calls to nl_langinfo does not corrupt previous results
	char *results[16];
	results[0] = nl_langinfo(CODESET);
	results[1] = nl_langinfo(AM_STR);
	results[2] = nl_langinfo(PM_STR);

	results[3] = nl_langinfo(DAY_1);
	results[4] = nl_langinfo(DAY_2);
	results[5] = nl_langinfo(DAY_3);

	results[6] = nl_langinfo(ABDAY_1);
	results[7] = nl_langinfo(ABDAY_2);
	results[8] = nl_langinfo(ABDAY_3);

	results[9] = nl_langinfo(MON_1);
	results[10] = nl_langinfo(MON_2);
	results[11] = nl_langinfo(MON_3);

	results[12] = nl_langinfo(ABMON_1);
	results[13] = nl_langinfo(ABMON_2);
	results[14] = nl_langinfo(ABMON_3);

	ASSERT_STREQ(results[0], "C");
	ASSERT_STREQ(results[1], "AM");
	ASSERT_STREQ(results[2], "PM");

	ASSERT_STREQ(results[3], "Sunday");
	ASSERT_STREQ(results[4], "Monday");
	ASSERT_STREQ(results[5], "Tuesday");

	ASSERT_STREQ(results[6], "Sun");
	ASSERT_STREQ(results[7], "Mon");
	ASSERT_STREQ(results[8], "Tue");

	ASSERT_STREQ(results[9], "January");
	ASSERT_STREQ(results[10], "February");
	ASSERT_STREQ(results[11], "March");

	ASSERT_STREQ(results[12], "Jan");
	ASSERT_STREQ(results[13], "Feb");
	ASSERT_STREQ(results[14], "Mar");

	return 0;
}

int test_CODESET_UTF8()
{
	setlocale(LC_ALL, ".utf8");
	char *result = nl_langinfo(CODESET);
	ASSERT_STREQ(result, "UTF-8");

	return 0;
}

int test_cycle()
{
	// NOTE: If MAX_LANGINFO_INVOKE_COUNT is changed this function needs to be modified
	char *results[2 * MAX_LANGINFO_INVOKE_COUNT];
	for (int i = 0; i < MAX_LANGINFO_INVOKE_COUNT; i++)
	{
		results[i] = nl_langinfo(i % 55);
	}

	for (int i = 0; i < MAX_LANGINFO_INVOKE_COUNT / 2; i++)
	{
		results[i + MAX_LANGINFO_INVOKE_COUNT] = nl_langinfo((i + MAX_LANGINFO_INVOKE_COUNT) % 55);
	}

	// Test edge cases
	ASSERT_STREQ(results[MAX_LANGINFO_INVOKE_COUNT - 1], "Mar");                                   // ABMON_3
	ASSERT_STREQ(results[MAX_LANGINFO_INVOKE_COUNT + MAX_LANGINFO_INVOKE_COUNT / 2 - 1], "^[yY]"); // YESEXPR

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_langinfo());
	TEST(test_CODESET_UTF8());
	TEST(test_cycle());

	VERIFY_RESULT_AND_EXIT();
}
