/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdio.h>
#include <unistd.h>
#include <test-macros.h>
#include <errno.h>

int test_getdelim()
{
	FILE *f;
	ssize_t result;
	size_t size = 0;
	char *buffer = NULL;
	const char *filename = "t-getdelim";

	f = fopen(filename, "w+");
	result = fwrite("abcdefghijklmnopqrstuvwxyz0123456789", 1, 36, f);
	ASSERT_EQ(result, 36);
	fseek(f, 0, SEEK_SET);
	ASSERT_EQ(ftell(f), 0);

	result = getdelim(NULL, NULL, 'a', f);
	ASSERT_EQ(result, -1);
	ASSERT_ERRNO(EINVAL);

	result = getdelim(&buffer, &size, 'i', f);
	ASSERT_EQ(result, 9);
	ASSERT_STREQ(buffer, "abcdefghi");

	result = getdelim(&buffer, &size, 'l', f);
	ASSERT_EQ(result, 3);
	ASSERT_STREQ(buffer, "jkl");

	result = getdelim(&buffer, &size, '2', f);
	ASSERT_EQ(result, 17);
	ASSERT_STREQ(buffer, "mnopqrstuvwxyz012");

	result = getdelim(&buffer, &size, 'a', f);
	ASSERT_EQ(result, -1);
	ASSERT_EQ(feof(f), 1);

	free(buffer);
	ASSERT_SUCCESS(fclose(f));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_getline()
{
	FILE *f;
	ssize_t result;
	size_t size = 64;
	char *buffer = (char *)malloc(size);
	const char *filename = "t-getline";

	f = fopen(filename, "w+");
	result = fwrite("abcdefghij\nklmnopqrstuvwxyz\n0123456789", 1, 38, f);
	ASSERT_EQ(result, 38);
	fseek(f, 0, SEEK_SET);
	ASSERT_EQ(ftell(f), 0);

	result = getline(&buffer, &size, f);
	ASSERT_EQ(result, 11);
	ASSERT_EQ(size, 64);
	ASSERT_STREQ(buffer, "abcdefghij\n");

	result = getline(&buffer, &size, f);
	ASSERT_EQ(result, 17);
	ASSERT_EQ(size, 64);
	ASSERT_STREQ(buffer, "klmnopqrstuvwxyz\n");

	result = getline(&buffer, &size, f);
	ASSERT_EQ(result, -1);
	ASSERT_EQ(size, 64);
	ASSERT_EQ(feof(f), 1);

	free(buffer);
	ASSERT_SUCCESS(fclose(f));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

void cleanup()
{
	remove("t-getdelim");
	remove("t-getline");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_getdelim());
	TEST(test_getline());

	VERIFY_RESULT_AND_EXIT();
}
