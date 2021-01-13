/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <dirent.h>
#include <test-macros.h>

void test_NULL()
{
	DIR *D = NULL;
	ASSERT_EQ(closedir(D), -1);
}

void test_okay()
{
	DIR *D = opendir(".");
	ASSERT_EQ(closedir(D), 0);
}

int main()
{
	test_NULL();
	test_okay();
	return 0;
}
