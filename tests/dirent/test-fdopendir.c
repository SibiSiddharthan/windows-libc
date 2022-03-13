/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

int test_ENOTDIR()
{
	errno = 0;
	DIR *D = NULL;
	const char *filename = "t-fdopendir.file";

	int fd = open(filename, O_WRONLY | O_CREAT, 0700);
	ASSERT_NOTEQ(fd, -1);

	D = fdopendir(fd);
	ASSERT_ERRNO(ENOTDIR);
	ASSERT_NULL(D);

	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_okay()
{
	errno = 0;
	int fd;
	DIR *D = NULL;

	fd = open(".", O_RDONLY);
	ASSERT_NOTEQ(fd, -1);

	D = fdopendir(fd);
	ASSERT_ERRNO(0);
	ASSERT_NOTNULL(D);
	ASSERT_SUCCESS(closedir(D));

	// closedir should have closed the underlying file descriptor.
	ASSERT_FAIL(close(fd));
	ASSERT_ERRNO(EBADF);

	return 0;
}

void cleanup()
{
	remove("t-fdopendir.file");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_ENOTDIR());
	TEST(test_okay());

	VERIFY_RESULT_AND_EXIT();
}
