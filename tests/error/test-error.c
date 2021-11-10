/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <error.h>
#include <stdio.h>
#include <test-macros.h>
#include <errno.h>
#include <string.h>

const char *filename = "t-error.log";

void setup()
{
	freopen(filename, "w+", stderr);
}

int cleanup()
{
	ASSERT_SUCCESS(fclose(stderr));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int check_log(const char *content)
{
	char buf[260];
	fseek(stderr, 0, SEEK_SET);
	ASSERT_EQ(ftell(stderr), 0);
	fread(buf, 1, 260, stderr);
	ASSERT_MEMEQ(buf, content, (int)strlen(content));

	// truncate file to make testing easier
	ASSERT_SUCCESS(ftruncate(fileno(stderr), 0));
	lseek(fileno(stderr), 0, SEEK_SET);

	return 0;
}

int test_error()
{
	char buffer[512];
	error(0, EINVAL, "sample error");
	snprintf(buffer, 512, "test-error: %s: %s\n", "sample error", strerror(EINVAL));
	ASSERT_SUCCESS(check_log(buffer))
	ASSERT_EQ(error_message_count, 1);
	return 0;
}

int test_error_at_line()
{
	char buffer[512];
	error_at_line(0, EINVAL, __FILE__, __LINE__, "sample error at line");
	snprintf(buffer, 512, "test-error: %s:%d: %s: %s\n", __FILE__, __LINE__ - 1, "sample error at line", strerror(EINVAL));
	ASSERT_SUCCESS(check_log(buffer))
	ASSERT_EQ(error_message_count, 2);
	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	setup();
	TEST(test_error());
	TEST(test_error_at_line());
	cleanup();

	VERIFY_RESULT_AND_EXIT();
}
