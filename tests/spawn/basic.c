/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>

int main()
{
	int length;
	char buffer[64];

	length = read(STDIN_FILENO, buffer, 64);
	write(STDOUT_FILENO, buffer, length);

	return 0;
}
