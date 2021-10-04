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
		exit(1);                                                                                                     \
	}

#define ASSERT_NULL(pointer)                                                         \
	if (pointer != NULL)                                                             \
	{                                                                                \
		printf("Assertion failed at %s:%d. Pointer not NULL\n", __FILE__, __LINE__); \
		exit(1);                                                                     \
	}

#define ASSERT_EQ(actual, expected)                                                                                         \
	if (actual != expected)                                                                                                 \
	{                                                                                                                       \
		printf("Assertion failed at %s:%d. Expected value of %lld but got %lld\n", __FILE__, __LINE__, (long long)expected, \
			   (long long)actual);                                                                                          \
		exit(1);                                                                                                            \
	}

#define ASSERT_NOTEQ(val_1, val_2)                                                                                                       \
	if (val_1 == val_2)                                                                                                                  \
	{                                                                                                                                    \
		printf("Assertion failed at %s:%d. %lld should not be equal to %lld\n", __FILE__, __LINE__, (long long)val_1, (long long)val_2); \
		exit(1);                                                                                                                         \
	}

#define ASSERT_STREQ(actual, expected)                                                                                    \
	if (strcmp(actual, expected) != 0)                                                                                    \
	{                                                                                                                     \
		printf("Assertion failed at %s:%d. Expected value of \n%s\nbut got\n%s\n", __FILE__, __LINE__, expected, actual); \
		exit(1);                                                                                                          \
	}

#define ASSERT_WSTREQ(actual, expected)                                                                                     \
	if (wcscmp(actual, expected) != 0)                                                                                      \
	{                                                                                                                       \
		printf("Assertion failed at %s:%d. Expected value of \n%ls\nbut got\n%ls\n", __FILE__, __LINE__, expected, actual); \
		exit(1);                                                                                                            \
	}

#define ASSERT_MEMEQ(actual, expected, size)                                                                                              \
	if (memcmp(actual, expected, size) != 0)                                                                                              \
	{                                                                                                                                     \
		printf("Assertion failed at %s:%d. Expected value of \n%.*s\nbut got\n%.*s\n", __FILE__, __LINE__, size, expected, size, actual); \
		exit(1);                                                                                                                          \
	}

#endif
