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

int do_open(int dirfd, const char *name, int oflags, mode_t perm);

FILE *wlibc_fopen(const char *restrict name, const char *restrict mode)
{
	VALIDATE_PATH(name, ENOENT, NULL);

	if (mode == NULL)
	{
		errno = EINVAL;
		return NULL;
	}

	int flags = parse_mode(mode);
	int fd = do_open(AT_FDCWD, name, flags | O_NOTDIR, 0700);

	if (fd == -1)
	{
		return NULL;
	}

	FILE *stream = create_stream(fd, _IOBUFFER_INTERNAL | _IOFBF | get_buf_mode(flags), 512);
	return stream;
}
