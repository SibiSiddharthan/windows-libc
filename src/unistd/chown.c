/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/fcntl.h>
#include <errno.h>
#include <unistd.h>

int do_chown(HANDLE handle, uid_t owner, gid_t group)
{
	// TODO
	errno = ENOTSUP;
	return 0;
}

int common_chown(int dirfd, const char *path, uid_t owner, gid_t group, int flags)
{
	// This requires 'SeTakeOwnershipPrivilege'.
	HANDLE handle = just_open(dirfd, path, READ_CONTROL | WRITE_OWNER, flags == AT_SYMLINK_NOFOLLOW ? FILE_OPEN_REPARSE_POINT : 0);
	if (handle == INVALID_HANDLE_VALUE)
	{
		// errno will be set by just_open
		return -1;
	}

	int result = do_chown(handle, owner, group);
	NtClose(handle);
	return result;
}

int wlibc_common_chown(int dirfd, const char *path, uid_t owner, gid_t group, int flags)
{
	if (owner < 0 || group < 0)
	{
		errno = EINVAL;
		return -1;
	}

	if (flags != 0 && flags != AT_SYMLINK_NOFOLLOW && flags != AT_EMPTY_PATH)
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

		// fds opened by 'open' don't have WRITE_OWNER privelege, need to reopen file. TODO
		return do_chown(get_fd_handle(dirfd), owner, group);
	}

	return common_chown(dirfd, path, owner, group, flags);
}
