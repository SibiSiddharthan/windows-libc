/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <Windows.h>
#include <internal/error.h>
#include <errno.h>
#include <internal/fcntl.h>
#include <fcntl.h>

int wlibc_pipe(int pipefd[2])
{
	return wlibc_pipe2(pipefd, 0);
}

int wlibc_pipe2(int pipefd[2], int flags)
{
	HANDLE read_handle, write_handle;
	if (!CreatePipe(&read_handle, &write_handle, NULL, 4096))
	{
		map_win32_error_to_wlibc(GetLastError());
		return -1;
	}

	return 1;
}
