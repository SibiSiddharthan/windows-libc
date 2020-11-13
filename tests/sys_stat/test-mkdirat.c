/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <sys/stat.h>
#include <fcntl.h>
#include <test-macros.h>
#include <errno.h>
#include <Windows.h>
#include <unistd.h>

/*
  This was the first 'at' function I wrote.
  The fd checks are performed here.
*/

void test_EBADF()
{
	errno = 0;
	int status = mkdirat(-1, "t-mkdir", 0700);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EBADF);
}

void test_ENOTDIR()
{
	errno = 0;
	int fd = creat("test", 0);
	int status = mkdirat(fd, "t-mkdir", 0700);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOTDIR);
	close(fd);
	unlink("test");
}

void test_abs_path()
{
	errno = 0;
	int fd = open("CMakeFiles/", O_RDONLY);
	char path[260];
	getcwd(path, 260);
	strcat(path, "/t-mkdir");
	int status = mkdirat(fd, path, 0700);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(GetFileAttributesA("t-mkdir"), FILE_ATTRIBUTE_DIRECTORY);
	rmdir("t-mkdir");
	close(fd);
}

void test_okay()
{
	errno = 0;
	int fd = open("CMakeFiles/", O_RDONLY);
	int status = mkdirat(fd, "t-mkdir", 0700);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(GetFileAttributesA("CMakeFiles/t-mkdir"), FILE_ATTRIBUTE_DIRECTORY);
	rmdir("CMakeFiles/t-mkdir");
	close(fd);
}

int main()
{
	// fd checks
	test_EBADF();
	test_ENOTDIR();
	test_abs_path();

	test_okay();
	return 0;
}
