/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <errno.h>
#include <sys/times.h>

#define TO_CLOCK_T(number) ((clock_t)(number) / (10000000 / CLOCKS_PER_SEC))

clock_t wlibc_times(struct tms *tmsbuf)
{
	NTSTATUS status;
	KERNEL_USER_TIMES user_times;
	SYSTEM_TIMEOFDAY_INFORMATION time_info;

	if (tmsbuf == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	status = NtQueryInformationProcess(NtCurrentProcess(), ProcessTimes, &user_times, sizeof(KERNEL_USER_TIMES), NULL);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	// Ignore these fields.
	tmsbuf->tms_cstime = 0;
	tmsbuf->tms_cutime = 0;

	tmsbuf->tms_stime = TO_CLOCK_T(user_times.KernelTime.QuadPart);
	tmsbuf->tms_utime = TO_CLOCK_T(user_times.UserTime.QuadPart);

	status = NtQuerySystemInformation(SystemTimeOfDayInformation, &time_info, sizeof(time_info), NULL);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return TO_CLOCK_T(time_info.CurrentTime.QuadPart - time_info.BootTime.QuadPart);
}
