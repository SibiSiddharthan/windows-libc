/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int test_tmpdir()
{
	printf("Temp directory: %s\n", P_tmpdir);
	return 0;
}

int test_tmpfile()
{
	// Just check whether stream is valid.
	FILE *stream = tmpfile();
	ASSERT_NOTNULL(stream);
	ASSERT_SUCCESS(fclose(stream));
	return 0;
}

int test_tmpnam()
{
	int status;
	char *filename = NULL;
	FILE *stream = NULL;
	char tempname[L_tmpnam];

	filename = tmpnam(NULL);
	printf("Temporary file: %s\n", filename);

	status = access(filename, F_OK);
	ASSERT_EQ(status, -1);

	strcpy(tempname, P_tmpdir);
	strcat(tempname, "file");

	stream = fopen(tempname, "wD");
	ASSERT_NOTNULL(stream);

	// The filename exists now, tmpnam should fail.
	filename = tmpnam("file");
	ASSERT_NULL(filename);

	ASSERT_SUCCESS(fclose(stream));

	return 0;
}

int test_tempnam()
{
	int status;
	char *filename = NULL;

	filename = tempnam(NULL, NULL);
	printf("Temporary file: %s\n", filename);

	status = access(filename, F_OK);
	ASSERT_EQ(status, -1);

	free(filename);

	filename = tempnam(NULL, "prefix");
	printf("Temporary file: %s\n", filename);

	status = access(filename, F_OK);
	ASSERT_EQ(status, -1);

	free(filename);

	filename = tempnam("dir", "prefix");
	printf("Temporary file: %s\n", filename);

	status = access(filename, F_OK);
	ASSERT_EQ(status, -1);

	free(filename);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_tmpdir());
	TEST(test_tmpfile());
	TEST(test_tmpnam());
	TEST(test_tempnam());

	VERIFY_RESULT_AND_EXIT();
}
