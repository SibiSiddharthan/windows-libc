/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <fcntl.h>
#include <unistd.h>
#include <test-macros.h>
#include <errno.h>
#include <sys/stat.h>

static const char *test_content = "Hello World!";
static const size_t sizeof_test_content = 12;

static int verify_file_contents(int dirfd, const char *filename)
{
	ssize_t length;
	char buf[16];
	int fd;

	fd = openat(dirfd, filename, O_RDONLY);
	if (fd == -1)
	{
		return -1;
	}

	length = read(fd, buf, 16);
	if (length != (ssize_t)sizeof_test_content)
	{
		return -1;
	}
	if (memcmp(buf, test_content, length) != 0)
	{
		return -1;
	}

	close(fd);
	return 0;
}

static int write_file_contents(int fd)
{
	ssize_t length;

	length = write(fd, test_content, sizeof_test_content);
	if (length != (ssize_t)sizeof_test_content)
	{
		return -1;
	}

	return 0;
}

int test_ENOENT()
{
	errno = 0;
	int status = link("t-link-nonexistent", "dummy");
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOENT);
	return 0;
}

int test_EEXIST()
{
	errno = 0;
	int status;
	int fd;
	const char *source = "t-link1";
	const char *target = "t-link2";

	fd = creat(source, 0700);
	ASSERT_SUCCESS(close(fd));

	fd = creat(target, 0700);
	ASSERT_SUCCESS(close(fd));

	status = link(source, target);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EEXIST);

	ASSERT_SUCCESS(unlink(source));
	ASSERT_SUCCESS(unlink(target));

	return 0;
}

int test_file()
{
	int status;
	int fd;
	const char *source = "t-link";
	const char *target = "t-link.lnk";

	fd = creat(source, 0700);
	ASSERT_SUCCESS(write_file_contents(fd));
	ASSERT_SUCCESS(close(fd));

	status = link(source, target);
	ASSERT_EQ(status, 0);

	ASSERT_SUCCESS(verify_file_contents(AT_FDCWD, target));

	ASSERT_SUCCESS(unlink(source));
	ASSERT_SUCCESS(unlink(target));

	return 0;
}

int test_symlink()
{
	int status;
	int fd;
	char rbuf[16];
	const char *source = "t-link";
	const char *source_symlink = "t-link.sym";
	const char *target = "t-link.sym.lnk";

	fd = creat(source, 0700);
	ASSERT_SUCCESS(write_file_contents(fd));
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(symlink(source, source_symlink));

	status = link(source_symlink, target);
	ASSERT_EQ(status, 0);

	status = (int)readlink(target, rbuf, 16);
	ASSERT_EQ(status, 6);
	ASSERT_MEMEQ(rbuf, source, 6);

	ASSERT_SUCCESS(verify_file_contents(AT_FDCWD, target));

	ASSERT_SUCCESS(unlink(source_symlink));
	ASSERT_SUCCESS(unlink(source));
	ASSERT_SUCCESS(unlink(target));

	return 0;
}

int test_linkat()
{
	int status;
	int fd, dirfd1, dirfd2;
	const char *dirname = "t-linkat.dir";
	const char *source = "t-linkat";
	const char *target = "t-linkat.lnk";

	ASSERT_SUCCESS(mkdir(dirname, 0700));

	dirfd1 = open(".", O_RDONLY);
	dirfd2 = open(dirname, O_RDONLY);

	fd = openat(dirfd2, source, O_CREAT | O_WRONLY, 0700);
	ASSERT_SUCCESS(write_file_contents(fd));
	ASSERT_SUCCESS(close(fd));

	// same fd
	status = linkat(dirfd2, source, dirfd2, target, 0);
	ASSERT_EQ(status, 0);

	ASSERT_SUCCESS(verify_file_contents(dirfd2, target));

	// different fd
	status = linkat(dirfd2, source, dirfd1, target, 0);
	ASSERT_EQ(status, 0);

	ASSERT_SUCCESS(verify_file_contents(dirfd1, target));

	ASSERT_SUCCESS(unlinkat(dirfd2, target, 0));
	ASSERT_SUCCESS(unlinkat(dirfd1, target, 0));
	ASSERT_SUCCESS(unlinkat(dirfd2, source, 0));

	ASSERT_SUCCESS(close(dirfd1));
	ASSERT_SUCCESS(close(dirfd2));

	ASSERT_SUCCESS(rmdir(dirname));

	return 0;
}

