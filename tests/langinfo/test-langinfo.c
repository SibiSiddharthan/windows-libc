/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <test-macros.h>
#include <langinfo.h>
#include <locale.h>

int test_langinfo()
{
	char *result;

	result = nl_langinfo(CODESET);
	ASSERT_STREQ(result, "C");

	result = nl_langinfo(AM_STR);
	ASSERT_STREQ(result, "AM");
	result = nl_langinfo(PM_STR);
	ASSERT_STREQ(result, "PM");

	result = nl_langinfo(DAY_1);
	ASSERT_STREQ(result, "Sunday");
	result = nl_langinfo(DAY_2);
	ASSERT_STREQ(result, "Monday");
	result = nl_langinfo(DAY_3);
	ASSERT_STREQ(result, "Tuesday");

	result = nl_langinfo(ABDAY_1);
	ASSERT_STREQ(result, "Sun");
	result = nl_langinfo(ABDAY_2);
	ASSERT_STREQ(result, "Mon");
	result = nl_langinfo(ABDAY_3);
	ASSERT_STREQ(result, "Tue");

	result = nl_langinfo(MON_1);
	ASSERT_STREQ(result, "January");
	result = nl_langinfo(MON_2);
	ASSERT_STREQ(result, "February");
	result = nl_langinfo(MON_3);
	ASSERT_STREQ(result, "March");

	result = nl_langinfo(ABMON_1);
	ASSERT_STREQ(result, "Jan");
	result = nl_langinfo(ABMON_2);
	ASSERT_STREQ(result, "Feb");
	result = nl_langinfo(ABMON_3);
	ASSERT_STREQ(result, "Mar");

	return 0;
}

int test_CODESET_UTF8()
{
	setlocale(LC_ALL, ".utf8");
	char *result = nl_langinfo(CODESET);
	ASSERT_STREQ(result, "UTF-8");

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_langinfo());
	TEST(test_CODESET_UTF8());

	VERIFY_RESULT_AND_EXIT();
}
