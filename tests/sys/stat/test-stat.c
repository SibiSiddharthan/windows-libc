/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/stat.h>
#include <unistd.h>

int test_ENOENT()
{
	struct stat statbuf;
	int status = stat("", &statbuf);
	ASSERT_ERRNO(ENOENT);
	ASSERT_EQ(status, -1);

	return 0;
}

int test_REGrw()
{
	int status;
	struct stat statbuf;
	const char *filename = "t-stat-rw";

	int fd = creat(filename, S_IREAD | S_IWRITE);
	ASSERT_SUCCESS(close(fd));

	status = stat(filename, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | S_IREAD | S_IWRITE));

	ASSERT_SUCCESS(unlink(filename));
	return 0;
}

int test_REGrx()
{
	int status;
	struct stat statbuf;
	const char *filename = "t-stat-rx";

	int fd = creat(filename, S_IREAD | S_IEXEC);
	ASSERT_SUCCESS(close(fd));

	status = stat(filename, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | S_IREAD | S_IEXEC));

	ASSERT_SUCCESS(unlink(filename));
	return 0;
}

int test_REGrwx()
{
	int status;
	struct stat statbuf;
	const char *filename = "t-stat-rwx";

	int fd = creat(filename, S_IREAD | S_IWRITE | S_IEXEC);
	ASSERT_SUCCESS(close(fd));

	status = stat(filename, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | S_IREAD | S_IWRITE | S_IEXEC));

	ASSERT_SUCCESS(unlink(filename));
	return 0;
}

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

int test_lstat()
{

	int status;
	struct stat statbuf;
	const char *filename = "t-lstat";
	const char *filename_symlink = "t-lstat.sym";

	int fd = creat(filename, 0760);
	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(symlink(filename, filename_symlink));

	status = stat(filename_symlink, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | 0760));
	ASSERT_EQ(statbuf.st_size, 0);

	status = lstat(filename_symlink, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFLNK | 0700));
	ASSERT_EQ(statbuf.st_size, 7); // Change this if the filename is changed

	ASSERT_SUCCESS(unlink(filename_symlink));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_fstat()
{
	int status;
	struct stat statbuf;
	const char *filename = "t-fstat";

	int fd = creat(filename, 0760);
	ASSERT_NOTEQ(fd, -1);
	write(fd, "hello", 5);

	status = fstat(fd, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | 0760));
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
	fd = openat(dirfd, filename, O_CREAT | O_WRONLY, 0766);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(symlinkat(filename, dirfd, filename_symlink));

	// work like stat
	status = fstatat(dirfd, filename_symlink, &statbuf, 0);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | 0766));
	ASSERT_EQ(statbuf.st_size, 0);

	// work like lstat
	status = fstatat(dirfd, filename_symlink, &statbuf, AT_SYMLINK_NOFOLLOW);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFLNK | 0700));
	ASSERT_EQ(statbuf.st_size, 9);

	// AT_EMPTY_PATH tests
	// dereferences symlink
	fd = openat(dirfd, filename_symlink, O_RDONLY);
	ASSERT_NOTEQ(fd, -1);

	status = fstatat(fd, NULL, &statbuf, AT_EMPTY_PATH);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | 0766));
	ASSERT_EQ(statbuf.st_size, 0);

	ASSERT_SUCCESS(close(fd));

	// no symlink dereference
	fd = openat(dirfd, filename_symlink, O_PATH | O_NOFOLLOW);
	ASSERT_NOTEQ(fd, -1);

	status = fstatat(fd, NULL, &statbuf, AT_EMPTY_PATH);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFLNK | 0700));
	ASSERT_EQ(statbuf.st_size, 9);

	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(unlinkat(dirfd, filename, 0));
	ASSERT_SUCCESS(unlinkat(dirfd, filename_symlink, 0));
	ASSERT_SUCCESS(close(dirfd));

	ASSERT_SUCCESS(rmdir(dirname));

	return 0;
}