int test_linkat_AT_SYMLINK_FOLLOW()
{
	int status;
	int fd, dirfd;
	char rbuf[64];
	const char *dirname = "t-linkat-symlink-follow.dir";
	const char *source = "t-linkat-symlink-follow.file";
	const char *source_symlink = "t-linkat-symlink-follow.file.sym";
	const char *target = "t-linkat-symlink-follow.file.lnk";

	ASSERT_SUCCESS(mkdir(dirname, 0700));

	dirfd = open(dirname, O_RDONLY);
	fd = openat(dirfd, source, O_CREAT | O_WRONLY, 0700);
	ASSERT_SUCCESS(write_file_contents(fd));
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(symlinkat(source, dirfd, source_symlink));

	status = linkat(dirfd, source_symlink, AT_FDCWD, target, AT_SYMLINK_FOLLOW);
	ASSERT_EQ(status, 0);

	errno = 0;
	status = (int)readlinkat(AT_FDCWD, target, rbuf, 64);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EINVAL);

	ASSERT_SUCCESS(verify_file_contents(AT_FDCWD, target));

	ASSERT_SUCCESS(unlink(target));
	ASSERT_SUCCESS(unlinkat(dirfd, source, 0));
	ASSERT_SUCCESS(unlinkat(dirfd, source_symlink, 0));

	ASSERT_SUCCESS(close(dirfd));
	ASSERT_SUCCESS(rmdir(dirname));

	return 0;
}

int test_linkat_AT_EMPTY_PATH()
{
	errno = 0;
	int status;
	int fd;
	const char *filename = "t-linkat-empty-path";

	fd = open(".", O_WRONLY | O_TMPFILE | O_EXCL, 0700);
	ASSERT_NOTEQ(fd, -1);
	ASSERT_SUCCESS(write_file_contents(fd));

	// trying link tmpfile opened with O_EXCL should error with EBADF
	status = linkat(fd, NULL, AT_FDCWD, filename, AT_EMPTY_PATH);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EBADF);

	ASSERT_SUCCESS(close(fd));

	// try again, this time with no O_EXCL, should succeed
	fd = open(".", O_WRONLY | O_TMPFILE, 0700);
	ASSERT_NOTEQ(fd, -1);
	ASSERT_SUCCESS(write_file_contents(fd));

	status = linkat(fd, NULL, AT_FDCWD, filename, AT_EMPTY_PATH);
	ASSERT_EQ(status, 0);

	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(verify_file_contents(AT_FDCWD, filename));

	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

void cleanup()
{
	remove("t-link1");
	remove("t-link2");
	remove("t-link");
	remove("t-link.lnk");
	remove("t-link.sym");
	remove("t-link.sym.lnk");

	remove("t-linkat");
	remove("t-linkat.lnk");
	remove("t-linkat.dir/t-linkat.lnk");
	remove("t-linkat.dir");
	remove("t-linkat-symlink-follow.file.lnk");
	remove("t-linkat-symlink-follow.dir/t-linkat-symlink-follow.file");
	remove("t-linkat-symlink-follow.dir/t-linkat-symlink-follow.file.sym");
	remove("t-linkat-symlink-follow.dir");
	remove("t-linkat-empty-path");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	// link
	TEST(test_ENOENT());
	TEST(test_EEXIST());
	TEST(test_file());
	TEST(test_symlink());

	// linkat
	TEST(test_linkat());
	TEST(test_linkat_AT_SYMLINK_FOLLOW());
	TEST(test_linkat_AT_EMPTY_PATH());

	VERIFY_RESULT_AND_EXIT();
}
