/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <stdlib.h>
#include <stdlib-ext.h>
#include <string.h>
#include <unistd.h>

#define NUM_OF_RUNS 64

static int check_for_X(const char *template)
{
	for (int i = 0; template[i] != '\0'; ++i)
	{
		if (template[i] == 'X')
		{
			return 1;
		}
	}

	return 0;
}

int test_invalid_templates()
{
	int fd;

	errno = 0;
	fd = mkstemps("noxs", 0);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(EINVAL);

	errno = 0;
	fd = mkstemps("nomorexs", 0);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(EINVAL);

	errno = 0;
	fd = mkstemps("fewXXXX", 0);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(EINVAL);

	errno = 0;
	fd = mkstemps("fewXXXXabcd", 4);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(EINVAL);

	errno = 0;
	fd = mkstemps("invXXXXXXabc", 0);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(EINVAL);

	errno = 0;
	fd = mkstemps("invXXXXXXabc", 2);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(EINVAL);

	return 0;
}

int test_mkstemp()
{
	const char *mkstemp_template = "fileXXXXXX";

	for (int i = 0; i < NUM_OF_RUNS; ++i)
	{
		int fd;
		char template[64];

		memset(template, 0, 64);
		strcpy(template, mkstemp_template);

		errno = 0;
		fd = mkstemp(template);
		ASSERT_SUCCESS(close(fd));
		ASSERT_ERRNO(0);

		ASSERT_SUCCESS(check_for_X(template));
		ASSERT_MEMEQ(template, "file", 4);

		ASSERT_SUCCESS(unlink(template));
	}

	return 0;
}

int test_mkostemp()
{
	const char *mkostemp_template = "fileXXXXXX";

	for (int i = 0; i < NUM_OF_RUNS; ++i)
	{
		int fd;
		char template[64];

		memset(template, 0, 64);
		strcpy(template, mkostemp_template);

		errno = 0;
		fd = mkostemp(template, O_CLOEXEC);
		ASSERT_SUCCESS(close(fd));
		ASSERT_ERRNO(0);

		ASSERT_SUCCESS(check_for_X(template));
		ASSERT_MEMEQ(template, "file", 4);

		ASSERT_SUCCESS(unlink(template));
	}

	return 0;
}

int test_mkstemps()
{
	const char *mkstemps_template = "fileXXXXXXabcde";

	for (int i = 0; i < NUM_OF_RUNS; ++i)
	{
		int fd;
		char template[64];

		memset(template, 0, 64);
		strcpy(template, mkstemps_template);

		errno = 0;
		fd = mkstemps(template, 5);
		ASSERT_SUCCESS(close(fd));
		ASSERT_ERRNO(0);

		ASSERT_SUCCESS(check_for_X(template));
		ASSERT_MEMEQ(template, "file", 4);
		ASSERT_MEMEQ(template + 10, "abcde\0", 6);

		ASSERT_SUCCESS(unlink(template));
	}

	return 0;
}

int test_mkostemps()
{
	const char *mkostemps_template = "fileXXXXXXabcdef";

	for (int i = 0; i < NUM_OF_RUNS; ++i)
	{
		int fd;
		char template[64];

		memset(template, 0, 64);
		strcpy(template, mkostemps_template);

		errno = 0;
		fd = mkostemps(template, 6, O_CLOEXEC);
		ASSERT_SUCCESS(close(fd));
		ASSERT_ERRNO(0);

		ASSERT_SUCCESS(check_for_X(template));
		ASSERT_MEMEQ(template, "file", 4);
		ASSERT_MEMEQ(template + 10, "abcdef\0", 7);

		ASSERT_SUCCESS(unlink(template));
	}

	return 0;
}

int test_mktemp()
{
	const char *mktemp_template = "fileXXXXXX";

	for (int i = 0; i < NUM_OF_RUNS; ++i)
	{
		char *result;
		char template[64];

		memset(template, 0, 64);
		strcpy(template, mktemp_template);

		errno = 0;
		result = mktemp(template);
		ASSERT_NOTNULL(result);
		ASSERT_ERRNO(0);

		ASSERT_SUCCESS(check_for_X(template));
		ASSERT_MEMEQ(template, "file", 4);

		// mktemp does not create a file
		ASSERT_FAIL(unlink(result));
	}

	return 0;
}

int test_mkdtemp()
{
	const char *mkdtemp_template = "dirXXXXXX";
	for (int i = 0; i < NUM_OF_RUNS; ++i)
	{
		char *result;
		char template[64];

		memset(template, 0, 64);
		strcpy(template, mkdtemp_template);

		errno = 0;
		result = mkdtemp(template);
		ASSERT_NOTNULL(result);
		ASSERT_ERRNO(0);

		ASSERT_SUCCESS(check_for_X(template));
		ASSERT_MEMEQ(template, "dir", 3);

		ASSERT_SUCCESS(rmdir(result));
	}

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_invalid_templates());
	TEST(test_mkstemp());
	TEST(test_mkostemp());
	TEST(test_mkstemps());
	TEST(test_mkostemps());
	TEST(test_mktemp());
	TEST(test_mkdtemp());

	VERIFY_RESULT_AND_EXIT();
}
