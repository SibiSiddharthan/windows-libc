/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <test-macros.h>

void test_fflush()
{
	FILE *f = fopen("t-fflush", "w");
	fwrite("hello", 1, 5, f);
	size_t pos = lseek(fileno(f), 0, SEEK_CUR);
	ASSERT_EQ(pos, 0);

	int result = fflush(f);
	ASSERT_EQ(result, 0);
	pos = lseek(fileno(f), 0, SEEK_CUR);
	ASSERT_EQ(pos, 5);

	fclose(f);

	// check contents
	int fd = open("t-fflush", O_RDONLY);
	char buffer[16];
	ssize_t read_result = read(fd, buffer, 16);
	ASSERT_EQ(read_result, 5);
	ASSERT_MEMEQ(buffer, "hello", 5);
	close(fd);
	unlink("t-fflush");
}

void test_fflushall()
{
	size_t pos;

	FILE *f1 = fopen("t-fflushall1", "w");
	fwrite("hello", 1, 5, f1);
	pos = lseek(fileno(f1), 0, SEEK_CUR);
	ASSERT_EQ(pos, 0);

	FILE *f2 = fopen("t-fflushall2", "w");
	fwrite("world!", 1, 6, f2);
	pos = lseek(fileno(f2), 0, SEEK_CUR);
	ASSERT_EQ(pos, 0);
	
	int result = fflush(NULL);
	ASSERT_EQ(result, 0);

	pos = lseek(fileno(f1), 0, SEEK_CUR);
	ASSERT_EQ(pos, 5);

	pos = lseek(fileno(f2), 0, SEEK_CUR);
	ASSERT_EQ(pos, 6);

	fclose(f1);
	fclose(f2);

	unlink("t-fflushall1");
	unlink("t-fflushall2");
}

int main()
{
	test_fflush();
	test_fflushall();
	return 0;
}
