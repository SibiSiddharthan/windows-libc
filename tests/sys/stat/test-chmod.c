/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <test-macros.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int test_chmod_file()
{
	int fd;
	int status;
	struct stat statbuf;
	mode_t base_perms = 0;
	uid_t uid = getuid();
	const char *filename = "t-chmod.file";

	if (uid == ROOT_UID)
	{
		base_perms = S_IREAD | S_IWRITE | S_IEXEC;
	}

	// Start with no permissions
	fd = creat(filename, 0);
	ASSERT_SUCCESS(close(fd));

	status = stat(filename, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | base_perms));

	for (int i = 0; i < 512; ++i)
	{
		status = chmod(filename, i);
		ASSERT_EQ(status, 0);

		status = stat(filename, &statbuf);
		ASSERT_EQ(status, 0);
		ASSERT_EQ(statbuf.st_mode, (S_IFREG | base_perms | i));
	}

	ASSERT_SUCCESS(unlink(filename));
	return 0;
}

int test_chmod_dir()
{
	int status;
	struct stat statbuf;
	mode_t base_perms = 0;
	uid_t uid = getuid();
	const char *dirname = "t-chmod.dir";

	if (uid == ROOT_UID)
	{
		base_perms = S_IREAD | S_IWRITE | S_IEXEC;
	}

	// Start with no permissions
	ASSERT_SUCCESS(mkdir(dirname, 0));

	status = stat(dirname, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFDIR | base_perms));

	for (int i = 0; i < 512; ++i)
	{
		status = chmod(dirname, i);
		ASSERT_EQ(status, 0);

		status = stat(dirname, &statbuf);
		ASSERT_EQ(status, 0);
		ASSERT_EQ(statbuf.st_mode, (S_IFDIR | base_perms | i));
	}

	ASSERT_SUCCESS(rmdir(dirname));
	return 0;
}

int test_lchmod_file()
{
	int fd;
	int status;
	struct stat statbuf;
	const char *filename = "t-lchmod.file";
	const char *filename_symlink = "t-lchmod.file.sym";

	fd = creat(filename, 0770);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(symlink(filename, filename_symlink));

	status = stat(filename_symlink, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | 0770));

	status = lstat(filename_symlink, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFLNK | 0700));

	status = lchmod(filename_symlink, 0777);
	ASSERT_EQ(status, 0);

	status = lstat(filename_symlink, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFLNK | 0777));

	// The original file's permissions should not change.
	status = stat(filename_symlink, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | 0770));

	ASSERT_SUCCESS(unlink(filename));
	ASSERT_SUCCESS(unlink(filename_symlink));
	return 0;
}

int test_lchmod_dir()
{
	int status;
	struct stat statbuf;
	const char *dirname = "t-lchmod.dir";
	const char *dirname_symlink = "t-lchmod.dir.sym";

	ASSERT_SUCCESS(mkdir(dirname,0770));
	ASSERT_SUCCESS(symlink(dirname, dirname_symlink));

	status = stat(dirname_symlink, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFDIR | 0770));

	status = lstat(dirname_symlink, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFLNK | 0700));

	status = lchmod(dirname_symlink, 0777);
	ASSERT_EQ(status, 0);

	status = lstat(dirname_symlink, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFLNK | 0777));

	// The original file's permissions should not change.
	status = stat(dirname_symlink, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFDIR | 0770));

	ASSERT_SUCCESS(rmdir(dirname));
	ASSERT_SUCCESS(rmdir(dirname_symlink));
	return 0;
}

int test_fchmod()
{
	int fd;
	int status;
	struct stat statbuf;
	const char *filename = "t-fchmod";

	fd = creat(filename, 0700);

	status = stat(filename, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | 0700));

	status = fchmod(fd, 0777);
	ASSERT_EQ(status, 0);

	status = stat(filename, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | 0777));

	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(unlink(filename));
	return 0;
}

void cleanup()
{
	remove("t-chmod.file");
	remove("t-chmod.dir");
	remove("t-lchmod.file");
	remove("t-lchmod.file.sym");
	remove("t-lchmod.dir");
	remove("t-lchmod.dir.sym");
	remove("t-fchmod");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_chmod_file());
	TEST(test_chmod_dir());
	TEST(test_lchmod_file());
	TEST(test_lchmod_dir());
	TEST(test_fchmod());

	VERIFY_RESULT_AND_EXIT();
}
