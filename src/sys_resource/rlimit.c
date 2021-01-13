/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <sys/resource.h>
#include <errno.h>
#include <Windows.h>
#include <stdio.h>

const rlim_t max_files = 8192;

const rlim_t max_file_size = 17592186044416; // 16 TB
rlim_t soft_file_size = 17592186044416;      // Prevent C2099 MSVC

const rlim_t max_as = 140737488355328; // 128 TB with IMAGE_FILE_LARGE_ADDRESS_AWARE(default)
rlim_t soft_as = 140737488355328;      // Prevent C2099 MSVC

int wlibc_getrlimit(int resource, struct rlimit *rlim)
{
	switch (resource)
	{
	case RLIMIT_CORE:
	case RLIMIT_CPU:
	case RLIMIT_DATA:
		rlim->rlim_max = RLIM_INFINITY;
		rlim->rlim_cur = RLIM_INFINITY;
		break;
	case RLIMIT_STACK:
		ULONG_PTR low, high;
		GetCurrentThreadStackLimits(&low, &high);
		rlim->rlim_max = high - low;
		rlim->rlim_cur = rlim->rlim_max;
		break;
	case RLIMIT_AS:
		rlim->rlim_cur = soft_as;
		rlim->rlim_max = max_as;
		break;
	case RLIMIT_FSIZE:
		rlim->rlim_cur = soft_file_size;
		rlim->rlim_max = max_file_size;
		break;
	case RLIMIT_NOFILE:
		rlim->rlim_cur = _getmaxstdio();
		rlim->rlim_max = max_files;
		break;
	default:
		errno = EINVAL;
		return -1;
	}

	return 0;
}

int wlibc_setrlimit(int resource, const struct rlimit *rlim)
{
	switch (resource)
	{
	case RLIMIT_CORE:
	case RLIMIT_CPU:
	case RLIMIT_DATA:
	case RLIMIT_STACK:
	case RLIMIT_AS:
	case RLIMIT_FSIZE:
		break;
	case RLIMIT_NOFILE:
		if (rlim->rlim_cur > max_files || rlim->rlim_max > max_files)
		{
			errno = EINVAL;
			return -1;
		}
		_setmaxstdio(rlim->rlim_cur);
		break;
	default:
		errno = EINVAL;
		return -1;
	}

	return 0;
}
