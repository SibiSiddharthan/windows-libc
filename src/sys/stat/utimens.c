/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

LARGE_INTEGER timespec_to_LARGE_INTEGER(const struct timespec time)
{
	LARGE_INTEGER L;
	L.QuadPart = time.tv_sec * 10000000 + time.tv_nsec / 100;
	L.QuadPart += 116444736000000000LL;
	return L;
}

int do_utimens(HANDLE handle, const struct timespec times[2])
{
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	FILE_BASIC_INFORMATION info;
	LARGE_INTEGER current_time;

	status = NtQueryInformationFile(handle, &io, &info, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	if (times == NULL || times[0].tv_nsec == UTIME_NOW || times[1].tv_nsec == UTIME_NOW)
	{
		// Only fetch the current time if this condition is satisfied.
		// This functions operates in user space only (i.e no syscall)
		GetSystemTimeAsFileTime((FILETIME *)&current_time);
	}

	if (times == NULL)
	{
		info.LastAccessTime.QuadPart = current_time.QuadPart;
		info.LastWriteTime.QuadPart = current_time.QuadPart;
	}
	else
	{
		if (times[0].tv_nsec == UTIME_NOW)
		{
			info.LastAccessTime.QuadPart = current_time.QuadPart;
		}
		else if (times[0].tv_nsec != UTIME_OMIT)
		{
			info.LastAccessTime.QuadPart = timespec_to_LARGE_INTEGER(times[0]).QuadPart;
		}

		if (times[1].tv_nsec == UTIME_NOW)
		{
			info.LastWriteTime.QuadPart = current_time.QuadPart;
		}
		else if (times[1].tv_nsec != UTIME_OMIT)
		{
			info.LastWriteTime.QuadPart = timespec_to_LARGE_INTEGER(times[1]).QuadPart;
		}
	}

	status = NtSetInformationFile(handle, &io, &info, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}

int common_utimens(int dirfd, const char *path, const struct timespec times[2], int flags)
{
	HANDLE handle = just_open(dirfd, path, FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES, flags == AT_SYMLINK_NOFOLLOW ? FILE_OPEN_REPARSE_POINT : 0);
	if (handle == INVALID_HANDLE_VALUE)
	{
		// errno wil be set by just_open
		return -1;
	}

	int result = do_utimens(handle, times);
	NtClose(handle);

	return result;
}

int wlibc_common_utimens(int dirfd, const char *path, const struct timespec times[2], int flags)
{
	if (flags != 0 && flags != AT_EMPTY_PATH && flags != AT_SYMLINK_NOFOLLOW)
	{
		errno = EINVAL;
		return -1;
	}

	if (flags != AT_EMPTY_PATH)
	{
		VALIDATE_PATH_AND_DIRFD(path, dirfd);
		return common_utimens(dirfd, path, times, flags);
	}
	else
	{
		if (!validate_fd(dirfd))
		{
			errno = EBADF;
			return -1;
		}

		return do_utimens(get_fd_handle(dirfd), times);
	}
}
