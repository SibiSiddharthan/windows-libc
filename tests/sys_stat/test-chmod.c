/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <sys/stat.h>
#include <unistd.h>
#include <test-macros.h>
#include <errno.h>
#include <fcntl.h>

int test_chmod_file()
{
	int fd;
	int status;
	struct stat statbuf;
	const char *filename = "t-chmod.file";

	// Start with no permissions
	fd = creat(filename, 0);
	ASSERT_SUCCESS(close(fd));

	status = stat(filename, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, S_IFREG);

	for (int i = 0; i < 512; ++i)
	{
		status = chmod(filename, i);
		ASSERT_EQ(status, 0);

		status = stat(filename, &statbuf);
		ASSERT_EQ(status, 0);
		ASSERT_EQ(statbuf.st_mode, (S_IFREG | i));
	}

	ASSERT_SUCCESS(unlink(filename));
	return 0;
}

int test_chmod_dir()
{
	int status;
	struct stat statbuf;
	const char *dirname = "t-chmod.file";

	// Start with no permissions
	ASSERT_SUCCESS(mkdir(dirname, 0));

	status = stat(dirname, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, S_IFDIR);

	for (int i = 0; i < 512; ++i)
	{
		status = chmod(dirname, i);
		ASSERT_EQ(status, 0);

		status = stat(dirname, &statbuf);
		ASSERT_EQ(status, 0);
		ASSERT_EQ(statbuf.st_mode, (S_IFDIR | i));
	}

	ASSERT_SUCCESS(rmdir(dirname));
	return 0;
}

int test_fchmod()
{
	int fd;
	int status;
	struct stat statbuf;
	const char *filename = "t-fchmod";

	fd = creat(filename, 0700);

	status = stat(filename, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | 0700));

	status = fchmod(fd, 0777);
	ASSERT_EQ(status, 0);

	status = stat(filename, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | 0777));

	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(unlink(filename));
	return 0;
}

void cleanup()
{
	remove("t-chmod.file");
	remove("t-chmod.dir");
	remove("t-fchmod");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_chmod_file());
	TEST(test_chmod_dir());
	TEST(test_fchmod());

	VERIFY_RESULT_AND_EXIT();
}
