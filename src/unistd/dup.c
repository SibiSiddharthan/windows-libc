/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <errno.h>
#include <unistd.h>
#include <wlibc_errors.h>
#include <Windows.h>
#include <fcntl.h>
#include <fcntl_internal.h>

int common_dup(int oldfd, int newfd, int flags)
{
	HANDLE process = GetCurrentProcess();
	HANDLE source = get_fd_handle(oldfd);
	HANDLE target;

	if (!DuplicateHandle(process, source, process, &target, 0, TRUE, DUPLICATE_SAME_ACCESS))
	{
		map_win32_error_to_wlibc(GetLastError());
		return -1;
	}

	// For dup
	if (newfd == -1)
	{
		int fd = register_to_fd_table(target, get_fd_path(oldfd), get_fd_type(oldfd), get_fd_flags(oldfd));
		return fd;
	}

	// For dup2,3
	else
	{
		// newfd exists in the table
		if (validate_fd(newfd))
		{
			int status = close_fd(newfd);
			if (status == -1)
			{
				return -1;
			}
		}

		insert_into_fd_table(newfd, target, get_fd_path(oldfd), get_fd_type(oldfd), get_fd_flags(oldfd) | flags);

		return newfd;
	}
}

int wlibc_dup(int fd)
{
	if (!validate_active_ffd(fd))
	{
		return -1;
	}

	return common_dup(fd, -1, 0);
}

int wlibc_dup2(int oldfd, int newfd)
{
	if (!validate_active_ffd(oldfd))
	{
		return -1;
	}

	if (newfd < 0)
	{
		errno = EINVAL;
		return -1;
	}

	if (newfd == oldfd)
	{
		return newfd;
	}

	return common_dup(oldfd, newfd, 0);
}

int wlibc_dup3(int oldfd, int newfd, int flags)
{
	if (!validate_active_ffd(oldfd))
	{
		return -1;
	}

	if (flags != 0 && flags != O_CLOEXEC)
	{
		errno = EINVAL;
		return -1;
	}

	if (newfd < 0)
	{
		errno = EINVAL;
		return -1;
	}

	if (newfd == oldfd)
	{
		errno = EINVAL;
		return -1;
	}

	return common_dup(oldfd, newfd, flags);
}
