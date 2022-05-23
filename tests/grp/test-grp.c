/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <errno.h>
#include <grp.h>
#include <unistd.h>

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
		ASSERT_EQ(status, 0);
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

int test_getgrnam()
{
	int status;
	struct group *entry, *grnam_entry, *grnam_r_result, grnam_r_entry;
	char buffer[1024];
	const char *not_a_group = "notagroup";

	while (1)
	{
		entry = getgrent();
		if (entry == NULL)
		{
			break;
		}

		// getgrnam
		grnam_entry = getgrnam(entry->gr_name);
		if (grnam_entry == NULL)
		{
			printf("Search for group '%s' failed\n", entry->gr_name);
			return -1;
		}

		ASSERT_STREQ(grnam_entry->gr_name, entry->gr_name);
		ASSERT_EQ(grnam_entry->gr_gid, entry->gr_gid);
		for (int i = 0; entry->gr_mem[i] != NULL; ++i)
		{
			ASSERT_STREQ(grnam_entry->gr_mem[i], entry->gr_mem[i]);
		}

		// getgrnam_r
		status = getgrnam_r(entry->gr_name, &grnam_r_entry, buffer, 1024, &grnam_r_result);
		ASSERT_EQ(status, 0);
		if (grnam_r_result == NULL)
		{
			printf("Search for group '%s' failed\n", entry->gr_name);
			return -1;
		}

		ASSERT_STREQ(grnam_r_entry.gr_name, entry->gr_name);
		ASSERT_EQ(grnam_r_entry.gr_gid, entry->gr_gid);
		for (int i = 0; entry->gr_mem[i] != NULL; ++i)
		{
			ASSERT_STREQ(grnam_r_entry.gr_mem[i], entry->gr_mem[i]);
		}
	}

	endgrent();

	// Non existent groups
	errno = 0;
	grnam_entry = getgrnam(not_a_group);
	ASSERT_NULL(grnam_entry);
	ASSERT_ERRNO(ENOENT);

	status = getgrnam_r(not_a_group, &grnam_r_entry, buffer, 1024, &grnam_r_result);
	ASSERT_NULL(grnam_r_result);
	ASSERT_EQ(status, ENOENT);

	// Small buffer error
	status = getgrnam_r("Administrators", &grnam_r_entry, buffer, 16, &grnam_r_result);
	ASSERT_NULL(grnam_r_result);
	ASSERT_EQ(status, ERANGE);

	return 0;
}

int test_getgrgid()
{
	int status;
	struct group *entry, *grgid_entry, *grgid_r_result, grgid_r_entry;
	char buffer[1024];
	gid_t non_existent_group = 200;

	while (1)
	{
		entry = getgrent();
		if (entry == NULL)
		{
			break;
		}

		// getgrgid
		grgid_entry = getgrgid(entry->gr_gid);
		if (grgid_entry == NULL)
		{
			printf("Search for group '%s' failed\n", entry->gr_name);
			return -1;
		}

		ASSERT_STREQ(grgid_entry->gr_name, entry->gr_name);
		ASSERT_EQ(grgid_entry->gr_gid, entry->gr_gid);
		for (int i = 0; entry->gr_mem[i] != NULL; ++i)
		{
			ASSERT_STREQ(grgid_entry->gr_mem[i], entry->gr_mem[i]);
		}

		// getgrgid_r
		status = getgrgid_r(entry->gr_gid, &grgid_r_entry, buffer, 1024, &grgid_r_result);
		ASSERT_EQ(status, 0);
		if (grgid_r_result == NULL)
		{
			printf("Search for group '%s' failed\n", entry->gr_name);
			return -1;
		}

		ASSERT_STREQ(grgid_r_entry.gr_name, entry->gr_name);
		ASSERT_EQ(grgid_r_entry.gr_gid, entry->gr_gid);
		for (int i = 0; entry->gr_mem[i] != NULL; ++i)
		{
			ASSERT_STREQ(grgid_r_entry.gr_mem[i], entry->gr_mem[i]);
		}
	}

	endgrent();

	// Non existent groups
	errno = 0;
	grgid_entry = getgrgid(non_existent_group);
	ASSERT_NULL(grgid_entry);
	ASSERT_ERRNO(ENOENT);

	status = getgrgid_r(non_existent_group, &grgid_r_entry, buffer, 1024, &grgid_r_result);
	ASSERT_NULL(grgid_r_result);
	ASSERT_EQ(status, ENOENT);

	return 0;
}

int test_root()
{
	struct group *entry_id = NULL, *entry_name = NULL;

	entry_id = getgrgid(ROOT_UID);
	ASSERT_NOTNULL(entry_id)

	entry_name = getgrnam("root");
	ASSERT_NOTNULL(entry_id)

	ASSERT_STREQ(entry_id->gr_name, entry_name->gr_name);
	ASSERT_EQ(entry_id->gr_gid, entry_name->gr_gid);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_getgrent());
	TEST(test_getgrent_r());
	TEST(test_getgrnam());
	TEST(test_getgrgid());
	TEST(test_root());

	VERIFY_RESULT_AND_EXIT();
}
