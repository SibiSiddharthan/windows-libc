/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdio.h>
#include <fcntl.h>
#include <internal/fcntl.h>
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
	ASSERT_NULL(f);
	ASSERT_ERRNO(EINVAL);

#if 0
	// This actually won't write anything to the file.
	// Let's keep this bit just in case we plan to use hooks again.
	size_t flength = fwrite((void *)"hello", 1, 6, f);
	fclose(f);
	//ASSERT_EQ(validate_fd(fd), 0);

	fd = open("t-fdopen", O_RDONLY | O_EXCL);
	char rbuf[16];
	ssize_t llength = read(fd, rbuf, 16);
	ASSERT_EQ(llength, 0);
#endif

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
