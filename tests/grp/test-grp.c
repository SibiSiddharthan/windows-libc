/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <test-macros.h>
#include <errno.h>
#include <grp.h>

int test_getgrent()
{
	struct group *entry;

	while (1)
	{
		entry = getgrent();
		if (entry == NULL)
		{
			break;
		}

		printf("Group\n");
		printf("gr_name : %s\n", entry->gr_name);
		printf("gr_gid  : %d\n", entry->gr_gid);
		printf("gr_mem  : %s", entry->gr_mem[0] == NULL ? NULL : "");
		for (int i = 0; entry->gr_mem[i] != NULL; ++i)
		{
			printf("%s ", entry->gr_mem[i]);
		}
		printf("\n");
		printf("\n");
	}

	endgrent();

	return 0;
}

int test_getgrent_r()
{
	int status;
	struct group entry;
	struct group *result;
	char buffer[1024];

	while (1)
	{
		status = getgrent_r(&entry, buffer, 1024, &result);
		if (result == NULL)
		{
			break;
		}

		printf("Group\n");
		printf("gr_name  : %s\n", entry.gr_name);
		printf("gr_gid   : %d\n", entry.gr_gid);
		printf("gr_mem  : %s", entry.gr_mem[0] == NULL ? NULL : "");
		for (int i = 0; entry.gr_mem[i] != NULL; ++i)
		{
			printf("%s ", entry.gr_mem[i]);
		}
		printf("\n");
		printf("\n");
	}

	endgrent();

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_getgrent());
	TEST(test_getgrent_r());

	VERIFY_RESULT_AND_EXIT();
}
