/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <dirent.h>
#include <errno.h>
#include <test-macros.h>

void test_NULL()
{
	errno = 0;
	DIR *D = opendir(NULL);
	ASSERT_ERRNO(ENOENT);
	ASSERT_NULL(D);
}

void test_ENOENT()
{
	errno = 0;
	DIR *D = opendir("junk");
	ASSERT_ERRNO(ENOENT);
	ASSERT_NULL(D);
}

void test_ENOTDIR()
{
	errno = 0;
	DIR *D = opendir("CMakeCache.txt");
	ASSERT_ERRNO(ENOTDIR);
	ASSERT_NULL(D);
}

void test_okay()
{
	errno = 0;
	DIR *D = opendir(".");
	ASSERT_ERRNO(0);
	closedir(D);
}

void test_okay_slash()
{
	errno = 0;
	DIR *D = opendir("./");
	ASSERT_ERRNO(0);
	closedir(D);
}

int main()
{
	test_NULL();
	test_ENOENT();
	test_ENOTDIR();
	test_okay();
	test_okay_slash();
	return 0;
}
