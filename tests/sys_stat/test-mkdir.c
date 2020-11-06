/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <sys/stat.h>
#include <test-macros.h>
#include <errno.h>
#include <Windows.h>
#include <unistd.h>

void test_ENOENT()
{
	errno = 0;
	int status = mkdir("", 0700);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOENT);
}

void test_EEXIST()
{
	errno = 0;
	int status = mkdir("CMakeFiles", 0700);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EEXIST);
}

void test_okay()
{
	errno = 0;
	int status = mkdir("t-mkdir", 0700);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(GetFileAttributesA("t-mkdir"), FILE_ATTRIBUTE_DIRECTORY);
	rmdir("t-mkdir");
}

int main()
{
	test_ENOENT();
	test_EEXIST();
	test_okay();
	return 0;
}
