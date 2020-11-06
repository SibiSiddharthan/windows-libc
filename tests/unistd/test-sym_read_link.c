/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <fcntl.h>
#include <test-macros.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <Windows.h>

#define MAX_PATH 260

void test_symlink_EEXIST()
{
	errno = 0;
	int fd = creat("t-symlink", 0700);
	close(fd);
	int status = symlink("junk", "t-symlink");
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EEXIST);
	unlink("t-symlink");
}

void test_symlink_ENOENT()
{
	errno = 0;
	int status = symlink("junk", "");
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOENT);
}

void test_symlink_EINVAL()
{
	errno = 0;
	int status = symlink("", "t-symlink");
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EINVAL);
}

void test_readlink_ENOENT1()
{
	errno = 0;
	char buf[MAX_PATH];
	ssize_t length = readlink("", buf, MAX_PATH);
	ASSERT_ERRNO(ENOENT);
	ASSERT_EQ(length, -1);
}

// unresolved symlink
void test_readlink_ENOENT2()
{
	errno = 0;
	char buf[MAX_PATH];
	int fd = creat("t-readlink", 0700);
	close(fd);
	symlink("t-readlink", "t-readlink.sym");
	unlink("t-readlink");
	ssize_t length = readlink("t-readlink.sym", buf, MAX_PATH);
	ASSERT_ERRNO(ENOENT);
	ASSERT_EQ(length, -1);
	unlink("t-readlink.sym");
}

void test_readlink_EINVAL()
{
	errno = 0;
	int fd = creat("t-readlink", 0700);
	close(fd);
	char buf[MAX_PATH];
	ssize_t length = readlink("t-readlink", buf, MAX_PATH);
	ASSERT_ERRNO(EINVAL);
	ASSERT_EQ(length, -1);
	unlink("t-readlink");
}

void test_okay_file()
{
	int fd = creat("t-readlink", 0700);
	close(fd);
	int status = symlink("t-readlink", "t-readlink.sym");
	ASSERT_EQ(status, 0);
	char buf[MAX_PATH];
	ssize_t length = readlink("t-readlink.sym", buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, 10);
	ASSERT_STREQ(buf, "t-readlink");
	status = unlink("t-readlink.sym");
	ASSERT_EQ(status, 0);
	unlink("t-readlink");
}

void test_okay_dir()
{
	mkdir("t-readlink.dir", 0700);
	int status = symlink("t-readlink.dir", "sym-t-readlink.dir");
	ASSERT_EQ(status, 0);
	char buf[MAX_PATH];
	ssize_t length = readlink("sym-t-readlink.dir", buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, 14);
	ASSERT_STREQ(buf, "t-readlink.dir");
	status = rmdir("sym-t-readlink.dir");
	ASSERT_EQ(status, 0);
	rmdir("t-readlink.dir");
}

void test_readlink_abs()
{
	int fd = creat("t-readlink", 0700);
	close(fd);
	int status = symlink("t-readlink", "t-readlink.sym");
	ASSERT_EQ(status, 0);
	char abspath[MAX_PATH];
	getcwd(abspath, MAX_PATH);
	strcat(abspath, "/t-readlink.sym");
	char buf[MAX_PATH];
	ssize_t length = readlink(abspath, buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, 10);
	ASSERT_STREQ(buf, "t-readlink");
	unlink("t-readlink.sym");
	unlink("t-readlink");
}

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

void test_small_bufsize1()
{
	int fd = creat("t-readlink", 0700);
	close(fd);
	int status = symlink("t-readlink", "t-readlink.sym");
	ASSERT_EQ(status, 0);
	char buf[6];
	ssize_t length = readlink("t-readlink.sym", buf, 5);
	buf[length] = '\0';
	ASSERT_EQ(length, 5);
	ASSERT_STREQ(buf, "t-rea");
	unlink("t-readlink.sym");
	unlink("t-readlink");
}

