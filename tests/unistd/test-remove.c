/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <fcntl.h>
#include <tests/test.h>
#include <errno.h>
#include <sys/stat.h>

int test_ENOENT()
{
	int status;

	errno = 0;
	status = unlink("junk");
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOENT);

	errno = 0;
	status = rmdir("junk");
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOENT);

	errno = 0;
	status = remove("junk");
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOENT);

	errno = 0;
	status = unlink("");
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOENT);

	errno = 0;
	status = rmdir("");
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOENT);

	errno = 0;
	status = remove("");
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOENT);

	return 0;
}

int test_unlink()
{
	errno = 0;
	int status;
	int fd;
	const char *filename = "t-unlink";

	fd = creat(filename, 0700);
	ASSERT_SUCCESS(close(fd));

	status = unlink(filename);
	ASSERT_EQ(status, 0);

	fd = open("t-unlink", O_RDONLY | O_EXCL);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(ENOENT);

	return 0;
}

int test_unlink_noperms()
{
	errno = 0;
	int status;
	int fd;
	const char *filename = "t-unlink-noperms";

	fd = creat(filename, 0000);
	ASSERT_SUCCESS(close(fd));

	status = unlink(filename);
	ASSERT_EQ(status, 0);

	fd = open("t-unlink", O_RDONLY | O_EXCL);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(ENOENT);

	return 0;
}

int test_unlink_readonly()
{
	errno = 0;
	int status;
	int fd;
	const char *filename = "t-unlink-readonly";

	fd = open(filename, O_CREAT | O_WRONLY | O_READONLY);
	ASSERT_SUCCESS(close(fd));

	status = unlink(filename);
	ASSERT_EQ(status, 0);

	fd = open("t-unlink", O_RDONLY | O_EXCL);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(ENOENT);

	return 0;
}

int test_unlink_symlink()
{
	int status;
	int fd;
	const char *filename = "t-unlink";
	const char *filename_symlink = "t-unlink.sym";

	fd = creat(filename, 0700);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(symlink(filename, filename_symlink));

	status = unlink(filename);
	ASSERT_EQ(status, 0);

	errno = 0;
	fd = open(filename, O_RDONLY | O_EXCL);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(ENOENT);

	status = unlink(filename_symlink);
	ASSERT_EQ(status, 0);

	errno = 0;
	fd = open(filename_symlink, O_RDONLY | O_NOFOLLOW | O_PATH);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(ENOENT);

	return 0;
}

int test_unlinkat()
{
	errno = 0;
	int status;
	int fd, dirfd;
	const char *dirname = "t-unlinkat.dir";
	const char *filename = "t-unlinkat.file";

	ASSERT_SUCCESS(mkdir(dirname, 0700));

	dirfd = open(dirname, O_RDONLY);
	fd = openat(dirfd, filename, O_CREAT | O_WRONLY, 0700);
	ASSERT_SUCCESS(close(fd));

	status = unlinkat(dirfd, filename, 0);
	ASSERT_EQ(status, 0);

	fd = openat(dirfd, filename, O_RDONLY);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(ENOENT);

	ASSERT_SUCCESS(close(dirfd));

	status = unlinkat(AT_FDCWD, dirname, AT_REMOVEDIR);
	ASSERT_EQ(status, 0);
	ASSERT_FAIL(rmdir(dirname));

	return 0;
}

int test_rmdir_ENOTDIR()
{
	errno = 0;
	int status;
	int fd;
	const char *filename = "t-rmdir.file";

	fd = creat(filename, 0700);
	ASSERT_SUCCESS(close(fd));

	status = rmdir(filename);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOTDIR);

	status = unlink(filename);
	ASSERT_EQ(status, 0);

	return 0;
}

int test_rmdir_ENOTEMPTY()
{
	errno = 0;
	int status;
	int fd, dirfd;
	const char *dirname = "t-rmdir-notempty.dir";
	const char *filename = "t-rmdir.file";

	ASSERT_SUCCESS(mkdir(dirname, 0700));

	dirfd = open(dirname, O_RDONLY);
	fd = openat(dirfd, filename, O_CREAT | O_WRONLY, 0700);
	ASSERT_SUCCESS(close(fd));

	status = rmdir(dirname);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOTEMPTY);

	status = unlinkat(dirfd, filename, 0);
	ASSERT_EQ(status, 0);

	ASSERT_SUCCESS(close(dirfd));

	status = rmdir(dirname);
	ASSERT_EQ(status, 0);

	return 0;
}

int test_rmdir()
{
	int status;
	int fd;
	const char *dirname = "t-rmdir";

	ASSERT_SUCCESS(mkdir(dirname, 0700));

	errno = 0;
	status = unlink(dirname);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EISDIR);

	status = rmdir(dirname);
	ASSERT_EQ(status, 0);

	errno = 0;
	fd = open(dirname, O_RDONLY | O_EXCL);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(ENOENT);

	return 0;
}

int test_rmdir_symlink()
{
	int status;
	int fd;
	const char *dirname = "t-rmdir.dir";
	const char *dirname_symlink = "t-rmdir.sym";

	ASSERT_SUCCESS(mkdir(dirname, 0700));
	ASSERT_SUCCESS(symlink(dirname, dirname_symlink));

	/* This is not POSIX behaviour. According to POSIX
	   `rmdir` should not be able to remove a symlink to a directory with errno = ENOTDIR.
	*/
	errno = 0;
	status = rmdir(dirname_symlink);
	ASSERT_EQ(status, 0);

	status = rmdir(dirname);
	ASSERT_EQ(status, 0);

	errno = 0;
	fd = open(dirname, O_PATH | O_NOFOLLOW);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(ENOENT);

	errno = 0;
	fd = open(dirname_symlink, O_PATH | O_NOFOLLOW);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(ENOENT);

	return 0;
}

int test_remove()
{
	int status;
	int fd;
	const char *filename = "t-remove.file";
	const char *filename_symlink = "t-remove.file.sym";
	const char *dirname = "t-remove.dir";
	const char *dirname_symlink = "t-remove.dir.sym";

	ASSERT_SUCCESS(mkdir(dirname, 0700));
	ASSERT_SUCCESS(symlink(dirname, dirname_symlink));

	fd = creat(filename, 0700);
	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(symlink(filename, filename_symlink));

	status = remove(filename);
	ASSERT_EQ(status, 0);
	status = remove(filename_symlink);
	ASSERT_EQ(status, 0);
	status = remove(dirname);
	ASSERT_EQ(status, 0);
	status = remove(dirname_symlink);
	ASSERT_EQ(status, 0);

	errno = 0;
	fd = open(filename, O_PATH | O_NOFOLLOW);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(ENOENT);

	errno = 0;
	fd = open(filename_symlink, O_PATH | O_NOFOLLOW);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(ENOENT);

	errno = 0;
	fd = open(dirname, O_PATH | O_NOFOLLOW);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(ENOENT);

	errno = 0;
	fd = open(dirname_symlink, O_PATH | O_NOFOLLOW);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(ENOENT);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();
	// There is no point performing cleanup here as we are testing the cleanup functions here

	TEST(test_ENOENT());

	// unlink
	TEST(test_unlink());
	TEST(test_unlink_noperms());
	TEST(test_unlink_readonly());
	TEST(test_unlink_symlink());
	TEST(test_unlinkat());

	// rmdir
	TEST(test_rmdir_ENOTDIR());
	TEST(test_rmdir_ENOTEMPTY());
	TEST(test_rmdir());
	TEST(test_rmdir_symlink());

	// remove
	TEST(test_remove());

	VERIFY_RESULT_AND_EXIT();
}
