/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <test-macros.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int test_redirect_stderr()
{
	FILE *f;
	int fd;
	char rbuf[16];
	ssize_t llength;
	const char *filename = "t-freopen-stderr";

	f = freopen(filename, "w", stderr);
	fprintf(stderr, "hello");
	ASSERT_EQ(fileno(f), 2);
	ASSERT_SUCCESS(fclose(f));

	fd = open(filename, O_RDONLY | O_EXCL);
	ASSERT_EQ(fd, 2);
	llength = read(fd, rbuf, 16);
	ASSERT_EQ(llength, 5);
	ASSERT_MEMEQ(rbuf, "hello", (int)llength);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(unlink(filename));
	return 0;
}

void cleanup()
{
	remove("t-freopen-stderr");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);
	TEST(test_redirect_stderr());
	// test_reopen_same_file();
	VERIFY_RESULT_AND_EXIT();
}
