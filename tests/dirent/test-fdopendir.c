/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <dirent.h>
#include <errno.h>
#include <test-macros.h>
#include <fcntl.h>
#include <unistd.h>

void test_EBADF()
{
	errno = 0;
	int fd = open("CMakeCache.txt", O_RDONLY);
	DIR *D = fdopendir(fd);
	ASSERT_ERRNO(EBADF);
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
	test_EBADF();
	test_okay();
	return 0;
}
