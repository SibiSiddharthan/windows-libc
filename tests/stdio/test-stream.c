/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdio_ext.h>

int test_buffer()
{
	int status;
	size_t size;
	void *buffer;
	FILE *stream;
	const char *filename = "t-buffer";

	stream = fopen(filename, "w");
	ASSERT_NOTNULL(stream);

	status = setvbuf(stream, NULL, _IOLBF, 1024);
	ASSERT_EQ(status, 0);

	ASSERT_EQ(__fbufsize(stream), 1024);
	ASSERT_EQ(__fbufmode(stream), _IOLBF);
	ASSERT_EQ(__flbf(stream), 1);

	buffer = (void *)__freadptr(stream, &size);
	ASSERT_EQ(size, 1024);

	status = setvbuf(stream, NULL, _IONBF, 0);
	ASSERT_EQ(status, 0);

	ASSERT_EQ(__fbufsize(stream), 0);
	ASSERT_EQ(__fbufmode(stream), _IONBF);
	ASSERT_EQ(__fnbf(stream), 1);

	buffer = (void *)__freadptr(stream, &size);
	ASSERT_EQ(size, 0);
	ASSERT_NULL(buffer);

	ASSERT_SUCCESS(fclose(stream));
	ASSERT_SUCCESS(remove(filename));

	return 0;
}

int test_mode()
{
	FILE *stream;
	const char *filename = "t-mode";

	stream = fopen(filename, "w");
	ASSERT_NOTNULL(stream);

	ASSERT_EQ(__freadable(stream), 0);
	ASSERT_EQ(__fwritable(stream), 1);
	ASSERT_EQ(__freading(stream), 0);
	ASSERT_EQ(__fwriting(stream), 1);

	ASSERT_SUCCESS(fclose(stream));

	stream = fopen(filename, "r");
	ASSERT_NOTNULL(stream);

	ASSERT_EQ(__freadable(stream), 1);
	ASSERT_EQ(__fwritable(stream), 0);
	ASSERT_EQ(__freading(stream), 1);
	ASSERT_EQ(__fwriting(stream), 0);

	ASSERT_SUCCESS(fclose(stream));

	stream = fopen(filename, "r+");
	ASSERT_NOTNULL(stream);

	ASSERT_EQ(__freadable(stream), 1);
	ASSERT_EQ(__fwritable(stream), 1);
	ASSERT_EQ(__freading(stream), 0);
	ASSERT_EQ(__fwriting(stream), 0);

	ASSERT_SUCCESS(fclose(stream));
	ASSERT_SUCCESS(remove(filename));

	return 0;
}

int test_pending()
{
	int status;
	ssize_t result;
	FILE *stream;
	char buf[16];
	const char *filename = "t-buffer";

	stream = fopen(filename, "w+");
	ASSERT_NOTNULL(stream);

	result = fwrite("hello", 1, 5, stream);
	ASSERT_EQ(result, 5);
	ASSERT_EQ(__fpending(stream), 5);
	ASSERT_EQ(__freadahead(stream), 0);
	ASSERT_EQ(__fwriting(stream), 1);

	fflush(stream);
	ASSERT_EQ(__fpending(stream), 0);

	rewind(stream);

	result = fread(buf, 1, 2, stream);
	ASSERT_EQ(result, 2);
	ASSERT_EQ(__freadahead(stream), 3);
	ASSERT_EQ(__fpending(stream), 0);
	ASSERT_EQ(__freading(stream), 1);

	__freadptrinc(stream, 2);
	ASSERT_EQ(__freadahead(stream), 1);
	__fpurge(stream);
	ASSERT_EQ(__freadahead(stream), 0);

	// Do it again, but turn off buffering.
	status = setvbuf(stream, NULL, _IONBF, 0);
	ASSERT_EQ(status, 0);
	rewind(stream);

	result = fwrite("hello", 1, 5, stream);
	ASSERT_EQ(result, 5);
	ASSERT_EQ(__fpending(stream), 0);
	ASSERT_EQ(__fwriting(stream), 1);

	rewind(stream);

	result = fread(buf, 1, 2, stream);
	ASSERT_EQ(result, 2);
	ASSERT_EQ(__freadahead(stream), 0);
	ASSERT_EQ(__freading(stream), 1);

	ASSERT_SUCCESS(fclose(stream));
	ASSERT_SUCCESS(remove(filename));

	return 0;
}

void cleanup()
{
	remove("t-buffer");
	remove("t-mode");
	remove("t-pending");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_buffer());
	TEST(test_mode());
	TEST(test_pending());

	VERIFY_RESULT_AND_EXIT();
}
