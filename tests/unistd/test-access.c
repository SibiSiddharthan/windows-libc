/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <test-macros.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

int test_ENOENT()
{
	errno = 0;
	int status = access("", F_OK);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOENT);
	return 0;
}

int test_EINVAL()
{
	errno = 0;
	int status = access("valid-filename", 8);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EINVAL);
	return 0;
}

int test_DIR()
{
	errno = 0;
	int status;
	const char *dirname = "t-access.dir";

	ASSERT_SUCCESS(mkdir(dirname, 0700));

	status = access(dirname, F_OK | R_OK | W_OK | X_OK);
	ASSERT_EQ(status, 0);

	ASSERT_SUCCESS(rmdir(dirname));

	return 0;
}

int test_FILE()
{
	int fd;
	int status;
	const char *filename = "t-access.file";

	fd = creat(filename, 0600);
	ASSERT_SUCCESS(close(fd));

	status = access(filename, F_OK | R_OK | W_OK | X_OK);
	ASSERT_EQ(status, -1);
	status = access(filename, F_OK | R_OK | W_OK);
	ASSERT_EQ(status, 0);

	ASSERT_SUCCESS(chmod(filename, S_IREAD));

	status = access(filename, F_OK | R_OK | W_OK);
	ASSERT_EQ(status, -1);
	status = access(filename, F_OK | R_OK);
	ASSERT_EQ(status, 0);

	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_symlink()
{
	int fd;
	int status;
	const char *filename = "t-access";
	const char *filename_symlink = "t-access.sym";

	fd = creat(filename, 0700);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(symlink(filename, filename_symlink));

	status = access(filename_symlink, F_OK | R_OK | W_OK | X_OK);
	ASSERT_EQ(status, 0);

	ASSERT_SUCCESS(unlink(filename_symlink));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_faccessat()
{
	int fd, dirfd;
	int status;
	const char *dirname = "t-faccessat.dir";
	const char *filename = "t-faccessat.file";
	const char *filename_symlink = "t-faccessat.file.sym";

	ASSERT_SUCCESS(mkdir(dirname, 0700));

	dirfd = open(dirname, O_RDONLY | O_EXCL);
	fd = openat(dirfd, filename, O_CREAT | O_WRONLY, 0700);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(symlinkat(filename, dirfd, filename_symlink));

	status = faccessat(dirfd, filename_symlink, F_OK | R_OK | W_OK | X_OK, 0);
	ASSERT_EQ(status, 0);
	status = faccessat(dirfd, filename_symlink, F_OK | R_OK | W_OK | X_OK, AT_SYMLINK_NOFOLLOW);
	ASSERT_EQ(status, 0);

	ASSERT_SUCCESS(unlinkat(dirfd, filename, 0));
	ASSERT_SUCCESS(unlinkat(dirfd, filename_symlink, 0));
	ASSERT_SUCCESS(close(dirfd));
	ASSERT_SUCCESS(rmdir(dirname));

	return 0;
}

void cleanup()
{
	remove("t-access.dir");
	remove("t-access.file");
	remove("t-access");
	remove("t-access.sym");
	remove("t-faccessat.dir/t-faccessat.file.sym");
	remove("t-faccessat.dir/t-faccessat.file");
	remove("t-faccessat.dir");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_ENOENT());
	TEST(test_EINVAL());
	TEST(test_DIR());
	TEST(test_FILE());
	TEST(test_symlink());
	TEST(test_faccessat());

	VERIFY_RESULT_AND_EXIT();
}
