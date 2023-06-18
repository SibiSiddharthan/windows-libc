/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	for (int i = 0; i < argc; ++i)
	{
		write(STDOUT_FILENO, argv[i], strlen(argv[i]) + 1);
	}

	return 0;
}
