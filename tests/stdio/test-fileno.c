/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <test-macros.h>
#include <stdio.h>

int test_std_streams()
{
	int fd;
	fd = fileno(stdin);
	ASSERT_EQ(fd, 0);
	fd = fileno(stdout);
	ASSERT_EQ(fd, 1);
	fd = fileno(stderr);
	ASSERT_EQ(fd, 2);
	return 0;
}

int test_fopens()
{
	FILE *f1, *f2, *f3;

	f1 = fopen("t-fopens1", "wD");
	ASSERT_EQ(fileno(f1), 3);
	f2 = fopen("t-fopens2", "wD");
	ASSERT_EQ(fileno(f2), 4);

	ASSERT_SUCCESS(fclose(f1));

	f3 = fopen("t-fopens2", "wD");
	ASSERT_EQ(fileno(f3), 3);

	ASSERT_SUCCESS(fclose(f2));
	ASSERT_SUCCESS(fclose(f3));

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();
	TEST(test_std_streams());
	TEST(test_fopens());
	VERIFY_RESULT_AND_EXIT();
}
