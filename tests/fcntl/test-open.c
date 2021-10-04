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

void test_ENOENT()
{
	errno = 0;
	int fd = open("", O_RDONLY | O_EXCL);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(ENOENT);
}

void test_create()
{
	errno = 0;
	int fd = open("t-open", O_RDONLY | O_CREAT, 0700);
	ASSERT_EQ(fd, 3);
	close(fd);
	int status = unlink("t-open");
	ASSERT_EQ(status, 0);
}

void test_create_directory()
{
	errno = 0;
	int fd = open("t-open-dir/", O_RDONLY | O_CREAT, 0700);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(EISDIR);
}

void test_bad_path()
{
	errno = 0;
	int fd = open("t-bad*|/", O_RDONLY | O_CREAT, 0700);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(EINVAL);
}

void test_EEXIST()
{
	errno = 0;
	int fd = open("t-open", O_RDONLY | O_CREAT, 0700);
	close(fd);
	fd = open("t-open", O_RDONLY | O_CREAT | O_EXCL, 0700);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(EEXIST)
	int status = unlink("t-open");
	ASSERT_EQ(status, 0);
}

void test_ENOTDIR()
{
	errno = 0;
	int fd = open("t-open", O_RDONLY | O_CREAT, 0700);
	close(fd);
	fd = open("t-open", O_RDONLY | O_DIRECTORY);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(ENOTDIR);
	int status = unlink("t-open");
	ASSERT_EQ(status, 0);
}

void test_EISDIR()
{
	errno = 0;
	int fd = open(".", O_RDWR | O_DIRECTORY);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(EISDIR);
}

void test_ELOOP()
{
	errno = 0;
	int fd = open("t-open", O_RDONLY | O_CREAT, 0700);
	close(fd);
	symlink("t-open", "t-open.sym");
	fd = open("t-open.sym", O_RDONLY | O_NOFOLLOW);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(ELOOP);
	unlink("t-open.sym");
	unlink("t-open");
}

void test_dir_without_slashes()
{
	errno = 0;
	int fd = open("CMakeFiles", O_RDONLY);
	ASSERT_EQ(fd, 3);
	close(fd);
}

void test_dir_with_slashes()
{
	errno = 0;
	int fd = open("CMakeFiles/", O_RDONLY);
	ASSERT_EQ(fd, 3);
	close(fd);
}

void test_O_RDONLY()
{
	errno = 0;
	int fd = open("t-open", O_RDONLY | O_CREAT, 0700);
	ASSERT_EQ(fd, 3);
	char *buf = "hello";
	ssize_t length = write(fd, buf, 5);
	ASSERT_EQ(length, -1);
	ASSERT_ERRNO(EACCES);
	close(fd);
}

void test_O_PATH()
{
	errno = 0;
	int fd = open("t-open", O_WRONLY | O_CREAT, 0700);
	ASSERT_EQ(fd, 3);
	char *buf = "hello";
	ssize_t length = write(fd, buf, 5);
	ASSERT_EQ(length, 5);
	close(fd);
	fd = open("t-open", O_PATH);
	char rbuf[16];
	length = read(fd, rbuf, 5);
	ASSERT_EQ(length, -1);
	ASSERT_ERRNO(EACCES);
	close(fd);
}

void test_O_TRUNC()
{
	errno = 0;
	int fd = open("t-open", O_RDWR | O_CREAT | O_TRUNC, 0700);
	ASSERT_EQ(fd, 3);
	char rbuf[16];
	ssize_t length = read(fd, rbuf, 5);
	ASSERT_EQ(length, 0);
	close(fd);
	unlink("t-open");
}

void test_O_TMPFILE()
{
	errno = 0;
	int fd = open(".", O_WRONLY | O_CREAT | O_TMPFILE, 0700);
	ASSERT_EQ(fd, 3);
	close(fd);
}

void test_O_NOATIME()
{
	errno = 0;
	char buf[16];
	int fd;
	struct stat before, after;

	fd = creat("t-open-NOATIME", 0700);
	write(fd, "hello", 5);
	close(fd);

	fd = open("t-open-NOATIME", O_RDONLY);
	fstat(fd, &before);
	read(fd, buf, 16);
	ASSERT_MEMEQ(buf, "hello", 5);
	fstat(fd, &after);
	close(fd);
	// check tv_nsec only as tv_sec might be equal
	ASSERT_NOTEQ(before.st_atim.tv_nsec, after.st_atim.tv_nsec);

	memset(buf, 0, 16);

	fd = open("t-open-NOATIME", O_RDONLY | O_NOATIME);
	fstat(fd, &before);
	read(fd, buf, 16);
	ASSERT_MEMEQ(buf, "hello", 5);
	fstat(fd, &after);
	close(fd);
	ASSERT_EQ(before.st_atim.tv_sec, after.st_atim.tv_sec);
	ASSERT_EQ(before.st_atim.tv_nsec, after.st_atim.tv_nsec);

	unlink("t-open-NOATIME");
}

void test_null()
{
	errno = 0;
	char buf[16];
	ssize_t result;

	int fd = open("/dev/null", O_RDWR);
	ASSERT_EQ(fd, 3);

	result = read(fd, buf, 16);
	ASSERT_EQ(result, 0);
	ASSERT_ERRNO(0);

	result = write(fd, "abc", 3);
	ASSERT_EQ(result, 3);
	ASSERT_ERRNO(0);

	close(fd);
}

int main()
{
	test_ENOENT();
	test_create();
	test_create_directory();
	test_bad_path();
	test_EEXIST();
	test_ENOTDIR();
	test_EISDIR();
	test_ELOOP();
	test_dir_with_slashes();
	test_dir_without_slashes();
	test_O_RDONLY();
	test_O_PATH();
	test_O_TRUNC();
	test_O_TMPFILE();
	test_O_NOATIME();
	test_null();
	return 0;
}
