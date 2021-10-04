/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <fcntl.h>
#include <string.h>
#include <internal/stdio.h>

int parse_mode(const char *mode)
{
	int flags = 0;
	for (int i = 0; mode[i] != '\0'; i++)
	{
		switch (mode[i])
		{
		case 'r':
			flags |= O_RDONLY;
			break;
		case 'w':
			flags |= O_WRONLY | O_CREAT | O_TRUNC;
			break;
		case '+':
			flags &= ~O_WRONLY; // remove O_WRONLY as O_RDONLY is 0
			flags |= O_RDWR;
			break;
		case 'a':
			flags |= O_APPEND | O_CREAT;
			break;
		// GNU Extenstions
		case 'x':
			flags |= O_EXCL;
			break;
		case 'm':
			// flags // mmap TODO
			break;
		// Microsoft Extensions
		case 'b':
			flags |= O_BINARY;
			break;
		case 't':
			flags |= O_TEXT;
			break;
		case 'c':
			flags |= O_SYNC;
			break;
		case 'N':
		case 'e': // glibc O_CLOEXEC
			flags |= O_NOINHERIT;
			break;
		case 'S':
			flags |= O_SEQUENTIAL;
			break;
		case 'R':
			flags |= O_RANDOM;
			break;
		case 'T': // fallthrough
		case 'D':
			flags |= O_SHORT_LIVED;
			break;
		default:
			break;
		}
	}

	return flags;
}

int get_buf_mode(int flags)
{
	if (flags & O_RDWR)
	{
		return _IOBUFFER_RDWR;
	}
	else if (flags & O_WRONLY)
	{
		return _IOBUFFER_WRONLY;
	}
	else //(flags & O_RDONLY)
	{
		return _IOBUFFER_RDONLY;
	}
}
