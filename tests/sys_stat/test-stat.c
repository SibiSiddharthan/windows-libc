/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <sys/stat.h>
#include <unistd.h>
#include <test-macros.h>
#include <errno.h>
#include <fcntl.h>

void test_ENOENT()
{
	struct stat statbuf;
	int status = stat("", &statbuf);
	ASSERT_ERRNO(ENOENT);
	ASSERT_EQ(status, -1);
}

void test_REGrw()
{
	int fd = creat("t-stat", S_IREAD | S_IWRITE);
	close(fd);
	struct stat statbuf;
	int status = stat("t-stat", &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | S_IREAD | S_IWRITE));
	unlink("t-stat");
}

void test_REGrx()
{
	int fd = creat("t-stat.exe", S_IREAD);
	close(fd);
	struct stat statbuf;
	int status = stat("t-stat.exe", &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | S_IREAD | S_IEXEC));
	unlink("t-stat.exe");
}

void test_REGrwx()
{
	int fd = creat("t-stat.exe", S_IREAD | S_IWRITE);
	close(fd);
	struct stat statbuf;
	int status = stat("t-stat.exe", &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | S_IREAD | S_IWRITE | S_IEXEC));
	unlink("t-stat.exe");
}

void test_DIR()
{
	mkdir("t-stat", 0700);
	struct stat statbuf;

	int status = stat("t-stat", &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFDIR | S_IREAD | S_IWRITE | S_IEXEC));

	status = stat("t-stat/", &statbuf); // Try with slashes also
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFDIR | S_IREAD | S_IWRITE | S_IEXEC));
	ASSERT_EQ(statbuf.st_size, statbuf.st_blksize); // size of directory = size of a cluster

	rmdir("t-stat");
}

void test_lstat()
{
	int fd = creat("t-stat", S_IREAD | S_IWRITE);
	close(fd);
	symlink("t-stat", "t-stat.sym");

	struct stat statbuf;

	int status = stat("t-stat.sym", &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | S_IREAD | S_IWRITE));
	ASSERT_EQ(statbuf.st_size, 0);

	status = lstat("t-stat.sym", &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFLNK | S_IREAD | S_IWRITE | S_IEXEC)); // symlinks have all permissions
	ASSERT_EQ(statbuf.st_size, 6);

	unlink("t-stat.sym");
	unlink("t-stat");
}

void test_hardlinks()
{
	int fd = creat("t-stat1", S_IREAD);
	close(fd);
	struct stat statbuf;
	int status = stat("t-stat1", &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_nlink, 1);
	ino_t l1 = statbuf.st_ino;

	link("t-stat1", "t-stat2");

	status = stat("t-stat2", &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_nlink, 2);
	ino_t l2 = statbuf.st_ino;

	ASSERT_EQ(l1, l2);

	unlink("t-stat1");
	unlink("t-stat2");
}

void test_fstat()
{
	int fd = creat("t-fstat", S_IREAD | S_IWRITE);
	write(fd, "hello", 5);
	struct stat statbuf;
	int status = fstat(fd, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | S_IREAD | S_IWRITE));
	ASSERT_EQ(statbuf.st_size, 5);
	close(fd);
	unlink("t-fstat");
}

void test_fstatat()
{
	int dirfd = open("CMakeFiles", O_RDONLY | O_EXCL);
	int fd = creat("CMakeFiles/t-fstatat", S_IREAD);
	close(fd);

	symlinkat("t-fstatat", dirfd, "t-fstatat.sym");
	struct stat statbuf;

	int status = fstatat(dirfd, "t-fstatat.sym", &statbuf, 0);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | S_IREAD));
	ASSERT_EQ(statbuf.st_size, 0);

	status = fstatat(dirfd, "t-fstatat.sym", &statbuf, AT_SYMLINK_NOFOLLOW);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFLNK | S_IREAD | S_IWRITE | S_IEXEC));
	ASSERT_EQ(statbuf.st_size, 9);

	unlinkat(dirfd, "t-fstatat", 0);
	unlinkat(dirfd, "t-fstatat.sym", 0);
	close(dirfd);
}

int main()
{
	test_ENOENT();
	test_REGrw();
	test_REGrx();
	test_REGrwx();
	test_DIR();
	test_lstat();
	test_hardlinks();
	test_fstat();
	test_fstatat();
	return 0;
}
