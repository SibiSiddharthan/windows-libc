/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <fcntl.h>
#include <unistd.h>
#include <tests/test.h>
#include <errno.h>

const char *content = "hello1\nhello2\n";

int test_pio()
{
	errno = 0;
	int fd;
	ssize_t length;
	char rbuf[16];
	const char *filename = "t-pio";

	fd = open(filename, O_RDWR | O_CREAT, 0700);
	ASSERT_NOTEQ(fd, -1);

	length = write(fd, (void *)content, 14);
	ASSERT_EQ(length, 14);
	ASSERT_EQ(lseek(fd, 0, SEEK_CUR), 14);

	length = pread(fd, rbuf, 5, 6);
	ASSERT_EQ(length, 5);
	ASSERT_MEMEQ(rbuf, "\nhell", 5);
	ASSERT_EQ(lseek(fd, 0, SEEK_CUR), 14);

	length = pread(fd, rbuf, 5, 15);
	ASSERT_EQ(length, 0);
	ASSERT_EQ(lseek(fd, 0, SEEK_CUR), 14);

	length = pwrite(fd, "world", 5, 0);
	ASSERT_EQ(length, 5);
	ASSERT_EQ(lseek(fd, 0, SEEK_CUR), 14);


	lseek(fd, 0, SEEK_SET);

	length = read(fd, rbuf, 10);
	ASSERT_EQ(length, 10);
	ASSERT_MEMEQ(rbuf, "world1\nhel", 10);
	ASSERT_EQ(lseek(fd, 0, SEEK_CUR), 10);

	length = pwrite(fd, "world", 5, 20);
	ASSERT_EQ(length, 5);
	ASSERT_EQ(lseek(fd, 0, SEEK_CUR), 10);

	length = read(fd, rbuf, 16);
	ASSERT_EQ(length, 15);
	ASSERT_MEMEQ(rbuf, "lo2\n\0\0\0\0\0\0world", 15);
	ASSERT_EQ(lseek(fd, 0, SEEK_CUR), 25);

	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_pio_null()
{
	errno = 0;
	int fd;
	ssize_t length;
	char rbuf[16];

	fd = open("/dev/null", O_RDWR | O_CREAT, 0700);
	ASSERT_NOTEQ(fd, -1);

	length = pwrite(fd, (void *)content, 14 , 10);
	ASSERT_EQ(length, 14);

	length = pread(fd, rbuf, 16, 5);
	ASSERT_EQ(length, 0);

	ASSERT_SUCCESS(close(fd));
	return 0;
}

void cleanup()
{
	remove("t-pio");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_pio());
	TEST(test_pio_null());

	VERIFY_RESULT_AND_EXIT();
}
