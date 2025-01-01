/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int test_redirect_stderr()
{
	FILE *stream;
	int fd;
	char rbuf[16];
	ssize_t result;
	const char *filename = "t-freopen-stderr";

	stream = freopen(filename, "w", stderr);
	fprintf(stderr, "hello");
	ASSERT_EQ(fileno(stream), 2);
	ASSERT_SUCCESS(fclose(stream));

	fd = open(filename, O_RDONLY | O_EXCL);
	ASSERT_EQ(fd, 2);
	result = read(fd, rbuf, 16);
	ASSERT_EQ(result, 5);
	ASSERT_MEMEQ(rbuf, "hello", (int)result);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(unlink(filename));
	return 0;
}

int test_reopen_same_file()
{
	FILE *stream;
	int fd;
	ssize_t result;
	char rbuf[16];
	const char *filename = "t-freopen-samefile";

	fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0700);
	result = write(fd, "hello", 5);
	ASSERT_EQ(result, 5);
	ASSERT_SUCCESS(close(fd));

	stream = fopen(filename, "r");
	ASSERT_NOTNULL(stream);

	errno = 0;
	result = fwrite("world", 1, 5, stream);
	ASSERT_EQ(result, 0);
	ASSERT_NOTEQ(ferror(stream), 0);
	ASSERT_ERRNO(EACCES);

	stream = freopen(NULL, "a", stream);
	ASSERT_NOTNULL(stream);

	result = fwrite("world", 1, 5, stream);
	ASSERT_EQ(result, 5);

	ASSERT_SUCCESS(fclose(stream));

	fd = open(filename, O_RDONLY, 0700);
	result = read(fd, rbuf, 32);
	ASSERT_EQ(result, 10);
	ASSERT_MEMEQ(rbuf, "helloworld", (int)result);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(unlink(filename));
	return 0;
}

void cleanup()
{
	remove("t-freopen-stderr");
	remove("t-freopen-samefile");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_redirect_stderr());
	TEST(test_reopen_same_file());

	VERIFY_RESULT_AND_EXIT();
}
