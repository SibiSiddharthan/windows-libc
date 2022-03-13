/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

static const char *test_content = "Hello World!";
static const size_t sizeof_test_content = 12;

static int verify_file_contents(int dirfd, const char *filename)
{
	ssize_t length;
	char buf[16];
	int fd;

	fd = openat(dirfd, filename, O_RDONLY);
	if (fd == -1)
	{
		return -1;
	}

	length = read(fd, buf, 16);
	if (length != (ssize_t)sizeof_test_content)
	{
		return -1;
	}
	if (memcmp(buf, test_content, length) != 0)
	{
		return -1;
	}

	close(fd);
	return 0;
}

static int verify_file_contents_given(int dirfd, const char *filename, const char *content)
{
	ssize_t length;
	char buf[16];
	int fd;

	fd = openat(dirfd, filename, O_RDONLY);
	if (fd == -1)
	{
		return -1;
	}

	length = read(fd, buf, 16);
	if (length != (ssize_t)strlen(content))
	{
		return -1;
	}
	if (memcmp(buf, content, length) != 0)
	{
		return -1;
	}

	close(fd);
	return 0;
}

static int write_file_contents(int fd)
{
	ssize_t length;

	length = write(fd, test_content, sizeof_test_content);
	if (length != (ssize_t)sizeof_test_content)
	{
		return -1;
	}

	return 0;
}

int test_file()
{
	int status;
	int fd;
	const char *old_filename = "t-rename-old.file";
	const char *new_filename = "t-rename-new.file";

	fd = creat(old_filename, 0700);
	ASSERT_SUCCESS(write_file_contents(fd));
	ASSERT_SUCCESS(close(fd));

	status = rename(old_filename, new_filename);
	ASSERT_EQ(status, 0);

	ASSERT_SUCCESS(verify_file_contents(AT_FDCWD, new_filename));
	ASSERT_FAIL(verify_file_contents(AT_FDCWD, old_filename));

	ASSERT_SUCCESS(unlink(new_filename));
	ASSERT_FAIL(unlink(old_filename));

	return 0;
}

int test_dir()
{
	int status;
	int fd, dirfd;
	const char *old_dirname = "t-rename-old.dir";
	const char *new_dirname = "t-rename-new.dir";
	const char *filename = "t-rename.file";

	ASSERT_SUCCESS(mkdir(old_dirname, 0700));

	dirfd = open(old_dirname, O_RDONLY);
	fd = openat(dirfd, filename, O_CREAT | O_WRONLY, 0700);
	ASSERT_SUCCESS(write_file_contents(fd));
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(close(dirfd));

	status = rename(old_dirname, new_dirname);
	ASSERT_EQ(status, 0);

	dirfd = open(new_dirname, O_RDONLY);
	ASSERT_SUCCESS(verify_file_contents(dirfd, filename));

	ASSERT_SUCCESS(unlinkat(dirfd, filename, 0));
	ASSERT_SUCCESS(close(dirfd));

	ASSERT_SUCCESS(rmdir(new_dirname));
	ASSERT_FAIL(rmdir(old_dirname));

	return 0;
}

int test_overwrite_file()
{
	int status;
	int fd;
	const char *old_filename = "t-rename-overwrite-old.file";
	const char *new_filename = "t-rename-overwrite-new.file";

	fd = creat(old_filename, 0700);
	ASSERT_SUCCESS(write_file_contents(fd));
	ASSERT_SUCCESS(close(fd));

	// empty file
	fd = creat(new_filename, 0700);
	ASSERT_SUCCESS(close(fd));

	status = rename(old_filename, new_filename);
	ASSERT_EQ(status, 0);

	ASSERT_SUCCESS(verify_file_contents(AT_FDCWD, new_filename));
	ASSERT_FAIL(verify_file_contents(AT_FDCWD, old_filename));

	ASSERT_SUCCESS(unlink(new_filename));
	ASSERT_FAIL(unlink(old_filename));

	return 0;
}

