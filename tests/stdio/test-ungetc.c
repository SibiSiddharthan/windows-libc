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

void test_ungetc()
{
	FILE *f = fopen("t-ungetc", "w+");
	int result;

	// nothing has been read in yet, ungetc should return EOF
	result = ungetc('a', f);
	ASSERT_EQ(result, EOF);

	fwrite("hello world!", 1, 12, f);

	// Something has been written, ungetc only operates on input stream so EOF again.
	result = ungetc('a', f);
	ASSERT_EQ(result, EOF);

	rewind(f);

	char buffer[16];
	fread(buffer, 1, 16, f);
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

	fread(buffer, 1, 16, f);
	ASSERT_MEMEQ(buffer, "ba", 2);

	fclose(f);

	unlink("t-ungetc");
}

int main()
{
	test_ungetc();
	return 0;
}
