/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <sys/stat.h>
#include <fcntl.h>
#include <test-macros.h>
#include <errno.h>
#include <Windows.h>
#include <unistd.h>

void test_same_fd()
{
	int fd;
	fd = creat("CMakeFiles/t-link1", 0700);
	close(fd);
	fd = open("CMakeFiles/", O_RDONLY);
	int status = linkat(fd, "t-link1", fd, "t-link2", 0);
	ASSERT_EQ(status, 0);
	close(fd);
	status = unlink("CMakeFiles/t-link1");
	ASSERT_EQ(status, 0);
	status = unlink("CMakeFiles/t-link2");
	ASSERT_EQ(status, 0);
}

void test_different_fd()
{
	int fd = creat("CMakeFiles/t-link1", 0700);
	close(fd);
	int fd1 = open("CMakeFiles/", O_RDONLY);
	int fd2 = open("..", O_RDONLY);
	int status = linkat(fd1, "t-link1", fd2, "t-link2", 0);
	ASSERT_EQ(status, 0);
	status = unlink("CMakeFiles/t-link1");
	ASSERT_EQ(status, 0);
	status = unlink("../t-link2");
	ASSERT_EQ(status, 0);
	close(fd1);
	close(fd2);
}

void test_AT_SYMLINK_FOLLOW()
{
	int fd = creat("CMakeFiles/t-link1", 0700);
	close(fd);
	int fd1 = open("CMakeFiles/", O_RDONLY);
	symlinkat("t-link1", fd1, "t-link1.sym");
	int fd2 = open("..", O_RDONLY);
	int status = linkat(fd1, "t-link1.sym", fd2, "t-link2", AT_SYMLINK_FOLLOW);
	ASSERT_EQ(status, 0);
	char buf[MAX_PATH];
	ssize_t length = readlinkat(fd2, "t-link2", buf, MAX_PATH);
	ASSERT_EQ(length, -1);
	ASSERT_ERRNO(EINVAL);
	status = unlink("CMakeFiles/t-link1.sym");
	ASSERT_EQ(status, 0);
	status = unlink("CMakeFiles/t-link1");
	ASSERT_EQ(status, 0);
	status = unlink("../t-link2");
	ASSERT_EQ(status, 0);
	close(fd1);
	close(fd2);
}

void test_AT_SYMLINK_FOLLOW_abs()
{
	int fd = creat("CMakeFiles/t-link1", 0700);
	close(fd);
	int fd1 = open("CMakeFiles/", O_RDONLY);
	symlinkat("t-link1", fd1, "t-link1.sym");
	char abspath[MAX_PATH];
	getcwd(abspath, MAX_PATH);
	strcat(abspath, "/CMakeFiles/t-link1.sym");
	int fd2 = open("..", O_RDONLY);
	int status = linkat(fd1, abspath, fd2, "t-link2", AT_SYMLINK_FOLLOW);
	ASSERT_EQ(status, 0);
	char buf[MAX_PATH];
	ssize_t length = readlinkat(fd2, "t-link2", buf, MAX_PATH);
	ASSERT_EQ(length, -1);
	ASSERT_ERRNO(EINVAL);
	status = unlink("CMakeFiles/t-link1.sym");
	ASSERT_EQ(status, 0);
	status = unlink("CMakeFiles/t-link1");
	ASSERT_EQ(status, 0);
	status = unlink("../t-link2");
	ASSERT_EQ(status, 0);
	close(fd1);
	close(fd2);
}

void test_AT_EMPTY_PATH()
{
	int fd = open(".", O_WRONLY | O_TMPFILE, 0700);
	ssize_t result;

	result = write(fd, "hello", 5);
	ASSERT_EQ(result, 5);

	int status = linkat(fd, NULL, AT_FDCWD, "t-link-at-empty-path", AT_EMPTY_PATH);
	ASSERT_EQ(status, 0);
	close(fd);

	fd = open("t-link-at-empty-path", O_RDONLY);
	char buf[8];
	result = read(fd, buf, 8);
	ASSERT_EQ(result, 5);
	ASSERT_MEMEQ(buf, "hello", 5);
	close(fd);

	status = unlink("t-link-at-empty-path");
	ASSERT_EQ(status, 0);
}

int main()
{
	test_same_fd();
	test_different_fd();
	test_AT_SYMLINK_FOLLOW();
	test_AT_SYMLINK_FOLLOW_abs();
	test_AT_EMPTY_PATH();
	return 0;
}
