/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <fcntl.h>
#include <test-macros.h>
#include <errno.h>

void test_lesser_length()
{
	char *write_buffer = "hello";
	int fd = creat("t-truncate", 0700);
	write(fd, write_buffer, 5);
	close(fd);

	int result = truncate("t-truncate", 3);
	ASSERT_EQ(result, 0);

	fd = open("t-truncate", O_RDONLY);
	char read_buffer[16];
	ASSERT_EQ(read(fd, read_buffer, 16), 3);
	ASSERT_EQ(memcmp(read_buffer, "hel", 3), 0)

	close(fd);
	unlink("t-truncate");
}

void test_greater_length()
{
	char *write_buffer = "hello";
	int fd = creat("t-truncate", 0700);
	write(fd, write_buffer, 5);
	close(fd);

	int result = truncate("t-truncate", 10);
	ASSERT_EQ(result, 0);

	fd = open("t-truncate", O_RDONLY);
	char read_buffer[16];
	ASSERT_EQ(read(fd, read_buffer, 16), 10);
	ASSERT_EQ(memcmp(read_buffer, "hello\0\0\0\0\0", 10), 0)

	close(fd);
	unlink("t-truncate");
}

void test_ftruncate()
{
	int fd = open("t-truncate", O_CREAT | O_RDWR, 0700);
	char *write_buffer = "hello";
	write(fd, write_buffer, 5);

	int result = ftruncate(fd, 10);
	ASSERT_EQ(result, 0);

	lseek(fd, 0, SEEK_SET);
	char read_buffer[16];
	ASSERT_EQ(read(fd, read_buffer, 16), 10);
	ASSERT_EQ(memcmp(read_buffer, "hello\0\0\0\0\0", 10), 0)

	close(fd);
	unlink("t-truncate");
}

void test_readonly()
{
	int fd = creat("t-truncate", S_IREAD);
	close(fd);
	int result = truncate("t-truncate", 10);
	ASSERT_EQ(result, -1);
	ASSERT_ERRNO(EACCES);
	unlink("t-truncate");
}

int main()
{
	test_lesser_length();
	test_greater_length();
	test_ftruncate();
	test_readonly();
	return 0;
}
