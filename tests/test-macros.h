/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_TEST_MACROS_H
#define WLIBC_TEST_MACROS_H

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <wchar.h>

#define ASSERT_ERRNO(expected)                                                                                       \
	if (errno != expected)                                                                                           \
	{                                                                                                                \
		printf("Assertion failed at %s:%d. Expected value of %d but got %d\n", __FILE__, __LINE__, expected, errno); \
		return 1;                                                                                                    \
	}

#define ASSERT_NULL(pointer)                                                         \
	if (pointer != NULL)                                                             \
	{                                                                                \
		printf("Assertion failed at %s:%d. Pointer not NULL\n", __FILE__, __LINE__); \
		return 1;                                                                    \
	}

#define ASSERT_NOTNULL(pointer)                                                     \
	if (pointer == NULL)                                                            \
	{                                                                               \
		printf("Assertion failed at %s:%d. Pointer is NULL\n", __FILE__, __LINE__); \
		return 1;                                                                   \
	}

#define ASSERT_EQ(actual, expected)                                                                                        \
	if (actual != expected)                                                                                                \
	{                                                                                                                      \
		printf("Assertion failed at %s:%d in %s. Expected value of %lld but got %lld\n", __FILE__, __LINE__, __FUNCTION__, \
			   (long long)expected, (long long)actual);                                                                    \
		return 1;                                                                                                          \
	}

#define ASSERT_NOTEQ(val_1, val_2)                                                                                                       \
	if (val_1 == val_2)                                                                                                                  \
	{                                                                                                                                    \
		printf("Assertion failed at %s:%d. %lld should not be equal to %lld\n", __FILE__, __LINE__, (long long)val_1, (long long)val_2); \
		return 1;                                                                                                                        \
	}

#define ASSERT_STREQ(actual, expected)                                                                                    \
	if (strcmp(actual, expected) != 0)                                                                                    \
	{                                                                                                                     \
		printf("Assertion failed at %s:%d. Expected value of \n%s\nbut got\n%s\n", __FILE__, __LINE__, expected, actual); \
		return 1;                                                                                                         \
	}

#define ASSERT_WSTREQ(actual, expected)                                                                                     \
	if (wcscmp(actual, expected) != 0)                                                                                      \
	{                                                                                                                       \
		printf("Assertion failed at %s:%d. Expected value of \n%ls\nbut got\n%ls\n", __FILE__, __LINE__, expected, actual); \
		return 1;                                                                                                           \
	}

#define ASSERT_MEMEQ(actual, expected, size)                                                                                              \
	if (memcmp(actual, expected, size) != 0)                                                                                              \
	{                                                                                                                                     \
		printf("Assertion failed at %s:%d. Expected value of \n%.*s\nbut got\n%.*s\n", __FILE__, __LINE__, size, expected, size, actual); \
		return 1;                                                                                                                         \
	}

#define ASSERT_SUCCESS(op)                                                                                                          \
	{                                                                                                                               \
		if (op == -1)                                                                                                               \
		{                                                                                                                           \
			printf("Operation %s called at %s:%d in %s failed but should have succeeded\n", #op, __FILE__, __LINE__, __FUNCTION__); \
			return 1;                                                                                                               \
		}                                                                                                                           \
	}

#define ASSERT_FAIL(op)                                                                                                             \
	{                                                                                                                               \
		if (op != -1)                                                                                                               \
		{                                                                                                                           \
			printf("Operation %s called at %s:%d in %s succeeded but should have failed\n", #op, __FILE__, __LINE__, __FUNCTION__); \
			return 1;                                                                                                               \
		}                                                                                                                           \
	}

#define INITIAILIZE_TESTS()       \
	int __wlibc_num_of_tests = 0; \
	int __wlibc_test_result = 0;  \
	void (*__wlibc_cleanup_on_failure)() = NULL;

#define CLEANUP(cleanup) __wlibc_cleanup_on_failure = cleanup;

#define TEST(test)                                                              \
	++__wlibc_num_of_tests;                                                     \
	{                                                                           \
		int __local_status = test;                                              \
		if (__local_status)                                                     \
			++__wlibc_test_result;                                              \
		printf("%s -- %s\n", #test, __local_status == 0 ? "passed" : "failed"); \
	}

#define VERIFY_RESULT_AND_EXIT()                                                                                                        \
	printf("%d tests (%d passed, %d failed)\n", __wlibc_num_of_tests, __wlibc_num_of_tests - __wlibc_test_result, __wlibc_test_result); \
	if (__wlibc_test_result != 0 && __wlibc_cleanup_on_failure != NULL)                                                                 \
	{                                                                                                                                   \
		printf("%d tests failed, cleaning up byproducts of tests\n", __wlibc_test_result);                                              \
		__wlibc_cleanup_on_failure();                                                                                                   \
	}                                                                                                                                   \
	else                                                                                                                                \
	{                                                                                                                                   \
		printf("All tests passed\n");                                                                                                   \
	}                                                                                                                                   \
	return __wlibc_test_result == 0 ? 0 : 1;

#endif
