/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <fcntl.h>
#include <test-macros.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_PATH 260

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
	if (length != sizeof_test_content)
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

static int write_file_contents(int fd)
{
	ssize_t length;

	length = write(fd, test_content, sizeof_test_content);
	if (length != sizeof_test_content)
	{
		return -1;
	}

	return 0;
}

int test_symlink_ENOENT()
{
	errno = 0;
	int status = symlink("junk", "");
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOENT);
	return 0;
}

int test_symlink_EINVAL()
{
	errno = 0;
	int status;
	const char *filename = "t-symlink-inval";

	status = symlink("", filename);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EINVAL);
	ASSERT_FAIL(unlink(filename));

	return 0;
}

int test_symlink_EEXIST()
{
	errno = 0;
	int status;
	int fd;
	const char *filename = "t-symlink-exist";

	fd = creat(filename, 0700);
	ASSERT_SUCCESS(close(fd));

	status = symlink("junk", filename);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EEXIST);

	ASSERT_SUCCESS(unlink(filename));
	return 0;
}

int test_readlink_ENOENT()
{
	errno = 0;
	char buf[MAX_PATH];
	ssize_t length = readlink("", buf, MAX_PATH);
	ASSERT_ERRNO(ENOENT);
	ASSERT_EQ(length, -1);
	return 0;
}

int test_readlink_EINVAL()
{
	errno = 0;
	int fd;
	ssize_t length;
	char buf[MAX_PATH];
	const char *filename = "t-readlink-inval";

	fd = creat(filename, 0700);
	ASSERT_SUCCESS(close(fd));

	length = readlink(filename, buf, MAX_PATH);
	ASSERT_ERRNO(EINVAL);
	ASSERT_EQ(length, -1);

	ASSERT_SUCCESS(unlink(filename));
	return 0;
}

