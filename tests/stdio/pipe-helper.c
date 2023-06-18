/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		return 1; // Failed
	}

	int i = atoi(argv[1]);
	char buf[16];
	switch (i)
	{
	case 1:
		// test_read
		write(STDOUT_FILENO, "hello", 5);
		break;
	case 2:
		// test_write
		read(STDIN_FILENO, buf, 5);
		buf[5] = '\0';
		if (strcmp(buf, "hello"))
		{
			write(STDERR_FILENO, "write failed", 12);
			return 1; // Failed
		}
		break;
	case 3:
		// test_read_cr
		write(STDOUT_FILENO, "hello\r\nworld", 12);
		break;
	case 4:
		// test_write_cr
		read(STDIN_FILENO, buf, 12);
		buf[12] = '\0';
		if (strcmp(buf, "hello\r\nworld"))
		{
			write(STDERR_FILENO, "write_cr failed", 15);
			return 1; // Failed
		}
		break;
	default:
		return 1;
	}
	return 0;
}