int test_id()
{
	int status;
	struct stat statbuf;
	const char *filename = "t-stat-id";

	int fd = creat(filename, 0760);
	ASSERT_SUCCESS(close(fd));

	status = stat(filename, &statbuf);
	ASSERT_EQ(status, 0);

	ASSERT_EQ(statbuf.st_uid, getuid());
	ASSERT_EQ(statbuf.st_gid, getgid());

	ASSERT_SUCCESS(unlink(filename));
	return 0;
}

int test_permissions_file()
{
	int fd;
	int status;
	struct stat statbuf;
	mode_t base_perms = 0;
	uid_t uid = getuid();
	char filename[32];

	if (uid == ROOT_UID)
	{
		base_perms = S_IREAD | S_IWRITE | S_IEXEC;
	}

	for (int i = 0; i < 512; ++i)
	{
		snprintf(filename, 32, "t-perms.file.%04o", i);

		fd = creat(filename, i);
		ASSERT_SUCCESS(close(fd));

		status = stat(filename, &statbuf);
		ASSERT_EQ(status, 0);
		ASSERT_EQ(statbuf.st_mode, (S_IFREG | base_perms | i));

		ASSERT_SUCCESS(unlink(filename));
	}

	return 0;
}

int test_permissions_dir()
{
	int status;
	struct stat statbuf;
	mode_t base_perms = 0;
	uid_t uid = getuid();
	char dirname[32];

	if (uid == ROOT_UID)
	{
		base_perms = S_IREAD | S_IWRITE | S_IEXEC;
	}

	for (int i = 0; i < 512; ++i)
	{
		snprintf(dirname, 32, "t-perms.dir.%04o", i);

		ASSERT_SUCCESS(mkdir(dirname, i));

		status = stat(dirname, &statbuf);
		ASSERT_EQ(status, 0);
		ASSERT_EQ(statbuf.st_mode, (S_IFDIR | base_perms | i));

		ASSERT_SUCCESS(rmdir(dirname));
	}

	return 0;
}

int test_root()
{
	int status;
	char cd[256];
	struct stat cd_stat, root_stat;

	getcwd(cd, 256);
	cd[3] = '\0';

	// stat
	status = stat(cd, &cd_stat);
	ASSERT_EQ(status, 0);

	status = stat("/", &root_stat);
	ASSERT_EQ(status, 0);

	if (memcmp(&cd_stat, &root_stat, offsetof(struct stat, st_atim)) != 0)
	{
		printf("stat of root failed.\n");
		return 1;
	}

	// lstat
	status = lstat(cd, &cd_stat);
	ASSERT_EQ(status, 0);

	status = lstat("/", &root_stat);
	ASSERT_EQ(status, 0);

	if (memcmp(&cd_stat, &root_stat, offsetof(struct stat, st_atim)) != 0)
	{
		printf("lstat of root failed.\n");
		return 1;
	}

	return 0;
}

void cleanup()
{
	remove("t-stat-rw");
	remove("t-stat-rx");
	remove("t-stat-rwx");
	remove("t-stat.dir");
	remove("t-lstat");
	remove("t-lstat.sym");
	remove("t-stat1");
	remove("t-stat2");
	remove("t-fstat");
	remove("t-fstatat.dir/t-fstatat");
	remove("t-fstatat.dir/t-fstatat.sym");
	remove("t-fstatat.dir");
	remove("t-stat-id");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_ENOENT());
	if (getuid() != ROOT_UID)
	{
		// These tests fail when run under admin priveleges as by design admins have all access.
		TEST(test_REGrw());
		TEST(test_REGrx());
	}

	TEST(test_REGrwx());
	TEST(test_DIR());
	TEST(test_hardlinks());
	TEST(test_lstat());
	TEST(test_fstat());
	TEST(test_fstatat());
	TEST(test_id());

	TEST(test_permissions_file());
	TEST(test_permissions_dir());

	TEST(test_root());

	VERIFY_RESULT_AND_EXIT();
}
