/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <test-macros.h>
#include <errno.h>
#include <fcntl.h>

void test_EINVAL()
{
	errno = 0;
	int status = chdir("");
	ASSERT_ERRNO(EINVAL);
	ASSERT_EQ(status, -1);
}

void test_okay()
{
	int status = chdir("CMakeFiles");
	ASSERT_EQ(status, 0);
	int fd = creat("t-chdir", 0700);
	close(fd);
	status = chdir("..");
	ASSERT_EQ(status, 0);
	status = unlink("CMakeFiles/t-chdir");
	ASSERT_EQ(status, 0);
}

void test_okay_with_slashes()
{
	int status = chdir("CMakeFiles/");
	ASSERT_EQ(status, 0);
	int fd = creat("t-chdir", 0700);
	close(fd);
	status = chdir("../");
	ASSERT_EQ(status, 0);
	status = unlink("CMakeFiles/t-chdir");
	ASSERT_EQ(status, 0);
}

void test_fchdir()
{
	int dirfd = open("CMakeFiles", O_RDONLY | O_EXCL);
	int status = fchdir(dirfd);
	int fd = creat("t-chdir", 0700);
	close(fd);
	status = unlinkat(dirfd, "t-chdir", 0);
	ASSERT_EQ(status, 0);
	close(dirfd);
}

void test_dot()
{
	int status = chdir(".");
	ASSERT_EQ(status, 0);
	int fd = creat("t-chdir", 0700);
	close(fd);
	status = unlink("t-chdir");
	ASSERT_EQ(status, 0);
}

int main()
{
	test_EINVAL();
	test_okay();
	test_okay_with_slashes();
	test_fchdir();
	test_dot();
	return 0;
}
