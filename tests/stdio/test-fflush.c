/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int test_fflush()
{
	FILE *f;
	size_t pos, count;
	int result;
	int fd;
	char buffer[16];
	ssize_t read_result;
	const char *filename = "t-fflush";

	f = fopen(filename, "w");
	count = fwrite("hello", 1, 5, f);
	ASSERT_EQ(count, 5);

	pos = lseek(fileno(f), 0, SEEK_CUR);
	ASSERT_EQ(pos, 0);

	result = fflush(f);
	ASSERT_EQ(result, 0);
	pos = lseek(fileno(f), 0, SEEK_CUR);
	ASSERT_EQ(pos, 5);

	ASSERT_SUCCESS(fclose(f));

	// check contents
	fd = open(filename, O_RDONLY);
	read_result = read(fd, buffer, 16);
	ASSERT_EQ(read_result, 5);
	ASSERT_MEMEQ(buffer, "hello", 5);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(unlink(filename));
	return 0;
}

int test_fflushall()
{
	FILE *f1, *f2;
	size_t pos, count;
	int result;
	const char *filename1 = "t-fflushall1";
	const char *filename2 = "t-fflushall2";

	f1 = fopen(filename1, "w");
	count = fwrite("hello", 1, 5, f1);
	ASSERT_EQ(count, 5);
	pos = lseek(fileno(f1), 0, SEEK_CUR);
	ASSERT_EQ(pos, 0);

	f2 = fopen(filename2, "w");
	count = fwrite("world!", 1, 6, f2);
	ASSERT_EQ(count, 6);
	pos = lseek(fileno(f2), 0, SEEK_CUR);
	ASSERT_EQ(pos, 0);

	result = fflush(NULL);
	ASSERT_EQ(result, 0);

	pos = lseek(fileno(f1), 0, SEEK_CUR);
	ASSERT_EQ(pos, 5);

	pos = lseek(fileno(f2), 0, SEEK_CUR);
	ASSERT_EQ(pos, 6);

	ASSERT_SUCCESS(fclose(f1));
	ASSERT_SUCCESS(fclose(f2));

	ASSERT_SUCCESS(unlink(filename1));
	ASSERT_SUCCESS(unlink(filename2));

	return 0;
}

void cleanup()
{
	remove("t-fflush");
	remove("t-fflushall1");
	remove("t-fflushall2");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_fflush());
	TEST(test_fflushall());

	VERIFY_RESULT_AND_EXIT();
}
