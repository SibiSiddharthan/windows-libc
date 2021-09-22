/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdio.h>
#include <test-macros.h>
#include <unistd.h>

void test_read()
{
	FILE *f = popen("pipe-helper 1", "r");

	char buf[16];
	ssize_t length = fread(buf, 1, 16, f);
	ASSERT_EQ(length, 5);
	buf[5] = '\0';
	ASSERT_STREQ(buf, "hello");

	int status = pclose(f);
	ASSERT_EQ(status, 0);
}

void test_write()
{
	FILE *f = popen("pipe-helper 2", "w");

	ssize_t length = fwrite("hello", 1, 5, f);
	ASSERT_EQ(length, 5);

	int status = pclose(f);
	ASSERT_EQ(status, 0);
}

void test_read_cr()
{
	FILE *f = popen("pipe-helper 3", "r");

	char buf[16];
	ssize_t length = fread(buf, 1, 16, f);
	ASSERT_EQ(length, 12);
	buf[12] = '\0';
	ASSERT_STREQ(buf, "hello\r\nworld");

	int status = pclose(f);
	ASSERT_EQ(status, 0);
}

void test_write_cr()
{
	FILE *f = popen("pipe-helper 4", "w");

	ssize_t length = fwrite("hello\r\nworld", 1, 12, f);
	ASSERT_EQ(length, 12);

	int status = pclose(f);
	ASSERT_EQ(status, 0);
}

int main()
{
	test_read();
	test_write();
	test_read_cr();
	test_write_cr();
	return 0;
}
