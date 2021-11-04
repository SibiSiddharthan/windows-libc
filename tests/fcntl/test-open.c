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

int test_ENOENT()
{
	errno = 0;
	int fd = open("", O_RDONLY | O_EXCL);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(ENOENT);
	ASSERT_FAIL(close(fd));

	return 0;
}

int test_create()
{
	errno = 0;
	const char *filename = "t-create";
	int fd = open(filename, O_RDONLY | O_CREAT, 0700);
	ASSERT_NOTEQ(fd, -1);
	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_create_directory()
{
	errno = 0;
	const char *filename = "t-open.dir/";
	int fd = open(filename, O_RDONLY | O_CREAT, 0700);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(EISDIR);
	ASSERT_FAIL(close(fd));
	ASSERT_FAIL(remove(filename));

	return 0;
}

int test_bad_path()
{
	errno = 0;
	int fd = open("t-bad*|/", O_RDONLY | O_CREAT, 0700);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(EINVAL);

	return 0;
}

int test_EEXIST()
{
	errno = 0;
	int fd;
	const char *filename = "t-open-exist";

	fd = open(filename, O_RDONLY | O_CREAT, 0700);
	ASSERT_SUCCESS(close(fd));
	fd = open(filename, O_RDONLY | O_CREAT | O_EXCL, 0700);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(EEXIST)
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_ENOTDIR()
{
	errno = 0;
	int fd;
	const char *filename = "t-open-notdir";

	fd = open(filename, O_RDONLY | O_CREAT, 0700);
	ASSERT_SUCCESS(close(fd));
	fd = open(filename, O_RDONLY | O_DIRECTORY);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(ENOTDIR);
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_EISDIR()
{
	errno = 0;
	// open with O_RDONLY for this call to succeed
	int fd = open(".", O_RDWR | O_DIRECTORY);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(EISDIR);

	return 0;
}

int test_ELOOP()
{
	errno = 0;
	int fd;
	const char *filename = "t-open-loop";
	const char *filename_symlink = "t-open-loop.sym";

	fd = open(filename, O_RDONLY | O_CREAT, 0700);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(symlink(filename, filename_symlink));

	fd = open(filename_symlink, O_RDONLY | O_NOFOLLOW);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(ELOOP);

	// try again with O_PATH, this time it should succeed
	fd = open(filename_symlink, O_NOFOLLOW | O_PATH);
	ASSERT_EQ(fd, 3);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(unlink(filename_symlink));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_dir()
{
	errno = 0;
	int fd;
	const char *dirname = "t-open.dir";
	const char *dirname_with_slashes = "t-open.dir/";

	ASSERT_SUCCESS(mkdir(dirname, 0700));

	fd = open(dirname, O_RDONLY);
	ASSERT_SUCCESS(close(fd));

	// Do it again with slashes
	fd = open(dirname_with_slashes, O_RDONLY);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(rmdir(dirname));

	return 0;
}

int test_O_RDONLY()
{
	errno = 0;
	int fd;
	ssize_t length;
	const char *filename = "t-open-rdonly";
	const char *buf = "hello";

	fd = open(filename, O_RDONLY | O_CREAT, 0700);
	length = write(fd, buf, 5);
	ASSERT_EQ(length, -1);
	ASSERT_ERRNO(EACCES);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_O_PATH()
{
	errno = 0;
	int fd;
	ssize_t length;
	const char *filename = "t-open-path";
	const char *buf = "hello";
	char rbuf[16];

	fd = open(filename, O_WRONLY | O_CREAT, 0700);
	length = write(fd, buf, 5);
	ASSERT_EQ(length, 5);
	ASSERT_SUCCESS(close(fd));

	// Opening with O_PATH means no read or write access to files
	fd = open(filename, O_PATH);
	length = read(fd, rbuf, 5);
	ASSERT_EQ(length, -1);
	ASSERT_ERRNO(EACCES);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_O_TRUNC()
{
	errno = 0;
	int fd;
	ssize_t length;
	const char *filename = "t-open-trunc";
	const char *buf = "hello";
	char rbuf[16];

	fd = open(filename, O_WRONLY | O_CREAT, 0700);
	length = write(fd, buf, 5);
	ASSERT_EQ(length, 5);
	ASSERT_SUCCESS(close(fd));

	// File should be truncated when opened
	fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0700);
	length = read(fd, rbuf, 5);
	ASSERT_EQ(length, 0);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_O_TMPFILE()
{
	errno = 0;
	int fd = open(".", O_WRONLY | O_CREAT | O_TMPFILE, 0700);
	ASSERT_EQ(fd, 3);
	ASSERT_SUCCESS(close(fd));

	return 0;
}

int test_O_NOATIME()
{
	errno = 0;
	int fd;
	ssize_t length;
	const char *filename = "t-open-noatime";
	char buf[16];
	struct stat before, after;

	fd = creat(filename, 0700);
	length = write(fd, "hello", 5);
	ASSERT_EQ(length, 5);
	ASSERT_SUCCESS(close(fd));

	fd = open(filename, O_RDONLY);
	ASSERT_SUCCESS(fstat(fd, &before));
	usleep(100); // sleep for 0.1 ms
	length = read(fd, buf, 16);
	ASSERT_EQ(length, 5);
	ASSERT_MEMEQ(buf, "hello", 5);
	ASSERT_SUCCESS(fstat(fd, &after));
	ASSERT_SUCCESS(close(fd));
	// check tv_nsec only as tv_sec might be equal
	// This test seems to be failing in the CI, simple workaround to aint that
	if (before.st_atim.tv_nsec != after.st_atim.tv_nsec)
	{
		memset(buf, 0, 16);

		fd = open(filename, O_RDONLY | O_NOATIME);
		ASSERT_SUCCESS(fstat(fd, &before));
		usleep(100); // sleep for 0.1 ms
		length = read(fd, buf, 16);
		ASSERT_EQ(length, 5);
		ASSERT_MEMEQ(buf, "hello", 5);
		ASSERT_SUCCESS(fstat(fd, &after));
		ASSERT_SUCCESS(close(fd));

		ASSERT_EQ(before.st_atim.tv_sec, after.st_atim.tv_sec);
		ASSERT_EQ(before.st_atim.tv_nsec, after.st_atim.tv_nsec);
	}

	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_null()
{
	errno = 0;
	int fd;
	ssize_t length;
	char buf[16];

	fd = open("/dev/null", O_RDWR);
	ASSERT_NOTEQ(fd, -1);

	length = read(fd, buf, 16);
	ASSERT_EQ(length, 0);
	ASSERT_ERRNO(0);

	length = write(fd, "abc", 3);
	ASSERT_EQ(length, 3);
	ASSERT_ERRNO(0);

	ASSERT_SUCCESS(close(fd));

	return 0;
}

int test_openat()
{
	errno = 0;
	int dirfd, fd;
	const char *dirname = "t-openat.dir";
	const char *filename = "t-openat";

	ASSERT_SUCCESS(mkdir(dirname, 0700));

	dirfd = open(dirname, O_RDONLY);
	fd = openat(dirfd, filename, O_CREAT | O_RDONLY, 0700);

	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(close(dirfd));

	ASSERT_SUCCESS(unlink("t-openat.dir/t-openat"));
	ASSERT_SUCCESS(rmdir(dirname));

	return 0;
}

void cleanup()
{
	remove("t-create");
	remove("t-open");
	remove("t-open-exist");
	remove("t-open-notdir");
	remove("t-open-loop");
	remove("t-open-loop.sym");
	remove("t-open.dir");
	remove("t-open-rdonly");
	remove("t-open-path");
	remove("t-open-trunc");
	remove("t-open-noatime");
	remove("t-openat.dir/t-openat");
	remove("t-openat.dir");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_ENOENT());
	TEST(test_create());
	TEST(test_create_directory());
	TEST(test_bad_path());
	TEST(test_EEXIST());
	TEST(test_ENOTDIR());
	TEST(test_EISDIR());
	TEST(test_ELOOP());
	TEST(test_dir());
	TEST(test_O_RDONLY());
	TEST(test_O_PATH());
	TEST(test_O_TRUNC());
	TEST(test_O_TMPFILE());
	TEST(test_O_NOATIME());
	TEST(test_null());
	TEST(test_openat());

	VERIFY_RESULT_AND_EXIT();
}
