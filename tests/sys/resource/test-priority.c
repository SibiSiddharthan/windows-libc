/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <tests/test.h>
#include <stdbool.h>
#include <sys/resource.h>

static bool have_increase_base_priority_privilege = false;

static void check_for_increase_base_priority_privilege()
{
	NTSTATUS status;
	ULONG privilege = SE_INC_BASE_PRIORITY_PRIVILEGE;
	PVOID state;

	status = RtlAcquirePrivilege(&privilege, 1, 0, &state);
	if (status == STATUS_SUCCESS)
	{
		have_increase_base_priority_privilege = true;
		RtlReleasePrivilege(state);
	}
}

int test_priority()
{
	int status;

	for (int i = 1; i <= PRIO_MAX; ++i)
	{
		status = setpriority(PRIO_PROCESS, 0, i);
		ASSERT_EQ(status, 0);

		status = getpriority(PRIO_PROCESS, 0);
		// The status might be negative, make sure errno is 0.
		ASSERT_ERRNO(0);

		if (have_increase_base_priority_privilege)
		{
			ASSERT_EQ(status, i);
		}
	}

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();
	check_for_increase_base_priority_privilege();
	TEST(test_priority());
	VERIFY_RESULT_AND_EXIT();
}
