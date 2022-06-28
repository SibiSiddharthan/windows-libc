/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	if (argc == 3)
	{
		int mode = atoi(argv[1]);
		if (mode == 1)
		{
			int duration = atoi(argv[2]);
			usleep(duration * 1000);
		}
		if (mode == 2)
		{
			int sig = atoi(argv[2]);
			raise(sig);
		}
	}
	return 0;
}
