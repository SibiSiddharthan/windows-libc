/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <dlfcn.h>
#include <test-macros.h>

typedef unsigned long DWORD;
typedef unsigned short WORD;

#define ALL_PROCESSOR_GROUPS 0xffff
DWORD GetMaximumProcessorCount(WORD);

void test_dlopen()
{
	void *handle = dlopen("bogus.dll", 0);
	ASSERT_NULL(handle);
}

void test_dlsym()
{
	void *handle = dlopen("kernel32.dll", 0);
	DWORD expected = GetMaximumProcessorCount(ALL_PROCESSOR_GROUPS);
	DWORD (*MyGetMaximumProcessorCount)(WORD);
	MyGetMaximumProcessorCount = (DWORD(*)(WORD))dlsym(handle, "GetMaximumProcessorCount");
	DWORD actual = MyGetMaximumProcessorCount(ALL_PROCESSOR_GROUPS);
	dlclose(handle);
	ASSERT_EQ(actual, expected);
}

void test_dlclose()
{
	ASSERT_EQ(dlclose(NULL), -1);
	void *handle = dlopen("kernel32.dll", 0);
	int status = dlclose(handle);
	ASSERT_EQ(status, 0);
}

void test_dlerror1()
{
	void *handle = dlopen("bogus.dll", 0);
	char *actual_err = dlerror();
	ASSERT_STREQ(actual_err, "The specified module could not be found.\r\n")
}

void test_dlerror2()
{
	void *handle = dlopen("kernel32.dll", 0);
	void (*dummy)();
	dummy = (void (*)())dlsym(handle, "dummy");
	char *actual_err = dlerror();
	ASSERT_STREQ(actual_err, "The specified procedure could not be found.\r\n")
	dlclose(handle);
}

int main()
{
	test_dlopen();
	test_dlsym();
	test_dlclose();
	test_dlerror1();
	test_dlerror2();

	return 0;
}
