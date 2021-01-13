/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <fcntl.h>
#include <unistd.h>
#include <test-macros.h>
#include <errno.h>

const char *content = "hello1\nhello2\n";

void test_EBADF()
{
	errno = 0;
	int fd = open("t-read", O_RDONLY | O_CREAT, 0700);
	close(fd);
	fd = open("t-read", O_RDONLY | O_EXCL | O_PATH);
	char rbuf[16];
	ssize_t length = read(fd, rbuf, 16);
	ASSERT_EQ(length, -1);
	ASSERT_ERRNO(EBADF);
	close(fd);
	unlink("t-read");
}

void test_EISDIR()
{
	errno = 0;
	int fd = open("CMakefiles", O_RDONLY);
	char rbuf[16];
	ssize_t length = read(fd, rbuf, 16);
	ASSERT_EQ(length, -1);
	ASSERT_ERRNO(EISDIR);
	close(fd);
}

void test_read_write()
{
	errno = 0;
	int fd = open("t-read-write", O_RDWR | O_CREAT, 0700);
	ssize_t length;

	length = write(fd, (void *)content, 7);
	ASSERT_EQ(length, 7);

	length = write(fd, (void *)(content + 7), 7);
	ASSERT_EQ(length, 7);

	lseek(fd, 0, SEEK_SET);
	char rbuf[16];
	length = read(fd, rbuf, 16);
	ASSERT_EQ(length, 14);
	rbuf[length] = '\0';
	ASSERT_STREQ(rbuf, content);

	close(fd);
	unlink("t-read-write");
}

void test_O_APPEND()
{
	errno = 0;
	int fd = open("t-read-write", O_RDWR | O_CREAT | O_APPEND, 0700);
	ssize_t length;

	length = write(fd, (void *)content, 7);
	ASSERT_EQ(length, 7);
	lseek(fd, 0, SEEK_SET);

	length = write(fd, (void *)(content + 7), 7);
	ASSERT_EQ(length, 7);
	lseek(fd, 0, SEEK_SET);

	char rbuf[16];
	length = read(fd, rbuf, 16);
	ASSERT_EQ(length, 14);
	rbuf[length] = '\0';
	ASSERT_STREQ(rbuf, content);

	close(fd);
	unlink("t-read-write");
}

void test_no_O_APPEND()
{
	errno = 0;
	int fd = open("t-read-write", O_RDWR | O_CREAT, 0700);
	ssize_t length;

	length = write(fd, (void *)content, 7);
	ASSERT_EQ(length, 7);
	lseek(fd, 0, SEEK_SET);

	length = write(fd, (void *)(content + 7), 7);
	ASSERT_EQ(length, 7);
	lseek(fd, 0, SEEK_SET);

	char rbuf[16];
	length = read(fd, rbuf, 16);
	ASSERT_EQ(length, 7);
	rbuf[length] = '\0';
	ASSERT_STREQ(rbuf, "hello2\n");

	close(fd);
	unlink("t-read-write");
}

// Only for files
void test_lseek()
{
	errno = 0;
	int fd = open("t-lseek", O_RDWR | O_CREAT | O_TRUNC, 0700);
	ssize_t length;
	off_t offset;
	char rbuf[16];

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

	close(fd);
	unlink("t-lseek");
}

int main()
{
	test_EBADF();
	test_EISDIR();
	test_read_write();
	test_O_APPEND();
	test_no_O_APPEND();
	test_lseek();
	return 0;
}
