/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <test-macros.h>
#include <errno.h>

int test_EBADF()
{
	errno = 0;
	int status = mkdirat(-1, "dummy", 0700);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EBADF);
	return 0;
}

int test_ENOTDIR()
{
	errno = 0;
	int fd, status;
	const char *filename = "not-a-directory";

	fd = creat(filename, 0700);
	ASSERT_NOTEQ(fd, -1);
	status = mkdirat(fd, "dummy", 0700);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOTDIR);

	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_absolute_path()
{
	errno = 0;
	int status;
	char absolute_dirpath[260];
	const char *dirname = "absolute-path.dir";

	getcwd(absolute_dirpath, 260);
	strcat(absolute_dirpath, "/");
	strcat(absolute_dirpath, dirname);

	status = mkdirat(-1, absolute_dirpath, 0700);
	ASSERT_EQ(status, 0);

	ASSERT_SUCCESS(rmdir(dirname));
	return 0;
}

void cleanup()
{
	remove("not-a-directory");
	remove("absolute-path.dir");
}

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_EBADF());
	TEST(test_ENOTDIR());
	TEST(test_absolute_path());

	VERIFY_RESULT_AND_EXIT();
}