// Cross directory
void test_small_bufsize2()
{
	mkdir("t-readlink", 0700);
	int fd = creat("../file", 0700);
	close(fd);
	int status = symlink("../../file", "t-readlink/file.sym");
	ASSERT_EQ(status, 0);
	char buf[6];
	ssize_t length = readlink("t-readlink/file.sym", buf, 5);
	buf[length] = '\0';
	ASSERT_EQ(length, 5);
	ASSERT_STREQ(buf, "../..");
	unlink("t-readlink/file.sym");
	unlink("../file");
	rmdir("t-readlink");
}

void test_symlink_abs_file()
{
	int fd = creat("t-readlink", 0700);
	close(fd);
	char abspath[MAX_PATH];
	getcwd(abspath, MAX_PATH);
	strcat(abspath, "/t-readlink.sym");
	int status = symlink("t-readlink", abspath);
	ASSERT_EQ(status, 0);
	char buf[MAX_PATH];
	ssize_t length = readlink("t-readlink.sym", buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, 10);
	ASSERT_STREQ(buf, "t-readlink");
	status = unlink("t-readlink.sym");
	ASSERT_EQ(status, 0);
	unlink("t-readlink");
}

// Target is absolute path
void test_symlink_abs_dir1()
{
	mkdir("t-readlink.dir", 0700);
	char abspath[MAX_PATH];
	getcwd(abspath, MAX_PATH);
	strcat(abspath, "/t-readlink.dir");
	int status = symlink(abspath, "sym-t-readlink.dir");
	ASSERT_EQ(status, 0);
	char buf[MAX_PATH];
	ssize_t length = readlink("sym-t-readlink.dir", buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, 14);
	ASSERT_STREQ(buf, "t-readlink.dir");
	status = rmdir("sym-t-readlink.dir");
	ASSERT_EQ(status, 0);
	rmdir("t-readlink.dir");
}

// Source is absolute path
void test_symlink_abs_dir2()
{
	mkdir("t-readlink.dir", 0700);
	char abspath[MAX_PATH];
	getcwd(abspath, MAX_PATH);
	strcat(abspath, "/sym-t-readlink.dir");
	int status = symlink("t-readlink.dir", abspath);
	ASSERT_EQ(status, 0);
	char buf[MAX_PATH];
	ssize_t length = readlink("sym-t-readlink.dir", buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, 14);
	ASSERT_STREQ(buf, "t-readlink.dir");
	status = rmdir("sym-t-readlink.dir");
	ASSERT_EQ(status, 0);
	rmdir("t-readlink.dir");
}

// Both are absolute paths
void test_symlink_abs_dir3()
{
	mkdir("t-readlink.dir", 0700);
	char abspath_source[MAX_PATH], abspath_target[MAX_PATH];
	getcwd(abspath_source, MAX_PATH);
	getcwd(abspath_target, MAX_PATH);

	strcat(abspath_source, "/t-readlink.dir");
	strcat(abspath_target, "/sym-t-readlink.dir");
	int status = symlink(abspath_source, abspath_target);
	ASSERT_EQ(status, 0);
	char buf[MAX_PATH];
	ssize_t length = readlink("sym-t-readlink.dir", buf, MAX_PATH);
	buf[length] = '\0';
	ASSERT_EQ(length, 14);
	ASSERT_STREQ(buf, "t-readlink.dir");
	status = rmdir("sym-t-readlink.dir");
	ASSERT_EQ(status, 0);
	rmdir("t-readlink.dir");
}

int main()
{
	test_symlink_EEXIST();
	test_symlink_ENOENT();
	test_symlink_EINVAL();

	test_readlink_ENOENT1();
	test_readlink_ENOENT2();
	test_readlink_EINVAL();

	// Combined tests
	test_okay_file();
	test_okay_dir();
	test_readlink_abs();
	test_sub_directory();
	test_parent_directory();
	test_cross_directory1();
	test_cross_directory2();
	test_cross_directory4();

	// readlink specific
	test_small_bufsize1();
	test_small_bufsize2();

	// symlink specific
	test_symlink_abs_file();
	test_symlink_abs_dir1();
	test_symlink_abs_dir2();
	test_symlink_abs_dir3();
	return 0;
}