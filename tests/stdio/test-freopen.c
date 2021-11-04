/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <test-macros.h>
#include <errno.h>

int test_redirect_stdout()
{
	FILE *f;
	int fd;
	char rbuf[16];
	ssize_t llength;
	const char *filename = "t-freopen-stdout";

	f = freopen(filename, "w", stdout);
	printf("hello");
	ASSERT_EQ(fileno(f), 1);
	ASSERT_SUCCESS(fclose(f));

	fd = open(filename, O_RDONLY | O_EXCL);
	ASSERT_EQ(fd, 1);
	llength = read(fd, rbuf, 16);
	ASSERT_EQ(llength, 5);
	ASSERT_MEMEQ(rbuf, "hello", (int)llength);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(unlink(filename));
	return 0;
}

void cleanup()
{
	remove("t-freopen-stdout");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);
	TEST(test_redirect_stdout());
	// test_reopen_same_file();
	VERIFY_RESULT_AND_EXIT();
}
