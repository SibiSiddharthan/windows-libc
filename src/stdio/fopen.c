/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <internal/misc.h>
#include <errno.h>
#include <internal/stdio.h>
#include <wchar.h>
#include <fcntl.h>

int parse_mode(const char *mode);
int get_buf_mode(int flags);
int do_open(int dirfd, const char *name, int oflags, mode_t perm);

FILE *wlibc_fopen(const char *restrict name, const char *restrict mode)
{
	if (name == NULL)
	{
		errno = ENOENT;
		return NULL;
	}

	if (mode == NULL)
	{
		errno = EINVAL;
		return NULL;
	}

	int flags = parse_mode(mode);

#if 0
	wchar_t *wname = NULL;
	// This conversion should be moved inside common_open itself. TODO/FIXME
	if (strcmp(name, "/dev/null") == 0)
	{
		wname = L"NUL";
	}
	else
	{
		wname = mb_to_wc(name);
	}

	int fd = common_open(wname, flags, 0);

	if (strcmp(name, "/dev/null") != 0)
	{
		free(wname);
	}
#endif
	int fd = do_open(AT_FDCWD, name, flags | O_NOTDIR , 0700);

	if (fd == -1)
	{
		return NULL;
	}

	FILE *stream = create_stream(fd, _IOBUFFER_INTERNAL | _IOFBF | get_buf_mode(flags), 512);
	return stream;
}
