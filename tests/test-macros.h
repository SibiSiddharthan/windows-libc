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

#define ASSERT_ERRNO(expected)                                                                                         \
	if (errno != expected)                                                                                             \
	{                                                                                                                  \
		printf_s("Assertion failed at %s:%d. Expected value of %d but got %d\n", __FILE__, __LINE__, expected, errno); \
		exit(1);                                                                                                       \
	}

#define ASSERT_NULL(pointer)                                                           \
	if (pointer != NULL)                                                               \
	{                                                                                  \
		printf_s("Assertion failed at %s:%d. Pointer not NULL\n", __FILE__, __LINE__); \
		exit(1);                                                                       \
	}

#define ASSERT_EQ(actual, expected)                                                                                     \
	if (actual != expected)                                                                                             \
	{                                                                                                                   \
		printf_s("Assertion failed at %s:%d. Expected value of %d but got %d\n", __FILE__, __LINE__, expected, actual); \
		exit(1);                                                                                                        \
	}

#define ASSERT_STREQ(actual, expected)                                                                                      \
	if (strcmp(actual, expected) != 0)                                                                                      \
	{                                                                                                                       \
		printf_s("Assertion failed at %s:%d. Expected value of \n%s\nbut got\n%s\n", __FILE__, __LINE__, expected, actual); \
		exit(1);                                                                                                            \
	}

#endif
