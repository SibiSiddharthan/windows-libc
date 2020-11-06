/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <stdio-hooks.h>
#include <fcntl.h>
#include <fcntl_internal.h>
#include <unistd.h>
#include <test-macros.h>
#include <errno.h>

void test_EISDIR()
{
	errno = 0;
	int fd = open("CMakeFiles", O_RDONLY);
	FILE *f = fdopen(fd, "r");
	ASSERT_NULL(f);
	ASSERT_ERRNO(EISDIR);
	close(fd);
}

void test_wrong_access()
{
	errno = 0;
	int fd = open("t-fdopen", O_RDONLY | O_CREAT, 0700);
	FILE *f = fdopen(fd, "w");
	// This actually won't write anything to the file
	size_t flength = fwrite((void *)"hello", 1, 6, f);
	fclose(f);
	ASSERT_EQ(validate_fd(fd), 0);

	fd = open("t-fdopen", O_RDONLY | O_EXCL);
	char rbuf[16];
	ssize_t llength = read(fd, rbuf, 16);
	ASSERT_EQ(llength, 0);
	close(fd);

	unlink("t-fdopen");
}

void test_correct_access()
{
	errno = 0;
	int fd = open("t-fdopen", O_RDWR | O_CREAT, 0700);
	FILE *f = fdopen(fd, "w");
	size_t flength = fwrite((void *)"hello", 1, 5, f);
	fclose(f);
	ASSERT_EQ(validate_fd(fd), 0);

	fd = open("t-fdopen", O_RDONLY | O_EXCL);
	char rbuf[16];
	ssize_t llength = read(fd, rbuf, 16);
	ASSERT_EQ(llength, 5);
	rbuf[llength] = '\0';
	ASSERT_STREQ(rbuf, "hello");
	close(fd);

	unlink("t-fdopen");
}

int main()
{
	test_EISDIR();
	test_wrong_access();
	test_correct_access();
	return 0;
}