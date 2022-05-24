/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>

// From stat.c
int do_stat(HANDLE handle, struct stat *restrict statbuf);

static struct statx_timestamp timespec_to_timestamp(struct timespec *restrict st_time)
{
	struct statx_timestamp stx_time;
	stx_time.tv_sec = st_time->tv_sec;
	stx_time.tv_nsec = st_time->tv_nsec;
	return stx_time;
}

int do_statx(HANDLE handle, unsigned int mask, struct statx *restrict statxbuf)
{
	int result;
	struct stat statbuf;

	UNREFERENCED_PARAMETER(mask);

	memset(statxbuf, 0, sizeof(struct statx));
	result = do_stat(handle, &statbuf);

	// For now just call do_stat and call it a day. MAYBE TODO
	if (result == 0)
	{
		statxbuf->stx_attributes = 0;
		statxbuf->stx_nlink = statbuf.st_nlink;
		statxbuf->stx_uid = statbuf.st_uid;
		statxbuf->stx_gid = statbuf.st_gid;
		statxbuf->stx_mode = (uint16_t)statbuf.st_mode;
		statxbuf->stx_ino = statbuf.st_ino;
		statxbuf->stx_size = statbuf.st_size;
		statxbuf->stx_blocks = statbuf.st_blocks;
		statxbuf->stx_blksize = statbuf.st_blksize;
		statxbuf->stx_attributes = statbuf.st_attributes;
		statxbuf->stx_atime = timespec_to_timestamp(&statbuf.st_atim);
		statxbuf->stx_mtime = timespec_to_timestamp(&statbuf.st_mtim);
		statxbuf->stx_ctime = timespec_to_timestamp(&statbuf.st_ctim);
		statxbuf->stx_btime = timespec_to_timestamp(&statbuf.st_birthtim);
		statxbuf->stx_rdev_minor = statbuf.st_rdev;
		statxbuf->stx_dev_minor = statbuf.st_dev;
		statxbuf->stx_rdev_major = 0;
		statxbuf->stx_dev_major = 0;
		statxbuf->stx_mask = STATX_ALL;
	}

	return result;
}

int common_statx(int dirfd, const char *restrict path, int flags, unsigned int mask, struct statx *restrict statxbuf)
{
	int result;

	HANDLE handle = just_open(dirfd, path, FILE_READ_ATTRIBUTES | READ_CONTROL, flags == AT_SYMLINK_NOFOLLOW ? FILE_OPEN_REPARSE_POINT : 0);
	if (handle == INVALID_HANDLE_VALUE)
	{
		// errno will be set by just_open
		return -1;
	}

	result = do_statx(handle, mask, statxbuf);

	NtClose(handle);
	return result;
}

int wlibc_statx(int dirfd, const char *restrict path, int flags, unsigned int mask, struct statx *restrict statxbuf)
{
	if (statxbuf == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	if (flags != 0 && flags != AT_SYMLINK_NOFOLLOW && flags != AT_EMPTY_PATH && flags != AT_STATX_SYNC_AS_STAT)
	{
		errno = EINVAL;
		return -1;
	}

	if (mask > STATX_ALL)
	{
		errno = EINVAL;
		return -1;
	}

	if (flags != AT_EMPTY_PATH)
	{
		VALIDATE_PATH_AND_DIRFD(path, dirfd);
	}
	else
	{
		if (!validate_fd(dirfd))
		{
			errno = EBADF;
			return -1;
		}

		return do_statx(get_fd_handle(dirfd), mask, statxbuf);
	}

	return common_statx(dirfd, path, flags, mask, statxbuf);
}
