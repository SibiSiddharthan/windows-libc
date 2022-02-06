/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <sys/wait.h>

pid_t wlibc_wait(int *wstatus)
{
	return wlibc_waitpid(-1, wstatus, 0);
}
