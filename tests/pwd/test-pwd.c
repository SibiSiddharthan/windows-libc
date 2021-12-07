/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <test-macros.h>
#include <errno.h>
#include <pwd.h>

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

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_getpwent());
	TEST(test_getpwent_r());

	VERIFY_RESULT_AND_EXIT();
}
