/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <test-macros.h>
#include <errno.h>

void test_EISDIR()
{
	errno = 0;
	FILE *f = fopen("CMakeFiles", "r");
	ASSERT_NULL(f);
	ASSERT_ERRNO(EISDIR);
}

/*
  For the below tests we try to mix fread, read and fwrite, write
  to see if they can be used together.
  These were originally written for testing the hooks to msvcrt.
  NOTE: stdio is not meant to be used in this way.
*/
void test_wplus()
{
	FILE *f = fopen("t-fopen", "w+");

	size_t flength = fwrite((void *)"hello1", 1, 6, f);
	ASSERT_EQ(flength, 6);

	int fd = fileno(f);
	ASSERT_EQ(fd, 3);

	flength = fwrite((void *)"hello2", 1, 6, f);
	ASSERT_EQ(flength, 6);

	fseek(f, 0, SEEK_SET);

	char rbuf[16];
	ssize_t llength = read(fd, rbuf, 16);
	ASSERT_EQ(llength, 12);
	rbuf[llength] = '\0';
	ASSERT_STREQ(rbuf, "hello1hello2");

	fclose(f);
	unlink("t-fopen");
}

void test_aplus()
{
	FILE *f = fopen("t-fopen", "a+");

	size_t flength = fwrite((void *)"hello1", 1, 6, f);
	ASSERT_EQ(flength, 6);

	int fd = fileno(f);
	ASSERT_EQ(fd, 3);

	off_t offset = lseek(fd, 0, SEEK_SET);
	ASSERT_EQ(offset, 0);

	flength = fwrite((void *)"hello2", 1, 6, f);
	ASSERT_EQ(flength, 6);

	fseek(f, 0, SEEK_SET);

	char rbuf[16];
	ssize_t llength = read(fd, rbuf, 16);
	ASSERT_EQ(llength, 12);
	rbuf[llength] = '\0';
	ASSERT_STREQ(rbuf, "hello1hello2");

	fclose(f);
	unlink("t-fopen");
}

void test_rplus()
{
	int fd = creat("t-fopen", 0700);
	close(fd);
	FILE *f = fopen("t-fopen", "r+");

	fd = fileno(f);
	ASSERT_EQ(fd, 3);

	ssize_t llength = write(fd, (void *)"hello1", 6);
	ASSERT_EQ(llength, 6);

	llength = write(fd, (void *)"hello2", 6);
	ASSERT_EQ(llength, 6);

	int pos = fseek(f, 0, SEEK_SET);
	ASSERT_EQ(pos, 0);

	char rbuf[16];
	size_t flength = fread(rbuf, 1, 16, f);
	ASSERT_EQ(flength, 12);
	rbuf[flength] = '\0';
	ASSERT_STREQ(rbuf, "hello1hello2");

	fclose(f);
	unlink("t-fopen");
}

int main()
{
	// test_EISDIR();
	test_wplus();
	test_aplus();
	test_rplus();
	return 0;
}
