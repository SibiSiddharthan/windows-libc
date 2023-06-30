/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int setup()
{
	int fd;

	ASSERT_SUCCESS(mkdir("t", 0700));

	fd = creat("t/a1", 0700);
	ASSERT_SUCCESS(close(fd));
	fd = creat("t/a2", 0700);
	ASSERT_SUCCESS(close(fd));
	fd = creat("t/a3", 0700);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(mkdir("t/d1", 0700));
	ASSERT_SUCCESS(mkdir("t/d2", 0700));

	return 0;
}

int cleanup()
{
	ASSERT_SUCCESS(rmdir("t/d1"));
	ASSERT_SUCCESS(rmdir("t/d2"));
	ASSERT_SUCCESS(unlink("t/a1"));
	ASSERT_SUCCESS(unlink("t/a2"));
	ASSERT_SUCCESS(unlink("t/a3"));
	ASSERT_SUCCESS(rmdir("t"));

	return 0;
}

int selector(const struct dirent *entry)
{
	// Select only directories.
	if (entry->d_type == DT_DIR)
	{
		return 1;
	}

	return 0;
}

int compare(const struct dirent **a, const struct dirent **b)
{
	// Sort reverse alphabetically.
	return -alphasort(a, b);
}

int test_scandir()
{
	/* We step through the directory and check whether the order of files
	   listed is correct (alphabetical), it's deduced type is correct.
	*/
	struct dirent **entries = NULL;
	int count = scandirat(AT_FDCWD, "t", &entries, selector, compare);

	ASSERT_EQ(count, 4);

	// 1
	ASSERT_STREQ(entries[0]->d_name, "d2");
	ASSERT_EQ(entries[0]->d_namlen, 2);
	ASSERT_EQ(entries[0]->d_type, DT_DIR);

	// 2
	ASSERT_STREQ(entries[1]->d_name, "d1");
	ASSERT_EQ(entries[1]->d_namlen, 2);
	ASSERT_EQ(entries[1]->d_type, DT_DIR);

	// 3
	ASSERT_STREQ(entries[2]->d_name, "..");
	ASSERT_EQ(entries[2]->d_namlen, 2);
	ASSERT_EQ(entries[2]->d_type, DT_DIR);

	// 4
	ASSERT_STREQ(entries[3]->d_name, ".");
	ASSERT_EQ(entries[3]->d_namlen, 1);
	ASSERT_EQ(entries[3]->d_type, DT_DIR);

	// Free the entries.
	for (int i = 0; i < count; ++i)
	{
		free(entries[i]);
	}
	free(entries);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	// If the setup or cleanup fails exit the program
	if (setup() == 0)
	{
		TEST(test_scandir());
		if (cleanup() == 1)
		{
			printf("Cleanup failed\n");
			exit(1);
		}
	}
	else
	{
		printf("Setup failed\n");
		exit(1);
	}

	VERIFY_RESULT_AND_EXIT();
}
