/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/fcntl.h>
#include <internal/stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

FILE *wlibc_fdopen(int fd, const char *mode)
{
	fdinfo info;
	get_fdinfo(fd, &info);

	if (info.type == DIRECTORY_HANDLE || info.type == INVALID_HANDLE)
	{
		errno = (info.type == INVALID_HANDLE ? EBADF : EISDIR);
		return NULL;
	}

	int std_flags = parse_mode(mode);
	int fd_flags = info.flags;

	int important_fd_flags = fd_flags & (O_APPEND | O_ACCMODE);
	int important_std_flags = std_flags & (O_APPEND | O_ACCMODE);

	if ((important_fd_flags & O_APPEND) != (important_std_flags & O_APPEND))
	{
		errno = EINVAL;
		return NULL;
	}

	// Underlying fd was not opened for writing but the stream is requesting write access
	if (important_std_flags & O_ACCMODE)
	{
		if ((important_fd_flags & O_ACCMODE) == 0)
		{
			errno = EINVAL;
			return NULL;
		}
	}

	FILE *stream = create_stream(fd, _IOBUFFER_INTERNAL | _IOFBF | get_buf_mode(fd_flags), 512);
	return stream;
}
