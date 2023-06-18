/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int test_EISDIR()
{
	errno = 0;
	int fd;
	FILE *f;

	fd = open(".", O_RDONLY);
	ASSERT_NOTEQ(fd, -1);

	f = fdopen(fd, "r");
	ASSERT_NULL(f);
	ASSERT_ERRNO(EISDIR);

	ASSERT_SUCCESS(close(fd));
	return 0;
}

int test_wrong_access()
{
	errno = 0;
	int fd;
	FILE *f;
	const char *filename = "t-fdopen-wrong-access";

	fd = open(filename, O_RDONLY | O_CREAT, 0700);
	ASSERT_NOTEQ(fd, -1);
	f = fdopen(fd, "w");
	ASSERT_NULL(f);
	ASSERT_ERRNO(EINVAL);

#if 0
	// This actually won't write anything to the file.
	// Let's keep this bit just in case we plan to use hooks again.
	size_t flength = fwrite((void *)"hello", 1, 6, f);
	fclose(f);
	//ASSERT_EQ(validate_fd(fd), 0);

	fd = open(filename, O_RDONLY | O_EXCL);
	char rbuf[16];
	ssize_t llength = read(fd, rbuf, 16);
	ASSERT_EQ(llength, 0);
#endif

	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_correct_access()
{
	errno = 0;
	int fd;
	size_t flength;
	ssize_t llength;
	char rbuf[16];
	FILE *f;
	const char *filename = "t-fdopen-correct-access";

	fd = open(filename, O_RDWR | O_CREAT, 0700);
	ASSERT_NOTEQ(fd, -1);

	f = fdopen(fd, "w");
	ASSERT_EQ(fileno(f), 3);
	flength = fwrite((void *)"hello", 1, 5, f);
	ASSERT_EQ(flength, 5);
	ASSERT_SUCCESS(fclose(f));
	ASSERT_FAIL(close(fd));
	ASSERT_ERRNO(EBADF);

	fd = open(filename, O_RDONLY | O_EXCL);
	llength = read(fd, rbuf, 16);
	ASSERT_EQ(llength, 5);
	ASSERT_MEMEQ(rbuf, "hello", (int)llength);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(unlink(filename));
	return 0;
}

void cleanup()
{
	remove("t-fdopen-wrong-access");
	remove("t-fdopen-correct-access");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_EISDIR());
	TEST(test_wrong_access());
	TEST(test_correct_access());

	VERIFY_RESULT_AND_EXIT();
}