int test_overwrite_dir()
{
	int status;
	int fd, dirfd;
	const char *old_dirname = "t-rename-overwrite-old.dir";
	const char *new_dirname = "t-rename-overwrite-new.dir";
	const char *filename = "t-rename-overwrite.file";

	ASSERT_SUCCESS(mkdir(old_dirname, 0700));
	ASSERT_SUCCESS(mkdir(new_dirname, 0700));

	dirfd = open(old_dirname, O_RDONLY);
	fd = openat(dirfd, filename, O_CREAT | O_WRONLY, 0700);
	ASSERT_SUCCESS(write_file_contents(fd));
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(close(dirfd));

	status = rename(old_dirname, new_dirname);
	ASSERT_EQ(status, 0);

	dirfd = open(new_dirname, O_RDONLY);
	ASSERT_SUCCESS(verify_file_contents(dirfd, filename));
	ASSERT_SUCCESS(close(dirfd));

	ASSERT_FAIL(rmdir(old_dirname));

	// create the old directory again with a empty file
	ASSERT_SUCCESS(mkdir(old_dirname, 0700));
	dirfd = open(old_dirname, O_RDONLY);

	fd = openat(dirfd, filename, O_CREAT | O_WRONLY, 0700);
	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(close(dirfd));

	status = rename(old_dirname, new_dirname);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOTEMPTY);

	// check whether the file in new directory is untouched
	dirfd = open(new_dirname, O_RDONLY);
	ASSERT_SUCCESS(verify_file_contents(dirfd, filename));

	ASSERT_SUCCESS(unlinkat(dirfd, filename, 0));
	ASSERT_SUCCESS(close(dirfd));

	dirfd = open(old_dirname, O_RDONLY);
	ASSERT_SUCCESS(unlinkat(dirfd, filename, 0));
	ASSERT_SUCCESS(close(dirfd));

	ASSERT_SUCCESS(rmdir(new_dirname));
	ASSERT_SUCCESS(rmdir(old_dirname));

	return 0;
}

int test_symlink()
{
	int status;
	int fd;
	ssize_t length;
	char buf[260];
	const char *filename = "t-rename-symlink";
	const char *old_symlink = "t-rename-symlink-old.sym";
	const char *new_symlink = "t-rename-symlink-new.sym";

	fd = creat(filename, 0700);
	ASSERT_SUCCESS(write_file_contents(fd));
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(symlink(filename, old_symlink));

	status = rename(old_symlink, new_symlink);
	ASSERT_EQ(status, 0);

	length = readlink(new_symlink, buf, 260);
	buf[length] = '\0';
	ASSERT_EQ(length, strlen(filename));
	ASSERT_STREQ(buf, filename);

	ASSERT_SUCCESS(verify_file_contents(AT_FDCWD, new_symlink));
	ASSERT_FAIL(verify_file_contents(AT_FDCWD, old_symlink));

	ASSERT_SUCCESS(unlink(filename));
	ASSERT_SUCCESS(unlink(new_symlink));
	ASSERT_FAIL(unlink(old_symlink));

	return 0;
}

int test_hardlink()
{
	int status;
	int fd;
	const char *filename = "t-rename-hardlink";
	const char *linkname = "t-rename-hardlink.lnk";

	fd = creat(filename, 0700);
	ASSERT_SUCCESS(write_file_contents(fd));
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(link(filename, linkname));

	status = rename(filename, linkname);
	ASSERT_EQ(status, 0);

	ASSERT_SUCCESS(verify_file_contents(AT_FDCWD, linkname));
	ASSERT_SUCCESS(verify_file_contents(AT_FDCWD, filename));

	// this should also be a no-op
	status = rename(filename, filename);
	ASSERT_EQ(status, 0);

	ASSERT_SUCCESS(verify_file_contents(AT_FDCWD, filename));

	ASSERT_SUCCESS(unlink(filename));
	ASSERT_SUCCESS(unlink(linkname));

	return 0;
}

