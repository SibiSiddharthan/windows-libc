/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <errno.h>
#include <unistd.h>
#include <internal/error.h>
#include <Windows.h>
#include <fcntl.h>
#include <internal/fcntl.h>

int do_dup(int oldfd, int newfd, int flags)
{
	NTSTATUS status;
	HANDLE source = get_fd_handle(oldfd);
	HANDLE target;

	if (flags == O_CLOEXEC)
	{
		// The new handle should not be inheritable.
		status = NtDuplicateObject(NtCurrentProcess(), source, NtCurrentProcess(), &target, 0, OBJ_CASE_INSENSITIVE, DUPLICATE_SAME_ACCESS);
	}
	else
	{
		status = NtDuplicateObject(NtCurrentProcess(), source, NtCurrentProcess(), &target, 0, 0,
								   DUPLICATE_SAME_ACCESS | DUPLICATE_SAME_ATTRIBUTES);
	}

	// For dup
	if (newfd == -1)
	{
		return register_to_fd_table(target, get_fd_type(oldfd), get_fd_flags(oldfd));
	}

	// For dup2, dup3
	else
	{
		// newfd exists in the table
		if (validate_fd(newfd))
		{
			status = (NTSTATUS)close_fd(newfd);
			if (status == -1)
			{
				return -1;
			}
		}

		insert_into_fd_table(newfd, target, get_fd_type(oldfd), get_fd_flags(oldfd) | flags);

		return newfd;
	}
}

int wlibc_common_dup(int oldfd, int newfd, int flags)
{
	if (!validate_fd(oldfd))
	{
		errno = EBADF;
		return -1;
	}

	if (flags != 0 && flags != O_CLOEXEC)
	{
		errno = EINVAL;
		return -1;
	}

	// This will be only be reachable in the case of dup2.
	if (newfd == oldfd)
	{
		return newfd;
	}

	return do_dup(oldfd, newfd, flags);
}
