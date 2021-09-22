/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/stdio.h>
#include <internal/fcntl.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

int parse_mode(const char *mode);
int get_buf_mode(int flags);

FILE *wlibc_fdopen(int fd, const char *mode)
{
	enum handle_type _type = get_fd_type(fd);
	if (_type == DIRECTORY_HANDLE || _type == INVALID_HANDLE)
	{
		errno = (_type == INVALID_HANDLE ? EBADF : EISDIR);
		return NULL;
	}

	int std_flags = parse_mode(mode);
	int fd_flags = get_fd_flags(fd);

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
