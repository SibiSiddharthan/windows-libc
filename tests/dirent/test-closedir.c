/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
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