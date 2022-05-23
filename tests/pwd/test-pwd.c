/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <errno.h>
#include <pwd.h>
#include <unistd.h>

int test_getpwent()
{
	struct passwd *entry;

	while (1)
	{
		entry = getpwent();
		if (entry == NULL)
		{
			break;
		}

		printf("User\n");
		printf("pw_name  : %s\n", entry->pw_name);
		printf("pw_uid   : %d\n", entry->pw_uid);
		printf("pw_gid   : %d\n", entry->pw_gid);
		printf("pw_gecos : %s\n", entry->pw_gecos);
		printf("pw_dir   : %s\n", entry->pw_dir);
		printf("\n");
	}

	endpwent();

	return 0;
}

int test_getpwent_r()
{
	int status;
	struct passwd entry;
	struct passwd *result;
	char buffer[512];

	while (1)
	{
		status = getpwent_r(&entry, buffer, 512, &result);
		ASSERT_EQ(status, 0);
		if (result == NULL)
		{
			break;
		}

		printf("User\n");
		printf("pw_name  : %s\n", entry.pw_name);
		printf("pw_uid   : %d\n", entry.pw_uid);
		printf("pw_gid   : %d\n", entry.pw_gid);
		printf("pw_gecos : %s\n", entry.pw_gecos);
		printf("pw_dir   : %s\n", entry.pw_dir);
		printf("\n");
	}

	setpwent();

	// Small buffer error
	status = getpwent_r(&entry, buffer, 16, &result);
	ASSERT_NULL(result);
	ASSERT_EQ(status, ERANGE);

	endpwent();

	return 0;
}

int test_getpwnam()
{
	int status;
	struct passwd *entry, *pwnam_entry, *pwnam_r_result, pwnam_r_entry;
	char buffer[512];
	const char *not_an_user = "notanuser";

	while (1)
	{
		entry = getpwent();
		if (entry == NULL)
		{
			break;
		}

		// getpwnam
		pwnam_entry = getpwnam(entry->pw_name);
		if (pwnam_entry == NULL)
		{
			printf("Search for user '%s' failed\n", entry->pw_name);
			return -1;
		}

		ASSERT_STREQ(pwnam_entry->pw_name, entry->pw_name);
		ASSERT_EQ(pwnam_entry->pw_uid, entry->pw_uid);
		ASSERT_EQ(pwnam_entry->pw_gid, entry->pw_gid);
		if (entry->pw_dir != NULL)
		{
			ASSERT_STREQ(pwnam_entry->pw_dir, entry->pw_dir);
		}

		// getpwnam_r
		status = getpwnam_r(entry->pw_name, &pwnam_r_entry, buffer, 512, &pwnam_r_result);
		ASSERT_EQ(status, 0);
		if (pwnam_r_result == NULL)
		{
			printf("Search for user '%s' failed\n", entry->pw_name);
			return -1;
		}

		ASSERT_STREQ(pwnam_r_entry.pw_name, entry->pw_name);
		ASSERT_EQ(pwnam_r_entry.pw_uid, entry->pw_uid);
		ASSERT_EQ(pwnam_r_entry.pw_gid, entry->pw_gid);
		if (entry->pw_dir != NULL)
		{
			ASSERT_STREQ(pwnam_r_entry.pw_dir, entry->pw_dir);
		}
	}

	endpwent();

	// Non existent users
	errno = 0;
	pwnam_entry = getpwnam(not_an_user);
	ASSERT_NULL(pwnam_entry);
	ASSERT_ERRNO(ENOENT);

	status = getpwnam_r(not_an_user, &pwnam_r_entry, buffer, 512, &pwnam_r_result);
	ASSERT_NULL(pwnam_r_result);
	ASSERT_EQ(status, ENOENT);

	return 0;
}

int test_getpwuid()
{
	int status;
	struct passwd *entry, *pwuid_entry, *pwuid_r_result, pwuid_r_entry;
	char buffer[512];
	uid_t non_existent_uid = 1;

	while (1)
	{
		entry = getpwent();
		if (entry == NULL)
		{
			break;
		}

		// getpwnam
		pwuid_entry = getpwuid(entry->pw_uid);
		if (pwuid_entry == NULL)
		{
			printf("Search for user '%s' failed\n", entry->pw_name);
			return -1;
		}

		ASSERT_STREQ(pwuid_entry->pw_name, entry->pw_name);
		ASSERT_EQ(pwuid_entry->pw_uid, entry->pw_uid);
		ASSERT_EQ(pwuid_entry->pw_gid, entry->pw_gid);
		if (entry->pw_dir != NULL)
		{
			ASSERT_STREQ(pwuid_entry->pw_dir, entry->pw_dir);
		}

		// getpwnam_r
		status = getpwuid_r(entry->pw_uid, &pwuid_r_entry, buffer, 512, &pwuid_r_result);
		ASSERT_EQ(status, 0);
		if (pwuid_r_result == NULL)
		{
			printf("Search for user '%s' failed\n", entry->pw_name);
			return -1;
		}

		ASSERT_STREQ(pwuid_r_entry.pw_name, entry->pw_name);
		ASSERT_EQ(pwuid_r_entry.pw_uid, entry->pw_uid);
		ASSERT_EQ(pwuid_r_entry.pw_gid, entry->pw_gid);
		if (entry->pw_dir != NULL)
		{
			ASSERT_STREQ(pwuid_r_entry.pw_dir, entry->pw_dir);
		}
	}

	endpwent();

	// Non existent users
	errno = 0;
	pwuid_entry = getpwuid(non_existent_uid);
	ASSERT_NULL(pwuid_entry);
	ASSERT_ERRNO(ENOENT);

	status = getpwuid_r(non_existent_uid, &pwuid_r_entry, buffer, 512, &pwuid_r_result);
	ASSERT_NULL(pwuid_r_result);
	ASSERT_EQ(status, ENOENT);

	return 0;
}

int test_root()
{
	struct passwd *entry_id = NULL, *entry_name = NULL;

	entry_id = getpwuid(ROOT_UID);
	ASSERT_NOTNULL(entry_id)

	entry_name = getpwnam("root");
	ASSERT_NOTNULL(entry_id)

	ASSERT_STREQ(entry_id->pw_name, entry_name->pw_name);
	ASSERT_EQ(entry_id->pw_uid, entry_name->pw_uid);
	ASSERT_EQ(entry_id->pw_gid, entry_name->pw_gid);
	ASSERT_STREQ(entry_id->pw_dir, entry_name->pw_dir);
	ASSERT_STREQ(entry_id->pw_gecos, entry_name->pw_gecos);
	ASSERT_STREQ(entry_id->pw_shell, entry_name->pw_shell);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_getpwent());
	TEST(test_getpwent_r());
	TEST(test_getpwnam())
	TEST(test_getpwuid())
	TEST(test_root())

	VERIFY_RESULT_AND_EXIT();
}
