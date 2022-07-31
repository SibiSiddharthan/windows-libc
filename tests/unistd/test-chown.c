/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>

// In the following tests, only change the group of the file.
// To change the owner we would need a valid owner with lower privileges other than the one running this program.
gid_t users_gid = 545;

int test_chown()
{
	int fd;
	int status;
	struct stat statbuf;
	const char *filename = "t-chown";

	fd = creat(filename, 0770);
	ASSERT_SUCCESS(close(fd));

	status = stat(filename, &statbuf);
	ASSERT_EQ(status, 0);

	ASSERT_EQ(statbuf.st_uid, getuid());
	ASSERT_EQ(statbuf.st_gid, getgid());

	status = chown(filename, -1, users_gid);
	ASSERT_EQ(status, 0);

	status = stat(filename, &statbuf);
	ASSERT_EQ(status, 0);

	ASSERT_EQ(statbuf.st_uid, getuid());
	ASSERT_EQ(statbuf.st_gid, users_gid);

	ASSERT_SUCCESS(unlink(filename));
	return 0;
}

int test_lchown()
{
	int fd;
	int status;
	struct stat statbuf;
	const char *filename = "t-lchown";
	const char *filename_symlink = "t-lchown.sym";

	fd = creat(filename, 0770);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(symlink(filename, filename_symlink));

	status = stat(filename_symlink, &statbuf);
	ASSERT_EQ(status, 0);

	ASSERT_EQ(statbuf.st_uid, getuid());
	ASSERT_EQ(statbuf.st_gid, getgid());

	status = lstat(filename_symlink, &statbuf);
	ASSERT_EQ(status, 0);

	ASSERT_EQ(statbuf.st_uid, getuid());
	ASSERT_EQ(statbuf.st_gid, getgid());

	status = lchown(filename_symlink, -1, users_gid);
	ASSERT_EQ(status, 0);

	status = lstat(filename_symlink, &statbuf);
	ASSERT_EQ(status, 0);

	ASSERT_EQ(statbuf.st_uid, getuid());
	ASSERT_EQ(statbuf.st_gid, users_gid);

	// The original file's permissions should not change.
	status = stat(filename_symlink, &statbuf);
	ASSERT_EQ(status, 0);

	ASSERT_EQ(statbuf.st_uid, getuid());
	ASSERT_EQ(statbuf.st_gid, getgid());

	ASSERT_SUCCESS(unlink(filename));
	ASSERT_SUCCESS(unlink(filename_symlink));
	return 0;
}

int test_fchown()
{
	int fd;
	int status;
	struct stat statbuf;
	const char *filename = "t-fchown";

	fd = creat(filename, 0700);

	status = fstat(fd, &statbuf);
	ASSERT_EQ(status, 0);

	ASSERT_EQ(statbuf.st_uid, getuid());
	ASSERT_EQ(statbuf.st_gid, getgid());

	errno = 0;
	status = fchown(fd, -1, users_gid);
	ASSERT_EQ(status, 0);

	status = fstat(fd, &statbuf);
	ASSERT_EQ(status, 0);

	ASSERT_EQ(statbuf.st_uid, getuid());
	ASSERT_EQ(statbuf.st_gid, users_gid);

	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(unlink(filename));
	return 0;
}

void cleanup()
{
	remove("t-chown");
	remove("t-lchown");
	remove("t-lchown.sym");
	remove("t-fchown");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_chown());
	TEST(test_lchown());
	TEST(test_fchown());

	VERIFY_RESULT_AND_EXIT();
}
