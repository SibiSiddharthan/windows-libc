/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <tests/test.h>
#include <stdbool.h>
#include <sys/resource.h>
#include <unistd.h>

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

int test_nice()
{
	int status;
	int initial_nice = getpriority(PRIO_PROCESS, 0);

	for (int i = 0; i <= 2; ++i)
	{
		status = nice(i);
		ASSERT_EQ(status, 0);

		initial_nice += i;

		status = getpriority(PRIO_PROCESS, 0);
		ASSERT_EQ(status, initial_nice);
	}

	// We are decreasing the nice value here. We require 'SE_INC_BASE_PRIORITY_PRIVILEGE'.
	for (int i = -2; i <= 0; ++i)
	{
		errno = 0;
		status = nice(i);
		if (errno == EPERM)
		{
			ASSERT_EQ(status, -1);
			ASSERT_EQ(have_increase_base_priority_privilege, false);

			status = getpriority(PRIO_PROCESS, 0);
			ASSERT_EQ(status, initial_nice);
		}
		else
		{
			ASSERT_EQ(status, 0);
			ASSERT_EQ(have_increase_base_priority_privilege, true);

			initial_nice += i;

			status = getpriority(PRIO_PROCESS, 0);
			ASSERT_EQ(status, initial_nice);
		}
	}

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();
	check_for_increase_base_priority_privilege();
	TEST(test_nice());
	VERIFY_RESULT_AND_EXIT();
}
