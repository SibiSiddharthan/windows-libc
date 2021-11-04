/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <fcntl.h>
#include <test-macros.h>
#include <errno.h>
#include <sys/stat.h>

int test_lesser_length()
{
	int status;
	int fd;
	ssize_t length;
	char read_buffer[16];
	const char *filename = "t-truncate-lesser";
	const char *write_buffer = "hello";

	fd = creat(filename, 0700);
	ASSERT_EQ(fd, 3);
	length = write(fd, write_buffer, 5);
	ASSERT_EQ(length, 5);
	ASSERT_SUCCESS(close(fd));

	status = truncate(filename, 3);
	ASSERT_EQ(status, 0);

	fd = open(filename, O_RDONLY);
	ASSERT_EQ(fd, 3);
	length = read(fd, read_buffer, 16);
	ASSERT_EQ(length, 3);
	ASSERT_MEMEQ(read_buffer, "hel", 3);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_greater_length()
{
	int status;
	int fd;
	ssize_t length;
	char read_buffer[16];
	const char *filename = "t-truncate-greater";
	const char *write_buffer = "hello";

	fd = creat(filename, 0700);
	ASSERT_EQ(fd, 3);
	length = write(fd, write_buffer, 5);
	ASSERT_EQ(length, 5);
	ASSERT_SUCCESS(close(fd));

	status = truncate(filename, 10);
	ASSERT_EQ(status, 0);

	fd = open(filename, O_RDONLY);
	ASSERT_EQ(fd, 3);
	length = read(fd, read_buffer, 16);
	ASSERT_EQ(length, 10);
	ASSERT_MEMEQ(read_buffer, "hello\0\0\0\0\0", 10);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_ftruncate()
{
	int status;
	int fd;
	off_t offset;
	ssize_t length;
	char read_buffer[16];
	const char *filename = "t-ftruncate";
	const char *write_buffer = "hello";

	fd = open(filename, O_CREAT | O_RDWR, 0700);
	ASSERT_EQ(fd, 3);
	length = write(fd, write_buffer, 5);
	ASSERT_EQ(length, 5);

	status = ftruncate(fd, 10);
	ASSERT_EQ(status, 0);

	// ftruncate does not alter the file pointer
	offset = lseek(fd, 0, SEEK_CUR);
	ASSERT_EQ(offset, 5);
	length = read(fd, read_buffer, 16);
	ASSERT_EQ(length, 5);
	ASSERT_MEMEQ(read_buffer, "\0\0\0\0\0", 5);

	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_readonly()
{
	int status;
	int fd;
	const char *filename = "t-truncate-readonly";

	fd = creat(filename, S_IREAD);
	ASSERT_EQ(fd, 3);
	ASSERT_SUCCESS(close(fd));

	status = truncate(filename, 10);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EACCES);

	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

void cleanup()
{
	remove("t-truncate-lesser");
	remove("t-truncate-greater");
	remove("t-ftruncate");
	remove("t-truncate-readonly");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_lesser_length());
	TEST(test_greater_length());
	TEST(test_ftruncate());
	TEST(test_readonly());

	VERIFY_RESULT_AND_EXIT();
}