int test_file()
{
	int status;
	int fd;
	ssize_t length;
	char buf[MAX_PATH];
	const char *filename = "t-readlink.file";
	const char *filename_symlink = "t-readlink.file.sym";

	fd = creat(filename, 0700);
	ASSERT_SUCCESS(write_file_contents(fd));
	ASSERT_SUCCESS(close(fd));

	status = symlink(filename, filename_symlink);
	ASSERT_EQ(status, 0);

	length = readlink(filename_symlink, buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, strlen(filename));
	ASSERT_STREQ(buf, filename);

	ASSERT_SUCCESS(verify_file_contents(AT_FDCWD, filename_symlink));

	ASSERT_SUCCESS(unlink(filename_symlink));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_dir()
{
	int status;
	int fd, dirfd;
	ssize_t length;
	char buf[MAX_PATH];
	const char *dirname = "t-readlink.dir";
	const char *dirname_symlink = "t-readlink.dir.sym";
	const char *filename = "t-readlink.file";

	ASSERT_SUCCESS(mkdir(dirname, 0700));

	dirfd = open(dirname, O_RDONLY);
	fd = openat(dirfd, filename, O_CREAT | O_WRONLY, 0700);
	ASSERT_EQ(fd, 4);
	ASSERT_SUCCESS(write_file_contents(fd));
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(close(dirfd));

	status = symlink(dirname, dirname_symlink);
	ASSERT_EQ(status, 0);

	length = readlink(dirname_symlink, buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, strlen(dirname));
	ASSERT_STREQ(buf, dirname);

	dirfd = open(dirname_symlink, O_RDONLY);
	ASSERT_SUCCESS(verify_file_contents(dirfd, filename));

	ASSERT_SUCCESS(unlinkat(dirfd, filename, 0));
	ASSERT_SUCCESS(close(dirfd));

	ASSERT_SUCCESS(rmdir(dirname_symlink));
	ASSERT_SUCCESS(rmdir(dirname));

	return 0;
}

int test_absolute_path()
{
	int status;
	int fd;
	ssize_t length;
	int absolute_length;
	char filename_absolute[MAX_PATH], linkname_absolute[MAX_PATH], buf[MAX_PATH];
	const char *filename = "t-readlink-abs";
	const char *linkname = "t-readlink-abs.sym";

	fd = creat(filename, 0700);
	ASSERT_SUCCESS(write_file_contents(fd));
	ASSERT_SUCCESS(close(fd));

	getcwd(linkname_absolute, MAX_PATH);
	strcat(linkname_absolute, "/");
	strcat(linkname_absolute, linkname);

	getcwd(filename_absolute, MAX_PATH);
	strcat(filename_absolute, "/");
	strcat(filename_absolute, filename);
	absolute_length = strlen(filename_absolute);

	// case 1: source, target relative
	status = symlink(filename, linkname);
	ASSERT_EQ(status, 0);

	length = readlink(linkname_absolute, buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, strlen(filename));
	ASSERT_STREQ(buf, filename);

	length = readlink(linkname, buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, strlen(filename));
	ASSERT_STREQ(buf, filename);

	ASSERT_SUCCESS(verify_file_contents(AT_FDCWD, linkname));
	ASSERT_SUCCESS(unlink(linkname));

	// case 2: source is absolute, target relative
	status = symlink(filename_absolute, linkname);
	ASSERT_EQ(status, 0);

	length = readlink(linkname_absolute, buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, absolute_length);
	ASSERT_STREQ(buf, filename_absolute);

	length = readlink(linkname, buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, absolute_length);
	ASSERT_STREQ(buf, filename_absolute);

	ASSERT_SUCCESS(verify_file_contents(AT_FDCWD, linkname));
	ASSERT_SUCCESS(unlink(linkname));

	// case 3: source is relative, target absolute
	status = symlink(filename, linkname_absolute);
	ASSERT_EQ(status, 0);

	length = readlink(linkname_absolute, buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, strlen(filename));
	ASSERT_STREQ(buf, filename);

	length = readlink(linkname, buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, strlen(filename));
	ASSERT_STREQ(buf, filename);

	ASSERT_SUCCESS(verify_file_contents(AT_FDCWD, linkname_absolute));
	ASSERT_SUCCESS(unlink(linkname));

	// case 4: source, target absolute
	status = symlink(filename_absolute, linkname_absolute);
	ASSERT_EQ(status, 0);

	length = readlink(linkname_absolute, buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, absolute_length);
	ASSERT_STREQ(buf, filename_absolute);

	length = readlink(linkname, buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, absolute_length);
	ASSERT_STREQ(buf, filename_absolute);

	ASSERT_SUCCESS(verify_file_contents(AT_FDCWD, linkname_absolute));
	ASSERT_SUCCESS(unlink(linkname));

	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_relative_path()
{
	int status;
	int fd;
	ssize_t length;
	char buf[MAX_PATH];

	const char *dirname = "t-readlink-relative.dir";
	const char *dirname_child = "t-readlink-relative.dir/t-readlink-relative-child.dir";
	const char *filename = "t-readlink-relative.dir/t-readlink-relative.file";
	const char *symlink_parent = "t-readlink-relative.file.sym";
	const char *symlink_child = "t-readlink-relative.dir/t-readlink-relative-child.dir/t-readlink-relative.file.sym";
	const char *source_child = "../t-readlink-relative.file";

	ASSERT_SUCCESS(mkdir(dirname, 0700));
	ASSERT_SUCCESS(mkdir(dirname_child, 0700));

	fd = creat(filename, 0700);
	ASSERT_SUCCESS(write_file_contents(fd));
	ASSERT_SUCCESS(close(fd));

	// file -> dir/file
	status = symlink(filename, symlink_parent);
	ASSERT_EQ(status, 0);

	length = readlink(symlink_parent, buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, strlen(filename));
	ASSERT_STREQ(buf, filename);

	ASSERT_SUCCESS(verify_file_contents(AT_FDCWD, symlink_parent));
	ASSERT_SUCCESS(unlink(symlink_parent));

	// file -> ../file
	status = symlink(source_child, symlink_child);
	ASSERT_EQ(status, 0);

	length = readlink(symlink_child, buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, strlen(source_child));
	ASSERT_STREQ(buf, source_child);

	ASSERT_SUCCESS(verify_file_contents(AT_FDCWD, symlink_child));
	ASSERT_SUCCESS(unlink(symlink_child));

	// cleanup
	ASSERT_SUCCESS(unlink(filename));
	ASSERT_SUCCESS(rmdir(dirname_child));
	ASSERT_SUCCESS(rmdir(dirname));

	return 0;
}

int test_multilevel_symlink()
{
	int status;
	int fd;
	ssize_t length;
	char buf[MAX_PATH];
	const char *filename = "t-symlink-multi";
	const char *filename_sym1 = "t-symlink-multi.sym1";
	const char *filename_sym2 = "t-symlink-multi.sym2";

	fd = creat(filename, 0700);
	ASSERT_SUCCESS(write_file_contents(fd));
	ASSERT_SUCCESS(close(fd));

	status = symlink(filename, filename_sym1);
	ASSERT_EQ(status, 0);
	status = symlink(filename_sym1, filename_sym2);
	ASSERT_EQ(status, 0);

	length = readlink(filename_sym2, buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, strlen(filename_sym1));
	ASSERT_STREQ(buf, filename_sym1);

	length = readlink(filename_sym1, buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, strlen(filename));
	ASSERT_STREQ(buf, filename);

	ASSERT_SUCCESS(verify_file_contents(AT_FDCWD, filename_sym2));
	ASSERT_SUCCESS(verify_file_contents(AT_FDCWD, filename_sym1));

	ASSERT_SUCCESS(unlink(filename));
	ASSERT_SUCCESS(unlink(filename_sym1));
	ASSERT_SUCCESS(unlink(filename_sym2));

	return 0;
}

#if 0

// Redundant tests

void test_sub_directory()
{
	mkdir("t-readlink", 0700);
	int fd = creat("t-readlink/file", 0700);
	close(fd);
	int status = symlink("file", "t-readlink/file.sym");
	ASSERT_EQ(status, 0);
	char buf[MAX_PATH];
	ssize_t length = readlink("t-readlink/file.sym", buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, 4);
	ASSERT_STREQ(buf, "file");
	unlink("t-readlink/file.sym");
	unlink("t-readlink/file");
	rmdir("t-readlink");
}

void test_parent_directory()
{
	int fd = creat("../file", 0700);
	close(fd);
	int status = symlink("file", "../file.sym");
	ASSERT_EQ(status, 0);
	char buf[MAX_PATH];
	ssize_t length = readlink("../file.sym", buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, 4);
	ASSERT_STREQ(buf, "file");
	unlink("../file.sym");
	unlink("../file");
}

// symlink is in subdirectory
void test_cross_directory1()
{
	mkdir("t-readlink", 0700);
	int fd = creat("file", 0700);
	close(fd);
	int status = symlink("../file", "t-readlink/file.sym");
	ASSERT_EQ(status, 0);
	char buf[MAX_PATH];
	ssize_t length = readlink("t-readlink/file.sym", buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, 7);
	ASSERT_STREQ(buf, "../file");
	unlink("t-readlink/file.sym");
	unlink("file");
	rmdir("t-readlink");
}

// symlink is in parent directory
void test_cross_directory2()
{
	mkdir("t-readlink", 0700);
	int fd = creat("t-readlink/file", 0700);
	close(fd);
	int status = symlink("t-readlink/file", "file.sym");
	ASSERT_EQ(status, 0);
	char buf[MAX_PATH];
	ssize_t length = readlink("file.sym", buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, 15);
	ASSERT_STREQ(buf, "t-readlink/file");
	unlink("t-readlink/file");
	unlink("file.sym");
	rmdir("t-readlink");
}

/* Try to link this way
   ??/tests/unistd/t-readlink/file.sym -> ??/tests/t-readlink/file
*/
void test_cross_directory3()
{
	mkdir("t-readlink", 0700);
	mkdir("../t-readlink", 0700);
	int fd = creat("../t-readlink/file", 0700);
	close(fd);
	int status = symlink("../../t-readlink/file", "t-readlink/file.sym");
	ASSERT_EQ(status, 0);
	char buf[MAX_PATH];
	ssize_t length = readlink("t-readlink/file.sym", buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, 21);
	ASSERT_STREQ(buf, "../../t-readlink/file");
	unlink("../t-readlink/file");
	unlink("t-readlink/file.sym");
	rmdir("t-readlink");
	rmdir("../t-readlink");
}

// reverse of above test
void test_cross_directory4()
{
	mkdir("t-readlink", 0700);
	mkdir("../t-readlink", 0700);
	int fd = creat("t-readlink/file", 0700);
	close(fd);
	int status = symlink("../unistd/t-readlink/file", "../t-readlink/file.sym");
	ASSERT_EQ(status, 0);
	char buf[MAX_PATH];
	ssize_t length = readlink("../t-readlink/file.sym", buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, 25);
	ASSERT_STREQ(buf, "../unistd/t-readlink/file");
	unlink("../t-readlink/file.sym");
	unlink("t-readlink/file");
	rmdir("t-readlink");
	rmdir("../t-readlink");
}

#endif

int test_readlink_small_buffer()
{
	int status;
	int fd;
	ssize_t length;
	char buf[6];
	const char *filename = "t-readlink-small-buffer";
	const char *filename_symlink = "t-readlink-small-buffer.sym";

	fd = creat(filename, 0700);
	ASSERT_SUCCESS(close(fd));

	status = symlink(filename, filename_symlink);
	ASSERT_EQ(status, 0);

	length = readlink(filename_symlink, buf, 5);
	buf[length] = '\0';
	ASSERT_EQ(length, 5);
	ASSERT_STREQ(buf, "t-rea");

	ASSERT_SUCCESS(unlink(filename));
	ASSERT_SUCCESS(unlink(filename_symlink));

	return 0;
}

int test_readlink_unresolved()
{
	int status;
	ssize_t length;
	char buf[MAX_PATH];
	const char *filename = "t-readlink-unresolved";
	const char *filename_symlink = "t-readlink-unresolved.sym";

	status = symlink(filename, filename_symlink);
	ASSERT_EQ(status, 0);

	length = readlink(filename_symlink, buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, strlen(filename));
	ASSERT_STREQ(buf, filename);

	ASSERT_SUCCESS(unlink(filename_symlink)); // check whether a file is created
	ASSERT_FAIL(unlink(filename));

	return 0;
}

int test_at()
{
	int status;
	int fd, dirfd;
	ssize_t length;
	char buf[MAX_PATH];
	const char *dirname = "t-readlinkat.dir";
	const char *filename = "t-symlinkat";
	const char *filename_symlink = "t-symlinkat.sym";

	ASSERT_SUCCESS(mkdir(dirname, 0700));

	dirfd = open(dirname, O_RDONLY);
	fd = openat(dirfd, filename, O_CREAT | O_WRONLY, 0700);
	ASSERT_SUCCESS(write_file_contents(fd));
	ASSERT_SUCCESS(close(fd));

	status = symlinkat(filename, dirfd, filename_symlink);
	ASSERT_EQ(status, 0);

	length = readlinkat(dirfd, filename_symlink, buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, strlen(filename));
	ASSERT_STREQ(buf, filename);

	ASSERT_SUCCESS(verify_file_contents(dirfd, filename_symlink));

	ASSERT_SUCCESS(unlinkat(dirfd, filename_symlink, 0));
	ASSERT_SUCCESS(unlinkat(dirfd, filename, 0));

	ASSERT_SUCCESS(close(dirfd));
	ASSERT_SUCCESS(rmdir(dirname));

	return 0;
}

int test_at_empty_path()
{
	int status;
	int fd;
	ssize_t length;
	char buf[MAX_PATH];
	const char *filename = "t-readlinkat-empty-path";
	const char *filename_symlink = "t-readlinkat-empty-path.sym";

	fd = creat(filename, 0700);
	ASSERT_SUCCESS(close(fd));

	status = symlink(filename, filename_symlink);
	ASSERT_EQ(status, 0);

	fd = open(filename_symlink, O_NOFOLLOW | O_PATH);

	length = readlinkat(fd, "", buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, strlen(filename));
	ASSERT_STREQ(buf, filename);

	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(unlink(filename));
	ASSERT_SUCCESS(unlink(filename_symlink));

	return 0;
}

void cleanup()
{
	remove("t-symlink-exist");
	remove("t-readlink-inval");

	remove("t-readlink.file");
	remove("t-readlink.file.sym");

	remove("t-readlink.dir/t-readlink.file");
	remove("t-readlink.dir");
	remove("t-readlink.dir.sym");

	remove("t-readlink-abs");
	remove("t-readlink-abs.sym");

	remove("t-readlink-relative.dir/t-readlink-relative-child.dir/t-readlink-relative.file.sym");
	remove("t-readlink-relative.dir/t-readlink-relative.file");
	remove("t-readlink-relative.dir/t-readlink-relative-child.dir");
	remove("t-readlink-relative.dir");
	remove("t-readlink-relative.file.sym");

	remove("t-symlink-multi");
	remove("t-symlink-multi.sym1");
	remove("t-symlink-multi.sym2");

	remove("t-readlink-small-buffer");
	remove("t-readlink-small-buffer.sym");
	remove("t-readlink-unresolved.sym");

	remove("t-readlinkat.dir/t-symlinkat");
	remove("t-readlinkat.dir/t-symlinkat.sym");
	remove("t-readlinkat.dir");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_symlink_ENOENT());
	TEST(test_symlink_EINVAL());
	TEST(test_symlink_EEXIST());

	TEST(test_readlink_ENOENT());
	TEST(test_readlink_EINVAL());

	// Combined tests
	TEST(test_file());
	TEST(test_dir());
	TEST(test_absolute_path());
	TEST(test_relative_path());
	TEST(test_multilevel_symlink());

	// readlink specific
	TEST(test_readlink_small_buffer());
	TEST(test_readlink_unresolved());

	// at tests
	TEST(test_at());
	TEST(test_at_empty_path());

	VERIFY_RESULT_AND_EXIT();
}
