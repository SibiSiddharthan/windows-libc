/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int test_ungetc()
{
	int result;
	char buffer[16];
	size_t count;
	FILE *f;
	const char *filename = "t-ungetc";

	f = fopen(filename, "w+");

	// nothing has been read in yet, ungetc should return EOF
	result = ungetc('a', f);
	ASSERT_EQ(result, EOF);

	count = fwrite("hello world!", 1, 12, f);
	ASSERT_EQ(count, 12);

	// Something has been written, ungetc only operates on input stream so EOF again.
	result = ungetc('a', f);
	ASSERT_EQ(result, EOF);

	rewind(f);

	count = fread(buffer, 1, 16, f);
	ASSERT_EQ(count, 12);
	ASSERT_MEMEQ(buffer, "hello world!", 12);
	ASSERT_EQ(ftell(f), 12);
	ASSERT_EQ(feof(f), 1);

	result = ungetc('a', f);
	ASSERT_EQ(result, 'a');
	ASSERT_EQ(ftell(f), 11);
	ASSERT_EQ(feof(f), 0); // eof bit should be cleared

	result = ungetc('b', f);
	ASSERT_EQ(result, 'b');
	ASSERT_EQ(ftell(f), 10);
	ASSERT_EQ(feof(f), 0);

	count = fread(buffer, 1, 16, f);
	ASSERT_EQ(count, 2);
	ASSERT_MEMEQ(buffer, "ba", 2);

	ASSERT_SUCCESS(fclose(f));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

void cleanup()
{
	remove("t-ungetc");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);
	TEST(test_ungetc());
	VERIFY_RESULT_AND_EXIT();
}
