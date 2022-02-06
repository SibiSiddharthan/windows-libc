/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <test-macros.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int test_NULL()
{
	errno = 0;

	DIR *D = opendir(NULL);
	ASSERT_ERRNO(ENOENT);
	ASSERT_NULL(D);
	ASSERT_FAIL(closedir(D));

	return 0;
}

int test_ENOENT()
{
	errno = 0;

	DIR *D = opendir("junk");
	ASSERT_ERRNO(ENOENT);
	ASSERT_NULL(D);
	ASSERT_FAIL(closedir(D));

	return 0;
}

int test_ENOTDIR()
{
	errno = 0;
	DIR *D = NULL;
	const char *filename = "t-opendir.file";
	const char *filename_with_slash = "t-opendir.file/";

	int fd = creat(filename, 0700);
	ASSERT_SUCCESS(close(fd));

	D = opendir(filename);
	ASSERT_ERRNO(ENOTDIR);
	ASSERT_NULL(D);
	ASSERT_FAIL(closedir(D));

	// try again but with a trailing slash this time, should fail as well
	D = opendir(filename_with_slash);
	ASSERT_ERRNO(ENOTDIR);
	ASSERT_NULL(D);
	ASSERT_FAIL(closedir(D));

	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_cwd()
{
	errno = 0;
	DIR *D = NULL;

	D = opendir(".");
	ASSERT_ERRNO(0);
	ASSERT_NOTNULL(D);
	ASSERT_SUCCESS(closedir(D));

	// again with trailing slash
	D = opendir("./");
	ASSERT_ERRNO(0);
	ASSERT_NOTNULL(D);
	ASSERT_SUCCESS(closedir(D));

	return 0;
}

int test_dir()
{
	errno = 0;
	DIR *D = NULL;
	const char *dirname = "t-opendir1";
	const char *dirname_with_trailing_slash = "t-opendir1/";

	ASSERT_SUCCESS(mkdir(dirname, 0700));

	D = opendir(dirname);
	ASSERT_NOTNULL(D);
	ASSERT_SUCCESS(closedir(D));

	// do it again, with trailing slash this time
	D = opendir(dirname_with_trailing_slash);
	ASSERT_NOTNULL(D);
	ASSERT_SUCCESS(closedir(D));

	ASSERT_SUCCESS(rmdir(dirname));

	return 0;
}

void cleanup()
{
	remove("t-opendir.file");
	remove("t-opendir1");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_NULL());
	TEST(test_ENOENT());
	TEST(test_ENOTDIR());
	TEST(test_cwd());
	TEST(test_dir());

	VERIFY_RESULT_AND_EXIT();
}
