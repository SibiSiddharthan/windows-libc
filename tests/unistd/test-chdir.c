/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <tests/test.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

const char *dirname = "t-chdir.dir";

int test_ENOENT()
{
	errno = 0;
	int status = chdir("");
	ASSERT_ERRNO(ENOENT);
	ASSERT_EQ(status, -1);
	return 0;
}

int test_okay()
{
	int status;
	int fd;
	const char *filename = "t-chdir.file";

	status = chdir(dirname);
	ASSERT_EQ(status, 0);

	fd = creat(filename, 0700);
	ASSERT_SUCCESS(close(fd));

	status = chdir("..");
	ASSERT_EQ(status, 0);

	status = unlink("t-chdir.dir/t-chdir.file");
	ASSERT_EQ(status, 0);

	return 0;
}

int test_okay_with_slashes()
{
	int status;
	int fd;
	const char *filename = "t-chdir.file";
	const char *dirname_with_slashes = "t-chdir.dir/";

	status = chdir(dirname_with_slashes);
	ASSERT_EQ(status, 0);

	fd = creat(filename, 0700);
	ASSERT_SUCCESS(close(fd));

	status = chdir("../");
	ASSERT_EQ(status, 0);

	status = unlink("t-chdir.dir/t-chdir.file");
	ASSERT_EQ(status, 0);

	return 0;
}

int test_fchdir()
{
	int status;
	int fd, dirfd_old, dirfd_new;
	const char *filename = "t-fchdir.file";

	dirfd_old = open(".", O_RDONLY);
	ASSERT_NOTEQ(dirfd_old, -1);

	dirfd_new = open(dirname, O_RDONLY);
	ASSERT_NOTEQ(dirfd_new, -1);

	status = fchdir(dirfd_new);
	ASSERT_EQ(status, 0);

	fd = creat(filename, 0700);
	ASSERT_EQ(fd, 5);
	ASSERT_SUCCESS(close(fd));

	status = unlinkat(dirfd_new, filename, 0);
	ASSERT_EQ(status, 0);
	ASSERT_SUCCESS(close(dirfd_new));

	status = fchdir(dirfd_old);
	ASSERT_EQ(status, 0);
	ASSERT_SUCCESS(close(dirfd_old));

	return 0;
}

int test_dot()
{
	int status;
	int fd;
	const char *filename = "t-chdir";

	status = chdir(".");
	ASSERT_EQ(status, 0);

	fd = creat(filename, 0700);
	ASSERT_SUCCESS(close(fd))

	status = unlink(filename);
	ASSERT_EQ(status, 0);

	return 0;
}

int test_fchdir_cdrive()
{
	int status;
	int dirfd_old, dirfd_new;
	const char *cdrive = "C:";
	const char *cdrive_with_slash = "C:/";
	char cwd[256];

	dirfd_old = open(".", O_RDONLY);
	ASSERT_NOTEQ(dirfd_old, -1);

	dirfd_new = open(cdrive, O_PATH);
	ASSERT_NOTEQ(dirfd_new, -1);

	status = fchdir(dirfd_new);
	ASSERT_EQ(status, 0);
	ASSERT_SUCCESS(close(dirfd_new));

	memset(cwd, 0, 256);
	getcwd(cwd, 256);
	ASSERT_STREQ(cwd, "C:");

	dirfd_new = open(cdrive_with_slash, O_PATH);
	ASSERT_NOTEQ(dirfd_new, -1);

	status = fchdir(dirfd_new);
	ASSERT_EQ(status, 0);
	ASSERT_SUCCESS(close(dirfd_new));

	memset(cwd, 0, 256);
	getcwd(cwd, 256);
	ASSERT_STREQ(cwd, "C:");

	status = fchdir(dirfd_old);
	ASSERT_EQ(status, 0);
	ASSERT_SUCCESS(close(dirfd_old));

	return 0;
}

void cleanup()
{
	remove("t-chdir.dir/t-chdir.file");
	remove("t-chdir.dir/t-fchdir.file");
}

int main()
{
	INITIAILIZE_TESTS();

	mkdir(dirname, 0700);

	TEST(test_ENOENT());
	TEST(test_okay());
	TEST(test_okay_with_slashes());
	TEST(test_fchdir());
	TEST(test_dot());
	TEST(test_fchdir_cdrive());

	if (rmdir(dirname) == -1)
	{
		cleanup();
		rmdir(dirname);
	}

	VERIFY_RESULT_AND_EXIT();
}
