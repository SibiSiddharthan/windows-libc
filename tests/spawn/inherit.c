/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdlib.h>
#include <string.h>

#ifdef TESTING_MSVCRT
#	define TEST_CONTENT "Hello World From mscvrt!!!"
#	define read         _read
#	define write        _write

typedef int ssize_t;
_ACRTIMP int __cdecl _read(int fd, void *bufffer, unsigned int count);
_ACRTIMP int __cdecl _write(int fd, void const *bufffer, unsigned int count);
#	define WRITE_CONTENT "#####"

#else // TESTING_WLIBC
#	include <unistd.h>
#	define TEST_CONTENT  "Hello World From wlibc!!!"
#	define WRITE_CONTENT "@@@@@"
#endif

#define SIZEOF_TEST_CONTENT  (sizeof(TEST_CONTENT) - 1)
#define SIZEOF_WRITE_CONTENT (sizeof(WRITE_CONTENT) - 1)

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		return 1;
	}

	// We are only going inherit fd < 10. This makes writing a simple stoi.
	int number_of_fds = (int)(argv[1][0] - '0');
	if (number_of_fds == 5) // Files.
	{
		int fd_r = (int)(argv[2][0] - '0');
		int fd_w = (int)(argv[3][0] - '0');
		int fd_a = (int)(argv[4][0] - '0');
		int fd_rw = (int)(argv[5][0] - '0');
		int fd_ra = (int)(argv[6][0] - '0');

		char buf[64];
		ssize_t length;

		// read
		length = read(fd_r, buf, 64);
		if (length != SIZEOF_TEST_CONTENT)
		{
			return 2;
		}

		if (memcmp(buf, TEST_CONTENT, SIZEOF_TEST_CONTENT) != 0)
		{
			return 3;
		}

		// write
		length = write(fd_w, WRITE_CONTENT, SIZEOF_WRITE_CONTENT);
		if (length != 5)
		{
			return 4;
		}

		// append
		length = write(fd_a, WRITE_CONTENT, SIZEOF_WRITE_CONTENT);
		if (length != 5)
		{
			return 5;
		}

		// read-write
		length = read(fd_rw, buf, 5);
		if (length != 5)
		{
			return 6;
		}

		if (memcmp(buf, "Hello", length) != 0)
		{
			return 7;
		}

		length = write(fd_rw, WRITE_CONTENT, SIZEOF_WRITE_CONTENT);
		if (length != SIZEOF_WRITE_CONTENT)
		{
			return 8;
		}

		// read-append
		length = read(fd_ra, buf, 5);
		if (length != SIZEOF_WRITE_CONTENT)
		{
			return 9;
		}

		if (memcmp(buf, "Hello", length) != 0)
		{
			return 10;
		}

		length = write(fd_ra, WRITE_CONTENT, SIZEOF_WRITE_CONTENT);
		if (length != SIZEOF_WRITE_CONTENT)
		{
			return 11;
		}
	}
	if (number_of_fds == 2) // Pipes.
	{
		int fd_r = (int)(argv[2][0] - '0');
		int fd_w = (int)(argv[3][0] - '0');

		char buf[64];
		ssize_t length;

		// Write something and read it back.
		length = write(fd_w, TEST_CONTENT, SIZEOF_TEST_CONTENT);
		if (length != SIZEOF_TEST_CONTENT)
		{
			return 2;
		}

		length = read(fd_r, buf, 64);
		if (length != SIZEOF_TEST_CONTENT)
		{
			return 3;
		}

		if (memcmp(buf, TEST_CONTENT, SIZEOF_TEST_CONTENT) != 0)
		{
			return 4;
		}
	}
	if (number_of_fds == 1) // Dev.
	{
		int fd = (int)(argv[2][0] - '0');
		ssize_t length;

		// Just check whether writing to NULL works.
		length = write(fd, TEST_CONTENT, SIZEOF_TEST_CONTENT);
		if (length != SIZEOF_TEST_CONTENT)
		{
			return 2;
		}
	}

	return 0;
}
