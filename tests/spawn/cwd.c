/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <string.h>
#include <unistd.h>

int main()
{
	char cwd[256];

	getcwd(cwd, 256);
	write(STDOUT_FILENO, cwd, strlen(cwd) + 1);

	return 0;
}
