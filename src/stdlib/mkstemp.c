/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#define _CRT_RAND_S

#include <internal/fcntl.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>

static bool validate_template(const char *template, int suffixlen, int *length)
{
	if (suffixlen < 0)
	{
		return false;
	}

	*length = (int)strlen(template);

	if (suffixlen + 6 > *length)
	{
		return false;
	}

	for (int i = 0; i < 6; ++i)
	{
		if (template[*length - suffixlen - 1 - i] != 'X')
		{
			return false;
		}
	}

	return true;
}

static void gen_tempstring(char *start, size_t length)
{
	char buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	unsigned int rn;

	rand_s(&rn);
	_ultoa_s(rn, buffer, 8, 36);

	for (size_t i = 0; i < length; ++i)
	{
		if (start[i] == 'X')
		{
			if (buffer[i] != '\0')
			{
				start[i] = buffer[i];
			}
			else
			{
				// convert 'X' -> 'x'
				start[i] |= 32;
			}
		}
	}
}

#define CREATE_FILE 1
#define CREATE_DIR  2
#define TRY_NAME    3

#define MAX_TRIES 32

int wlibc_common_mkstemp(char *template, int suffixlen, int flags, int operation)
{
	int status = -1; // also fd
	int length = 0;
	int try_count = 0;
	struct stat statbuf;

	VALIDATE_PATH(template, EINVAL, -1);

	if (operation < 1 || operation > 3)
	{
		// invalid usage
		errno = EINVAL;
		return -1;
	}

	if (!validate_template(template, suffixlen, &length))
	{
		errno = EINVAL;
		return -1;
	}

	switch (operation)
	{
	case CREATE_FILE:
	{
		while (status == -1 && try_count < MAX_TRIES)
		{
			++try_count;
			gen_tempstring(template + length - suffixlen - 6, 6);
			// flags still need to be validated so call openat for safety
			status = openat(AT_FDCWD, template, O_CREAT | O_EXCL | O_RDWR | flags, 0600);
		}
	}
	break;

	case CREATE_DIR:
	{
		while (status == -1 && try_count < MAX_TRIES)
		{
			++try_count;
			gen_tempstring(template + length - 6, 6);
			status = wlibc_common_mkdir(AT_FDCWD, template, 0700);
		}
	}
	break;

	case TRY_NAME:
	{
		while (try_count < MAX_TRIES)
		{
			++try_count;
			gen_tempstring(template + length - 6, 6);
			status = wlibc_common_stat(AT_FDCWD, template, &statbuf, AT_SYMLINK_NOFOLLOW);
			if (status == -1 && errno == ENOENT)
			{
				// clear errno
				errno = 0;
				// set as success so mktemp can return the template
				status = 0;
				break;
			}
		}
	}
	break;

	default:
		return -1; // unreachable
	}

	// clear any errno set
	errno = 0;
	return status;
}