int test_renameat()
{
	int status;
	int fd, olddirfd, newdirfd;
	const char *dirname = "t-renameat.dir";
	const char *old_filename = "t-renameat-old.file";
	const char *new_filename = "t-renameat-new.file";

	ASSERT_SUCCESS(mkdir(dirname, 0700));

	fd = creat(old_filename, 0700);
	ASSERT_SUCCESS(write_file_contents(fd));
	ASSERT_SUCCESS(close(fd));

	olddirfd = open(".", O_RDONLY);
	newdirfd = open(dirname, O_RDONLY);

	status = renameat(olddirfd, old_filename, newdirfd, new_filename);
	ASSERT_EQ(status, 0);

	ASSERT_SUCCESS(verify_file_contents(newdirfd, new_filename));
	ASSERT_FAIL(verify_file_contents(olddirfd, old_filename));

	ASSERT_SUCCESS(unlinkat(newdirfd, new_filename, 0));
	ASSERT_FAIL(unlinkat(olddirfd, old_filename, 0));

	ASSERT_SUCCESS(close(olddirfd));
	ASSERT_SUCCESS(close(newdirfd));
	ASSERT_SUCCESS(rmdir(dirname));

	return 0;
}

int test_renameat_noreplace()
{
	int status;
	int fd;
	ssize_t length;
	const char *old_filename = "t-renameat-noreplace-old.file";
	const char *new_filename = "t-renameat-noreplace-new.file";

	fd = creat(old_filename, 0700);
	length = write(fd, "Hello", 5);
	ASSERT_EQ(length, 5);
	ASSERT_SUCCESS(close(fd));

	fd = creat(new_filename, 0700);
	length = write(fd, "World", 5);
	ASSERT_EQ(length, 5);
	ASSERT_SUCCESS(close(fd));

	status = renameat2(AT_FDCWD, old_filename, AT_FDCWD, new_filename, RENAME_NOREPLACE);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EEXIST);

	// ensure files are intact
	ASSERT_SUCCESS(verify_file_contents_given(AT_FDCWD, old_filename, "Hello"));
	ASSERT_SUCCESS(verify_file_contents_given(AT_FDCWD, new_filename, "World"));

	ASSERT_SUCCESS(unlink(old_filename));
	ASSERT_SUCCESS(unlink(new_filename));

	return 0;
}

int test_renameat_exchange_file()
{
	int status;
	int fd;
	ssize_t length;
	const char *old_filename = "t-renameat-exchange-old.file";
	const char *new_filename = "t-renameat-exchange-new.file";

	fd = creat(old_filename, 0700);
	length = write(fd, "Hello", 5);
	ASSERT_EQ(length, 5);
	ASSERT_SUCCESS(close(fd));

	fd = creat(new_filename, 0700);
	length = write(fd, "World", 5);
	ASSERT_EQ(length, 5);
	ASSERT_SUCCESS(close(fd));

	status = renameat2(AT_FDCWD, old_filename, AT_FDCWD, new_filename, RENAME_EXCHANGE);
	ASSERT_EQ(status, 0);

	// ensure files are exchanged
	ASSERT_SUCCESS(verify_file_contents_given(AT_FDCWD, new_filename, "Hello"));
	ASSERT_SUCCESS(verify_file_contents_given(AT_FDCWD, old_filename, "World"));

	ASSERT_SUCCESS(unlink(old_filename));
	ASSERT_SUCCESS(unlink(new_filename));

	return 0;
}

