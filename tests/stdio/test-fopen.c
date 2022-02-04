/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <test-macros.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int test_EISDIR()
{
	errno = 0;
	FILE *f = fopen(".", "r");
	ASSERT_NULL(f);
	ASSERT_ERRNO(EISDIR);
	return 0;
}

/*
  For the below tests we try to mix fread, read and fwrite, write
  to see if they can be used together.
  These were originally written for testing the hooks to msvcrt.
  NOTE: stdio is not meant to be used in this way.
*/
int test_wplus()
{
	FILE *f;
	size_t flength;
	ssize_t llength;
	char rbuf[16];
	int fd;
	const char *filename = "t-fopen-w+";

	f = fopen(filename, "w+");
	ASSERT_NOTNULL(f);

	flength = fwrite((void *)"hello1", 1, 6, f);
	ASSERT_EQ(flength, 6);

	fd = fileno(f);
	flength = fwrite((void *)"hello2", 1, 6, f);
	ASSERT_EQ(flength, 6);

	fseek(f, 0, SEEK_SET);

	llength = read(fd, rbuf, 16);
	ASSERT_EQ(llength, 12);
	ASSERT_MEMEQ(rbuf, "hello1hello2", (int)llength);

	ASSERT_SUCCESS(fclose(f));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_aplus()
{
	FILE *f;
	size_t flength;
	ssize_t llength;
	off_t offset;
	char rbuf[16];
	int fd;
	const char *filename = "t-fopen-a+";

	f = fopen(filename, "a+");
	ASSERT_NOTNULL(f);

	flength = fwrite((void *)"hello1", 1, 6, f);
	ASSERT_EQ(flength, 6);

	fd = fileno(f);
	offset = lseek(fd, 0, SEEK_SET);
	ASSERT_EQ(offset, 0);

	flength = fwrite((void *)"hello2", 1, 6, f);
	ASSERT_EQ(flength, 6);

	fseek(f, 0, SEEK_SET);

	llength = read(fd, rbuf, 16);
	ASSERT_EQ(llength, 12);
	ASSERT_MEMEQ(rbuf, "hello1hello2", (int)llength);

	ASSERT_SUCCESS(fclose(f));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_rplus()
{
	FILE *f;
	size_t flength;
	ssize_t llength;
	char rbuf[16];
	int fd;
	int pos;
	const char *filename = "t-fopen-r+";

	fd = creat(filename, 0700);
	ASSERT_SUCCESS(close(fd));

	f = fopen(filename, "r+");
	ASSERT_NOTNULL(f);

	fd = fileno(f);
	llength = write(fd, (void *)"hello1", 6);
	ASSERT_EQ(llength, 6);

	llength = write(fd, (void *)"hello2", 6);
	ASSERT_EQ(llength, 6);

	pos = fseek(f, 0, SEEK_SET);
	ASSERT_EQ(pos, 0);

	flength = fread(rbuf, 1, 16, f);
	ASSERT_EQ(flength, 12);
	ASSERT_MEMEQ(rbuf, "hello1hello2", (int)flength);

	ASSERT_SUCCESS(fclose(f));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

void cleanup()
{
	remove("t-fopen-w+");
	remove("t-fopen-a+");
	remove("t-fopen-r+");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_EISDIR());
	TEST(test_wplus());
	TEST(test_aplus());
	TEST(test_rplus());

	VERIFY_RESULT_AND_EXIT();
}
