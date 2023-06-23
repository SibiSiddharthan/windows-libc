/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <sys/stat.h>
#include <sys/time.h>

int wlibc_common_utimes(int dirfd, const char *path, const struct timeval times[2], int flags)
{
	struct timespec timespec[2];

	if (times == NULL)
	{
		return wlibc_common_utimens(dirfd, path, NULL, flags);
	}

	timespec[0].tv_sec = times[0].tv_sec;
	timespec[0].tv_nsec = times[0].tv_usec * 1000;

	timespec[1].tv_sec = times[1].tv_sec;
	timespec[1].tv_sec = times[1].tv_usec * 1000;

	return wlibc_common_utimens(dirfd, path, timespec, flags);
}
