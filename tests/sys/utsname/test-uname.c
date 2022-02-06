/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <test-macros.h>
#include <sys/utsname.h>

// Not really a test, just see if it works
int test_uname()
{
	struct utsname name;
	int status;

	status = uname(&name);
	ASSERT_EQ(status, 0);

	printf("sysname    : %s\n", name.sysname);
	printf("version    : %s\n", name.version);
	printf("release    : %s\n", name.release);
	printf("machine    : %s\n", name.machine);
	printf("nodename   : %s\n", name.nodename);
	printf("domainname : %s\n", name.domainname);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();
	TEST(test_uname());
	VERIFY_RESULT_AND_EXIT()
}
