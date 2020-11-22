/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <fcntl.h>
#include <string.h>

int parse_mode(const char *mode)
{
	int flags = 0;
	int length = strlen(mode);
	for (int i = 0; i < length; i++)
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
			flags |= O_RDWR;
			break;
		case 'a':
			flags |= O_APPEND;
			break;
		case 'b':
			flags |= O_BINARY;
			break;
		case 't':
			flags |= O_TEXT;
			break;
		default:
			break;
		}
	}

	return flags;
}