int test_renameat_exchange_dir()
{
	int status;
	int fd, olddirfd, newdirfd;
	ssize_t length;
	const char *old_dirname = "t-rename-exchange-old.dir";
	const char *new_dirname = "t-rename-exchange-new.dir";
	const char *old_filename = "t-rename-exchange-old.file";
	const char *new_filename = "t-rename-exchange-new.file";

	ASSERT_SUCCESS(mkdir(old_dirname, 0700));
	ASSERT_SUCCESS(mkdir(new_dirname, 0700));

	olddirfd = open(old_dirname, O_RDONLY);
	newdirfd = open(new_dirname, O_RDONLY);

	fd = openat(olddirfd, old_filename, O_CREAT | O_WRONLY, 0700);
	length = write(fd, "Hello", 5);
	ASSERT_EQ(length, 5);
	ASSERT_SUCCESS(close(fd));

	fd = openat(newdirfd, new_filename, O_CREAT | O_WRONLY, 0700);
	length = write(fd, "World", 5);
	ASSERT_EQ(length, 5);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(close(olddirfd));
	ASSERT_SUCCESS(close(newdirfd));

	status = renameat2(AT_FDCWD, old_dirname, AT_FDCWD, new_dirname, RENAME_EXCHANGE);
	ASSERT_EQ(status, 0);

	olddirfd = open(old_dirname, O_RDONLY);
	newdirfd = open(new_dirname, O_RDONLY);

	ASSERT_SUCCESS(verify_file_contents_given(olddirfd, new_filename, "World"));
	ASSERT_SUCCESS(verify_file_contents_given(newdirfd, old_filename, "Hello"));

	ASSERT_SUCCESS(unlinkat(olddirfd, new_filename, 0));
	ASSERT_SUCCESS(unlinkat(newdirfd, old_filename, 0));

	ASSERT_SUCCESS(close(olddirfd));
	ASSERT_SUCCESS(close(newdirfd));

	ASSERT_SUCCESS(rmdir(old_dirname));
	ASSERT_SUCCESS(rmdir(new_dirname));

	return 0;
}

int test_renameat_EINVAL()
{
	int status = renameat2(AT_FDCWD, "dummy", AT_FDCWD, "dummy", RENAME_NOREPLACE | RENAME_EXCHANGE);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EINVAL);
	return 0;
}

void cleanup()
{
	remove("t-rename-old.file");
	remove("t-rename-new.file");

	remove("t-rename-old.dir/t-rename.file");
	remove("t-rename-new.dir/t-rename.file");
	remove("t-rename-old.dir");
	remove("t-rename-new.dir");

	remove("t-rename-overwrite-old.file");
	remove("t-rename-overwrite-new.file");

	remove("t-rename-overwrite-old.dir/t-rename-overwrite.file");
	remove("t-rename-overwrite-new.dir/t-rename-overwrite.file");
	remove("t-rename-overwrite-old.dir");
	remove("t-rename-overwrite-new.dir");

	remove("t-rename-symlink");
	remove("t-rename-symlink-old.sym");
	remove("t-rename-symlink-new.sym");

	remove("t-rename-hardlink");
	remove("t-rename-hardlink.lnk");

	remove("t-renameat.dir/t-renameat-new.file");
	remove("t-renameat.dir");
	remove("t-renameat-old.file");

	remove("t-renameat-noreplace-old.file");
	remove("t-renameat-noreplace-new.file");

	remove("t-renameat-exchange-old.file");
	remove("t-renameat-exchange-new.file");

	remove("t-rename-exchange-old.dir/t-rename-exchange-old.file");
	remove("t-rename-exchange-old.dir/t-rename-exchange-new.file");
	remove("t-rename-exchange-new.dir/t-rename-exchange-old.file");
	remove("t-rename-exchange-new.dir/t-rename-exchange-new.file");
	remove("t-rename-exchange-old.dir");
	remove("t-rename-exchange-new.dir");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	// rename tests
	TEST(test_file());
	TEST(test_dir());
	TEST(test_overwrite_file());
	TEST(test_overwrite_dir());
	TEST(test_symlink());
	TEST(test_hardlink());

	// renameat tests
	TEST(test_renameat());
	TEST(test_renameat_noreplace());
	TEST(test_renameat_exchange_file());
	TEST(test_renameat_exchange_dir());
	TEST(test_renameat_EINVAL());

	VERIFY_RESULT_AND_EXIT();
}
