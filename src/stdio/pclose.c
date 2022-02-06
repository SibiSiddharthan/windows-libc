/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/error.h>
#include <internal/process.h>
#include <internal/stdio.h>
#include <stdio.h>

int common_fclose(FILE *stream);

int wlibc_pclose(FILE *stream)
{
	VALIDATE_FILE_STREAM(stream, -1);

	HANDLE process = stream->phandle;
	common_fclose(stream);

	if (WaitForSingleObject(process, INFINITE) == WAIT_FAILED)
	{
		map_win32_error_to_wlibc(GetLastError());
		return -1;
	}
	DWORD exit_code = 0;
	if (!GetExitCodeProcess(process, &exit_code))
	{
		map_win32_error_to_wlibc(GetLastError());
		return -1;
	}

	return exit_code;
}
