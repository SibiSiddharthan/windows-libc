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

int test_ENOENT()
{
	struct stat statbuf;
	int status = stat("", &statbuf);
	ASSERT_ERRNO(ENOENT);
	ASSERT_EQ(status, -1);

	return 0;
}

#if 0
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
#endif

int test_DIR()
{
	int status;
	struct stat statbuf;
	const char *dirname = "t-stat.dir";
	const char *dirname_with_slashes = "t-stat.dir/";

	ASSERT_SUCCESS(mkdir(dirname, 0700));

	status = stat(dirname, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFDIR | S_IREAD | S_IWRITE | S_IEXEC));

	status = stat(dirname_with_slashes, &statbuf); // Try with slashes also
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFDIR | S_IREAD | S_IWRITE | S_IEXEC));
	ASSERT_EQ(statbuf.st_size, statbuf.st_blksize); // size of directory = size of a cluster

	ASSERT_SUCCESS(rmdir(dirname));

	return 0;
}

int test_lstat()
{

	int status;
	struct stat statbuf;
	const char *filename = "t-lstat";
	const char *filename_symlink = "t-lstat.sym";

	int fd = creat(filename, S_IREAD | S_IWRITE);
	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(symlink(filename, filename_symlink));

	status = stat(filename_symlink, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | S_IREAD | S_IWRITE));
	ASSERT_EQ(statbuf.st_size, 0);

	status = lstat(filename_symlink, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFLNK | S_IREAD | S_IWRITE | S_IEXEC)); // symlinks have all permissions
	ASSERT_EQ(statbuf.st_size, 7);                                        // Change this if the filename is changed

	ASSERT_SUCCESS(unlink(filename_symlink));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_hardlinks()
{
	int status;
	struct stat statbuf;
	ino_t l1, l2;
	const char *filename1 = "t-stat1";
	const char *filename2 = "t-stat2";

	int fd = creat(filename1, S_IREAD);
	ASSERT_SUCCESS(close(fd));

	status = stat(filename1, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_nlink, 1);
	l1 = statbuf.st_ino;

	ASSERT_SUCCESS(link(filename1, filename2));

	status = stat(filename2, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_nlink, 2);
	l2 = statbuf.st_ino;

	ASSERT_EQ(l1, l2);

	ASSERT_SUCCESS(unlink(filename1));
	ASSERT_SUCCESS(unlink(filename2));

	return 0;
}

int test_fstat()
{
	int status;
	struct stat statbuf;
	const char *filename = "t-fstat";

	int fd = creat(filename, S_IREAD | S_IWRITE);
	ASSERT_NOTEQ(fd, -1);
	write(fd, "hello", 5);

	status = fstat(fd, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | S_IREAD | S_IWRITE));
	ASSERT_EQ(statbuf.st_size, 5);

	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_fstatat()
{
	int status;
	int fd, dirfd;
	struct stat statbuf;
	const char *dirname = "t-fstatat.dir";
	const char *filename = "t-fstatat";
	const char *filename_symlink = "t-fstatat.sym";

	ASSERT_SUCCESS(mkdir(dirname, 0700));

	dirfd = open(dirname, O_RDONLY | O_EXCL);
	ASSERT_NOTEQ(dirfd, -1);
	fd = openat(dirfd, filename, O_CREAT | O_WRONLY, S_IREAD);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(symlinkat(filename, dirfd, filename_symlink));

	// work like stat
	status = fstatat(dirfd, filename_symlink, &statbuf, 0);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | S_IREAD));
	ASSERT_EQ(statbuf.st_size, 0);

	// work like lstat
	status = fstatat(dirfd, filename_symlink, &statbuf, AT_SYMLINK_NOFOLLOW);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFLNK | S_IREAD | S_IWRITE | S_IEXEC));
	ASSERT_EQ(statbuf.st_size, 9);

	// AT_EMPTY_PATH tests
	// dereferences symlink
	fd = openat(dirfd, filename_symlink, O_RDONLY);
	ASSERT_NOTEQ(fd, -1);

	status = fstatat(fd, NULL, &statbuf, AT_EMPTY_PATH);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | S_IREAD));
	ASSERT_EQ(statbuf.st_size, 0);

	ASSERT_SUCCESS(close(fd));

	// no symlink dereference
	fd = openat(dirfd, filename_symlink, O_PATH | O_NOFOLLOW);
	ASSERT_NOTEQ(fd, -1);

	status = fstatat(fd, NULL, &statbuf, AT_EMPTY_PATH);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFLNK | S_IREAD | S_IWRITE | S_IEXEC));
	ASSERT_EQ(statbuf.st_size, 9);

	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(unlinkat(dirfd, filename, 0));
	ASSERT_SUCCESS(unlinkat(dirfd, filename_symlink, 0));
	ASSERT_SUCCESS(close(dirfd));

	ASSERT_SUCCESS(rmdir(dirname));

	return 0;
}

void cleanup()
{
	remove("t-stat.dir");
	remove("t-lstat");
	remove("t-lstat.sym");
	remove("t-stat1");
	remove("t-stat2");
	remove("t-fstat");
	remove("t-fstatat.dir/t-fstatat");
	remove("t-fstatat.dir/t-fstatat.sym");
	remove("t-fstatat.dir");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_ENOENT());
	// test_REGrw(); Need to use ACCESS_MASK to determine this
	// test_REGrx(); Need to use ACCESS_MASK to determine this
	// test_REGrwx(); Need to use ACCESS_MASK to determine this
	TEST(test_DIR());
	TEST(test_lstat());
	TEST(test_hardlinks());
	TEST(test_fstat());
	TEST(test_fstatat());

	VERIFY_RESULT_AND_EXIT();
}
