/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <fcntl.h>
#include <fcntl_internal.h>
#include <errno.h>
#include <stdarg.h>

int common_dup(int oldfd, int newfd, int flags);

static int do_dupfd(int old_fd, int new_fd, int o_cloexec)
{
	// Find nearest available file descriptor to new_fd which is >= newfd
	while (validate_fd(new_fd))
	{
		++new_fd;
	}

	return common_dup(old_fd, new_fd, o_cloexec);
}

int wlibc_fcntl(int fd, int cmd, va_list args)
{
	if (!validate_fd(fd))
	{
		return -1;
	}

	switch (cmd)
	{
	case F_DUPFD:
		return do_dupfd(fd, va_arg(args, int), 0);
	case F_DUPFD_CLOEXEC:
		return do_dupfd(fd, va_arg(args, int), O_CLOEXEC);
	case F_GETFD:
		return (get_fd_flags(fd) & O_CLOEXEC);
	case F_SETFD:
		int f_setfd_flags = (va_arg(args, int) & O_CLOEXEC);
		add_fd_flags(fd, f_setfd_flags);
		return 0;
	case F_GETFL:
		return get_fd_flags(fd);
	case F_SETFL:
		int f_setfl_flags = (va_arg(args, int) & (O_APPEND | O_ASYNC | O_DIRECT | O_NOATIME | O_NONBLOCK));
		// only O_APPEND works for now.
		// O_DIRECT and O_NONBLOCK require reopening the file. TODO
		add_fd_flags(fd, f_setfl_flags);
		return 0;
	default:
		errno = EINVAL;
		return -1;
	}

	return 0;
}
