/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <fcntl.h>
#include <unistd.h>
#include <tests/test.h>
#include <errno.h>

const char *content = "hello1\nhello2\n";

int test_EISDIR()
{
	errno = 0;
	int fd;
	ssize_t length;
	char rbuf[16];

	fd = open(".", O_RDONLY);
	length = read(fd, rbuf, 16);
	ASSERT_EQ(length, -1);
	ASSERT_ERRNO(EISDIR);
	ASSERT_SUCCESS(close(fd));

	return 0;
}

int test_read_write()
{
	errno = 0;
	int fd;
	ssize_t length;
	char rbuf[16];
	const char *filename = "t-read-write";

	fd = open(filename, O_RDWR | O_CREAT, 0700);
	ASSERT_NOTEQ(fd, -1);

	length = write(fd, (void *)content, 7);
	ASSERT_EQ(length, 7);

	length = write(fd, (void *)(content + 7), 7);
	ASSERT_EQ(length, 7);

	lseek(fd, 0, SEEK_SET);
	length = read(fd, rbuf, 16);
	ASSERT_EQ(length, 14);
	rbuf[length] = '\0';
	ASSERT_STREQ(rbuf, content);

	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_O_APPEND()
{
	errno = 0;
	int fd;
	ssize_t length;
	char rbuf[16];
	const char *filename = "t-read-append";

	fd = open(filename, O_RDWR | O_CREAT | O_APPEND, 0700);
	ASSERT_NOTEQ(fd, -1);

	length = write(fd, (void *)content, 7);
	ASSERT_EQ(length, 7);
	lseek(fd, 0, SEEK_SET);

	length = write(fd, (void *)(content + 7), 7);
	ASSERT_EQ(length, 7);
	lseek(fd, 0, SEEK_SET);

	length = read(fd, rbuf, 16);
	ASSERT_EQ(length, 14);
	rbuf[length] = '\0';
	ASSERT_STREQ(rbuf, content);

	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_no_O_APPEND()
{
	errno = 0;
	int fd;
	ssize_t length;
	char rbuf[16];
	const char *filename = "t-read-write-seek";

	fd = open(filename, O_RDWR | O_CREAT, 0700);
	ASSERT_NOTEQ(fd, -1);

	length = write(fd, (void *)content, 7);
	ASSERT_EQ(length, 7);
	lseek(fd, 0, SEEK_SET);

	length = write(fd, (void *)(content + 7), 7);
	ASSERT_EQ(length, 7);
	lseek(fd, 0, SEEK_SET);

	length = read(fd, rbuf, 16);
	ASSERT_EQ(length, 7);
	rbuf[length] = '\0';
	ASSERT_STREQ(rbuf, "hello2\n");

	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

// Only for files
int test_lseek()
{
	errno = 0;
	int fd;
	ssize_t length;
	off_t offset;
	char rbuf[16];
	const char *filename = "t-lseek";

	fd = open("t-lseek", O_RDWR | O_CREAT | O_TRUNC, 0700);
	ASSERT_NOTEQ(fd, -1);

	length = write(fd, (void *)content, 7);
	ASSERT_EQ(length, 7);
	offset = lseek(fd, 2, SEEK_SET);
	ASSERT_EQ(offset, 2);

	length = read(fd, rbuf, 16);
	ASSERT_EQ(length, 5);
	rbuf[length] = '\0';
	ASSERT_STREQ(rbuf, "llo1\n");

	offset = lseek(fd, -2, SEEK_CUR);
	ASSERT_EQ(offset, 5);

	length = write(fd, (void *)(content + 7), 7);
	ASSERT_EQ(length, 7);

	offset = lseek(fd, 0, SEEK_SET);
	ASSERT_EQ(offset, 0)
	length = read(fd, rbuf, 16);
	ASSERT_EQ(length, 12);
	rbuf[length] = '\0';
	ASSERT_STREQ(rbuf, "hellohello2\n");

	offset = lseek(fd, 2, SEEK_END); // move past end of file
	ASSERT_EQ(offset, 14);
	length = read(fd, rbuf, 16);
	ASSERT_EQ(length, 0); // end of file
	length = write(fd, "\n", 1);
	ASSERT_EQ(length, 1);

	offset = lseek(fd, 0, SEEK_SET);
	ASSERT_EQ(offset, 0)
	length = read(fd, rbuf, 16);
	ASSERT_EQ(length, 15);
	ASSERT_MEMEQ(rbuf, "hellohello2\n\0\0\n", 15);

	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

void cleanup()
{
	remove("t-read-write");
	remove("t-read-append");
	remove("t-read-write-seek");
	remove("t-lseek");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_EISDIR());
	TEST(test_read_write());
	TEST(test_O_APPEND());
	TEST(test_no_O_APPEND());
	TEST(test_lseek());

	VERIFY_RESULT_AND_EXIT();
}
