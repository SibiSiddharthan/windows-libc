/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <test-macros.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

void test_dup()
{
	int fd = creat("t-dup", 0700);
	ASSERT_EQ(fd, 3);
	int nfd = dup(fd);
	ASSERT_EQ(nfd, 4);

	// Test if file pointers are working properly
	write(fd, "hello1", 6);
	ASSERT_EQ(lseek(fd, 0, SEEK_CUR), 6);
	ASSERT_EQ(lseek(nfd, 0, SEEK_CUR), 6);
	write(nfd, "hello2", 6);
	ASSERT_EQ(lseek(fd, 0, SEEK_CUR), 12);
	ASSERT_EQ(lseek(nfd, 0, SEEK_CUR), 12);

	int status;
	status = close(fd);
	ASSERT_EQ(status, 0);

	write(nfd, "hello3", 6);
	ASSERT_EQ(lseek(nfd, 0, SEEK_CUR), 18);
	close(nfd);
	ASSERT_EQ(status, 0);

	unlink("t-dup");
}

void test_dup2()
{
	int fd = creat("t-dup2", 0700);
	ASSERT_EQ(fd, 3);
	int nfd = dup2(fd, 9);
	ASSERT_EQ(nfd, 9);
	int nnfd = dup2(nfd, 11);
	ASSERT_EQ(nnfd, 11);

	int status;
	status = close(fd);
	ASSERT_EQ(status, 0);
	status = close(nfd);
	ASSERT_EQ(status, 0);
	status = close(nnfd);
	ASSERT_EQ(status, 0);

	unlink("t-dup2");
}

void test_dup2_EBADF()
{
	int fd = dup2(5, 5);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(EBADF);
}

void test_dup3_EINVAL()
{
	int fd = dup3(1, 1, 0);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(EINVAL);
}

int main()
{
	test_dup();
	test_dup2();
	test_dup2_EBADF();
	test_dup3_EINVAL();
	return 0;
}
