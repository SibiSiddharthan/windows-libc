/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <unistd.h>
#include <Windows.h>

pid_t wlibc_getpid()
{
	return GetCurrentProcessId();
}