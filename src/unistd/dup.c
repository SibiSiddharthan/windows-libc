/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <errno.h>
#include <unistd.h>

// Make the first parameter as fdinfo. TODO
int do_dup(int oldfd, int newfd, int flags)
{
	NTSTATUS status;
	HANDLE source;
	HANDLE target;
	fdinfo oldfd_info;

	get_fdinfo(oldfd, &oldfd_info);

	source = oldfd_info.handle;

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

	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	// For dup
	if (newfd == -1)
	{
		return register_to_fd_table(target, oldfd_info.type, oldfd_info.flags);
	}

	// For dup2, dup3
	else
	{
		// TODO do this entire block atomically.
		// newfd exists in the table
		if (validate_fd(newfd))
		{
			status = (NTSTATUS)close_fd(newfd);
			if (status == -1)
			{
				return -1;
			}
		}

		return insert_into_fd_table(newfd, target, oldfd_info.type, oldfd_info.flags | flags);
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
