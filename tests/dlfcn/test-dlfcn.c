/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <dlfcn.h>

// Definitions from Windows.h
typedef unsigned long DWORD;
typedef unsigned short WORD;

#define ALL_PROCESSOR_GROUPS 0xffff
__declspec(dllimport) DWORD __stdcall GetMaximumProcessorCount(WORD);

int test_invalid_dll()
{
	void *handle = NULL;
	char *actual_err = NULL;

	handle = dlopen("bogus.dll", 0);
	ASSERT_NULL(handle);
	actual_err = dlerror();
	ASSERT_STREQ(actual_err, "The specified module could not be found.")
	ASSERT_FAIL(dlclose(NULL));

	return 0;
}

int test_dlopen_dlclose_kernel32()
{
	void *handle = NULL;

	handle = dlopen("kernel32.dll", 0);
	ASSERT_NOTNULL(handle);
	ASSERT_SUCCESS(dlclose(handle));

	return 0;
}

int test_dlsym()
{
	void *handle = NULL;
	char *actual_err = NULL;
	DWORD actual, expected;
	DWORD (*MyGetMaximumProcessorCount)(WORD) = NULL;
	void (*dummy)() = NULL;

	handle = dlopen("kernel32.dll", 0);
	ASSERT_NOTNULL(handle);

	// Valid function
	MyGetMaximumProcessorCount = (DWORD(*)(WORD))dlsym(handle, "GetMaximumProcessorCount");
	ASSERT_NOTNULL(MyGetMaximumProcessorCount);
	actual = MyGetMaximumProcessorCount(ALL_PROCESSOR_GROUPS);
	expected = GetMaximumProcessorCount(ALL_PROCESSOR_GROUPS);
	ASSERT_EQ(actual, expected);

	// Invalid function
	dummy = (void (*)())dlsym(handle, "dummy");
	ASSERT_NULL(dummy);
	actual_err = dlerror();
	ASSERT_STREQ(actual_err, "The specified procedure could not be found.");

	ASSERT_SUCCESS(dlclose(handle));

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_invalid_dll());
	TEST(test_dlopen_dlclose_kernel32());
	TEST(test_dlsym());

	VERIFY_RESULT_AND_EXIT();
}
