/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdio.h>
#include <test-macros.h>
#include <unistd.h>

int test_read()
{
	int status;
	char buf[16];
	size_t length;
	FILE *f;

	f = popen("pipe-helper 1", "r");
	length = fread(buf, 1, 16, f);
	ASSERT_EQ(length, 5);
	ASSERT_MEMEQ(buf, "hello", (int)length);

	status = pclose(f);
	ASSERT_EQ(status, 0);

	return 0;
}

int test_write()
{
	int status;
	char buf[16];
	size_t length;
	FILE *f;

	f = popen("pipe-helper 2", "w");
	length = fwrite("hello", 1, 5, f);
	ASSERT_EQ(length, 5);

	status = pclose(f);
	ASSERT_EQ(status, 0);

	return 0;
}

int test_read_cr()
{
	int status;
	char buf[16];
	size_t length;
	FILE *f;

	f = popen("pipe-helper 3", "r");
	length = fread(buf, 1, 16, f);
	ASSERT_EQ(length, 12);
	ASSERT_MEMEQ(buf, "hello\r\nworld", (int)length);

	status = pclose(f);
	ASSERT_EQ(status, 0);

	return 0;
}

int test_write_cr()
{
	int status;
	char buf[16];
	size_t length;
	FILE *f;

	f = popen("pipe-helper 4", "w");
	length = fwrite("hello\r\nworld", 1, 12, f);
	ASSERT_EQ(length, 12);

	status = pclose(f);
	ASSERT_EQ(status, 0);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_read());
	TEST(test_write());
	TEST(test_read_cr());
	TEST(test_write_cr());

	VERIFY_RESULT_AND_EXIT();
}
