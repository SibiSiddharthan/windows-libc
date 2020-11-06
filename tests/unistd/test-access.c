/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <unistd.h>
#include <test-macros.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

void test_ENOENT()
{
	errno = 0;
	int status = access("", F_OK);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOENT);
}

void test_EINVAL()
{
	errno = 0;
	int status = access("CMakeFiles", 8);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EINVAL);
}

void test_DIR()
{
	errno = 0;
	int status = access("CMakeFiles", F_OK | R_OK | W_OK | X_OK);
	ASSERT_EQ(status, 0);
}

void test_FILE()
{
	int fd;
	fd = creat("t-access", 0700);
	close(fd);
	int status;

	status = access("t-access", F_OK | R_OK | W_OK | X_OK);
	ASSERT_EQ(status, -1);
	status = access("t-access", F_OK | R_OK | W_OK);
	ASSERT_EQ(status, 0);
	chmod("t-access", S_IREAD);
	status = access("t-access", F_OK | R_OK | W_OK);
	ASSERT_EQ(status, -1);
	status = access("t-access", F_OK | R_OK);
	ASSERT_EQ(status, 0);

	unlink("t-access");

	fd = creat("t-access.exe", 0700);
	close(fd);

	status = access("t-access.exe", F_OK | R_OK | X_OK);
	ASSERT_EQ(status, 0);
	unlink("t-access.exe");
}

void test_symlink()
{
	int fd;
	fd = creat("t-access", 0700);
	close(fd);
	symlink("t-access", "t-access.sym");
	int status;

	status = access("t-access.sym", F_OK | R_OK | W_OK | X_OK);
	ASSERT_EQ(status, -1);
	status = access("t-access.sym", F_OK | R_OK | W_OK);
	ASSERT_EQ(status, 0);

	unlink("t-access.sym");
	unlink("t-access");
}

void test_faccessat()
{
	int dirfd = open("CMakeFiles/", O_RDONLY | O_EXCL);
	int fd = creat("CMakeFiles/t-access", 0700);
	close(fd);
	symlinkat("t-access", dirfd, "t-access.sym");
	int status;

	status = faccessat(dirfd, "t-access.sym", F_OK | R_OK | W_OK | X_OK, 0);
	ASSERT_EQ(status, -1);
	status = faccessat(dirfd, "t-access.sym", F_OK | R_OK | W_OK | X_OK, AT_SYMLINK_NOFOLLOW);
	ASSERT_EQ(status, 0);

	unlinkat(dirfd, "t-access", 0);
	unlinkat(dirfd, "t-access.sym", 0);
	close(dirfd);
}

int main()
{
	test_ENOENT();
	test_EINVAL();
	test_DIR();
	test_FILE();
	test_symlink();
	test_faccessat();
	return 0;
}