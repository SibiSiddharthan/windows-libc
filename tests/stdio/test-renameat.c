/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdio-ext.h>
#include <fcntl.h>
#include <unistd.h>
#include <test-macros.h>
#include <errno.h>
#include <sys/stat.h>

void test_okay()
{
	int fd = creat("CMakeFiles/t-renameat", 0700);
	close(fd);
	int olddirfd = open("CMakeFiles", O_RDONLY | O_EXCL);
	int newdirfd = open("..", O_RDONLY | O_EXCL);

	int status = renameat(olddirfd, "t-renameat", newdirfd, "t-renameat.r");
	ASSERT_EQ(status, 0);

	status = unlinkat(newdirfd, "t-renameat.r", 0);
	ASSERT_EQ(status, 0);
	status = unlinkat(olddirfd, "t-renameat", 0);
	ASSERT_EQ(status, -1);

	close(olddirfd);
	close(newdirfd);
}

void test_exchange_file()
{
	int fd = creat("t-renameat2.1", 0700);
	close(fd);
	fd = creat("t-renameat2.2", 0700);
	close(fd);

	int status = renameat2(AT_FDCWD, "t-renameat2.1", AT_FDCWD, "t-renameat2.2", RENAME_EXCHANGE);
	ASSERT_EQ(status, 0);

	status = unlink("t-renameat2.1");
	ASSERT_EQ(status, 0);
	status = unlink("t-renameat2.2");
	ASSERT_EQ(status, 0);
}

void test_exchange_dir()
{
	mkdir("t-renameat2.1", 0700);
	mkdir("t-renameat2.2", 0700);
	int fd = creat("t-renameat2.1/a", 0700);
	close(fd);
	fd = creat("t-renameat2.2/b", 0700);
	close(fd);

	int status = renameat2(AT_FDCWD, "t-renameat2.1", AT_FDCWD, "t-renameat2.2", RENAME_EXCHANGE);
	ASSERT_EQ(status, 0);

	status = unlink("t-renameat2.1/b");
	ASSERT_EQ(status, 0);
	status = unlink("t-renameat2.2/a");
	ASSERT_EQ(status, 0);

	status = rmdir("t-renameat2.1");
	ASSERT_EQ(status, 0);
	status = rmdir("t-renameat2.2");
	ASSERT_EQ(status, 0);
}

void test_NOREPLACE()
{
	int fd = creat("t-renameat2.1", 0700);
	close(fd);
	fd = creat("t-renameat2.2", 0700);
	close(fd);

	int status = renameat2(AT_FDCWD, "t-renameat2.1", AT_FDCWD, "t-renameat2.2", RENAME_NOREPLACE);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EEXIST);

	status = unlink("t-renameat2.1");
	ASSERT_EQ(status, 0);
	status = unlink("t-renameat2.2");
	ASSERT_EQ(status, 0);
}

void test_EINVAL()
{
	int status = renameat2(AT_FDCWD, "dummy", AT_FDCWD, "dummy", RENAME_NOREPLACE | RENAME_EXCHANGE);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EINVAL);
}

int main()
{
	test_okay();
	test_exchange_file();
	test_exchange_dir();
	test_NOREPLACE();
	test_EINVAL();
	return 0;
}
