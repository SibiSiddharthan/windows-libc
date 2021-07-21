/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <dirent.h>
#include <errno.h>
#include <test-macros.h>
#include <fcntl.h>
#include <unistd.h>

void test_ENOTDIR()
{
	errno = 0;
	int fd = open("CMakeCache.txt", O_RDONLY);
	DIR *D = fdopendir(fd);
	ASSERT_ERRNO(ENOTDIR);
	ASSERT_NULL(D);
	close(fd);
}

void test_okay()
{
	errno = 0;
	int fd = open(".", O_RDONLY);
	DIR *D = fdopendir(fd);
	ASSERT_ERRNO(0);
	closedir(D);
}

int main()
{
	test_ENOTDIR();
	test_okay();
	return 0;
}
