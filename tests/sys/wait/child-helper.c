/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <signal.h>
#include <stdlib.h>
#include <Windows.h>

int main(int argc, char **argv)
{
	if (argc == 3)
	{
		int mode = atoi(argv[1]);
		if (mode == 1)
		{
			int duration = atoi(argv[1]);
			Sleep(duration);
		}
		if (mode == 2)
		{
			int sig = atoi(argv[1]);
			raise(sig);
		}
	}
	return 0;
}
